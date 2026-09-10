# FPGA AI Suite IP Architecture generation and network compilation

## Step 1: Setup python virtual environment
```
export _PIP_LOCATIONS_NO_WARN_ON_MISMATCH=1
python3 -m venv python_env
source python_env/bin/activate
python3 -m pip install --upgrade pip
python3 -m pip install -r requirements.txt  --extra-index-url https://download.pytorch.org/whl/cpu
export LD_LIBRARY_PATH=python_env/lib:$LD_LIBRARY_PATH
source openvino-src/setupvars.sh
source coredla-src/opt/altera/fpga_ai_suite_2025.1/dla/setupvars.sh
```

## Step 2: Convert YOLO v8 Nano detection model (`yolo8n.pt`) to ONNX model
```
python3
>>> from ultralytics import YOLO
>>> import onnx.utils
>>> import sys
>>> import os
>>> 
>>> model = YOLO("yolov8n.pt")
>>> model.export(format="onnx", imgsz=[640, 384])
>>> quit()
```

## Step 3: Scale input weights
The example design uses a layout transform which produces values in the range
0.0 to 255.0, but the ONNX graph is expecting samples in the range 0.0 to 1.0,
so the need to scale the weights in the first convolution accordingly.

```
>>> python3
>>> import onnx.utils
>>> import sys
>>> import os
>>> 
>>> model = onnx.load("yolov8n.onnx")
>>> for initializer in model.graph.initializer:
>>>     if initializer.name == "model.0.conv.weight":
>>>         w_in = onnx.numpy_helper.to_array(initializer)
>>>         w_out = w_in/255.0
>>>         tp_out = onnx.numpy_helper.from_array(w_out, initializer.name)
>>>         initializer.CopyFrom(tp_out)
>>> 
>>> for initializer in model.graph.initializer:
>>>     if initializer.name == "model.0.conv.weight":
>>>         w_in = onnx.numpy_helper.to_array(initializer)
>>> 
>>> with open("yolov8n_scaled_640_384.onnx", "wb") as f:
>>>     f.write(model.SerializeToString())
>>> 
>>> quit()
```

## Step 4: Converting ONNX model to OpenVINO™ IR format
```
export OPENVINO_OVC=${INTEL_OPENVINO_DIR}/python/openvino/tools/ovc/ovc.py
python3 ${OPENVINO_OVC} yolov8n-pose_scaled_640_384.onnx --compress_to_fp16=False --output_model yolov8n_ir_640_384
```

## Step 5: Generating FPGA AI Suite IP architecture file (`.arch`)
Start with a basic example achitecture (`base.arch`). The supported YOLO v8
Nano models do not use the SoftMax operator, so start with
`AGX5_Small_NoSoftmax.arch`:

```
cp mycoredla-src/opt/altera/fpga_ai_suite_2025.1/dla/example_architectures/AGX5_Small_NoSoftmax.arch base.arch
```

Perform an initial test compile with the base architecture (`base.arch`):

```
dla_compiler --network-file yolov8n_ir_640_384.xml --march=base.arch
```

Check the output:
```
[ INFO ] The input graph is split into 118 subgraph(s), CPU:58 FPGA:60.
[ WARNING ] Input graph is split into many subgraphs; this can be caused by unsupported architecture layers - check main_graph_dla_messages.txt.
```

You are aiming for FPGA:1, check `model_analyzer_report.txt`:
```
List of reasons for unsupporting layers:
- /model.0/act/Mul: This node is of type Swish, however, neither Sigmoid nor Tanh is enabled in the architecture.
- /model.1/act/Mul: This node is of type Swish, however, neither Sigmoid nor Tanh is enabled in the architecture.
...
```
## Step 6: Resolve sigmoid
Enable sigmoid in the architecture in `base.arch`:
```
activation {
...
  enable_sigmoid : true
...
}
```

Perform a second test compilation:
```
dla_compiler --network-file yolov8n_ir_640_384.xml --march=base.arch
```

Check the output:
```
[ INFO ] The input graph is split into 4 subgraph(s), CPU:2 FPGA:2.
[ WARNING ] Input graph is split into many subgraphs; this can be caused by unsupported architecture layers - check main_graph_dla_messages.txt.
```

You are aiming for FPGA:1, check `model_analyzer_report.txt`, for now ignore
unsupported layes at the detection head (i.e. those starting /model.22/):

```
List of reasons for unsupporting layers:
- /model.9/m/MaxPool: Pool (/model.9/m/MaxPool) kernel width/depth, height (5/1, 5) exceeds maximum window width, height (3, 3).
- /model.9/m_1/MaxPool: Pool (/model.9/m_1/MaxPool) kernel width/depth, height (5/1, 5) exceeds maximum window width, height (3, 3).
- /model.9/m_2/MaxPool: Pool (/model.9/m_2/MaxPool) kernel width/depth, height (5/1, 5) exceeds maximum window width, height (3, 3).
...
```
## Step 7: Resolve pool width/depth
Change pool max_window_height and max_window_width to 7 in `base.arch`
(max pool window must be one of [3x3], [7x7] or [13x13])
```
pool {
...
  max_window_height : 7
  max_window_width : 7
...
}
```

Perform a third test compilation:
```
dla_compiler --network-file yolov8n_ir_640_384.xml --march=base.arch
```

Check the output:
```
[ INFO ] The input graph is split into 2 subgraph(s), CPU:1 FPGA:1.
[ WARNING ] Input graph is split into many subgraphs; this can be caused by unsupported architecture layers - check main_graph_dla_messages.txt.
```

FPGA:1 achieved. Check `model_analyzer_report.txt`:
```
List of redundant modules. Modules activated in the arch file, but not used:
INFO: Arch file enables the clamp activation module, however, clamp activations are not used in the model.
To avoid allocating unnecessary resources, disable the clamp activation module in the arch file.
```

## Step 8: Disable clamp activation
Set enable_clamp to false in `base.arch`:
```
activation {
...
  enable_clamp : false
...
}
```

Perform a fourth test compile:
```
dla_compiler --network-file yolov8n_ir_640_384.xml --march=base.arch
```

## Step 9: Review subgraphs in graphviz viewer
Now check where the graph `hetero_subgraphs_main_graph.dot` is offloading to
CPU using graphviz viewer. You can see the graph offloads to CPU at three
points - `/model.22/Reshape`, `/model.22/Reshape_1`, and `/model.22/Reshape_2`.

Check `model_analyzer_report.txt` to confirm why they are offloaded:
```
- /model.22/Reshape: Expected the output dimension rank = 2, 4, 5 but got: 3.
- /model.22/Reshape_1: Expected the output dimension rank = 2, 4, 5 but got: 3.
- /model.22/Reshape_2: Expected the output dimension rank = 2, 4, 5 but got: 3.
```

At this point we can conclude that we are happy to run these layers in the
OpenVINO™ Arm* (third-party) CPU plugin. However the end of the network
contains simple manipulation code which could be implemented more optimally in
the SW Application. And this was the decision taken in the Camera Solution
System Example Design.

## Step 10: Removing CPU subgraph
Find the inputs to the `/model.22/Reshape`, `/model.22/Reshape_1`, and
`/model.22/Reshape_2`. For instance, load the `yolov8n.onnx` into a tool such as Netron:
```
    /model.22/Reshape input is /model.22/Concat_output_0
    /model.22/Reshape_1 input is /model.22/Concat_1_output_0
    /model.22/Reshape_2 input is /model.22/Concat_2_output_0"
```

Regenerate ONNX model in python, but use `onnx.utils` to split the network:
```
python3
>>> from ultralytics import YOLO
>>> import onnx.utils
>>> import sys
>>> import os
>>> 
>>> onnx.utils.extract_model(
>>>     "yolov8n_scaled_640_384.onnx",
>>>     "yolov8n_short_640_384.onnx",
>>>     ["images"],
>>>     ["/model.22/Concat_output_0", "/model.22/Concat_1_output_0", "/model.22/Concat_2_output_0"],
>>> )
>>> 
>>> quit()
```

## Step 11: Convert shortened ONNX model to OpenVINO™ IR format
```
python3 ${OPENVINO_OVC} yolov8n_short_640_384.onnx --compress_to_fp16=False --output_model yolov8n_ir_640_384
```

Perform a test compile of the new OpenVINO™ IR model:
```
dla_compiler --network-file yolov8n_ir_640_384.xml --march=base.arch
```

Check the output:
```
[ INFO ] The input graph is split into 1 subgraph(s), FPGA:1.
```

Note that there are now no CPU subgraphs.

## Step 12: Optimize the architecture
After the network model and `base.arch` have been generated, the architecture can be optimized:
```
dla_compiler --gen-arch
```

```
export MAX_ALM=32000
export MAX_M20K=400
export MAX_DPS=90
export ASSUMED_FMAX=250
# We will need a decent streaming buffer to achive high FPS, so don't bother analysing too small an architecture
export MIN_STREAMING_BUFFER=16384

dla_compiler \
    --gen-arch \
    --mmax-resources=${MAX_ALM},${MAX_M20K},${MAX_DPS} \
    --gen-min-sb=${MIN_STREAMING_BUFFER} \
    --network-file yolov8n_ir_640_384.xml \
    --march=base.arch \
    --fassumed-fmax-core=${ASSUMED_FMAX}
```

After processing many variations of the FPGA AI Suite IP architecture, the most
optimal architecture should be saved as `generated_arch.arch`

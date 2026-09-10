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

## Step 4: Removing CPU subgraph
In the Camera Solution System Example Design, the decision was taken to remove
the CPU subgrapgh from the graph and have an FPGA only graph with results
processing handled by the SW Application.

Find the inputs to the `/model.22/Reshape`, `/model.22/Reshape_1`, and
`/model.22/Reshape_2`. For instance, load the `yolov8n.onnx` into a tool such as Netron:
```
    /model.22/Reshape input is /model.22/Concat_output_0
    /model.22/Reshape_1 input is /model.22/Concat_1_output_0
    /model.22/Reshape_2 input is /model.22/Concat_2_output_0"
```

Regenerate onnx model in python, but use onnx.utils to split the network
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

## Step 5: Convert shortened ONNX model to OpenVINO� IR format
```
python3 ${OPENVINO_OVC} yolov8n_short_640_384.onnx --compress_to_fp16=False --output_model yolov8n_ir_640_384
```

## Step 6: Compile model
```
export ASSUMED_FMAX=250

dla_compiler \
    --network-file yolov8n_ir_640_384.xml \
    --march=generated_arch.arch \
    --fassumed-fmax-core=${ASSUMED_FMAX} \
    --foutput-format=open_vino_hetero \
    -o yolov8n-pose_dla_m2m_compiled_640_384.bin
```

Additional options for analysis:
```
    --fanalyze-performance
    --fanalyze-area
    --fdump-performance-report yolov8n-pose_dla_m2m_compiled_640_384-perf-report.txt
    --fdump-area-report yolov8n-pose_dla_m2m_compiled_640_384-area-report.txt
```

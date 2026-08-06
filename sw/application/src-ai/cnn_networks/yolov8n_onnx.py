#!/usr/bin/env python3

# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

from ultralytics import YOLO
import onnx.utils
import sys
import os

w = 320
h = 320
if len(sys.argv) == 3:
    w = int(sys.argv[1])
    h = int(sys.argv[2])

if os.path.isfile("./yolov8n.pt"):
    model = YOLO("./yolov8n.pt")
    with open("yolov8n_categories.txt", "w") as f:
        for idx in model.model.names:
            print(model.model.names[idx], file=f)
    onnx_path = model.export(format="onnx", opset=11, imgsz=[h, w])
    onnx_dir, onnx_filename_ext = os.path.split(onnx_path)
    onnx_filename, onnx_ext = os.path.splitext(onnx_filename_ext)
    new_onnx_filename = onnx_filename + '_{}_{}.onnx'.format(w, h)
    new_onnx_path = os.path.join(onnx_dir, new_onnx_filename)
    os.rename(onnx_path, new_onnx_path)
    onnx_path = new_onnx_path

    new_onnx_scaled_filename = onnx_filename + '_scaled_{}_{}.onnx'.format(w, h)
    onnx_scaled_path = os.path.join(onnx_dir, new_onnx_scaled_filename)

    new_onnx_short_filename = onnx_filename + '_short_{}_{}.onnx'.format(w, h)
    onnx_short_path = os.path.join(onnx_dir, new_onnx_short_filename)

    model = onnx.load(onnx_path)
    for initializer in model.graph.initializer:
        if initializer.name == "model.0.conv.weight":
            w_in = onnx.numpy_helper.to_array(initializer)
            w_out = w_in/255.0
            tp_out = onnx.numpy_helper.from_array(w_out, initializer.name)
            initializer.CopyFrom(tp_out)

    for initializer in model.graph.initializer:
        if initializer.name == "model.0.conv.weight":
            w_in = onnx.numpy_helper.to_array(initializer)

    with open(onnx_scaled_path, "wb") as f:
        f.write(model.SerializeToString())

    onnx.utils.extract_model(
        onnx_scaled_path,
        onnx_short_path,
        ["images"],
        ["/model.22/Concat_output_0", "/model.22/Concat_1_output_0", "/model.22/Concat_2_output_0"],
    )

if os.path.isfile("./yolov8n-pose.pt"):
    model = YOLO("./yolov8n-pose.pt")  # load an official model
    with open("yolov8n-pose_categories.txt", "w") as f:
        for idx in model.model.names:
            print(model.model.names[idx], file=f)
    onnx_path = model.export(format="onnx", opset=11, imgsz=[h, w])
    onnx_dir, onnx_filename_ext = os.path.split(onnx_path)
    onnx_filename, onnx_ext = os.path.splitext(onnx_filename_ext)
    new_onnx_filename = onnx_filename + '_{}_{}.onnx'.format(w, h)
    new_onnx_path = os.path.join(onnx_dir, new_onnx_filename)
    os.rename(onnx_path, new_onnx_path)
    onnx_path = new_onnx_path

    new_onnx_scaled_filename = onnx_filename + '_scaled_{}_{}.onnx'.format(w, h)
    onnx_scaled_path = os.path.join(onnx_dir, new_onnx_scaled_filename)

    new_onnx_short_filename = onnx_filename + '_short_{}_{}.onnx'.format(w, h)
    onnx_short_path = os.path.join(onnx_dir, new_onnx_short_filename)

    model = onnx.load(onnx_path)
    for initializer in model.graph.initializer:
        if initializer.name == "model.0.conv.weight":
            w_in = onnx.numpy_helper.to_array(initializer)
            w_out = w_in/255.0
            tp_out = onnx.numpy_helper.from_array(w_out, initializer.name)
            initializer.CopyFrom(tp_out)

    for initializer in model.graph.initializer:
        if initializer.name == "model.0.conv.weight":
            w_in = onnx.numpy_helper.to_array(initializer)

    with open(onnx_scaled_path, "wb") as f:
        f.write(model.SerializeToString())

    onnx.utils.extract_model(
        onnx_scaled_path,
        onnx_short_path,
        ["images"],
        ["/model.22/Concat_1_output_0", "/model.22/Concat_2_output_0", "/model.22/Concat_3_output_0", "/model.22/cv4.0/cv4.0.2/Conv_output_0", "/model.22/cv4.1/cv4.1.2/Conv_output_0", "/model.22/cv4.2/cv4.2.2/Conv_output_0"],
    )


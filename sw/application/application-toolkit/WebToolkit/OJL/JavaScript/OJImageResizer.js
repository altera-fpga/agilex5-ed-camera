/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJGraphics } from "./OJL.js";
import { OJRect } from "./OJL.js";
import { OJLib } from "./OJL.js";
import { UI } from "./OJL.js";
import { ObjectCallback } from "./OJL.js";
import { OJDataTransferWebSocket } from "./OJL.js";
import { OJServerLink } from "./OJL.js";

export class OJImageResizer
{
    constructor(load_image_params)
    {
        this._load_image_file_params = load_image_params;

        if (this._load_image_file_params != null)
        {
            if (OJLib._file_select_callback != null)
            {
                OJLib._file_select_callback.Destroy();
                OJLib._file_select_callback = null;
            }

            OJLib._file_select_callback = new ObjectCallback(this, "ImageFileSelected");

            UI._invisible_input.multiple = false;
            UI._invisible_input.value = "";
            UI._invisible_input.accept = "*.*";
            UI._invisible_input.click();
        }
    }

    Destroy()
    {
        if (this._data_transfer_web_socket != null)
            this._data_transfer_web_socket.Destroy();
    }

    ImageFileSelected(files)
    {
        for (let file of files)
        {
            let file_reader = new FileReader();
            let that = this;
            let file_read_cb = function() { that.ImageFileHasBeenRead(file_reader); };
            file_reader.onload = file_read_cb;
            file_reader.readAsDataURL(file);
        }
    }

    ImageFileHasBeenRead(file_reader)
    {
        let image = new Image();
        let that = this;
        let image_loaded_cb = function() { that.ImageLoaded(image); };
        image.onload = image_loaded_cb;
        image.src = file_reader.result;
    }

    ImageLoaded(source_image)
    {
        if (this._load_image_file_params != null)
        {
            let resize_image_width = this._load_image_file_params.resize_width;
            let resize_image_height = this._load_image_file_params.resize_height;

            let canvas = document.createElement("canvas");
            canvas.style.width = resize_image_width + "px";
            canvas.style.height = resize_image_height + "px";
            canvas.width = resize_image_width;
            canvas.height = resize_image_height;
            let graphics = new OJGraphics(canvas);
            let input_aspect = source_image.width / source_image.height;
            let output_aspect = resize_image_width / resize_image_height;
            let source_x = 0;
            let source_y = 0;
            let source_width = source_image.width;
            let source_height = source_image.height;

            if (input_aspect > output_aspect)
            {
                // Crop sides
                source_width = (output_aspect * source_image.height) | 0;
                source_x = ((source_image.width - source_width) / 2) | 0;
            }
            else
            {
                // Letterbox
                source_height = (source_image.width / output_aspect) | 0;
                source_y = ((source_image.height - source_height) / 2) | 0;
            }

            let destination_rect = new OJRect(0, 0, resize_image_width, resize_image_height);
            graphics.DrawImage(source_image, destination_rect,
                               source_x, source_y,
                               source_width, source_height);
            let image_data = graphics.GetImageData();

            this._data_transfer_web_socket = new OJDataTransferWebSocket("image");
            this._data_transfer_web_socket.SendData(image_data.data);

            let picture_control = OJServerLink.Get().GetControlItem(this._load_image_file_params.picture_control_id);
            if (picture_control != null)
                picture_control.SetImage(image_data);
        }
    }
}


/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJWindowElement, ANCHOR_TYPE } from "./OJL.js";
import { OJLib } from "./OJL.js";

export class OJPicon extends OJWindowElement
{
    constructor(json_picon)
    {
        // Base class constructor
        super();
        this._class_name = "OJPicon";

        this._picon_width = 224;
        this._picon_height = 224;
        this._canvas = document.createElement("canvas");
        //this._canvas.className = "generator_pattern_picon";
        OJLib.SetCanvasSize(this._canvas, this._picon_width, this._picon_height);
        // this._canvas.style.backgroundImage = _image_place_holder.GetURL();
        this._canvas.style.backgroundRepeat = "no-repeat";
        this._canvas.style.backgroundPosition = "center";
        this._client_area.appendChild(this._canvas);

        //this.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: this._picon_width });
        //this.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: this._picon_height });

        this.DecodePicon(json_picon);
    }

    Destroy()
    {
        super.Destroy();
        UI.RemoveFromParentElement(this._canvas);
    }

    Resize(x, y, width, height)
    {
        let size_changed = super.Resize(x, y, width, height);
        let image_x = (width - this._picon_width) / 2;
        let image_y = (height - this._picon_height) / 2;

        OJLib.SetElementLocation(this._canvas, image_x, image_y);

        return size_changed;
    }

    DecodePicon(json_picon)
    {
        if (json_picon == null)
            return;
        let width = json_picon.width;
        let height = json_picon.height;
        let resize_height = json_picon.resize_height;
        let resize_width = json_picon.resize_width;
        //this._canvas.className = "generator_pattern_picon";
        OJLib.SetCanvasSize(this._canvas, this._picon_width, this._picon_height);

        let canvas_style_height = height;
        if (!isNaN(resize_height) && resize_height > 0)
            canvas_style_height = resize_height;
        let canvas_style_width = width;
        if (!isNaN(resize_width) && resize_width > 0)
            canvas_style_width = resize_width;

        this._picon_width = canvas_style_width;
        this._picon_height = canvas_style_height;

        let data_str = json_picon.data;

        // This doesn't have alpha yet so we have to copy pixel by pixl
        let bytes = Decode64BitString(data_str);

        let dc = this._canvas.getContext("2d");
        this._canvas.width = width;
        this._canvas.height = height;
        this._canvas.style.width = canvas_style_width + "px";
        this._canvas.style.height = canvas_style_height + "px";

        let picon_x = 0;
        let picon_y = 0;

        if (this._is_pattern_picon)
        {
            if (width < 302)
                picon_x = (302 - width) / 2;
            else if (height < 169)
                picon_y = (169 - height) / 2;

            picon_x += 5;
            picon_y += 5;
        }

        this._canvas.style.left = picon_x + "px";
        this._canvas.style.top = picon_y + "px";

        let image_data = dc.createImageData(width, height);

        // Just move the data
        let n_pixels = width * height;
        let out_index = 0;
        let in_index = 0;

        for (let i = 0; i < n_pixels; i++)
        {
            image_data.data[out_index++] = bytes[in_index++];
            image_data.data[out_index++] = bytes[in_index++];
            image_data.data[out_index++] = bytes[in_index++];
            image_data.data[out_index++] = 255;
        }

        dc.putImageData(image_data, 0, 0);
    }

    LoadFromImageData(image_data)
    {
        this._canvas.width = image_data.width;
        this._canvas.height = image_data.height;
        this._canvas.style.width = image_data.width + "px";
        this._canvas.style.height = image_data.height + "px";

        let dc = this._canvas.getContext("2d");
        let len = image_data.data.length;
        dc.putImageData(image_data, 0, 0);
    }

    GetWidth()
    {
        return this._canvas.width;
    }

    GetHeight()
    {
        return parseInt(this._canvas.style.height);
    }

    DrawBoxes(categories) 
    {
        if (categories) 
        {
            for (let i = categories.length-1; i >= 0; i--) 
            {
                let category = categories[i];
                if (category.is_region) 
                {
                    let lc_category_name = category.category_name + " : " + Math.round(category.score) + "%";
                    let x_min = Math.round(category.x_min);
                    let y_min = Math.round(category.y_min);
                    let x_max = Math.round(category.x_max);
                    let y_max = Math.round(category.y_max);
                    let category_index = category.category_index;
                    let colour = parseInt(category.colour).toString(16);
                    colour = '#' + '0'.repeat(6 - colour.length) + colour;

                    let dc = this._canvas.getContext("2d");
                    dc.font = "20px Georgia";
                    dc.textBaseline = "bottom"; 
                    text_dim = dc.measureText(lc_category_name)
                    dc.strokeStyle = colour;
                    dc.lineWidth = 3;
                    dc.strokeRect(x_min, y_min, x_max - x_min, y_max - y_min);
                    dc.fillStyle = colour;
                    dc.fillRect(x_min, y_min - 24, text_dim.width + 4, 24);
                    dc.strokeRect(x_min, y_min - 24, text_dim.width + 4, 24);
                    dc.fillStyle = "#000000";
                    dc.fillText(lc_category_name, x_min + 2, y_min - 2);
                }
            }
        }
    }
}

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
import { OJModalDialog } from "./OJL.js";
import { OJGrid } from "./OJL.js";
import { OJWaiting } from "./OJL.js";
import { OJLabel } from "./OJL.js";
import { OJWebSocket } from "./OJL.js";
import { OJGraphics } from "./OJL.js";
import { OJTiffFile } from "./OJL.js";
import { OJTextButton } from "./OJL.js";
import { OJLib, ObjectCallback } from "./OJL.js";
import { ModifyMouseCoordinates } from "./OJL.js";

export class OJPictureDialog extends OJModalDialog
{
    constructor(params)
    {
        // Base class constructor
        super(1440 + 30, 810 + 115, "Image Capture");
        this._class_name = "OJPictureDialog";
        this.SetElementName("OJPictureDialog");

        this._canvas = document.createElement("canvas");
        this._canvas.className = "picture_image_class";
        this.GetElement().appendChild(this._canvas);
        this._capture_width = 0;
        this._capture_height = 0;

        this._waiting = new OJWaiting();
        this.AddChild(this._waiting);

        this._waiting.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _dynamic_position: 50, _fixed_offset: -60 });
        this._waiting.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 120 });
        this._waiting.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _dynamic_position: 50, _fixed_offset: -60 });
        this._waiting.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 120 });
            
        this._button_panel = new OJGrid();
        this._button_panel.SetBackgroundColour("#ffffff");
        this._button_panel.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -55 });
        this.AddChild(this._button_panel);

        let update_callback = new ObjectCallback(this, "OnUpdatePicture");
        this._update_button = new OJTextButton(null, "Update", { _click_callback: update_callback, _alignment: "centre" });
        this._update_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -260 });
        this._update_button.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 10 });
        this._button_panel.AddChild(this._update_button);

        let save_callback = new ObjectCallback(this, "OnSavePicture");
        this._save_button = new OJTextButton(null, "Save", { _click_callback: save_callback, _alignment: "centre" });
        this._save_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -390 });
        this._save_button.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 10 });
        this._button_panel.AddChild(this._save_button);

        let close_callback = new ObjectCallback(this, "OnOK");
        this._close_button = new OJTextButton(null, "Close", { _click_callback: close_callback, _alignment: "centre" });
        this._close_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -130 });
        this._close_button.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 10 });
        this._button_panel.AddChild(this._close_button);

        let toggle_histogram_callback = new ObjectCallback(this, "ToggleHistogram");
        this._histogram_button = new OJTextButton(null, "Show Histograms", { _click_callback: toggle_histogram_callback, _alignment: "centre" , _enabled: false });
        this._histogram_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -520 });
        this._histogram_button.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 10 });
        this._button_panel.AddChild(this._histogram_button);

        this._pixel_info_label = new OJLabel("", {
            _alignment		: "left",
            _single_line	: true
        });
        this._pixel_info_label.SetLeftAnchor({
            _type			: ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset	: 10
        });
        this._pixel_info_label.SetTopAnchor({
            _type			: ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset	: 10
        });
        this._button_panel.AddChild(this._pixel_info_label);

        this._web_socket = new OJWebSocket(OJLib._websocket_server_url, "picture", this);

        this._histogram_visible = false;

        this._r_histogram = new OJHistogram(this, "Red");
        this._g_histogram = new OJHistogram(this, "Green");
        this._b_histogram = new OJHistogram(this, "Blue");

        this._histogram_panel = new OJGrid();
        this._histogram_panel.SetBackgroundColour("#ffffff");
        this._histogram_panel.Show(false);
        this._histogram_panel.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._button_panel,
            _fixed_offset	: -this._r_histogram._height
        });
        this._histogram_panel.SetBottomAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._button_panel
        });
        this.AddChild(this._histogram_panel);

        this._histogram_panel.AddChild(this._r_histogram);
        this._histogram_panel.AddChild(this._g_histogram);
        this._histogram_panel.AddChild(this._b_histogram);

        let histogram_spacing = 30;
        this._r_histogram.SetLeftAnchor({
            _type			: ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset	: histogram_spacing
        });
        this._g_histogram.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._r_histogram,
            _fixed_offset	: histogram_spacing
        });
        this._b_histogram.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._g_histogram,
            _fixed_offset	: histogram_spacing
        });

        this._selected_area_x = 0;
        this._selected_area_y = 0;
        this._selected_area_width = this._capture_width;
        this._selected_area_height = this._capture_height;

        this._canvas_data = [];

        this._freeze_pixel_info = false;

        let self_obj = this;
        this._canvas.onmousedown = function(event) 
        {
            self_obj.OnMouseDown(event);
        };

        this._canvas.ondblclick = function(event) 
        {
            self_obj.OnDoubleClick(event);
        };

        this._canvas.onmousemove = function(event) 
        {
            self_obj.OnMouseMove(event);
        };

        this.GetElement().onkeydown = function(event) 
        {
            self_obj.OnKeyDown(event);
        };
    }

    Destroy()
    {
        super.Destroy();

        if (this._web_socket != null)
        {
            this._web_socket.Destroy();
            this._web_socket = null;
        }	
    }

    Close(event)
    {
    }

    Resize(x, y, width, height)
    {
        let size_changed = super.Resize(x, y, width, height);

        this._canvas.style.left = "15px";
        this._canvas.style.top = "15px";
        this._canvas.style.width = "1440px";
        this._canvas.style.height = "810px";

        return size_changed;
    }

    OnWebSocketReady(event)
    {
        this.OnUpdatePicture();
    }

    OnUpdatePicture(event)
    {
        // Trigger a capture on the server, which will be pushed 
        // to us through OnMessageWebSocket below
        this._waiting.Show(true);
        OJLib.ServerExecute("PictureDialogUpdateType1");
    }

    OnSavePicture(event)
    {
        let graphics = new OJGraphics(this._canvas);
        let image_data = graphics.CreateImageData();

        let tiff_file = new OJTiffFile(this._capture_width, this._capture_height, this._image_data);
        tiff_file.Save("Capture.tiff");
    }

    SendCommand(command, parameters)
    {
        let command_object = { Command: { _value: command, Params: parameters } };
        let json_string = JSON.stringify(command_object);
        this._web_socket.Send(json_string);
    }

    OnMessageWebSocket(web_socket, message)
    {
        let is_text = !message._is_binary;

        if (is_text)
        {
            let text = message._text_data;

            let json_object = JSON.parse(text);
            let action = json_object._action;

            let picture_details = json_object.PictureDetails;
            if (picture_details != null)
            {
                this._capture_width = picture_details.width;
                this._capture_height = picture_details.height;
                this._canvas.width = this._capture_width;
                this._canvas.height = this._capture_height;
                this._canvas_to_image_ratio = this._capture_width / parseInt(this._canvas.style.width);

                this.ResetSelectedArea();

                return;
            }
        }
        else
        {
            let graphics = new OJGraphics(this._canvas);
            let image_data = graphics.CreateImageData();
            let n_pixels = (this._capture_width * this._capture_height);
            
            let in_index = 0;
            let out_index = 0;
            this._image_data = message._binary_data;

            for (let i = 0; i < n_pixels; i++)
            {
                // The binary data is 10 bits packed into 16
                let low_bits  = this._image_data[in_index + 0];
                let high_bits = this._image_data[in_index + 1];
                image_data.data[out_index++] = (high_bits << 6) | (low_bits >> 2);
                this._image_data[in_index + 0] = ((low_bits & 0x3) << 6);
                this._image_data[in_index + 1] = (high_bits << 6) | (low_bits >> 2);
                in_index += 2;

                low_bits  = this._image_data[in_index + 0];
                high_bits = this._image_data[in_index + 1];
                image_data.data[out_index++] = (high_bits << 6) | (low_bits >> 2);
                this._image_data[in_index + 0] = ((low_bits & 0x3) << 6);
                this._image_data[in_index + 1] = (high_bits << 6) | (low_bits >> 2);
                in_index += 2;

                low_bits  = this._image_data[in_index + 0];
                high_bits = this._image_data[in_index + 1];
                image_data.data[out_index++] = (high_bits << 6) | (low_bits >> 2);
                this._image_data[in_index + 0] = ((low_bits & 0x3) << 6);
                this._image_data[in_index + 1] = (high_bits << 6) | (low_bits >> 2);
                in_index += 2;

                image_data.data[out_index++] = 255; // Alpha
            }

            graphics.PutImageData(image_data);
            this._canvas_data = image_data;

            this._histogram_button.Enable(true);

            this._waiting.Show(false);
            return;
        }
    }

    CanvasToImage(canvas_pixels)
    {
        let image_pixels = ((canvas_pixels * this._canvas_to_image_ratio) + 0.5) | 0;
        return image_pixels;
    }

    ImageToCanvas(image_pixels)
    {
        let canvas_pixels = ((image_pixels / this._canvas_to_image_ratio) + 0.5) | 0;
        return canvas_pixels;
    }

    SetPixelInfo(mouse_x, mouse_y)
    {
        if (this._pixel_info_label && !this._freeze_pixel_info) 
        {
            let index = (mouse_x + (mouse_y * this._capture_width)) * 4;
            this._pixel_info_label.SetValue(`Pixel Index: ${mouse_x}, ${mouse_y} | R: ${this._canvas_data.data[index]} | G: ${this._canvas_data.data[index + 1]} | B: ${this._canvas_data.data[index + 2]}`);
        }
    }

    ResetSelectedArea()
    {
        this._selected_area_x = 0;
        this._selected_area_y = 0;
        this._selected_area_width = this._capture_width;
        this._selected_area_height = this._capture_height;
    }

    IsCanvasBlank()
    {
        return this._canvas_data.length === 0;
    }

    OnMouseDown(event)
    {
        if (!this.IsCanvasBlank()) 
        {
            ModifyMouseCoordinates(event);

            this._draw_area = true;

            let image_x = this.CanvasToImage(event._offset_x);
            let image_y = this.CanvasToImage(event._offset_y);

            switch (event.which)
            {
                case 1:	// Left Mouse Button
                    break;
                case 3:	// Right Mouse Button
                    this._selected_area_x = image_x;
                    this._selected_area_y = image_y;
                    this._selected_area_width = 0;
                    this._selected_area_height = 0;
                    break;
                default:
                    break;
            }

            this.DrawSelectedArea();

            let self_obj = this;
            this._old_window_mouse_up = window.onmouseup;
            window.onmouseup = function(event) 
            {
                self_obj.OnMouseUp(event);
            };
        }
    }

    OnDoubleClick(event)
    {
        this.ResetSelectedArea();
        this.DrawSelectedArea()
        this.CalculateHistogram();
    }

    OnMouseMove(event)
    {
        if (!this.IsCanvasBlank()) 
        {
            ModifyMouseCoordinates(event);
            let image_x = this.CanvasToImage(event._offset_x);
            let image_y = this.CanvasToImage(event._offset_y);

            if (this._canvas && this._draw_area) 
            {
                this._selected_area_width = image_x - this._selected_area_x;
                this._selected_area_height = image_y - this._selected_area_y;

                this.DrawSelectedArea();
            }

            this.SetPixelInfo(image_x, image_y);
        }
    }

    OnMouseUp(event)
    {
        this._draw_area = false;
        this.DrawSelectedArea();
        this.CalculateHistogram();

        window.onmouseup = this._old_window_mouse_up;
    }

    OnKeyDown(event)
    {
        switch (event.keyCode) 
        {
            case 32: // Space
                this._freeze_pixel_info = !this._freeze_pixel_info;
                break;
            case 67: // C
                this.ResetSelectedArea();
                this.DrawSelectedArea()
                this.CalculateHistogram();
                break;
            default:
                break;
        }
    }

    DrawSelectedArea()
    {
        let context = this._canvas.getContext("2d");
        context.clearRect(0, 0, this._canvas.width, this._canvas.height);

        let graphics = new OJGraphics(this._canvas);
        graphics.PutImageData(this._canvas_data);

        context.lineWidth = 5;
        context.strokeStyle = "gold";
        context.strokeRect(this._selected_area_x, this._selected_area_y, this._selected_area_width, this._selected_area_height);
    }

    ToggleHistogram()
    {
        this._histogram_visible = !this._histogram_visible;

        let height_diff;
        let histogram_button_label;

        this._histogram_panel.Show(this._histogram_visible);

        if (this._histogram_visible) 
        {
            height_diff = this._r_histogram._height;
            histogram_button_label = "Hide Histograms";
            this.CalculateHistogram();
        } 
        else 
        {
            height_diff = -this._r_histogram._height;
            histogram_button_label = "Show Histograms";
        }

        let top_diff = (height_diff / -2);
        let style = this.GetElement().style;

        this.PositionAttachedChildren();
        this.Resize(
            parseInt(style.left),
            parseInt(style.top) + top_diff - this._header_height,
            parseInt(style.width),
            parseInt(style.height) + height_diff + this._header_height
        )

        this._histogram_button.SetLabel(histogram_button_label);
    }

    CalculateHistogram()
    {
        let start_x = this._selected_area_x;
        let start_y = this._selected_area_y;
        let end_x = this._selected_area_x;
        let end_y = this._selected_area_y;
        
        if (this._selected_area_width < 0)
            start_x += this._selected_area_width;
        else
            end_x += this._selected_area_width;
        
        if (this._selected_area_height < 0)
            start_y += this._selected_area_height;
        else
            end_y += this._selected_area_height;

        [this._r_histogram, this._g_histogram, this._b_histogram].forEach(function(hist) { hist.ResetValues(); });

        if (!this.IsCanvasBlank()) 
        {
            for (let row = start_y; row < end_y; row++) 
            {
                let row_start = row * this._capture_width * 4;
                for (let col = start_x; col < end_x; col++) 
                {
                    let pixel_index = row_start + (col * 4);
                    this._r_histogram.AddValue(this._canvas_data.data[pixel_index++]);
                    this._g_histogram.AddValue(this._canvas_data.data[pixel_index++]);
                    this._b_histogram.AddValue(this._canvas_data.data[pixel_index++]);
                }
            }

            if (this._histogram_visible)
                [this._r_histogram, this._g_histogram, this._b_histogram].forEach(function(hist) { hist.DrawHistogram() });
        }
    }
}

export class OJHistogram extends OJWindowElement
{
    static _max = 0;

    constructor(parent, label)
    {
        // Base class constructor
        super();
        this._class_name = "OJHistogram";
        this.SetElementName("OJHistogram");

        this._div = this.GetElement();

        this._values = new Array(256).fill(0);

        this._canvas = document.createElement("canvas");
        this._canvas.style.width = "100%";
        this._canvas.style.height = "100%";
        this._div.appendChild(this._canvas);

        this._label = label;

        this._width = 450;
        this._height = 300;

        this._div.style.width = this._width;
        this._div.style.height = this._height;
        this._div.style.display = "inherit";
        this._canvas.style.display = "inherit";

        this.SetRightAnchor({
            _type		: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size	: this._width
        });
        this.SetBottomAnchor({
            _type		: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size	: this._height
        });
    };

    AddValue(val)
    {
        if (val < this._values.length) 
        {
            this._values[val]++;

            if (this._values[val] > OJHistogram._max)
                OJHistogram._max = this._values[val];
        }
    }

    ResetValues()
    {
        for (let i = 0; i < this._values.length; i++)
            this._values[i] = 0;
        OJHistogram._max = 0;
    }

    DrawHistogram()
    {
        let context = this._canvas.getContext("2d");

        context.clearRect(0, 0, this._canvas.clientWidth, this._canvas.clientHeight);
        context.beginPath();

        let hist_side_spacing = 1;
        let hist_top_spacing = 5;
        let hist_bottom = 125 + hist_top_spacing;
        let label_size = 15;

        let hist_bar_v_spacing = (hist_bottom - hist_top_spacing) / OJHistogram._max;
        let hist_bar_pos = hist_side_spacing + 1;
        context.fillStyle = this._label;
        context.strokeStyle = this._label;
        this._values.forEach(function(val) 
        {
            context.moveTo(hist_bar_pos, hist_bottom);
            context.lineTo(hist_bar_pos, hist_bottom - (val * hist_bar_v_spacing));
            context.stroke();
            hist_bar_pos++;
        });

        context.textAlign = "centre";
        context.font = `${label_size}px Arial`;
        context.fillText(this._label, this._canvas.getBoundingClientRect().width / 2, hist_bottom + label_size);

        context.fillStyle = "black";
        context.strokeStyle = "black";
        context.beginPath();
        context.moveTo(hist_side_spacing, hist_top_spacing);
        context.lineTo(hist_side_spacing, hist_bottom);
        context.lineTo(this._width, hist_bottom);
        context.stroke();
    }
}
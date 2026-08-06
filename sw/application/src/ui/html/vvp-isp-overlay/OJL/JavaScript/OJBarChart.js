/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJServerLink } from "./OJLExtras.js";
import { OJWindowElement, ANCHOR_TYPE } from "./OJLExtras.js";
import { OJModalDialog } from "./OJLExtras.js";
import { OJGrid } from "./OJL.js"
import { OJControlContainer, OJControlItemBase } from "./OJLExtras.js";
import { OJLib } from "./OJLExtras.js";
import { wsvg } from "./OJLExtras.js";
import { OJControlItemHiddenValue } from "./OJLExtras.js";
import { OJScrollable } from "./OJLExtras.js";
import { OJLabel } from "./OJLExtras.js";

let bar_chart_min_height = 280;

export class OJBarChartControl extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);
        this._ready = false;

        this._num_bars = ui_element_json.value.num_bars;
        if (!this._num_bars)
            this._num_bars = 256;
        this._values = [];
        this._values[0] = ui_element_json.value.values1;
        this._values[1] = ui_element_json.value.values2;
        this._names = []
        this._names[0] = ui_element_json.value.name0;
        this._names[1] = ui_element_json.value.name1;

        let margin = 18;
        this._y_min = margin;
        this._y_max = Math.max(this._height - 40 - margin, this._y_min + 1);
        this._img_height = this._y_max - this._y_min;

        this._rect_width = Math.floor(this._width / this._num_bars);
        this._x_min = Math.floor((this._width % this._num_bars) / 2);
        this._x_max = this._x_min + this._num_bars * this._rect_width;
        this._img_width = this._x_max - this._x_min;

        this._control_height = 0;
        this._svg = wsvg.create("svg");
        this._svg.style.backgroundColor = "#d8d8de";
        this._svg.style.border = "#b7b7b7";
        this._svg.style.borderWidth = "1px";
        this._svg.style.borderStyle = "solid";

        this._view = wsvg.create("g");
        this._svg.appendChild(this._view);
        this.GetElement().appendChild(this._svg);
        this._marker_thickness = 7;
        this._border = 8;
        this._button_down = false;

        let center_pixel_translate = "translate(-0.5 -0.5)";
        this._window_polygon = wsvg.create("polygon");
        this._window_polygon.setAttribute("points", "2,2 4,2 4,4 2,4");
        this._window_polygon.setAttribute("stroke", "#303030");
        this._window_polygon.setAttribute("stroke-width", "1");
        this._window_polygon.setAttribute("fill", "#d8d8de");
        this._window_polygon.setAttribute("transform", center_pixel_translate);
        this._window_polygon.style.display = "block";
        this._view.appendChild(this._window_polygon);

        this._bars = [];
        this._bars[0] = [];
        this._bars[1] = [];
        for (let i = 0; i < this._num_bars; i++)
        {
            this._bars[0][i] = wsvg.create("polygon");
            this._bars[0][i].style.display = "block";
            this._bars[0][i].setAttribute("stroke", "#202020");
            this._bars[0][i].setAttribute("stroke-width", "1");
            this._bars[0][i].setAttribute("fill", "#0054ae");
            this._bars[0][i].setAttribute("opacity", "0.5");
            this._view.appendChild(this._bars[0][i]);

            this._bars[1][i] = wsvg.create("polygon");
            this._bars[1][i].style.display = "block";
            this._bars[1][i].setAttribute("stroke", "#202020");
            this._bars[1][i].setAttribute("stroke-width", "1");
            this._bars[1][i].setAttribute("fill", "#F08040");
            this._bars[1][i].setAttribute("opacity", "0.5");
            this._view.appendChild(this._bars[1][i]);
        }

        this._key_boxes = [];
        this._key_labels = [];
        this._axis_labels = [];

        if(this._names[0].length !== 0 && this._names[1].length !== 0)
        {
            for (let i = 0; i < 2; i++)
            {
                this._key_boxes[i] = wsvg.create("polygon");
                this._key_boxes[i].style.display = "block";
                this._key_boxes[i].setAttribute("stroke", "#202020");
                this._key_boxes[i].setAttribute("stroke-width", "1");
                this._view.appendChild(this._key_boxes[i]);

                this._key_labels[i] = new OJLabel();
                this._key_labels[i].GetParagraphStyle().textAlign = "left";
                this._key_labels[i].SetValue(" ");
                this.AddChild(this._key_labels[i]);
            }

            this._key_boxes[0].setAttribute("fill", "#0054ae"); // Blue
            this._key_boxes[1].setAttribute("fill", "#F08040"); // Red
        }

        this._cropping_label = new OJLabel();
        this._cropping_label.GetParagraphStyle().textAlign = "left";
        this._cropping_label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 175 });
        this._cropping_label.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 12 });
        this._cropping_label.SetValue(" ");
        this.AddChild(this._cropping_label);

        this._wireframe_height = bar_chart_min_height;

        this._ready = true;
    }

    Destroy()
    {
        super.Destroy();
        this._destroyed = true;
        this._ready = false;
    }

    // UpdateValue called by the framework when an update is received
    // from the server
    UpdateValue(value)
    {
        this._num_bars = value.num_bars;

        if(this._num_bars > 0)
        {
            this._values[0] = value.values0;
            this._values[1] = value.values1;
        }

        this.DrawChart();
    }

    GetHeight()
    {
        return this._wireframe_height;
    }

    GetControlHeight(parent_width, parent_height)
    {
        let height_new = 0;

        if(parent_height != null)
        {
            let parent_client_height = parent_height - this._header_height;
            height_new =  parent_client_height - (56 + (3 * 12));
        }

        this._wireframe_height = Math.max(height_new, bar_chart_min_height);

        return this._wireframe_height;
    }

    MakePoints(points_array)
    {
        let points = "";
        let x = 0;
        let y = 0;
        for (let i = 0; i < points_array.length; i++)
        {
            let point = points_array[i];
            let dx = point[0];
            let dy = point[1];
            x += dx;
            y += dy;
            let space = (i == 0) ? "" : " ";
            points += space + x.toString() + "," + y.toString();
        }

        return points;
    }

    Resize(x, y, width, height)
    {
        let size_changed = super.Resize(x, y, width, height);

        // 1 Pixel border on either side
        this._svg.setAttribute("width", width - 2);
        this._svg.setAttribute("height", height - 2);

        let border = this._border + 1;
        let window_points = this.MakePoints([[border, border],
                                            [width - border * 2, 0], 
                                            [0, height - border * 2], 
                                            [-width + border * 2, 0]]);
        this._window_polygon.setAttribute("points", window_points);

        this._rect_width = Math.floor(this._width / this._num_bars);

        let margin = 18;
        this._y_min = margin;
        this._y_max = Math.max(this._height - 40 - margin, this._y_min + 1);
        this._img_height = this._y_max - this._y_min;

        this._x_min = Math.floor((this._width % this._num_bars) / 2);
        this._x_max = this._x_min + this._num_bars * this._rect_width;
        this._img_width = this._x_max - this._x_min;

        this.CreateAxis();

        this.DrawChart();

        return size_changed;
    }

    CreateAxis()
    {
        this._x_axis_stubs = [];
        this._stub_interval = Math.ceil(32 / this._rect_width) * this._rect_width; // at least 32 pixel intervals, matched to bar width
        this._num_stubs = Math.floor(this._rect_width * this._num_bars / this._stub_interval) + 1;

        for (let i = 0; i < this._num_stubs; i++)
        {
            if (!this._x_axis_stubs[i])
            {
                this._x_axis_stubs[i] = wsvg.create("polygon");
                this._x_axis_stubs[i].style.display = "block";
                this._x_axis_stubs[i].setAttribute("stroke", "#202020");
                this._x_axis_stubs[i].setAttribute("stroke-width", "1");
                this._view.appendChild(this._x_axis_stubs[i]);
            }

            if (!this._axis_labels[i])
            {
                this._axis_labels[i] = new OJLabel();
                this._axis_labels[i].GetParagraphStyle().textAlign = "centre";
                this._axis_labels[i].SetValue(" ");
                this.AddChild(this._axis_labels[i]);
            }

            let x0 = this._x_min + i * this._stub_interval;
            let y0 = this._y_max;

            this._x_axis_stubs[i].setAttribute("points", (x0 + "," + y0 + " " + x0 + "," + (y0 + 4)));

            this._axis_labels[i].SetValue(i * this._stub_interval / this._rect_width);
            this._axis_labels[i].SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._x_min + i * this._stub_interval - 15 });
            this._axis_labels[i].SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._y_max + 5 });
            this._axis_labels[i].SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: this._stub_interval });
            this._axis_labels[i].SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 12 });
        }

        let box1_pts = [ [ this._x_min + 4, this._y_max + 26 ], [ this._x_min + 14, this._y_max + 26], [ this._x_min + 14, this._y_max + 36], [ this._x_min + 4, this._y_max + 36] ];
        let box2_pts = [ [ this._x_min + 154, this._y_max + 26 ], [ this._x_min + 164, this._y_max + 26], [ this._x_min + 164, this._y_max + 36], [ this._x_min + 154, this._y_max + 36] ];

        if(this._key_boxes.length == 2)
        {
            this._key_boxes[0].setAttribute("points", box1_pts);
            this._key_boxes[1].setAttribute("points", box2_pts);
        }

        if(this._key_labels.length == 2)
        {
            this._key_labels[0].SetValue(this._names[0]);
            this._key_labels[0].SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._x_min + 18 });
            this._key_labels[0].SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._y_max + 22 });
            this._key_labels[0].SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 150 });
            this._key_labels[0].SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 12 });

            this._key_labels[1].SetValue(this._names[1]);
            this._key_labels[1].SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._x_min + 168 });
            this._key_labels[1].SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._y_max + 22 });
            this._key_labels[1].SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 150 });
            this._key_labels[1].SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 12 });
        }

        this.PositionChildren();
    }

    DrawChart()
    {
        if (!(this._values[0] && this._values[1]))
        {
            return;
        }

        let max_2 = 0;

        let max_i = 0;
        let max_series = 0;

        // Find the largest value and keep track of its location, find the second largest value (in another location)

        for (let i = 0; i < this._num_bars; i++)
        {
            if (this._values[0][i] > this._values[max_series][max_i])
            {
                max_2 = this._values[max_series][max_i];
                max_i = i;
                max_series = 0;
            }
            else if (this._values[0][i] > max_2)
            {
                max_2 = this._values[0][i];
            }

            if (this._values[1][i] > this._values[max_series][max_i])
            {
                if (i != max_i)
                {
                    max_2 = this._values[max_series][max_i];
                }
                max_i = i;
                max_series = 1;
            }
            else if (this._values[1][i] > max_2)
            {
                if (i != max_i)
                {
                    max_2 = this._values[1][i];
                }
            }
        }

        // If the largest value is significantly larger than the second largest, crop the largest value
        let peak_ratio = this._values[max_series][max_i] / max_2;
        let max;
        let cropping;
        if (peak_ratio > 1.75)
        {
            max = max_2 * 1.3;
            cropping = true;
        }
        else
        {
            max = this._values[max_series][max_i];
            cropping = false;
        }

        this._cropping_label.SetValue(" ");

        let margin = 18;
        let v_scale = this._img_height - margin;

        // Do not stroke the bars in a small scale
        let bar_stroke = this._rect_width > 1 ? 1 : 0;        

        for (let i = 0; i < this._num_bars; i++)
        {
            
            let x0 = this._x_min + this._rect_width * i;
            let y0 = this._y_max;
            let x1 = x0 + this._rect_width;

            let pix_height0 = Math.min(1.0, this._values[0][i] / max) * v_scale + 1;
            let pix_height1 = Math.min(1.0, this._values[1][i] / max) * v_scale + 1;

            if (cropping && i == max_i)
            {
                // Show cropping indicator
                let label_offset = x0 - 175;
                if (label_offset < 0)
                {
                    label_offset = x1;
                }
                this._cropping_label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: label_offset });
                this._cropping_label.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._y_min + 5 });

                this._cropping_label.SetValue("Bar cropped for clarity");
                this.PositionChildren();
            }

            this._bars[0][i].setAttribute("stroke-width", bar_stroke);
            
            this._bars[0][i].setAttribute("points", (x0 + "," + y0 + " " + x1 + "," + y0 + " " + x1 + "," + (y0 - pix_height0) + " " + x0 + "," + (y0 - pix_height0)));
            this._bars[1][i].setAttribute("points", (x0 + "," + y0 + " " + x1 + "," + y0 + " " + x1 + "," + (y0 - pix_height1) + " " + x0 + "," + (y0 - pix_height1)));
        }        
    }

    UpdateServer()
    {
        var value_string = this._num_bars;
        value_string += "," + this._names[0];
        value_string += "," + this._values[0];
        value_string += "," + this._names[1];
        value_string += "," + this._values[1];
        this._value_changed_cb.Call(value_string);
    }

    static CreateCustomControl(element_type, params)
    {
        if (element_type == "OJBarChartControl")
        {
            return new OJBarChartControl(params);
        }
    }
}

// Register our control factory fn for the OJBarChartControl custom control
OJControlContainer.AddCustomControlFactory("OJBarChartControl", OJBarChartControl.CreateCustomControl);
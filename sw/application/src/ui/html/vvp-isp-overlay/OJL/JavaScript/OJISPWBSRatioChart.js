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
import { OJGrid } from "./OJLExtras.js";
import { OJControlContainer, OJControlItemBase } from "./OJLExtras.js";
import { OJLib } from "./OJLExtras.js";
import { wsvg } from "./OJLExtras.js";
import { OJControlItemHiddenValue } from "./OJLExtras.js";
import { OJScrollable } from "./OJLExtras.js";
import { OJLabel } from "./OJLExtras.js";

export class OJISPWBSRatioChart extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);
        this._ready = false;
        this._values_r = ui_element_json.value.values_r;
        this._values_g = ui_element_json.value.values_g;
        this._values_b = ui_element_json.value.values_b;

        this._selected = 0;

        let margin = 12;
        this._y_min = margin;
        this._y_max = Math.max([this._height - 40 - margin, this._y_min + 1]);
        this._img_height = this._y_max - this._y_min;

        this._x_min = margin;
        this._x_max = 800;
        this._img_width = this._x_max - this._x_min;

        this._control_height = 0;
        this._svg = wsvg.create("svg");
        this._svg.style.backgroundColor = "transparent";
        this._svg.style.border = "#b7b7b7";
        this._svg.style.borderWidth = "1px";
        this._svg.style.borderStyle = "transparent";


        this._view = wsvg.create("g");
        this._svg.appendChild(this._view);
        this.GetElement().appendChild(this._svg);
        this._marker_thickness = 7;
        this._border = 8;
        this._button_down = false;


        let center_pixel_translate = "translate(-0.5 -0.5)";
        this._selector_parent = wsvg.create("rect");
        this._selector_parent.setAttribute("x", 0);
        this._selector_parent.setAttribute("y", 0);
        this._selector_parent.setAttribute("width", 1);
        this._selector_parent.setAttribute("height", 1);
        this._selector_parent.setAttribute("stroke", "#303030");
        this._selector_parent.setAttribute("stroke-width", "2");
        this._selector_parent.setAttribute("fill", "#d8d8de");
        this._selector_parent.setAttribute("transform", center_pixel_translate);
        this._selector_parent.style.display = "block";
        this._view.appendChild(this._selector_parent);

        this._selection_grid = [];
        for (let i = 0; i < 49; i++)
        {
            /*
            this._values_r[i] = (Math.random()*0.8) + 0.2;
            this._values_g[i] = (Math.random()*0.8) + 0.2;
            this._values_b[i] = (Math.random()*0.8) + 0.2;
            let tot = this._values_r[i] + this._values_g[i] + this._values_b[i];
            this._values_r[i] = Math.floor((this._values_r[i]/tot) * 100);
            this._values_g[i] = Math.floor((this._values_g[i]/tot) * 100);
            this._values_b[i] = Math.floor((this._values_b[i]/tot) * 100);
            */
            this._selection_grid[i] = wsvg.create("rect");
            this._selection_grid[i].style.display = "block";
            this._selection_grid[i].setAttribute("stroke", "#202020");
            this._selection_grid[i].setAttribute("stroke-width", "1");
            this._selection_grid[i].setAttribute("fill", "transparent");
            this._selection_grid[i].setAttribute("opacity", "1.0");
            this._selection_grid[i].onmousedown = ItemSelected;
            this._selection_grid[i].parentObject = this;
            this._selection_grid[i]._index = i;
            this._view.appendChild(this._selection_grid[i]);
        }

        this._select_line = wsvg.create("rect");
        this._select_line.style.display = "block";
        this._select_line.setAttribute("stroke", "#202020");
        this._select_line.setAttribute("stroke-width", "5");
        this._select_line.setAttribute("fill", "transparent");
        this._select_line.setAttribute("opacity", "1.0");
        this._view.appendChild(this._select_line);

        this._value_labels = [];
        this._chart_labels = [];
        this._valueBars = [];
        this._graph_horizontal_line;

        for (let i = 0; i < 3; i++)
        {
            this._valueBars[i] = wsvg.create("rect");
            this._valueBars[i].style.display = "block";
            this._valueBars[i].setAttribute("stroke", "#202020");
            this._valueBars[i].setAttribute("stroke-width", "1");
            this._valueBars[i].setAttribute("fill", "#000000");
            this._valueBars[i].setAttribute("opacity", "1.0");
            this._view.appendChild(this._valueBars[i]);

            this._value_labels[i] = new OJLabel();
            this._value_labels[i].GetParagraphStyle().textAlign = "centre";
            this._value_labels[i].SetValue(" ");
            this.AddChild(this._value_labels[i]);

            this._chart_labels[i] = new OJLabel();
            this._chart_labels[i].GetParagraphStyle().textAlign = "centre";
            this._chart_labels[i].SetValue(" ");
            this.AddChild(this._chart_labels[i]);
        }

        this._graph_horizontal_line = wsvg.create("line");
        this._graph_horizontal_line.setAttribute("stroke", "#303030");
        this._graph_horizontal_line.setAttribute("stroke-width", "2");
        this._view.appendChild(this._graph_horizontal_line);

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
        this._values_r = value.values_r;
        this._values_g = value.values_g;
        this._values_b = value.values_b;

        this.DrawChart();
    }

    GetHeight()
    {
        return this._wireframe_height;
    }

    GetControlHeight(parent_width, parent_height)
    {
        if (parent_width != null)
        {
            let parent_client_height = parent_height - this._header_height;
            this._wireframe_height = Math.min(parent_client_height - (56 + (3 * 12)), 700);
        }

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

        this._width = width - 2;
        this._height = height - 2;

        this.DrawChart();

        return size_changed;
    }

    SelectGrid(index)
    {

    }

    ExtractColourString(r, g, b)
    {
        let maxValue = 0
        if ((r >= g) && (r >= b))
        {
            maxValue = r;
        }
        else if ((g >= r) && (g >= b))
        {
            maxValue = g;
        }
        else
        {
            maxValue = b;
        }

        let scaledR = Math.floor(255 * (r / maxValue));
        let rString = scaledR.toString(16);
        if (scaledR < 16)
        {
            rString = "0" + rString;
        }

        let scaledG = Math.floor(255 * (g / maxValue));
        let gString = scaledG.toString(16);
        if (scaledG < 16)
        {
            gString = "0" + gString;
        }

        let scaledB = Math.floor(255 * (b / maxValue));
        let bString = scaledB.toString(16);
        if (scaledB < 16)
        {
            bString = "0" + bString;
        }
        return "#" + rString + gString + bString;
    }

    DrawSelector(start_x, start_y, width, height)
    {
        this._selector_parent.setAttribute("x", start_x);
        this._selector_parent.setAttribute("y", start_y);
        this._selector_parent.setAttribute("width", width);
        this._selector_parent.setAttribute("height", height);

        this._grid_start_x = start_x; // Border thickness
        this._grid_start_y = start_y;
        this._grid_element_width = (width - 1) / 7;
        this._grid_element_height = (height - 1) / 7;

        for (let y = 0; y < 7; y++)
        {
            for (let x = 0; x < 7; x++)
            {
                let index = x + (y * 7)
                this._selection_grid[index].setAttribute("x", this._grid_start_x + (x * this._grid_element_width));
                this._selection_grid[index].setAttribute("y", this._grid_start_y + (y * this._grid_element_height));
                this._selection_grid[index].setAttribute("width", this._grid_element_width);
                this._selection_grid[index].setAttribute("height", this._grid_element_height);

                this._selection_grid[index].setAttribute("fill", this.ExtractColourString(this._values_r[index], this._values_g[index], this._values_b[index]))
            }
        }

        this._select_line.setAttribute("x", this._grid_start_x + ((this._selected % 7) * this._grid_element_width));
        this._select_line.setAttribute("y", this._grid_start_y + (Math.floor(this._selected / 7) * this._grid_element_height));
        this._select_line.setAttribute("width", this._grid_element_width);
        this._select_line.setAttribute("height", this._grid_element_height);
        this._select_line.setAttribute("stroke", "#303030");

    }

    DrawGraph(start_x, start_y, width, height)
    {
        let horiz_margin = 25; // From either side
        let verti_margin = 25; // From the bottom and top

        let graph_width = width - 2*horiz_margin;
        let graph_height = height - 2*verti_margin;
        let bar_thickness = 1;

        let spur_width = 6

        this._graph_horizontal_line.setAttribute("x1", start_x + horiz_margin);
        this._graph_horizontal_line.setAttribute("y1", start_y + verti_margin + graph_height);
        this._graph_horizontal_line.setAttribute("x2", start_x + horiz_margin + graph_width);
        this._graph_horizontal_line.setAttribute("y2", start_y + verti_margin + graph_height);

        // We want the bars to take up 80% of the graph width
        // With 5% gap between each, and 5% on each side
        // We don't use full height to give room for the text

        let value_bar_width = (graph_width * 0.8) / 3;
        let value_bar_gap = (graph_width * 0.2) / 4;

        let text_height = 18

        let max_value = this._values_r[this._selected];
        if (this._values_g[this._selected] > max_value)
        {
            max_value = this._values_g[this._selected];
        }
        if (this._values_b[this._selected] > max_value)
        {
            max_value = this._values_b[this._selected];
        }

        for (let i = 0; i < 3; i++)
        {
            // This is an abomination. But I didn't want to rework the code.
            let value = 0;
            if (i == 0)
            {
                value = this._values_r[this._selected];
            }
            else if (i == 1)
            {
                value = this._values_g[this._selected];
            }
            else
            {
                value = this._values_b[this._selected];
            }

            let value_bar_height = (value / max_value) * graph_height;
            let value_bar_start_x = start_x + horiz_margin + value_bar_gap + (i * (value_bar_width + value_bar_gap));
            let value_bar_start_y = start_y + verti_margin + graph_height - value_bar_height;

            this._valueBars[i].setAttribute("x", value_bar_start_x);
            this._valueBars[i].setAttribute("y", value_bar_start_y);
            this._valueBars[i].setAttribute("width", value_bar_width);
            this._valueBars[i].setAttribute("height", value_bar_height);
            this._valueBars[i].setAttribute("rx", 5);

            this._value_labels[i].SetValue((value / 100).toFixed(2).toString());
            this._value_labels[i].SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: value_bar_start_x });
            this._value_labels[i].SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: value_bar_start_y - (text_height) });
            this._value_labels[i].SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: value_bar_width });
            this._value_labels[i].SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: (text_height) });

            this._chart_labels[i].SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: value_bar_start_x });
            this._chart_labels[i].SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: start_y + verti_margin + graph_height + (text_height / 2) });
            this._chart_labels[i].SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: value_bar_width });
            this._chart_labels[i].SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: (text_height / 2) });
        }




        // Labels

        this._chart_labels[0].SetValue("R");
        this._valueBars[0].setAttribute("fill", "#FF1A1A");
        this._chart_labels[1].SetValue("G");
        this._valueBars[1].setAttribute("fill", "#33FF33");
        this._chart_labels[2].SetValue("B");
        this._valueBars[2].setAttribute("fill", "#0033FF");
    }

    DrawChart()
    {
        // 1 Pixel border on either side
        let border = this._border + 1;
        let edge_margin = 12;
        let graph_margin = 12;
        let graph_width = 350;
        let graph_height = 175;
        let graph_start_x = border + graph_margin + ((this._width - ((2*(border + graph_margin)) + graph_width)) / 2);
        let graph_start_y = 380
        this.DrawGraph(graph_start_x, graph_start_y, graph_width, graph_height, 0);

        let selector_start_x = border + edge_margin;
        let selector_start_y = border + edge_margin;
        let selector_width = this._width - selector_start_x - (border + edge_margin);
        let selector_height = 300;
        this.DrawSelector(selector_start_x, selector_start_y, selector_width, selector_height);

        this.PositionChildren();
    }

    UpdateServer()
    {
        var value_string = this._values[0] + "," + this._values[1] + "," + this._values[2];
        this._value_changed_cb.Call(value_string);
    }

    static CreateCustomControl(element_type, params)
    {
        if (element_type == "OJISPWBSRatioChart")
        {
            return new OJISPWBSRatioChart(params);
        }
    }
}


function ItemSelected(event)
{
    this.parentObject._selected = this._index;

    this.parentObject.DrawChart();
}

// Register our control factory fn for the OJBarChartControl custom control
OJControlContainer.AddCustomControlFactory("OJISPWBSRatioChart", OJISPWBSRatioChart.CreateCustomControl);
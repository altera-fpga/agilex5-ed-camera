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

export class OJISPBLSChart extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);
        this._ready = false;
        this._max_value = ui_element_json.value.max_value;
        this._min_value = ui_element_json.value.min_value;
        this._values = ui_element_json.value.values;
        this._chart_names = ui_element_json.value.chart_names;
        this._chart_colours = ui_element_json.value.chart_colours;

        let margin = 12;
        this._y_min = margin;
        this._y_max = Math.max([this._height - 40 - margin, this._y_min + 1]);
        this._img_height = this._y_max - this._y_min;

        this._x_min = margin;
        this._x_max = 800;
        this._img_width = this._x_max - this._x_min;

        this._control_height = 0;
        this._svg = wsvg.create("svg");
        this._svg.style.backgroundColor = "#transparent";
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
        this._window_polygon = wsvg.create("polygon");
        this._window_polygon.setAttribute("points", "2,2 4,2 4,4 2,4");
        this._window_polygon.setAttribute("stroke", "transparent");
        this._window_polygon.setAttribute("stroke-width", "1");
        this._window_polygon.setAttribute("fill", "transparent");
        this._window_polygon.setAttribute("transform", center_pixel_translate);
        this._window_polygon.style.display = "block";
        this._view.appendChild(this._window_polygon);
        
        this._graph_horizontal_lines = []
        this._graph_vertical_lines = []
        this._expected_value_lines = []
        this._graph_black_level_spur = []
        this._graph_max_value_spur = []
        this._value_labels = []
        this._max_value_labels = []
        this._min_value_labels = []
        this._expected_value_labels = []
        this._chart_labels = []
        this._valueBars = [];
        this._maxValueBars = []
    
        for (let i = 0; i < 4; i++)
        {
            this._maxValueBars[i] = wsvg.create("rect");
            this._maxValueBars[i].style.display = "block";
            this._maxValueBars[i].setAttribute("stroke", "#202020");
            this._maxValueBars[i].setAttribute("stroke-width", "1");
            this._maxValueBars[i].setAttribute("fill", "#F0F0F0");
            this._maxValueBars[i].setAttribute("opacity", "1.0");
            this._view.appendChild(this._maxValueBars[i]);
    
            this._valueBars[i] = wsvg.create("rect");
            this._valueBars[i].style.display = "block";
            this._valueBars[i].setAttribute("stroke", "#202020");
            this._valueBars[i].setAttribute("stroke-width", "1");
            this._valueBars[i].setAttribute("fill", "#000000");
            this._valueBars[i].setAttribute("opacity", "1.0");
            this._view.appendChild(this._valueBars[i]);

            this._graph_horizontal_lines[i] = wsvg.create("line");
            this._graph_horizontal_lines[i].setAttribute("stroke", "#303030");
            this._graph_horizontal_lines[i].setAttribute("stroke-width", "2");
            this._view.appendChild(this._graph_horizontal_lines[i]);

            this._graph_vertical_lines[i] = wsvg.create("line");
            this._graph_vertical_lines[i].setAttribute("stroke", "#303030");
            this._graph_vertical_lines[i].setAttribute("stroke-width", "2");
            this._view.appendChild(this._graph_vertical_lines[i]);

            this._expected_value_lines[i] = wsvg.create("line");
            this._expected_value_lines[i].setAttribute("stroke", "#a0a0a0");
            this._expected_value_lines[i].setAttribute("stroke-width", "2");
            this._expected_value_lines[i].style.display = "block";
            this._view.appendChild(this._expected_value_lines[i]);

            this._graph_black_level_spur[i] = wsvg.create("line");
            this._graph_black_level_spur[i].setAttribute("stroke", "#303030");
            this._graph_black_level_spur[i].setAttribute("stroke-width", "2");
            this._view.appendChild(this._graph_black_level_spur[i]);

            this._graph_max_value_spur[i] = wsvg.create("line");
            this._graph_max_value_spur[i].setAttribute("stroke", "#303030");
            this._graph_max_value_spur[i].setAttribute("stroke-width", "2");
            this._view.appendChild(this._graph_max_value_spur[i]);

            this._value_labels[i] = new OJLabel();
            this._value_labels[i].GetParagraphStyle().textAlign = "right";
            this._value_labels[i].SetValue(" ");
            this.AddChild(this._value_labels[i]);

            this._max_value_labels[i] = new OJLabel();
            this._max_value_labels[i].GetParagraphStyle().textAlign = "right";
            this._max_value_labels[i].SetValue(" ");
            this.AddChild(this._max_value_labels[i]);

            this._min_value_labels[i] = new OJLabel();
            this._min_value_labels[i].GetParagraphStyle().textAlign = "right";
            this._min_value_labels[i].SetValue(" ");
            this.AddChild(this._min_value_labels[i]);

            this._chart_labels[i] = new OJLabel();
            this._chart_labels[i].GetParagraphStyle().textAlign = "centre";
            this._chart_labels[i].SetValue(" ");
            this.AddChild(this._chart_labels[i]);
        }

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
        this._max_value = value.max_value;
        this._min_value = value.min_value;
        this._values = value.values;
        this._chart_names = value.chart_names;
        if (value.update_chart_colours)
        {
            this._chart_colours = value._chart_colours;
        }

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
            this._wireframe_height = parent_client_height - (56 + (3 * 12));
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

        let border = this._border + 1;
        let window_points = this.MakePoints([[border, border],
                                            [width - border * 2, 0], 
                                            [0, height - border * 2], 
                                            [-width + border * 2, 0]]);
        this._window_polygon.setAttribute("points", window_points);

        this.DrawChart();

        return size_changed;
    }


    DrawSingleGraph(start_x, start_y, width, height, i, col_string)
    {
        let horiz_margin = 50; // From the left
        let verti_margin = 25; // From the bottom

        let bar_width = width - horiz_margin;
        let bar_height = height - verti_margin;
        let bar_thickness = 1;

        let spur_width = 6
    
        this._graph_horizontal_lines[i].setAttribute("x1", start_x + horiz_margin - spur_width);
        this._graph_horizontal_lines[i].setAttribute("y1", start_y + bar_height - bar_thickness);
        this._graph_horizontal_lines[i].setAttribute("x2", start_x + horiz_margin + bar_width);
        this._graph_horizontal_lines[i].setAttribute("y2", start_y + bar_height - bar_thickness);

        this._graph_vertical_lines[i].setAttribute("x1", start_x + horiz_margin - bar_thickness);
        this._graph_vertical_lines[i].setAttribute("y1", start_y);
        this._graph_vertical_lines[i].setAttribute("x2", start_x + horiz_margin - bar_thickness);
        this._graph_vertical_lines[i].setAttribute("y2", start_y + bar_height);

        // We want the main bar to take up 75% of the width, and 95% of the height.
        // We don't use full height to give room for the text
        let value_bar_max_height = bar_height * 0.95
        let value_bar_width = bar_width * 0.75
        let value_bar_margin = start_x + horiz_margin + bar_thickness + ((bar_width - value_bar_width) / 2)

        this._maxValueBars[i].setAttribute("x", value_bar_margin);
        this._maxValueBars[i].setAttribute("y", start_y + (bar_height - value_bar_max_height));
        this._maxValueBars[i].setAttribute("width", value_bar_width);
        this._maxValueBars[i].setAttribute("height", value_bar_max_height - bar_thickness);
        this._maxValueBars[i].setAttribute("rx", 5);
        this._maxValueBars[i].setAttribute("fill", this._chart_colours[i]);

        let value_bar_height = ((this._values[i] - this._min_value) / (this._max_value - this._min_value)) * value_bar_max_height;
        
        this._valueBars[i].setAttribute("x", value_bar_margin);
        this._valueBars[i].setAttribute("y", start_y + bar_height - value_bar_height);
        this._valueBars[i].setAttribute("width", value_bar_width);
        this._valueBars[i].setAttribute("height", value_bar_height - bar_thickness);
        this._valueBars[i].setAttribute("rx", 5);

        this._expected_value_lines[i].setAttribute("x1", start_x + horiz_margin - spur_width);
        this._expected_value_lines[i].setAttribute("y1", start_y + (bar_height - value_bar_max_height) + (value_bar_max_height/2));
        this._expected_value_lines[i].setAttribute("x2", start_x + horiz_margin + bar_width);
        this._expected_value_lines[i].setAttribute("y2", start_y + (bar_height - value_bar_max_height) + (value_bar_max_height/2));
    
        // Now the line spurs

        this._graph_max_value_spur[i].setAttribute("x1", start_x + horiz_margin - bar_thickness - spur_width);
        this._graph_max_value_spur[i].setAttribute("y1", start_y + (bar_height - value_bar_max_height));
        this._graph_max_value_spur[i].setAttribute("x2", start_x + horiz_margin - bar_thickness);
        this._graph_max_value_spur[i].setAttribute("y2", start_y + (bar_height - value_bar_max_height));

        this._graph_black_level_spur[i].setAttribute("x1", start_x + horiz_margin - bar_thickness - spur_width);
        this._graph_black_level_spur[i].setAttribute("y1", start_y + (bar_height - value_bar_height));
        this._graph_black_level_spur[i].setAttribute("x2", start_x + horiz_margin - bar_thickness);
        this._graph_black_level_spur[i].setAttribute("y2", start_y + (bar_height - value_bar_height));

        // Labels
        let text_height = 18
        let text_width = horiz_margin - spur_width - 4
        this._value_labels[i].SetValue(this._values[i].toString());
        this._value_labels[i].SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: start_x });
        this._value_labels[i].SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: start_y + (bar_height - value_bar_height) - (text_height / 2) });
        this._value_labels[i].SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: text_width });
        this._value_labels[i].SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: (text_height / 2) });
    
        this._max_value_labels[i].SetValue(this._max_value.toString());
        this._max_value_labels[i].SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: start_x });
        this._max_value_labels[i].SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: start_y + (bar_height - value_bar_max_height) - (text_height / 2) });
        this._max_value_labels[i].SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: text_width });
        this._max_value_labels[i].SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: (text_height / 2) });

        this._min_value_labels[i].SetValue(this._min_value.toString());
        this._min_value_labels[i].SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: start_x });
        this._min_value_labels[i].SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: start_y + (bar_height) - (text_height / 2) });
        this._min_value_labels[i].SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: text_width });
        this._min_value_labels[i].SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: (text_height / 2) });
    
        this._chart_labels[i].SetValue(this._chart_names[i]);
        this._chart_labels[i].SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: start_x + horiz_margin });
        this._chart_labels[i].SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: start_y + bar_height + (text_height / 2) });
        this._chart_labels[i].SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: bar_width });
        this._chart_labels[i].SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: (text_height / 2) });
    
    }

    DrawChart()
    {
        // 1 Pixel border on either side
        let border = this._border + 1;
        let edge_margin = 12;
        let graph_margin = 12;
        let graph_width = 125;
        let graph_height = 175;
        this.DrawSingleGraph(border + edge_margin, border + edge_margin, graph_width, graph_height, 0);
        this.DrawSingleGraph(border + edge_margin + graph_margin + graph_width, border + edge_margin, graph_width, graph_height, 1);
        this.DrawSingleGraph(border + edge_margin, border + edge_margin + graph_margin + graph_height, graph_width, graph_height, 2);
        this.DrawSingleGraph(border + edge_margin + graph_margin + graph_width, border + edge_margin + graph_margin + graph_height, graph_width, graph_height, 3);
       
        this.PositionChildren();
    }

    UpdateServer()
    {
        var value_string = this._max_value;
        for (let i = 0; i < 4; i++)
        {
            value_string += "," + this._chart_names[i];
            value_string += "," + this._values[i];
        }
        this._value_changed_cb.Call(value_string);
    }

    static CreateCustomControl(element_type, params)
    {
        if (element_type == "OJISPBLSChart")
        {
            return new OJISPBLSChart(params);
        }
    }
}

// Register our control factory fn for the OJBarChartControl custom control
OJControlContainer.AddCustomControlFactory("OJISPBLSChart", OJISPBLSChart.CreateCustomControl);
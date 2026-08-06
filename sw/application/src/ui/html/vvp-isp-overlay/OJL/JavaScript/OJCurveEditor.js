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
import { OJGrid } from "./OJLExtras.js";
import { OJControlContainer, OJControlItemBase } from "./OJLExtras.js";
import { OJLib } from "./OJLExtras.js";
import { wsvg } from "./OJLExtras.js";
import { OJControlItemHiddenValue } from "./OJLExtras.js";
import { OJBarChartControl } from "./OJLExtras.js";
import { ElementDragger } from "./OJLExtras.js";

const max_control_points = 16;

function clamp(v, min, max){
    return (v < min) ? min : ((v > max) ? max : v);
}

function clampn(v){
    return clamp(v, 0, 1);
}

export class OJCurveEditorControl extends OJBarChartControl 
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);
        this._ready = false;

        this._num_control_points = ui_element_json.value.num_control_points;
        this._control_point_vals = ui_element_json.value.control_points;
        this._knot_vals = ui_element_json.value.knots;
        this._control_points = [];
        this._knots = [];
        this._connectors = [];
        this._control_point_pixel_vals = [];
        this._knot_pixel_vals = [];

        for (let i = 0; i < this._num_control_points; i++)
        {
            this._control_point_pixel_vals[i] = [];
            this._control_point_pixel_vals[i][0] = (this._x_min + (this._control_point_vals[i][0] * this._img_width)) << 0;
            this._control_point_pixel_vals[i][1] = ((1 - this._control_point_vals[i][1]) * this._img_height + this._y_min) << 0;
        }

        for (let i = 0; i < this._num_control_points + 1; i++)
        {
            this._knot_pixel_vals[i] = [];
            this._knot_pixel_vals[i][0] = (this._x_min + (this._knot_vals[i][0] * this._img_width)) << 0;
            this._knot_pixel_vals[i][1] = ((1 - this._knot_vals[i][1]) * this._img_height + this._y_min) << 0;
        }

        this._img = wsvg.create("g");
        this._view.appendChild(this._img);

        // let center_pixel_translate = "translate(-0.5 -0.5)";
        this._img_box = wsvg.create("rect");
        this._img_box.setAttribute("x", this._x_min);
        this._img_box.setAttribute("y", this._y_min);
        this._img_box.setAttribute("width", this._img_width);
        this._img_box.setAttribute("height", this._img_height);
        this._img_box.setAttribute("stroke", "#303030");
        this._img_box.setAttribute("stroke-width", "1");
        this._img_box.setAttribute("fill", "none");
        this._img_box.style.display = "block";
        this._img.appendChild(this._img_box);

        this._control_point_radius = 5;

        this._on_cntrl_change_cb = function(index, new_x, new_y)
        {
            if (this._connectors[index])
                this.UpdateConnector(index);
            if (this._connectors[index + 1])
                this.UpdateConnector(index + 1);

            this.UpdateCurve();
            this.UpdateServer();
        }

        this._on_knot_change_cb = function(index, new_x, new_y)
        {
            if (index == 0)
            {
                this.UpdateConnector(0);
            }
            else if (index == this._num_control_points)
            {
                this.UpdateConnector(this._num_control_points);
            }

            this.UpdateCurve();
            this.UpdateServer();
        }
        
        this.CreateCurve();
        this.CreatePoints();

        this.UpdateKnots();
        this.UpdateControlPoints();
        this.UpdateConnectors();
        this.UpdateCurve();

        this.PositionChildren();
    }

    Destroy()
    {
        super.Destroy();
    }

    CreatePoints()
    {
        let center_pixel_translate = "translate(-0.5 -0.5)";

        // Control Point connectors
        for (let i = 0 ; i < this._num_control_points + 1; i++)
        {
            let path = [];

            if (i == 0)
            {
                path = [ this._knot_pixel_vals[0], this._control_point_pixel_vals[0]];
            }
            else if (i == this._num_control_points)
            {
                path = [ this._control_point_pixel_vals[this._num_control_points - 1], this._knot_pixel_vals[this._num_control_points] ];
            }
            else
            {
                path = [ this._control_point_pixel_vals[i - 1], this._control_point_pixel_vals[i] ];
            }

            this._connectors[i] = wsvg.create("path");
            this._connectors[i].setAttribute("stroke", "#303030");
            this._connectors[i].setAttribute("fill", "none");
            this._connectors[i].setAttribute("stroke-width", "1px");
            this._connectors[i].setAttribute("stroke-dasharray", "4 2");
            this._connectors[i].setAttribute("transform", center_pixel_translate);
            this._connectors[i]._d = path;
            this._connectors[i].setAttribute("d", wsvg.path_data(path));
            this._img.appendChild(this._connectors[i]);
        }

        let drag = wsvg.drag();
        let point_owner = this;

        // Control points & Draggers
        let make_cntrl_callback = function(curve_editor, idx)
        {
            return function(elem)
            {
                elem._d[0] = clamp(elem._d[0], 0, curve_editor._width);
                elem._d[1] = clamp(elem._d[1], 0, curve_editor._height);

                curve_editor._control_point_pixel_vals[idx] = elem._d;
                let pix_values = curve_editor._control_point_pixel_vals[idx];

                curve_editor._control_point_vals[idx][0] = (pix_values[0] - curve_editor._x_min) / curve_editor._img_width;
                curve_editor._control_point_vals[idx][1] = 1.0 - ((pix_values[1] - curve_editor._y_min) / curve_editor._img_height);

                elem.setAttribute("transform", curve_editor.GetControlPointTransform(idx));

                if (point_owner._on_cntrl_change_cb)
                    point_owner._on_cntrl_change_cb(idx, elem._d[0], elem._d[1]);
            };
        };

        for (let i = 0; i < this._num_control_points; i++)
        {
            let point = wsvg.create("circle");
            point._d = this._control_point_pixel_vals[i];
            point.setAttribute("transform", this.GetControlPointTransform(i));
            point.setAttribute("r", this._control_point_radius);
            point.setAttribute("stroke", "#00aeef");
            point.setAttribute("stroke-width", "2px");
            point.setAttribute("fill", "#00dfff");

            this._img.appendChild(point);

            // Add dragging
            drag.add_svg(point, make_cntrl_callback(this, i));

            point.onmouseover = function(event)
            {
                this.setAttribute("stroke", "#0078a7");
                this.setAttribute("fill", "#0098b7");
            };
            point.onmouseout = function(event)
            {
                this.setAttribute("stroke", "#00aeef");
                this.setAttribute("fill", "#00dfff");
            };
            point.onmouseup = function(event)
            {
                this.setAttribute("stroke", "#00aeef");
                this.setAttribute("fill", "#00dfff");
            };

            this._control_points[i] = point;
        }


        // Knots & draggers
        let make_knot_callback = function(curve_editor, idx)
        {
            return function(elem)
            {
                // Ensure that start and end point x vals do not change
                if (idx == 0)
                    elem._d[0] = curve_editor._x_min;
                else if (idx == curve_editor._num_control_points)
                    elem._d[0] = curve_editor._x_max;

                // Ensure that the point is within bounds
                elem._d[0] = clamp(elem._d[0], curve_editor._x_min, curve_editor._x_max);
                elem._d[1] = clamp(elem._d[1], curve_editor._y_min, curve_editor._y_max);

                curve_editor._knot_pixel_vals[idx] = elem._d;
                let pix_values = curve_editor._knot_pixel_vals[idx];

                curve_editor._knot_vals[idx][0] = (pix_values[0] - curve_editor._x_min) / curve_editor._img_width;
                curve_editor._knot_vals[idx][1] = 1.0 - ((pix_values[1] - curve_editor._y_min) / curve_editor._img_height);

                elem.setAttribute("transform", curve_editor.GetKnotTransform(idx));

                if (point_owner._on_knot_change_cb)
                    point_owner._on_knot_change_cb(idx, elem._d[0], elem._d[1]);
            };
        };

        for (let i = 0; i < this._num_control_points + 1; i++)
        {
            let knot = wsvg.create("circle");
            knot._d = this._knot_pixel_vals[i];
            knot.setAttribute("transform", this.GetKnotTransform(i));
            knot.setAttribute("r", this._control_point_radius);
            knot.setAttribute("stroke", "#00aeef");
            knot.setAttribute("fill", "#00aeef");

            this._img.appendChild(knot);

            // Add dragging
            drag.add_svg(knot, make_knot_callback(this, i));

            knot.onmouseover = function(event)
            {
                this.setAttribute("stroke", "#0078a7");
                this.setAttribute("fill", "#0078a7");
            };
            knot.onmouseout = function(event)
            {
                this.setAttribute("stroke", "#00aeef");
                this.setAttribute("fill", "#00aeef");
            };
            knot.onmouseup = function(event)
            {
                this.setAttribute("stroke", "#00aeef");
                this.setAttribute("fill", "#00aeef");
            };

            this._knots[i] = knot;
        }
    }

    CreateCurve()
    {
        let center_pixel_translate = "translate(-0.5 -0.5)";

        let curve = wsvg.create("path");
        curve.setAttribute("stroke", "#303030");
        curve.setAttribute("fill", "none");
        curve.setAttribute("stroke-width", "2px");
        curve.setAttribute("transform", center_pixel_translate);

        let path = "M " + this._knot_pixel_vals[0][0] + " " + this._knot_pixel_vals[0][1];

        for (let i = 0; i < this._num_control_points; i++)
        {
            // NOTE: this creates a smooth spline by inverting the second control point of the previous segment and using that as the first control point
            // This ensures C1 continuity at the knots (i.e. same gradient)
            // To specify both control points use C instead of S
            path += " S " + this._control_point_pixel_vals[i][0] + " " + this._control_point_pixel_vals[i][1]
                + " " + this._knot_pixel_vals[i + 1][0] + " " + this._knot_pixel_vals[i + 1][1];
        }

        // curve._d = path;
        curve.setAttribute("d", path);
        this._img.appendChild(curve);
        this._curve = curve;
    }

    RemovePoints()
    {
        for (let i = 0; i < this._num_control_points; i++)
        {
            if (this._img.contains(this._control_points[i]))
                this._img.removeChild(this._control_points[i]);
            this._control_points[i] = 0;
        }

        for (let i = 0; i < this._num_control_points + 1; i++)
        {
            if (this._img.contains(this._connectors[i]))
                this._img.removeChild(this._connectors[i]);
            this._connectors[i] = 0;
        }

        for (let i = 0; i < this._num_control_points + 1; i++)
        {
            if (this._img.contains(this._knots[i]))
                this._img.removeChild(this._knots[i]);
            this._knots[i] = 0;
        }
    }

    GetControlPointTransform(index)
    {
        let pixels = this._control_point_pixel_vals[index];
        // Subtract 0.5 to centre the line in the middle of a pixel
        let transform = "translate(" + (pixels[0] - 0.5) + " " + (pixels[1] - 0.5) + ")";
        return transform;
    }

    GetKnotTransform(index)
    {
        let pixels = this._knot_pixel_vals[index];
        // Subtract 0.5 to centre the line in the middle of a pixel
        let transform = "translate(" + (pixels[0] - 0.5) + " " + (pixels[1] - 0.5) + ")";
        return transform;
    }

    UpdateCurve()
    {
        let path = "M " + this._knot_pixel_vals[0][0] + " " + this._knot_pixel_vals[0][1];

        for (let i = 0; i < this._num_control_points; i++)
        {
            // NOTE: this creates a smooth spline by inverting the second control point of the previous segment and using that as the first control point
            // This ensures C1 continuity at the knots (i.e. same gradient)
            // To specify both control points use C instead of S
            path += " S " + this._control_point_pixel_vals[i][0] + " " + this._control_point_pixel_vals[i][1]
                + " " + this._knot_pixel_vals[i + 1][0] + " " + this._knot_pixel_vals[i + 1][1];
        }

        this._curve.setAttribute("d", path);
        this._curve.setAttribute("stroke", "#303030");
    }

    UpdateKnots()
    {
        for (let i = 0; i < this._num_control_points + 1; i++)
        {
            this.UpdateKnot(i);
        }
    }

    UpdateKnot(index)
    {
        this._knots[index].setAttribute("transform", this.GetKnotTransform(index));
        this._knots[index]._d = this._knot_pixel_vals[index];
    }

    UpdateControlPoints()
    {
        for (let i = 0; i < this._num_control_points; i++)
        {
            this.UpdateControlPoint(i);
        }
    }

    UpdateControlPoint(index)
    {
        this._control_points[index].setAttribute("transform", this.GetControlPointTransform(index));
        this._control_points[index]._d = this._control_point_pixel_vals[index];
    }

    UpdateConnectors()
    {
        for (let i = 0; i < this._num_control_points + 1; i++)
        {
            this.UpdateConnector(i);
        }
    }

    UpdateConnector(index)
    {
        let path = [];

        if (index == 0)
        {
            path = [ this._knot_pixel_vals[0], this._control_point_pixel_vals[0]];
        }
        else if (index == this._num_control_points)
        {
            path = [ this._control_point_pixel_vals[this._num_control_points - 1], this._knot_pixel_vals[this._num_control_points] ];
        }
        else
        {
            path = [ this._control_point_pixel_vals[index - 1], this._control_point_pixel_vals[index] ];
        }

        this._connectors[index]._d = path;
        this._connectors[index].setAttribute("d", wsvg.path_data(path));
    }

    Resize(x, y, width, height)
    {
        let size_changed = super.Resize(x, y, width, height);

        this._img_box.setAttribute("x", this._x_min);
        this._img_box.setAttribute("y", this._y_min);
        this._img_box.setAttribute("width", this._img_width);
        this._img_box.setAttribute("height", this._img_height);

        this.UpdateControlPoints();
        this.UpdateConnectors();
        this.UpdateCurve();
        this.PositionChildren();

        return size_changed;
    }

    // UpdateValue called by the framework when an update is received
    // from the server
    UpdateValue(value)
    {
        super.UpdateValue(value);

        this._control_point_vals = value.control_points;
        this._knot_vals = value.knots;
        let num_control_points = value.num_control_points;

        this._control_point_pixel_vals = [];

        for (let i = 0; i < num_control_points; i++)
        {
            let tmp = this._control_point_vals[i];
            let tmpc = [clampn(tmp[0]), clampn(tmp[1])];
            this._control_point_vals[i] = tmpc;
            let pxl_x = (this._x_min + (tmpc[0] * this._img_width)) << 0;
            let pxl_y = ((1 - tmpc[1]) * this._img_height + this._y_min) << 0;
            this._control_point_pixel_vals[i] = [pxl_x, pxl_y];
        }

        this._knot_pixel_vals = [];

        for (let i = 0; i < num_control_points + 1; i++)
        {
            let tmp = this._knot_vals[i];
            let tmpc = [clampn(tmp[0]), clampn(tmp[1])];
            this._knot_vals[i] = tmpc;
            let pxl_x = (this._x_min + (tmpc[0] * this._img_width)) << 0;
            let pxl_y = ((1 - tmpc[1]) * this._img_height + this._y_min) << 0;
            this._knot_pixel_vals[i] = [pxl_x, pxl_y];
        }

        if (this._num_control_points != num_control_points)
        {
            // Delete connectors, control points
            this.RemovePoints();
            this._num_control_points = num_control_points;
            this.CreatePoints();
        }

        this.UpdateKnots();
        this.UpdateControlPoints();
        this.UpdateConnectors();
        this.UpdateCurve();

        this.PositionChildren();
    }

    UpdateServer()
    {
        var value_string = this._num_control_points;
        value_string += "," + this._control_point_vals;
        value_string += "," + this._knot_vals;
        this._value_changed_cb.Call(value_string);
    }

    static CreateCustomControl(element_type, params)
    {
        if (element_type == "OJCurveEditorControl")
        {
            return new OJCurveEditorControl(params);
        }
    }
}

// Register our control factory fn for the OJCurveEditorControl custom control
OJControlContainer.AddCustomControlFactory("OJCurveEditorControl", OJCurveEditorControl.CreateCustomControl);
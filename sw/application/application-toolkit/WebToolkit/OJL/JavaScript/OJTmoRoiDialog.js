/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";
import { OJServerLink } from "./OJL.js";
import { OJModalDialog } from "./OJL.js";
import { OJControlContainer, OJControlItemBase } from "./OJL.js";
import { OJLib, ElementDragger } from "./OJL.js";
import { wsvg } from "./OJL.js";

export class OJTmoRoiDialog extends OJModalDialog
{
    constructor(params)
    {
        let width = 1708;
        let height = 910;
        
        // Base class constructor
        super(width, height, params._dialog_title);
        this._control_panels = OJServerLink.Get().LoadControls(params.Controls, this);
    }

    Destroy()
    {
        // Remove the control panels
        OJServerLink.Get().RemoveControls(this._control_panels);

        super.Destroy();

        // The custom 'controls' are not necessarily children windows of this
        // dialog so might not have been destroyed by OJModalDialog.prototype.Destroy
        let custom_controls = this._control_panels._custom_controls;
        for (let custom_control of custom_controls)
            custom_control.Destroy();

        this._control_panels = {};
    }
}

export class OJRoiControl extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);

        this._ready = false;
        this._show_hide_elements = [];
        this.SetValueFromJSON(ui_element_json.value);

        this._x_minimum = 0;
        this._y_minimum = 0;
        this._x_maximum = 1;
        this._y_maximum = 1;
        this._minimum_width = 0.125;
        this._minimum_height = 0.2222;
        this._maximum_width = 1;
        this._maximum_height = 1;

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
        this._corners = [];
        this._draggers = [];

        let thickness = this._marker_thickness - 1;
        let length = 30 - 1;
        let inner_length = length - thickness;
        let tp1 = thickness / 2;
        let long_length = 60;

        let center_pixel_translate = "translate(-0.5 -0.5)";
        this._window_polygon = wsvg.create("polygon");
        this._window_polygon.setAttribute("points", "2,2 4,2 4,4 2,4");
        this._window_polygon.setAttribute("stroke", "#303030");
        this._window_polygon.setAttribute("stroke-width", "1");
        this._window_polygon.setAttribute("fill", "#d8d8de");
        this._window_polygon.setAttribute("transform", center_pixel_translate);
        this._view.appendChild(this._window_polygon);

        this._roi_rectangle = wsvg.create("polygon");
        this._roi_rectangle.setAttribute("points", "2,2 4,2 4,4 2,4");
        this._roi_rectangle.setAttribute("stroke", "#0068b5");
        this._roi_rectangle.setAttribute("stroke-width", "1");
        this._roi_rectangle.setAttribute("fill", "#e8e8ee"); 
        this._roi_rectangle.style.cursor = "move";
        this._roi_rectangle.setAttribute("transform", center_pixel_translate);
        this._show_hide_elements.push(this._roi_rectangle);

        let element_dragger = new ElementDragger(this, this._roi_rectangle, -1);
        this._draggers.push(element_dragger);
    
        this._view.appendChild(this._roi_rectangle);

        this._out_color = "#00aeef";
        this._over_color = "#0078a7";
        this._down_color = "#0062aa";

        for (let i = 0; i < 8; i++)
        {
            let corner = wsvg.create("polygon");
            let points = "";
            let cursor = "default";

            if (i == 0)
            {
                points = this.MakePoints([[-tp1, -tp1], [length, 0], [0, thickness], [-inner_length, 0], [0, inner_length], [-thickness, 0]]);
                cursor = "nwse-resize";
            }
            else if (i == 1)
            {
                points = this.MakePoints([[tp1, -tp1], [0, length], [-thickness, 0], [0, -inner_length], [-inner_length, 0], [0, -thickness]]);
                cursor = "nesw-resize";
            }
            if (i == 2)
            {
                points = this.MakePoints([[-tp1, tp1], [0, -length], [thickness, 0], [0, inner_length], [inner_length, 0], [0, thickness]]);
                cursor = "nesw-resize";
            }
            else if (i == 3)
            {
                points = this.MakePoints([[tp1, tp1], [-length, 0], [0, -thickness], [inner_length, 0], [0, -inner_length], [thickness, 0]]);
                cursor = "nwse-resize";
            }
            else if (i == 4)
            {
                points = this.MakePoints([[-(long_length) / 2, -tp1], [long_length, 0], [0, thickness], [-long_length, 0]]);
                cursor = "ns-resize";
            }
            else if (i == 5)
            {
                points = this.MakePoints([[-(long_length) / 2, -tp1], [long_length, 0], [0, thickness], [-long_length, 0]]);
                cursor = "ns-resize";
            }
            else if (i == 6)
            {
                points = this.MakePoints([[-tp1, -(long_length) / 2], [thickness, 0], [0, long_length], [-thickness, 0]]);
                cursor = "ew-resize";
            }
            else if (i == 7)
            {
                points = this.MakePoints([[-tp1, -(long_length) / 2], [thickness, 0], [0, long_length], [-thickness, 0]]);
                cursor = "ew-resize";
            }

            corner.setAttribute("points", points);
            corner.setAttribute("stroke", this._out_color);
            corner.setAttribute("fill", this._out_color);        
            corner.style.cursor = cursor;
            this._corners.push(corner);
            this._show_hide_elements.push(corner);

            this._view.appendChild(corner);

            let element_dragger = new ElementDragger(this, corner, i);
            this._draggers.push(element_dragger);
        }

        this._ready = true;
        this.ShowOrHideROI();
        this.UpdateInsideOutside();
    }

    Destroy()
    {
        super.Destroy();
        OJLib.DestroyArray(this._draggers);
        this._corners.length = 0;
        this._show_hide_elements.length = 0;
        this._destroyed = true;
    }

    ShowOrHideROI()
    {
        let display = (this._roi_enabled && this._tmo_enabled) ? "block" : "none";
        for (let element of this._show_hide_elements)
            element.style.display = display;
    }

    UpdateInsideOutside()
    {
        if (!this._tmo_enabled)
        {
            this._window_polygon.setAttribute("fill", "#d8d8de");
        }
        else if (this._roi_outside || !this._roi_enabled)
        {
            this._window_polygon.setAttribute("fill", "#e8e8ee"); 
            this._roi_rectangle.setAttribute("fill", "#d8d8de");
        }
        else
        {
            this._roi_rectangle.setAttribute("fill", "#e8e8ee"); 
            this._window_polygon.setAttribute("fill", "#d8d8de");
        }
    }

    DragStart(params)
    {
        let event = params._event;
        let corner_index = params._user_data;

        this._button_down_position = this.GetHandlePixels(corner_index);

        if (corner_index >= 0)
        {
            this._corners[corner_index].setAttribute("stroke", this._down_color);
            this._corners[corner_index].setAttribute("fill", this._down_color);  
        }
    }

    Drag(params)
    {
        let dx = params._dx;
        let dy = params._dy;
        let event = params._event;
        let corner_index = params._user_data;
        let new_x = this._button_down_position._dx + dx;
        let new_y = this._button_down_position._dy + dy;

        let full_width = this._width - (2 * this._border);
        let full_height = this._height - (2 * this._border);
        let x = new_x - this._border;
        let y = new_y - this._border;
        let nx = x / full_width;
        let ny = y / full_height;
        let right = this._roi_x + this._roi_width;
        let bottom = this._roi_y + this._roi_height;

        if (nx < this._x_minimum)
            nx = this._x_minimum;
        if (nx > this._x_maximum)
            nx = this._x_maximum;
        if (ny < this._y_minimum)
            ny = this._y_minimum;
        if (ny > this._y_maximum)
            ny = this._y_maximum;

        let fixed_right = false;
        let fixed_bottom = false;
        
        if (corner_index == -1)
        {
            this._roi_x = nx;
            this._roi_y = ny;

            if ((this._roi_x + this._roi_width) > this._x_maximum)
                this._roi_x = this._x_maximum - this._roi_width;

            if ((this._roi_y + this._roi_height) > this._y_maximum)
                this._roi_y = this._y_maximum - this._roi_height;
        }
        if (corner_index == 0)
        {
            this._roi_x = nx;
            this._roi_y = ny;
            this._roi_width = right - this._roi_x;
            this._roi_height = bottom - this._roi_y;

            fixed_right = true;
            fixed_bottom = true;
        }
        else if (corner_index == 1)
        {
            this._roi_y = ny;
            this._roi_height = bottom - this._roi_y;
            this._roi_width = nx - this._roi_x;
            fixed_bottom = true;
        }
        else if (corner_index == 2)
        {
            this._roi_x = nx;
            this._roi_width = right - this._roi_x;
            this._roi_height = ny - this._roi_y;
            fixed_right = true;
        }
        else if (corner_index == 3)
        {
            this._roi_width = nx - this._roi_x;
            this._roi_height = ny - this._roi_y;
        }
        else if (corner_index == 4)
        {
            this._roi_y = ny;
            this._roi_height = bottom - this._roi_y;
            fixed_bottom = true;
        }
        else if (corner_index == 5)
        {
            this._roi_height = ny - this._roi_y;
        }
        else if (corner_index == 6)
        {
            this._roi_x = nx;
            this._roi_width = right - this._roi_x;
            fixed_right = true;
        }
        else if (corner_index == 7)
        {
            this._roi_width = nx - this._roi_x;
        }

        if (this._roi_width < this._minimum_width)
            this._roi_width = this._minimum_width;

        if (this._roi_width > this._maximum_width)
            this._roi_width = this._maximum_width;

        if (fixed_right)
            this._roi_x = right - this._roi_width;

        if (this._roi_height < this._minimum_height)
            this._roi_height = this._minimum_height;

        if (this._roi_height > this._maximum_height)
            this._roi_height = this._maximum_height;
            
        if (fixed_bottom)
            this._roi_y = bottom - this._roi_height;        

        for (let i = 0; i < this._corners.length; i++)
            this._corners[i].setAttribute("transform", this.GetTransform(i));    

        this.UpdateRoiRectangle();
        this.UpdateServer();
    }

    DragEnd(params)
    {
        this.Drag(params);

        let event = params._event;
        let corner_index = params._user_data;

        if (corner_index >= 0)
        {
            this._corners[corner_index].setAttribute("stroke", this._out_color);
            this._corners[corner_index].setAttribute("fill", this._out_color);  
        }
    }

    DragMouseOver(params)
    {
        let event = params._event;
        let corner_index = params._user_data;
        if (corner_index >= 0)
        {
            this._corners[corner_index].setAttribute("stroke", this._over_color);
            this._corners[corner_index].setAttribute("fill", this._over_color);  
        }
    }

    DragMouseOut(params)
    {
        let event = params._event;
        let corner_index = params._user_data;
        if (corner_index >= 0)
        {
            this._corners[corner_index].setAttribute("stroke", this._out_color);
            this._corners[corner_index].setAttribute("fill", this._out_color);  
        }
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

    GetTransform(index)
    {
        let pixels = this.GetHandlePixels(index);
        // Subtract 0.5 to centre the line in the middle of a pixel
        let transform = "translate(" + (pixels._dx - 0.5) + " " + (pixels._dy - 0.5) + ")";
        return transform;
    }

    GetHandlePixels(index)
    {
        let roi_x = 0;
        let roi_y = 0;
        let full_width = this._width - (2 * (this._border + 1));
        let full_height = this._height - (2 * (this._border + 1));
        
        if (index <= 0)
        {
            roi_x = this._roi_x;
            roi_y = this._roi_y;
        }
        else if (index == 1)
        {
            roi_x = this._roi_x + this._roi_width;
            roi_y = this._roi_y;
        }
        else if (index == 2)
        {
            roi_x = this._roi_x;
            roi_y = this._roi_y + this._roi_height;
        }
        else if (index == 3)
        {
            roi_x = this._roi_x + this._roi_width;
            roi_y = this._roi_y + this._roi_height;
        }
        else if (index == 4)
        {
            roi_x = this._roi_x + (this._roi_width / 2);
            roi_y = this._roi_y;
        }
        else if (index == 5)
        {
            roi_x = this._roi_x + (this._roi_width / 2);
            roi_y = this._roi_y + this._roi_height;
        }
        else if (index == 6)
        {
            roi_x = this._roi_x;
            roi_y = this._roi_y + (this._roi_height / 2);
        }
        else if (index == 7)
        {
            roi_x = this._roi_x + this._roi_width;
            roi_y = this._roi_y + (this._roi_height / 2);
        }

        let dx = (roi_x * full_width + this._border + 1 + 0.5) | 0;
        let dy = (roi_y * full_height + this._border + 1 + 0.5) | 0;
        return { _dx: dx, _dy: dy };
    }

    GetHeight()
    {
        return this._control_height;
    }

    GetControlHeight(parent_width, parent_height)
    { 
        if (parent_width == null)
            return 0;

        // Style border adds a pixels either side
        let outer_border = 13;
        let inner_border = 10;
        let control_width = parent_width - (2 * outer_border) - (2 * inner_border);
        let control_height = (control_width * 9) / 16 + (2 * inner_border);
        this._control_height = (control_height + 0.5) | 0;
        return this._control_height;
    }

    Resize(x, y, width, height)
    {
        let size_changed = super.Resize(x, y, width, height);

        // 1 Pixel border on either side
        this._svg.setAttribute("width", width - 2);
        this._svg.setAttribute("height", height - 2);  
        
        for (let i = 0; i < this._corners.length; i++)
            this._corners[i].setAttribute("transform", this.GetTransform(i));    

        let border = this._border + 1;
        let window_points = this.MakePoints([[border, border],
                                            [width - border * 2, 0], 
                                            [0, height - border * 2], 
                                            [-width + border * 2, 0]]);
        this._window_polygon.setAttribute("points", window_points);

        this.UpdateRoiRectangle();

        return size_changed;
    }

    UpdateControls(x, y, width, height)
    {
        for (let i = 0; i < this._corners.length; i++)
            this._corners[i].setAttribute("transform", this.GetTransform(i));    

        this.UpdateRoiRectangle();
    }

    UpdateRoiRectangle()
    {
        let top_left = this.GetHandlePixels(0);
        let top_right = this.GetHandlePixels(1);
        let bottom_left = this.GetHandlePixels(2);
        let bottom_right = this.GetHandlePixels(3);

        let roi_points = top_left._dx.toString() + "," + top_left._dy.toString() + " ";
        roi_points += top_right._dx.toString() + "," + top_right._dy.toString() + " ";
        roi_points += bottom_right._dx.toString() + "," + bottom_right._dy.toString() + " ";
        roi_points += bottom_left._dx.toString() + "," + bottom_left._dy.toString();
        this._roi_rectangle.setAttribute("points", roi_points);
    }

    SetValueFromJSON(params)
    {
        let roi_enabled_changed = (this._roi_enabled != params.enabled);
        let outside_changed = (this._roi_outside != params.outside);
        let tmo_enabled_changed = (this._tmo_enabled != params._tmo_enabled);

        this._roi_x = params.x;
        this._roi_y = params.y;
        this._roi_width = params.width;
        this._roi_height = params.height;
        this._roi_enabled = params.enabled;
        this._roi_outside = params.outside;
        this._tmo_enabled = params.tmo_enabled;

        if (this._ready)
        {
            if (roi_enabled_changed || tmo_enabled_changed)
                this.ShowOrHideROI();

            if (outside_changed || roi_enabled_changed || tmo_enabled_changed)
                this.UpdateInsideOutside();
        }
    }

    // UpdateValue called by the framework when an update is received
    // from the server
    UpdateValue(value)
    {
        this.SetValueFromJSON(value);
        this.UpdateControls();
    }

    UpdateServer()
    {
        var value_string = this._roi_x + "," + this._roi_y + "," + this._roi_width + "," + this._roi_height;
        this._value_changed_cb.Call(value_string);
    }

    static CreateCustomControl(element_type, params)
    {
        if (element_type == "OJRoiControl")
        {
            return new OJRoiControl(params);
        }
    }
}

// Register our control factory fn for the OJRoiControl custom controls
OJControlContainer.AddCustomControlFactory("OJRoiControl", OJRoiControl.CreateCustomControl);

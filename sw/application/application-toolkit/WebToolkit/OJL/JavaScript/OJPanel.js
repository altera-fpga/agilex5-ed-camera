/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJWindowElement } from "./OJL.js";
import { OJLib } from "./OJL.js";

let _next_panel_index = 1;

export class OJPanel extends OJWindowElement
{
    constructor(background_colour, parent_element)
    {
        // Base class constructor
        super();
        this._class_name = "OJPanel";
        this.SetElementName("OJPanel");

        // Set the class of the client area
        this._client_area.className = "panel_class";
        this._client_area.id = "OJPanel_" + _next_panel_index++;
        this._client_area.style.backgroundColor = background_colour;

        if (parent_element != null)
            parent_element.appendChild(this._client_area);

        this._button_callback = null;
        this._mouse_down = false;
        this._mouse_down_x = 0;
        this._mouse_down_y = 0;
        this._start_x = 0;
        this._start_y = 0;
        this._x = 0;
        this._y = 0;
    }

    Destroy()
    {
        super.Destroy();

        if (this._button_callback != null)
            this._button_callback.Destroy();
    }

    MakeDraggable()
    {
        this._button_callback = OJLib.RegisterButton(this, this._client_area);
    }

    Resize(x, y, width, height)
    {
        // Call base
        let size_changed = super.Resize(x, y, width, height);
        return size_changed;
    }

    OnLButtonDown(event)
    {
        //OJLib.Trace("OJPanel L Button Down");
        this._mouse_down = true;
        this._mouse_down_x = event._client_x;
        this._mouse_down_y = event._client_y;

        this._start_x = this._x;
        this._start_y = this._y;
    }

    OnLButtonUp(event)
    {
        //OJLib.Trace("OJPanel L Button Up");
        this._mouse_down = false;
    }

    OnMouseMove(event)
    {
        if (this._mouse_down)
        {
            //CancelTouchTimeout();

            let dx = event._client_x - this._mouse_down_x;
            this._x = this._start_x + dx;

            let dy = event._client_y - this._mouse_down_y;
            this._y = this._start_y + dy;

            this._client_area.style.top = this._y + "px";
            this._client_area.style.left = this._x + "px";

            //OJLib.Trace(this._client_area.style.left);
        }
    }
}


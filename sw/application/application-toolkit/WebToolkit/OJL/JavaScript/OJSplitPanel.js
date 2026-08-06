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

// If horizontal is true, this creates a left/right pair with vertical separator
// If horizontal is false, this creates a top/bottom pair with a horizontal separator

OJSplitPanel.prototype = new OJWindowElement();
function OJSplitPanel(first_window, second_window, horizontal, label, opts, header_opts)
{
    // Base class constructor
    OJWindowElement.call(this, null);
    this._class_name = "OJSplitPanel";
    this.SetElementName("OJSplitPanel");

    // Set the class of the client area
    this._client_area.className = "stack_class";

    this._spacing = 8;
    this._separator_position = 50;
    this._horizontal = horizontal;

    // Add the children
    if ((first_window != null) && (second_window != null))
    {
        this.AddChildToDOM(first_window);
        this._children.push(first_window);
        first_window._parent_window_element = this;

        this.AddChildToDOM(second_window);
        this._children.push(second_window);
        second_window._parent_window_element = this;
    }

     this._separator_button = document.createElement("div");
     this._separator_button.className = this._horizontal ? "horizontal_separator_class" : "vertical_separator_class";
     this._button_callbacks = OJLib.RegisterButton(this, this._separator_button);
     this._client_area.appendChild(this._separator_button);

    this._client_area.style.borderWidth = "0px";
    this._client_area.style.borderStyle = "none";
    
    if (label != null)
        this.UseWindowHeader(label, header_opts);	

    this._margin = 0;
}

OJSplitPanel.prototype.Destroy = function ()
{
    OJWindowElement.prototype.Destroy.call(this);

    if (this._button_callbacks != null)
        this._button_callbacks.Destroy();
};

OJSplitPanel.prototype.DetachChildren = function ()
{
    for (var i = 0; i < this._children.length; i++)
    {
        var child = this._children[i];
        child.RemoveFromDOM();
        child._parent_window_element = null;
    }

    this._children.length = 0;
};

OJSplitPanel.prototype.AttachChildren = function (first_window, second_window, position_children)
{
    this.DetachChildren();

    this.AddChildToDOM(first_window);
    this._children.push(first_window);
    first_window._parent_window_element = this;

    this.AddChildToDOM(second_window);
    this._children.push(second_window);
    second_window._parent_window_element = this;

    if (position_children)
        this.PositionChildren();
}


OJSplitPanel.prototype.Resize = function(x, y, width, height)
{
    // Call base
    var size_changed = OJWindowElement.prototype.Resize.call(this, x, y, width, height);
    this.PositionChildren();
    return size_changed;
};

OJSplitPanel.prototype.SetSplitPosition = function (position)
{
    this._separator_position = position;
    this.PositionChildren();
};

OJSplitPanel.prototype.GetChild = function (index)
{
    return this._children[index];
};

OJSplitPanel.prototype.PositionChildren = function ()
{
    if ((this._width == 0) || (this._height == 0) || (this._children.length == 0))
        return;
    
    var gap = 4;
    if ((this._separator_position == 0) || (this._separator_position == 100))
        gap = 0;

    if (this._horizontal)
    {
        var split_x = ((this._width * this._separator_position) / 100) | 0;
        this._children[0].Resize(0, 0, split_x - gap, this._height);
        
        OJLib.SetElementPosition(this._separator_button, split_x - gap, 0, 8, this._height);

        this._children[1].Resize(split_x + gap, 0, (this._width - split_x) - gap, this._height);
    }
    else
    {
        var split_y = ((this._height * this._separator_position) / 100) | 0;
        this._children[0].Resize(0, 0, this._width, split_y - gap);

        OJLib.SetElementPosition(this._separator_button, 0, split_y - gap, this._width, 8);

        this._children[1].Resize(0, split_y + gap, this._width, (this._height - split_y) - gap);
    }
};

OJSplitPanel.prototype.OnLButtonDown = function (event)
{
    OJLib.CaptureMouse(this._separator_button);
};

OJSplitPanel.prototype.OnLButtonUp = function (event)
{
    OJLib.ReleaseMouse(event);
};

OJSplitPanel.prototype.OnMouseMove = function (event)
{
    if (event._lbutton_down)
    {
        var y = event._offset_y;
        OJLib.Trace("Y = " + y);


    }
};

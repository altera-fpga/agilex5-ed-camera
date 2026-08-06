/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJStack } from "./OJL.js";
import { OJServerLink } from "./OJL.js";
import { OJTextButton } from "./OJL.js";
import { OJLib, UI, ObjectCallback } from "./OJL.js";

var all_context_menus = [];

class OJContextMenu extends OJStack
{
    constructor()
    {
        // Base class constructor
        super(null, { _have_border: false, _spacing: 2 });
        this._client_area.style.backgroundColor = "#000000";
        this._class_name = "OJContextMenu";

        this._is_shown = false;
        this._cancel_check_callback = new ObjectCallback(this, "OnCancelCheck");
        this._context_menu_commands = [];

        all_context_menus.push(this);
    }

    Destroy()
    {
        this.CloseIt();

        var i = all_context_menus.indexOf(this);
        all_context_menus.splice(i, 1);

        super.Destroy();
    }

    OnCancelCheck(event)
    {
        var x = event._client_x;
        var y = event._client_y;

        var stack_rectangle = UI.GetBoundingClientRect(this.GetElement());
        var cancel = false;

        if ((x < stack_rectangle.left) || (x > stack_rectangle.right))
            cancel = true;

        if ((y < stack_rectangle.top) || (y > stack_rectangle.bottom))
            cancel = true;

        if (cancel)
            this.CloseIt();
    }

    OpenIt(x, y)
    {
        if (this._is_shown)
            return;

        this._is_shown = true;

        var stack_element = this.GetElement();
        var rectangle = UI.GetBoundingClientRect(this.GetElement());
        stack_element.style.zIndex = 999999;
        stack_element.style.position = "absolute";
        var body = document.getElementById("_body");

        var height = this.GetHeight();
        var width = 200;

        for (var i = 0; i < this._children.length; i++)
        {
            if (this._children[i].MeasureWidth != null)
            {
                var child_width = this._children[i].MeasureWidth() + 20;
                if (child_width > width)
                    width = child_width;
            }
        }

        if ((y + height) > UI._screen_height)
            y = UI._screen_height - (height + 10);
        if ((x + width) > UI._screen_width)
            x = UI._screen_width - (width + 10);

        this.Resize(x, y, width, height);
        body.appendChild(stack_element);

        OJLib.RegisterForLButtonDown(this._cancel_check_callback);
        OJLib.RegisterForRButtonDown(this._cancel_check_callback);
    }

    CloseIt()
    {
        if (!this._is_shown)
            return;

        this._is_shown = false;

        // Remove from the DOM
        var stack_element = this.GetElement();
        stack_element.style.zIndex = 0;

        // All buttons must unregister for events 
        for (var i = 0; i < this._children.length; i++)
        {
            var child = this._children[i];
            child.UnregisterForEvents();
        }

        UI.RemoveFromParentElement(stack_element);

        OJLib.UnregisterForLButtonDown(this._cancel_check_callback);
        OJLib.UnregisterForRButtonDown(this._cancel_check_callback);
    }

    GetKeyColour(index)
    {
        var hue = index / 8;
        while (hue > 1)
            hue -= 1;

        var rgb = OJLib.HSL_to_RGB(hue, 0.2, 0.4);
        var str = "rgb(" + (rgb.R | 0) + "," + (rgb.G | 0) + "," + (rgb.B | 0) + ")";
        return str;
    }

    OnItemSelected(event)
    {
        var index = event._user_data;
        //OJLib.Trace("OJContextMenu OnItemSelected " + index);
        this.CloseIt();

        var command = this._context_menu_commands[index];
        OJServerLink.Get().ExecuteServerCommand(command);
    }

    AddItem(item_text, server_command)
    {
        var index = this.GetCount();
        var colour = this.GetKeyColour(index);

        this.AddChild(new OJTextButton(null, item_text,
            {
                _click_object: this,
                _click_callback: "OnItemSelected",
                _context_callback: "OnItemSelected",
                _user_data: index,
                _height: 40,
                _colour_key: colour,
                _is_popup: true
            }));

        this._context_menu_commands.push(server_command);
    }

    AddLocalItem(item_text, opts)
    {
        var index = this.GetCount();
        var colour = this.GetKeyColour(index);

        this.AddChild(new OJTextButton(null, item_text,
            {
                _click_object: opts._click_object,
                _click_callback: opts._click_callback,
                _context_callback: opts._click_callback,
                _user_data: index,
                _height: 40,
                _colour_key: colour,
                _is_popup: true
            }));

        this._context_menu_commands.push("");
    }
}


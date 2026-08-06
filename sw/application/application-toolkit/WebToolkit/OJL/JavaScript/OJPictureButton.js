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
import { OJServerLink } from "./OJL.js";
import { OJLib, UI } from "./OJL.js";
import { OJToolTip } from "./OJL.js";
import { LED_MODE_TYPE } from "./OJL.js";

// Either call with 
// new OJPictureButton(parent_element, url, url_over, { _command: "Command" }
// or
// new OJPictureButton(parent_element, url, url_over, { _object: click_object, _object_callback: click_callback }
export class OJPictureButton extends OJWindowElement
{
    constructor(parent_element, url, url_over, details, size)
    {
        // Base class constructor
        super(parent_element);
        this._class_name = "OJPictureButton";
        this.SetElementName("OJPictureButton");

        this._click_object = null;
        this._click_callback = null;
        this._click_command = null;
        this._over = false;
        this._user_data = null;
        this._up_background = null;
        this._press_and_hold_callback = null;
        this._wheel_callback = null;
        this._auto_repeat = false;
        this._repeating = false;
        this._auto_repeat_timeout = 0;
        this._no_mouse_over = false;
        this._lbutton_was_down = false;

        // This really needs to have a command interface with OJTextButton etc
        if (details != null)
        {
            if (details._command != null)
            {
                this._click_command = details._command;
            }
            else
            {
                this._click_object = details._object;
                
                if (details._object_callback != null)
                    this._click_callback = details._object_callback;
                
                if (details._down_callback != null)
                    this._down_callback = details._down_callback;
                
                if (details._up_callback != null)
                    this._up_callback = details._up_callback;

                if (details._press_and_hold_callback != null)
                    this._press_and_hold_callback = details._press_and_hold_callback
            }
            this._user_data = details._user_data;

            if (details._auto_repeat != null)
                this._auto_repeat = details._auto_repeat;

            if (details._no_mouse_over != null)
                this._no_mouse_over = details._no_mouse_over;
        }

        this._image = document.createElement("img");
        this._image._picture_button = this;
        this._image.className = "picture_button_class";
        this._image.ondragstart = function()
        {
            return false;
        };

        this._image.onload = function()
        {
            var width = this.width;
            var height = this.height;
            //this._picture_button.Resize(0, 0, width, height);
            this.style.width = width + "px";
            this.style.height = height + "px";
        };

        this._image.onblur = function()
        {
            if (this._picture_button != null)
                this._picture_button.OnBlur();
        };

        this._client_area.onblur = function()
        {
            if (this._window_element != null)
                this._window_element.OnBlur();
        };

        this._image.src = url;
        this._client_area.appendChild(this._image);

        this._up_url = url;
        this._over_url = url_over;

        this._over_brightness = 1.0;
        this._out_brightness = 1.0;

        if ((url_over == null) && !this._no_mouse_over)
        {
            this._over_brightness = 0.9;
            this._out_brightness = 1.0;
        }

        this.SetImageBrightness(this._out_brightness);

        this._button_callbacks = OJLib.RegisterButton(this, this._image);

        if (size != null)
        {
            // Used in OJGrid
            this._image.width = size._width;
            this._image.height = size._height;
            this._image.style.width = size._width + "px";
            this._image.style.height = size._height + "px";
        }
    }

    Destroy()
    {
        this._button_callbacks.Destroy();
        this._click_object = null;
        this._click_callback = null;
        this._image._picture_button = null;
        UI.RemoveFromParentElement(this._image);
        this._image = null;
        this._wheel_callback = null;
        super.Destroy();
        this._button_callbacks = null;
    }

    SetImageBrightness(brightness)
    {
        var brightness_str = "";
        if (brightness == this._out_brightness)
            brightness_str = "brightness(" + brightness + ")";
        else
            brightness_str = "drop-shadow(1px 1px 1px white)";
        this._image.style.filter = brightness_str;
    }

    UnregisterForEvents()
    {
        this._button_callbacks.UnregisterForEvents();
        super.UnregisterForEvents();
    }

    OnBlur()
    {
        this._repeating = false;
    }

    SwapImage(url, over_url)
    {
        this._up_url = url;
        this._over_url = over_url;

        var use_url = this._up_url;
        
        if (this._over && (this._over_url != null))
            use_url = this._over_url;

        this._image.src = use_url;
    }

    ManualPositioning()
    {
        this._client_area.className = "";
        this._client_area.style.position = "absolute";
        this._client_area.style.overflow = "visible";
    }

    OnClick(event)
    {
        if (this._tool_tip != null)
            OJToolTip.End(this._tool_tip, this._image);

            if (this._click_command != null)
        {
            // Issue the command directly
            OJServerLink.Get().ExecuteServerCommand(this._click_command);
            if (event.preventDefault != null)
                event.preventDefault();
        }
        else if (this._click_object != null)
        {
            if (this._click_callback != null)
            {
                event._user_data = this._user_data;
                this._click_object[this._click_callback](event);
                if (event.preventDefault != null)
                    event.preventDefault();
            }
        }

        return true; // Don't propagate
    }

    AutoRepeat(event)
    {
        this.OnLButtonDown(event);

        clearTimeout(this._auto_repeat_timeout);

        if (this._repeating)
            this._auto_repeat_timeout = setTimeout(PictureButtonAutoRepeat, 150, this, event);
    }

    OnContextMenu(event)
    {
        if (!this._enabled)
            return;

        if (this._press_and_hold_callback != null)
        {
            event._user_data = this._user_data;
            this._click_object[this._press_and_hold_callback](event);
        }

        if (this._auto_repeat)
        {
            this._repeating = true;
            this._auto_repeat_timeout = setTimeout(PictureButtonAutoRepeat, 150, this, event);
        }
    }

    OnLButtonDown(event)
    {
        if (!this._enabled)
            return;

        if ((this._click_object != null) && (this._down_callback != null))
        {
            event._user_data = this._user_data;
            this._click_object[this._down_callback](event);

            if (event.preventDefault != null)
                event.preventDefault();
        }

        this._lbutton_was_down = true;
    }

    OnLButtonUp(event)
    {
        this._lbutton_was_down = false;

        if (this._repeating)
        {
            this._repeating = false;
            clearTimeout(this._auto_repeat_timeout);
        }

        if (!this._enabled)
            return;

        if ((this._click_object != null) && (this._up_callback != null))
        {
            event._user_data = this._user_data;
            this._click_object[this._up_callback](event);

            if (event.preventDefault != null)
                event.preventDefault();
        }
    }

    OnMouseOver(event)
    {
        if (!this._enabled)
            return;

        if (this._tool_tip != null)
            OJToolTip.Start(this._tool_tip, this._image);

        this._over = true;

        if ((this._over_url != null) && (this._image != null))
            this._image.src = this._over_url;
        else
        {
            this.SetImageBrightness(this._over_brightness);
        }
    }

    OnMouseOut(event)
    {
        if (this._tool_tip != null)
            OJToolTip.End(this._tool_tip, this._image);

        if (!this._enabled)
            return;

        // Force button up when moving mouse out
        // This should cancel any ongoing autorepeat timers

        if (this._lbutton_was_down)
            this.OnLButtonUp(event);
        
        this._over = false;

        if ((this._over_url != null) && (this._image != null))
            this._image.src = this._up_url;
        else
        {
            this.SetImageBrightness(this._out_brightness);
        }
    }

    Resize(x, y, width, height)
    {
        var size_changed = super.Resize(x, y, width, height);

        var image_x = (width - this._image.width) / 2;
        var image_y = (height - this._image.height) / 2;

        OJLib.SetElementLocation(this._image, image_x, image_y);

        return size_changed;
    }

    OnMouseWheel(event)
    {
        var the_event = window.event || event;
        var delta = Math.max(-1, Math.min(1, (the_event.wheelDelta || -the_event.detail)));
        the_event.preventDefault();
        if (this._wheel_callback != null)
            this._wheel_callback.Call(delta);
    }

    Enable(state)
    {
        var changed = super.Enable(state);
        if (changed)
        {
            var use_brightness = state ? this._out_brightness : 0.3;

            this.SetImageBrightness(use_brightness);

            if (!this._enabled)
            {
                // Use up background
                if ((this._over_url != null) && (this._image != null))
                    this._image.src = this._up_url;
                else
                {
                    UI.SetStyleAttribute(this._client_area.style, "backgroundColor", this._up_background);
                }
            }
        }

        return changed;
    }
}

export class OJOnOffButton extends OJWindowElement
{
    constructor(state, item_changed_callback, ledMode = LED_MODE_TYPE.NO_LED)
    {
        super();
        this._class_name = "OJOnOffButton";
        this.SetElementName("OJOnOffButton");

        this._item_changed_callback = item_changed_callback;
        this._on = state;

        this._client_area.className = "on_off_button_container_class";

        this._image = document.createElement("img");
        this._image.className = "on_off_image_class";
        this._ledMode = ledMode

        this.SelectImage()

        this._image.ondragstart = function()
        {
            return false;
        };
        this._client_area.appendChild(this._image);

        this._image_button_callbacks = OJLib.RegisterButton(this, this._image);
        this._image_button_callbacks._call_click_on_mouse_down = true;
    }

    Destroy()
    {
        if (this._image_button_callbacks)
            this._image_button_callbacks.Destroy();
        this._image_button_callbacks = null;
        this._item_changed_callback = null;
        super.Destroy();
    }

    UnregisterForEvents()
    {
        this._image_button_callbacks.UnregisterForEvents();
        super.UnregisterForEvents();
    }

    SelectImage()
    {
        switch(this._ledMode) {
            case LED_MODE_TYPE.GREEN_LED:
                this._image.src = this._on ? OJLib._green_led._src : OJLib._grey_led._src;
                break;
            case LED_MODE_TYPE.RED_LED:
                this._image.src = this._on ? OJLib._red_led._src : OJLib._grey_led._src;
                break;
            case LED_MODE_TYPE.BLUE_LED:
                this._image.src = this._on ? OJLib._blue_led._src : OJLib._grey_led._src;
                break;
            default:
                if (OJServerLink.Get()._mono_ui)
                    this._image.src = this._on ? OJLib._on_image_mono._src : OJLib._off_image_mono._src;
                else
                    this._image.src = this._on ? OJLib._on_image._src : OJLib._off_image._src;
                break;
        }
    }

    Enable(state)
    {
        var changed = super.Enable(state);
        if (changed)
            this._image.style.opacity = state ? 1.0 : 0.4;

        return changed;
    }

    OnClick(event)
    {
        if (!this.IsEnabled())
            return;

        if (this._read_only)
        {
            event.preventDefault();
            WarningMessage("Read only value");
            return;
        }

        // Call callback if we have one
        if (this._item_changed_callback != null)
            this._item_changed_callback.Call(!this._on);
        else
            this.SetValue(this._on ? "false" : "true");

    }

    SetValue(value)
    {
        this.SetBooleanValue(value);
    }

    SetBooleanValue(value)
    {
        this._on = value;

        this.SelectImage()
    }

    GetValue()
    {
        return this._on;
    }

    Resize(x, y, width, height)
    {
        var size_changed = super.Resize(x, y, width, height);

        var image_width = 38; //this._image.width;
        var image_height = 38;
        OJLib.SetElementPosition(this._image, 0, (height - image_height) / 2, image_width, image_height);

        return size_changed;
    }

    HasFocus()
    {
        return (this._image === document.activeElement);
    }
}

function PictureButtonAutoRepeat(picture_button, event)
{
    picture_button.AutoRepeat(event);
}

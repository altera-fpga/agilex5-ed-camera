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

export let TEXT_CONTROL_TYPE =
{
    TCT_TEXT: 0,
    TCT_FLOATING_POINT: 1,
    TCT_INTEGER: 2,
    TCT_TIMECODE: 3,
    TCT_HEX_INTEGER: 4
};

export class OJTextControl extends OJWindowElement
{
    constructor(control_type, initial_value, changed_callback,
                minimum, maximum, increment)
    {
        // Base class constructor
        super();
        this._class_name = "OJTextControl";
        this.SetElementName("OJTextControl");

        this._control_type = control_type;
        this._changed_callback = changed_callback;
        this._apply_after_timeout = false;
        this._minimum = (minimum == null) ? 0 : minimum;
        this._maximum = (maximum == null) ? 0 : maximum;
        this._auto_set = false;
        this._input_control = document.createElement("input");
        this._input_control.type = "text";
        this._input_control.className = "input_control_class";
        this._input_control._text_control = this;
        this._input_control._window_element = this;
        this._units = "";
        this._mouse_event_listener = function(event)
        {
            if (this._text_control != null)
                this._text_control.MyMouseWheel(event);
        };

        this._button_callbacks = OJLib.RegisterButton(this, this._input_control, "open_keypad");

        this._input_control.onfocus = function(event)
        {
            event.preventDefault();

            if (this._text_control != null)
                this._text_control.OnFocus(this, event);
        };
        this._input_control.onblur = function()
        {
            if (this._text_control != null)
                this._text_control.OnBlur();
        };

        this._value = "";

        if (initial_value != null)
        {
            this._value = this.Format(initial_value);
            this.UpdateInputControlValue();
            this._input_control.scrollLeft = this._input_control.scrollWidth;
        }

        this._input_control.onkeypress = function(event)
        {
            if (this._text_control != null)
                this._text_control.OnKeyPress(event);
        };

        this._input_control.oninput = function(event)
        {
            if (this._text_control != null)
                this._text_control.OnInput(event);
        };

        this._width = 200;
        this._height = 24;
        this._increment = 1;
        this._value_to_display_fn = null;
        this._display_to_value_fn = null;
        this._value_to_display_context = null;
        this._show_decimal_places = null;

        if (this._control_type == TEXT_CONTROL_TYPE.TCT_FLOATING_POINT)
            this._increment = 0.05;

        if (increment != null)
            this._increment = increment;

        this._client_area.appendChild(this._input_control);
    }

    Destroy()
    {
        this._button_callbacks.Destroy();
        this._input_control.onkeypress = null;
        this._input_control.oninput = null;
        this._input_control._text_control = null;
        this._input_control._window_element = null;
        UI.RemoveFromParentElement(this._input_control);
        this.UnregisterMouseWheel(this._input_control);
        super.Destroy();
    }

    // Call conversion function named by the server with the 
    // internal value and a context supplied by the server.
    // You must have the given function in your JavaScript
    ValueToDisplay(value)
    {
        if (this._value_to_display_fn == null)
            return value;
        else
            return window[this._value_to_display_fn](value, this._value_to_display_context);
    }

    // Call conversion function named by the server with the 
    // internal value and a context supplied by the server.
    // You must have the given function in your JavaScript
    DisplayToValue(display)
    {
        if (this._display_to_value_fn == null)
            return display;
        else
            return "" + window[this._display_to_value_fn](display, this._value_to_display_context);
    }

    SetDecimalPlaces(decimal_places)
    {
        this._show_decimal_places = decimal_places;
        this._value = this.Format(this._value);
        this.UpdateInputControlValue();
    }

    UpdateInputControlValue()
    {
        let is_focussed = (this._input_control === document.activeElement);
        if (is_focussed || (this._units == ""))
        {
            if (this._control_type == TEXT_CONTROL_TYPE.TCT_TIMECODE)
                this._input_control.value = this.ValueToDisplay(this._value);
            else if (this._control_type == TEXT_CONTROL_TYPE.TCT_HEX_INTEGER)
                this._input_control.value = parseInt(this._value, 16).toString(16).toUpperCase();
            else
                this._input_control.value = this.ValueToDisplay(this._value);
        }
        else
        {
            if (this._control_type == TEXT_CONTROL_TYPE.TCT_TIMECODE)
            {
                let use_units = this.GetUnitString(this._value);
                this._input_control.value = this.ValueToDisplay(this._value) + " " + use_units;
            }
            else if (this._control_type == TEXT_CONTROL_TYPE.TCT_HEX_INTEGER)
                this._input_control.value = parseInt(this._value, 16).toString(16).toUpperCase() + " " + this._units;
            else
            {
                let use_units = this.GetUnitString(this._value);
                this._input_control.value = this.ValueToDisplay(this._value) + " " + use_units;
            }
        }
    }

    GetUnitString(value)
    {
        if (value === "1")
        {
            if (this._units.substr(this._units.length - 1, 1) == "s")
                return this._units.substr(0, this._units.length - 1);
            else
                return this._units;
        }
        else
            return this._units;
    }

    SetFocus()
    {
        this._input_control.focus();
    }

    HasFocus()
    {
        return (this._input_control === document.activeElement);
    }

    SetUnits(units)
    {
        this._units = units;
        this.UpdateInputControlValue();
    }

    SetPasswordControl()
    {
        this._input_control.type = "password";
    }

    SetIncrement(increment)
    {
        this._increment = increment;
    }

    SetValueToDisplayFn(value_to_display_fn, display_to_value_fn, context)
    {
        this._value_to_display_context = context;

        if ((value_to_display_fn != null) && (value_to_display_fn != ""))
        {
            this._value_to_display_fn = value_to_display_fn;
            this.UpdateInputControlValue();
        }

        if ((display_to_value_fn != null) && (display_to_value_fn != ""))
            this._display_to_value_fn = display_to_value_fn;
    }

    Enable(state)
    {
        let changed = super.Enable(state);

        if (changed)
        {
            this._input_control.disabled = !this._enabled || this._read_only;
            UI.SetStyleAttribute(this._input_control.style, "color", this._enabled ? null : UI._disabled_text_colour);

            this.UpdateSelectable();
        }

        return changed;
    }

    ReadOnly(state)
    {
        let changed = super.ReadOnly(state);
        if (changed)
        {
            this._input_control.readOnly = this._read_only;
            this._input_control.disabled = !this._enabled || this._read_only;
            this.UpdateSelectable();
        }

        return changed;
    }

    OnFocus(element, event)
    {
        // Register mouse wheel events
        element.addEventListener("mousewheel", this._mouse_event_listener);
        element.addEventListener("DOMMouseScroll", this._mouse_event_listener);

        this._has_focus = true;

        // Hide units, if any
        this.UpdateInputControlValue();
    }

    OnClick(event)
    {
        if (!this._input_control.readOnly)
        {
            if (event._is_touch_event == null)
                this._input_control.select();
        }

        // Popup the numeric keypad if touch event or there is no keyboard
        let show_keyboard = ((event._is_touch_event != null) && this.IsEnabled() && !this._read_only);

        if (show_keyboard)
            OJServerLink.Get().ShowKeypad(this);
    }

    UnregisterMouseWheel(element)
    {
        element.removeEventListener("mousewheel", this._mouse_event_listener);
        element.removeEventListener("DOMMouseScroll", this._mouse_event_listener);
    }

    AddToValue(delta)
    {
        if (this._read_only)
        {
            WarningMessage("Read only value");
            return;
        }

        let new_value = "";

        if (this._control_type == TEXT_CONTROL_TYPE.TCT_FLOATING_POINT)
        {
            if (this._value == "")
                this._value = "0";

            let float_value = parseFloat(this._value);
            if (delta > 0)
                float_value += this._increment;
            else
                float_value -= this._increment;

            // Snap to the increment
            float_value = Math.floor(float_value / this._increment + 0.5) * this._increment;

            new_value = float_value;
        }
        else if (this._control_type == TEXT_CONTROL_TYPE.TCT_INTEGER)
        {
            if (this._value == "")
                this._value = "0";

            let int_value = parseInt(this._value);

            if (delta > 0)
                int_value += this._increment;
            else
                int_value -= this._increment;

            new_value = this.ValidateInteger(int_value);
            }
        else if (this._control_type == TEXT_CONTROL_TYPE.TCT_HEX_INTEGER)
        {
            if (this._value == "")
                this._value = "0";

            let int_value = parseInt(this._value, 16);

            if (delta > 0)
                int_value += this._increment;
            else
                int_value -= this._increment;

            new_value = this.ValidateInteger(int_value);
        }
        else
            return;

        // Call callback if we have one
        if (this._changed_callback != null)
        {
            this._changed_callback.Call(new_value);
        }

        let auto_set = ((this._disable_set_when_focused != null) && this._disable_set_when_focused && this._has_focus);
        if (this._auto_set)
        {
            auto_set = true;
            new_value = parseFloat(new_value.toFixed(5));
        }

        if (auto_set)
        {
            // The callback from the server is filtered out, so set it 
            // directly in this case
            if ((new_value >= this._minimum) && (new_value <= this._maximum))
                this.SetValue(new_value);
        }
    }

    ValidateInteger(new_value)
    {
        if (new_value < this._minimum)
            new_value = this._minimum;
        else if (new_value > this._maximum)
            new_value = this._maximum;
        
        return new_value;
    }

    MyMouseWheel(event)
    {
        let the_event = window.event || event;
        let delta = Math.max(-1, Math.min(1, (the_event.wheelDelta || -the_event.detail)));
        the_event.preventDefault();

        this.AddToValue(delta);
    }

    UpdateValue(new_value, from_onscreen_keyboard)
    {
        if (this._changed_callback != null)
            this._changed_callback._object[this._changed_callback._method_name](new_value, from_onscreen_keyboard);
        else
            this.SetValue(new_value);
    }

    UpdateSelectable()
    {
        let selectable = this._enabled && !this._read_only;

        UI.SetStyleAttribute(this._input_control.style, "WebkitUserSelect", selectable ? null : "none" );
        UI.SetStyleAttribute(this._input_control.style, "MsUserSelect", selectable ? null : "none");
        UI.SetStyleAttribute(this._input_control.style, "userSelect", selectable ? null : "none");

        if (selectable)
            this._input_control.setAttribute("unselectable", "off");
        else
            this._input_control.setAttribute("unselectable", "on");
    }

    Format(value)
    {
        if (this._control_type == TEXT_CONTROL_TYPE.TCT_FLOATING_POINT)
        {
            let float_value = parseFloat(value);

            if (this._show_decimal_places != null)
                float_value = float_value.toFixed(this._show_decimal_places);

            let str = "" + float_value;
            return str;
        }
        else if (this._control_type == TEXT_CONTROL_TYPE.TCT_HEX_INTEGER)
        {
            return parseInt(value).toString(16).toUpperCase();
        }
        else if (this._control_type == TEXT_CONTROL_TYPE.TCT_TEXT)
        {
            //value = value.replace(/{/g, "<");
            //value = value.replace(/}/g, ">");
        }
        else if (this._control_type == TEXT_CONTROL_TYPE.TCT_INTEGER)
        {
            let int_value = Math.round(value);
            let str = "" + int_value;
            return str;
        }

        return value;
    }

    SetValue(value)
    {
        let new_value = this.Format(value); // if hex integer, convert to hex string
        if (this._control_type == TEXT_CONTROL_TYPE.TCT_TIMECODE)
        {
            while (new_value.length < 8)
            {
                new_value = "0" + new_value;
            }
        }

        let requires_conversion = (this._value_to_display_fn != null);

        if (requires_conversion || (new_value != this._value))
        {
            this._value = new_value;

            this.UpdateInputControlValue();
            this._input_control.scrollLeft = this._input_control.scrollWidth;

            if (this._control_type == TEXT_CONTROL_TYPE.TCT_TEXT)
                this._input_control.blur();
        }
    }

    SetBackgroundColour(colour)
    {
        UI.SetBackgroundColour(this._input_control, colour);
    }

    SetFromKeyPad(new_value)
    {
        this._input_control.value = new_value;

        // Fake enter key
        this.OnBlur();
    }

    GetValue()
    {
        // Strip units
        if (!this._has_focus && (this._units != ""))
        {
            let value = this._input_control.value;
            let last_space = value.lastIndexOf(" ");
            let use_value = value;
            if (last_space >= 0)
                use_value = value.substr(0, last_space);

            return this.DisplayToValue(use_value);
        }
        else
        {
            return this.DisplayToValue(this._input_control.value);
        }
    }

    GetHeight()
    {
        if (!this._shown)
            return 0;

        return this._height;
    }

    GetWidth()
    {
        return this._width;
    }

    Resize(x, y, width, height)
    {
        // Call base
        let size_changed = super.Resize(x, y, width, height);

        this.AssignWidth(width);
        this.AssignHeight(height);

        OJLib.SetElementPosition(this._input_control, 0, 0, width - 6, height - 4);
        this._input_control.scrollLeft = this._input_control.scrollWidth;

        return size_changed;
    }

    SetControlType(control_type)
    {
        this._control_type = control_type;
    }

    OnKeyDown(event)
    {
        if ((this._control_type == TEXT_CONTROL_TYPE.TCT_FLOATING_POINT) ||
            (this._control_type == TEXT_CONTROL_TYPE.TCT_INTEGER) ||
            (this._control_type == TEXT_CONTROL_TYPE.TCT_TIMECODE) ||
            (this._control_type == TEXT_CONTROL_TYPE.TCT_HEX_INTEGER))
        {
            if (this._control_type != TEXT_CONTROL_TYPE.TCT_TIMECODE)
            {   // NOT timecode (makes no sense)
                if (UI.GetKeyCode(event) == 40)
                {
                    // Down
                    this.AddToValue(-1);
                    OJLib.MarkEventAsHandled(event);
                    return;
                }
                else if (UI.GetKeyCode(event) == 38)
                {
                    // Up
                    this.AddToValue(1);
                    OJLib.MarkEventAsHandled(event);
                    return;
                }
            }
        }
    }

    OnKeyPress(event)
    {
        if (this._read_only)
        {
            event.preventDefault();
            WarningMessage("Read only value");
            return;
        }

        if (!this.IsEnabled())
        {
            event.preventDefault();
            return;
        }

        let range_selected = ((this._input_control.selectionStart == 0) && (this._input_control.selectionEnd > this._input_control.selectionStart));
        let in_start_position = ((this._input_control.selectionStart == 0) && (this._input_control.selectionEnd == 0));
        let key_code = UI.GetKeyCode(event);

        if (event.ctrlKey)
        {
            // Allow select all, cut, paste
            if (key_code == "a".charCodeAt(0))
                return;
            if (key_code == "c".charCodeAt(0))
                return;
            if (key_code == "v".charCodeAt(0))
                return;
        }

        if (this._control_type == TEXT_CONTROL_TYPE.TCT_FLOATING_POINT)
        {
            let current_text = this.GetValue();
            let minus = (current_text.indexOf("-") == 0);
            let decimal = (current_text.indexOf(".") >= 0);

            let is_number = false;

            if ((key_code >= "0".charCodeAt(0)) && (key_code <= "9".charCodeAt(0)))
                is_number = true;

            if ((current_text == "") && (key_code == "-".charCodeAt(0)) && !minus)
                is_number = true;

            if (key_code == ".".charCodeAt(0) && !decimal)
                is_number = true;

            if (current_text == "" && key_code == "-".charCodeAt(0) && !minus && !decimal)
                is_number = true;

            if (key_code == "-".charCodeAt(0))
            {
                if (range_selected)
                    is_number = true;
                else if (!minus && in_start_position)
                    is_number = true;
            }

            if (key_code == ".".charCodeAt(0) && range_selected)
                is_number = true;

            if (!is_number)
                event.preventDefault();
        }
        else if ((this._control_type == TEXT_CONTROL_TYPE.TCT_INTEGER) || (this._control_type == TEXT_CONTROL_TYPE.TCT_TIMECODE) || (this._control_type == TEXT_CONTROL_TYPE.TCT_HEX_INTEGER))
        {
            let current_text = this.GetValue();
            if ((key_code != "\r".charCodeAt(0)) && (current_text.length > 8))
            {
                event.preventDefault();
                return;
            }
            let minus = (current_text.indexOf("-") == 0);

            let is_number = false;

            if ((key_code >= "0".charCodeAt(0)) && (key_code <= "9".charCodeAt(0)))
                is_number = true;
            if (this._control_type == TEXT_CONTROL_TYPE.TCT_INTEGER)
            {   // allow '-'
                if ((current_text == "") && (key_code == "-".charCodeAt(0)) && !minus && !decimal)
                    is_number = true;
                if (key_code == "-".charCodeAt(0))
                {
                    if (range_selected)
                        is_number = true;
                    else if (!minus && in_start_position)
                        is_number = true;
                }
            }
            else if (this._control_type == TEXT_CONTROL_TYPE.TCT_HEX_INTEGER)
            {
                if ((key_code >= "a".charCodeAt(0)) && (key_code <= "f".charCodeAt(0)))
                    is_number = true;
                else if ((key_code >= "A".charCodeAt(0)) && (key_code <= "F".charCodeAt(0)))
                    is_number = true;
            }
            if (!is_number)
                event.preventDefault();
        }
        else if (this._control_type == TEXT_CONTROL_TYPE.TCT_TEXT)
        {
            let is_allowed = true;

            if (key_code == "'".charCodeAt(0))
                is_allowed = false;
            else if (key_code == '"'.charCodeAt(0))
                is_allowed = false;
            else if (key_code == '£'.charCodeAt(0))
                is_allowed = false;
            else if (key_code == '<'.charCodeAt(0))
                is_allowed = false;
            else if (key_code == '>'.charCodeAt(0))
                is_allowed = false;
            else if (key_code == '/'.charCodeAt(0))
                is_allowed = false;
            else if (key_code == '\\'.charCodeAt(0))
                is_allowed = false;
            else if (key_code == '¬'.charCodeAt(0))
                is_allowed = false;
            else if (key_code == '¦'.charCodeAt(0))
                is_allowed = false;
            else if (key_code == '|'.charCodeAt(0))
                is_allowed = false;
            else if (key_code == '`'.charCodeAt(0))
                is_allowed = false;
            else if (key_code == '*'.charCodeAt(0))
                is_allowed = false;
            else if (key_code == ':'.charCodeAt(0))
                is_allowed = false;
            else if (key_code == ';'.charCodeAt(0))
                is_allowed = false;

            if (!is_allowed)
                event.preventDefault();
        }

        // Return pressed ?
        if (key_code == "\r".charCodeAt(0))
        {
            if (this._timeout != null)
                clearTimeout(this._timeout);

            this._input_control.blur();
        }
    }

    OnBlur()
    {
        this._value = this.GetValue();
        this._has_focus = false;

        if (this._read_only)
            return;

        if (!this.IsEnabled())
            return;

        if (this._changed_callback != null)
        {
            let value = this._value;
            if (this._control_type == TEXT_CONTROL_TYPE.TCT_HEX_INTEGER)
                value = parseInt(this._value, 16).toString().toUpperCase();
            else if (this._control_type == TEXT_CONTROL_TYPE.TCT_TIMECODE)
            {   // check limits for each field
                let intVal = parseInt(this._value);
                let Frames = value % 100;
                if (Frames >= 60)
                    Frames = 0; // legalise
                value = Math.floor(value / 100);
                let Seconds = value % 100;
                if (Seconds >= 60)
                    Seconds = 0; // legalise
                value = Math.floor(value / 100);
                let Minutes = value % 100;
                if (Minutes >= 60)
                    Minutes = 0; // legalise
                value = Math.floor(value / 100);
                let Hours = value % 100;
                if (Hours >= 24)
                    Hours = 0; // legalise
                intVal = Frames + (Seconds * 100) + (Minutes * 10000) + (Hours * 1000000);
                value = intVal.toString();  // put correct value back
            }

            this._changed_callback.Call(value);
        }
        this.UnregisterMouseWheel(this._input_control);

        // Show units, if any
        this.UpdateInputControlValue();
    }

    OnInput(event)
    {
        if (this._timeout != null)
            clearTimeout(this._timeout);

        if (this._apply_after_timeout)
            this._timeout = setTimeout(ApplyChangesTimeout, 3000, this);
    }

    SetToolTip(tip, show_time)
    {
        super.SetToolTip(tip, show_time);

        if (this._button_callbacks == null)
            this._button_callbacks = OJLib.RegisterButton(this, this._paragraph);
    }

    OnActualMouseOver(event)
    {

    }

    OnMouseOver(event)
    {
        // Show tool tip even if disabled
        if (this._tool_tip != null)
            OJToolTip.Start(this._tool_tip, this._input_control);
    }

    OnMouseOut(event)
    {
        this.EndToolTip();
    }

    EndToolTip()
    {
        if (this._tool_tip != null)
            OJToolTip.End(this._tool_tip, this._input_control);
    }
}

function ApplyChangesTimeout(text_control)
{
    text_control._timeout = null;
    text_control._input_control.blur();
}

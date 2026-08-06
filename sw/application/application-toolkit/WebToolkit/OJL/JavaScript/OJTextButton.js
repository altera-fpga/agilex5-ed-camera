/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJWindowElement, ANCHOR_TYPE } from "./OJL.js";
import { OJServerLink } from "./OJL.js";
import { OJPicon } from "./OJL.js";
import { OJLib, UI } from "./OJL.js";

// opts: _click_object, _click_callback, _user_data, _context_callback,
//       _colour_key, _double_click_callback
export class OJTextButton extends OJWindowElement
{
    constructor(parent_element, label, opts)
    {
        // Base class constructor
        super();
        this._class_name = "OJTextButton";
        this.SetElementName("OJTextButton");

        if (parent_element != null)
            parent_element.appendChild(this._client_area);

        this._click_object = opts._click_object;
        this._click_callback = opts._click_callback;
        this._double_click_callback = opts._double_click_callback;
        this._key_down_callback = opts._key_down_callback;
        this._context_callback = opts._context_callback;
        this._escape_html = true;
        this._line_height = 0; // Auto set if 0
        this._text_colour = null;
        this._is_over = false;
        this._shadow_border_size = 4;
        this._text_hover_colour = null;
        this._disabled_background_color = "#c0c0c0";

        if (opts._height != null)
            this._height = opts._height;

        if (opts._allow_html != null)
            this._escape_html = false;

        this._text_div = this._client_area;

        if (opts._no_background != null)
            this._text_div.className = "text_button_no_background_class";
        else
            this._text_div.className = "text_button_class";

        this._background_colour = null;
        this._clicked_background_colour = OJServerLink.Get()._clicked_colour;
        this._latched_background_colour = UI._selected_background_colour;

        // Default size is 120 x 35
        this.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 35 });
        this.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 120 });

        this._latched = false;

        if (opts._enabled != null)
        {
            let enabled = opts._enabled;
            this.Enable(enabled);
        }

        if (opts._line_height != null)
        {
            this._line_height = opts._line_height;
            this._text_div.style.lineHeight = opts._line_height;
        }

        this.SetLabel(label);

        if (opts._alignment != null)
        {
            if ((opts._alignment == "centre") || (opts._alignment == "center"))
                this._client_area.style.textAlign = "center";
            else
                this._client_area.style.textAlign = opts._alignment;
        }

        this._button_callbacks = OJLib.RegisterButton(this, this._text_div, opts._user_data);
        this._user_data = opts._user_data;

        this._have_colour_key = (opts._colour_key != null);
        if (this._have_colour_key)
        {
            //this._text_div.left = "8px";
            this._colour_key_label = document.createElement("div");
            this._colour_key_label.className = "colour_key_class_side";
            let colour_key = OJServerLink.Get()._mono_ui ? "#505050" : opts._colour_key;
            this._colour_key_label.style.backgroundColor = colour_key;
            this.GetElement().appendChild(this._colour_key_label);
            this._text_div.style.textIndent = "15px";
        }

        if (opts._is_popup != null && opts._is_popup)
        {
            this._text_div.className = "popup_text_button_class";
            this._shadow_border_size = 0;
        }

        this._attach_picon_to_right = false;
        if (opts._picon != null)
        {
            this._attach_picon_to_right = opts._attach_picons_to_right;
            this._picon = new OJPicon(this._client_area, false);
            this._picon.DecodePicon(opts._picon);
            this._picon_button_callbacks = OJLib.RegisterButton(this, this._picon.GetElement(), opts._user_data);
            if (this._attach_picon_to_right)
                this.AssignHeight(this._picon.GetHeight() + 4);
        }
        else
            this._picon = null;

        if (opts._propagate_events && opts._event_callbacks)
        {
            this._propagate_events = opts._propagate_events;
            this._event_callbacks = opts._event_callbacks;

            // Copy user_data to fire back with events
            this._event_callbacks._user_data = opts._user_data;
        }

        if (opts._image_url != null)
        {
            // We have an image rather than text
            this._image = document.createElement("img");
            this._image._text_button = this;
            this._image.className = "picture_button_class";
            this._image.ondragstart = function()
            {
                return false;
            };
            this._image.onload = function()
            {
                let width = this.width;
                let height = this.height;
                this.style.width = width + "px";
                this.style.height = height + "px";
            };
            this._image.src = opts._image_url;
            this._image_button_callbacks = OJLib.RegisterButton(this, this._image, opts._user_data);
            this._text_div.appendChild(this._image);
            this._image_attachment = opts._image_attachment;
        }
    }

    Destroy()
    {
        if (this._image != null)
            this._image._text_button = null;
        this._button_callbacks.Destroy();
        this._button_callbacks = null;

        if (this._image_button_callbacks != null)
        {
            this._image_button_callbacks.Destroy();
            this._image_button_callbacks = null;
        }

        if (this._picon_button_callbacks != null)
        {
            this._picon_button_callbacks.Destroy();
            this._picon_button_callbacks = null;
        }

        if (this._picon != null)
            this._picon.Destroy();
        this._picon = null;
        this._image = null;

        this._click_object = null;
        this._click_callback = null;
        this._double_click_callback = null;

        //UI.RemoveFromParentElement(this._text_div);
        //this._text_div = null;

        UI.RemoveFromParentElement(this._colour_key_label);
        this._colour_key_label = null;

        super.Destroy();
    }

    UnregisterForEvents()
    {
        // Make sure we're unregistered for events
        if (this._button_callbacks != null)
            this._button_callbacks.UnregisterForEvents();

        if (this._image_button_callbacks != null)
            this._image_button_callbacks.UnregisterForEvents();

        if (this._picon_button_callbacks != null)
            this._picon_button_callbacks.UnregisterForEvents();

        super.UnregisterForEvents();
    }

    RemoveFromDOM()
    {
        // Make sure we're unregistered for events
        this.UnregisterForEvents();
        super.RemoveFromDOM();
    }

    OnKeyDown(event)
    {
        if (this.IsEnabled() && (this._key_down_callback != null))
            this._click_object[this._key_down_callback](event);
    }

    // Use JS rather than CSS to do the hover colour
    // because we want to highlight our text when the
    // picon is hovered
    OnMouseOver(event)
    {
        this._is_over = true;

        // Show tool tip even if disabled
        if (this._tool_tip != null)
            OJToolTip.Start(this._tool_tip, this._text_div, this._tool_tip_show_time);

        if (this.IsEnabled())
        {
            UI.SetStyleAttribute(this._text_div.style, "backgroundColor", OJServerLink.Get()._mouse_over_colour);

            if (this._text_hover_colour != null)
                UI.SetStyleAttribute(this._text_div.style, "color", this._text_hover_colour);

            // Propagate event if requested by owner
            if (this._event_callbacks && this._event_callbacks._onmouseover)
            {
                let callback = this._event_callbacks._onmouseover;
                callback.Call(this._event_callbacks._user_data);
            }
        }
    }

    SetBackgroundColour(colour)
    {
        this._background_colour = colour;
        UI.SetStyleAttribute(this._text_div.style, "backgroundColor", this._background_colour);
    }

    SetColour(colour)
    {
        this._text_colour = colour;
        UI.SetStyleAttribute(this._text_div.style, "color", this._text_colour);
    }

    SetHoverColour(colour)
    {
        this._text_hover_colour = colour;
    }

    OnActualMouseOut(event)
    {
        this._is_over = false;
        if (event._lbutton_down)
            UI.SetStyleAttribute(this._text_div.style, "backgroundColor", this._background_colour);
    }
    OnActualMouseOver(event)
    {
        this._is_over = false;
        if (event._lbutton_down)
            UI.SetStyleAttribute(this._text_div.style, "backgroundColor", OJServerLink.Get()._altera_dark_blue);
    }
    OnMouseOut(event)
    {
        this._is_over = false;
        this._hightlight_on_next_enter = false;

        if (this._tool_tip != null)
            OJToolTip.End(this._tool_tip, this._text_div);

        if (this.IsEnabled())
        {
            if (this._latched)
                this._text_div.style.backgroundColor = this._latched_background_colour;
            else
            {
                if (event._lbutton_down)
                {
                    UI.SetStyleAttribute(this._text_div.style, "backgroundColor", OJServerLink.Get()._altera_dark_blue);
                }
                else
                {
                    // Required for IE
                    UI.SetStyleAttribute(this._text_div.style, "backgroundColor", this._background_colour);
                }
            }

            if (this._text_hover_colour != null)
                UI.SetStyleAttribute(this._text_div.style, "color", this._text_colour);
            
            // Propagate event if requested by owner
            if (this._event_callbacks && this._event_callbacks._onmouseout)
            {
                let callback = this._event_callbacks._onmouseout;
                callback._object[callback._method_name](this._event_callbacks._user_data);
            }
        }
    }

    SetLabel(label)
    {
        if (label == null)
            label = "";

        if (this._escape_html)
        {
            this._label = OJLib.EscapeHtml(label);
            this._text_div.innerHTML = this._label;
        }
        else
        {
            this._label = label;
            this._text_div.innerHTML = label;
        }
    }

    FitText()
    {
        let width = this.MeasureWidth();
        this.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: width + 10 });
    }

    MeasureWidth()
    {
        let temp_canvas = document.createElement("canvas");
        let dc = temp_canvas.getContext('2d');
        dc.font = UI._large_font_name;
        let width = dc.measureText(this._label).width;

        if (this._have_colour_key)
            width += 10;

        return (width + 0.5) | 0;
    }

    GetButtonElement()
    {
        return this._text_div;
    }

    Enable(state)
    {
        let changed = OJWindowElement.prototype.Enable.call(this, state);

        if (changed)
        {
            if (state)
            {
                UI.SetStyleAttribute(this._text_div.style, "color", this._text_colour);
                UI.SetStyleAttribute(this._text_div.style, "backgroundColor", this._background_colour);
            }
            else
            {
                this._text_div.style.color = UI._disabled_text_colour;
                UI.SetStyleAttribute(this._text_div.style, "backgroundColor", this._disabled_background_color);
            }
        }

        return changed;
    }

    Latch(state)
    {
        this._latched = state;

        if (state)
            this._text_div.style.backgroundColor = this._latched_background_colour;
        else
            UI.SetStyleAttribute(this._text_div.style, "backgroundColor", this._background_colour);
    }

    OnLButtonDown(event)
    {
        if (this.IsEnabled())
        {
            OJLib.CaptureMouse(this._text_div);
            UI.SetStyleAttribute(this._text_div.style, "backgroundColor", this._clicked_background_colour);
        }
    }

    OnLButtonUp(event)
    {
        if (this.IsEnabled())
        {
            OJLib.ReleaseMouse(event);

            if (this._is_over)
                UI.SetStyleAttribute(this._text_div.style, "backgroundColor", OJServerLink.Get()._altera_dark_blue);
            else
                UI.SetStyleAttribute(this._text_div.style, "backgroundColor", this._background_colour);
        }
    }

    OnClick(event)
    {
        if (this._tool_tip != null)
            OJToolTip.End(this._tool_tip, this._text_div);

        if (this.IsEnabled())
        {
            if (typeof this._click_callback === "object")
                this._click_callback.Call(event);
            else
                this._click_object[this._click_callback](event);

            UI.SetStyleAttribute(this._text_div.style, "backgroundColor", this._background_colour);
            this._hightlight_on_next_enter = true;
        }
    }

    OnDoubleClick(event)
    {
        if (this.IsEnabled() && (this._double_click_callback != null))
            this._click_object[this._double_click_callback](event);
    }

    OnContextMenu(event)
    {
        if (this._context_callback != null)
            this._click_object[this._context_callback](event);
    }

    SetBold()
    {
        this.GetElement().style.fontWeight = "bold";
    }

    Resize(x, y, width, height)
    {
        let shadow_border = this._shadow_border_size;
        let size_changed = OJWindowElement.prototype.Resize.call(this, x, y, width - shadow_border, height - shadow_border);

        let line_height = height - shadow_border;
        if (this._line_height != 0)
            line_height = this._line_height;

        this._text_div.style.lineHeight = line_height + "px";

        return size_changed;
    }

    GetWidth()
    {
        return (this._width + this._shadow_border_size);
    }

    GetHeight()
    {
        if (this._shown)
            return this._height + this._shadow_border_size;
        else
            return 0;
    }

    SetTagColour(colour)
    {
        this._colour_key_label.style.backgroundColor = colour;
    }

    HideTagColour()
    {
        if (this._hidden_tag_colour == null)
            this._hidden_tag_colour = this._colour_key_label.style.backgroundColor;
        this._colour_key_label.style.backgroundColor = "#404040";
    }

    RestoreTagColour()
    {
        if (this._hidden_tag_colour != null)
            this.SetTagColour(this._hidden_tag_colour);
    }
}

export class OJToolTip
{
    constructor()
    {
        let body = document.getElementById("_body");
        this._div = document.createElement("div");
        body.appendChild(this._div);
        this._div.className = "tool_tip_class";
        this._div.style.font = UI._font_name;
        this._show_timeout = null;
        this._hide_timeout = null;
        this._text = "";
        this._paired_element = null;
        this._centre_of_element = false;
    }

    Destroy()
    {
        if (this._show_timeout != null)
            clearTimeout(this._show_timeout);
        if (this._hide_timeout != null)
            clearTimeout(this._hide_timeout);

        this._paired_element = null;
    }

    Begin(text, paired_element, show_timeout)
    {
        if (this._show_timeout != null)
            clearTimeout(this._show_timeout);
        if (this._hide_timeout != null)
            clearTimeout(this._hide_timeout);

        this._hide_timeout = null;

        if (paired_element._disallow_tool_tip == true)
            return;

        let ms_timeout = 750;
        if (show_timeout != null)
            ms_timeout = show_timeout;

        this._show_timeout = setTimeout(this.ShowTimeoutCB, ms_timeout, this);
        this._text = text;
        this._paired_element = paired_element;
    }

    ShowTimeoutCB(tool_tip_object)
    {
        tool_tip_object.ShowTimeout();
    }

    ShowTimeout()
    {
        if (this._show_timeout != null)
            clearTimeout(this._show_timeout);
        this._show_timeout = null;

        let size = UI.MeasureRegularText(this._text);

        // Position above the paired element
        if (this._paired_element == null)
            return;

        let rect = UI.GetBoundingClientRect(this._paired_element);
        let height = size.height + 12;
        let y = rect.top - height - 3;

        if (y < 0)
            y = rect.bottom + 3;

        let x = rect.left;
        let right = rect.left + size.width + 15;
        if (right > (UI._screen_width - 10))
            x = (UI._screen_width - 10) - (size.width + 15);

        if (this._centre_of_element)
        {
            x = rect.left + rect.width / 2 - (size.width / 2);
            y = rect.top + rect.height / 2 - (size.height / 2);
        }

        OJLib.SetElementPosition(this._div, x, y, size.width + 15, height);
        this._div.style.lineHeight = height + "px";

        this._div.style.display = "block";
        this._div.innerHTML = this._text;

        if (this._hide_timeout != null)
            clearTimeout(this._hide_timeout);
        this._hide_timeout = setTimeout(this.HideTimeoutCB, 3000, this);
    }

    ModifyText(text)
    {
        this._text = text;
        this.ShowTimeout();
    }

    HideTimeoutCB(tool_tip_object)
    {
        tool_tip_object.HideTimeout();
    }

    HideTimeout()
    {
        this.Stop();
    }

    Stop()
    {
        if (this._show_timeout != null)
            clearTimeout(this._show_timeout);
        if (this._hide_timeout != null)
            clearTimeout(this._hide_timeout);

        this._text = "";
        this._paired_element = null;
        this._show_timeout = null;
        this._hide_timeout = null;
        this._div.style.display = "none";
    }

    static _the_tool_tip = null;

    // show_time defaults to 750ms if you don't supply the parameter
    static Start(text, paired_element, show_time, centre_of_element)
    {
        if (OJToolTip._the_tool_tip == null)
            OJToolTip._the_tool_tip = new OJToolTip();

        // Clear any previous
        OJToolTip._the_tool_tip.Stop();

        if (centre_of_element != null)
            OJToolTip._the_tool_tip._centre_of_element = centre_of_element;
        else
            OJToolTip._the_tool_tip._centre_of_element = false;

        OJToolTip._the_tool_tip.Begin(text, paired_element, show_time);
    }

    static End(text, element)
    {
        if (OJToolTip._the_tool_tip != null)
            OJToolTip._the_tool_tip.Stop();
    }
}

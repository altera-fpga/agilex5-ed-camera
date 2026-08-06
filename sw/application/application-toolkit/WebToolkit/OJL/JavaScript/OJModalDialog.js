/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJGrid } from "./OJL.js";
import { OJLabel } from "./OJL.js";
import { OJServerLink } from "./OJL.js";
import { OJTextButton } from "./OJL.js";
import { ANCHOR_TYPE } from "./OJL.js";
import { OJLib, UI, ObjectCallback } from "./OJL.js";
import { OJScrollable } from "./OJL.js";

export class OJModalDialog extends OJGrid
{
    constructor(width, height, title)
    {
        // Base class constructor
        super();
        this._class_name = "OJModalDialog";
        this._window_header_right_edge = 0;
        this.SetElementName("OJModalDialog");
        this._client_area.className = "dialog_class";
        let close_callback = new ObjectCallback(this, "OnOK");
        let corner_radius = 8;
        let header_buttons =
        [
            {
                _callback: close_callback,
                _image: OJLib._close._src,
                _image_over: null,
                _tool_tip: "Close dialog",
                _width: 30,
                _height: 30
            }
        ];

        let header_opts =
        {
            _dialog_header: true,
            _corner_radius: corner_radius,
            _buttons: header_buttons,
            _draggable_window: true
        };

        this.UseWindowHeader((title == null) ? UI._product_name : title, header_opts);
        this.SetBackgroundColour(UI._dialog_background_colour);
        this._enter_closes_dialog = true;
        this._have_dynamically_resized = false;

        let x_offset = (width / -2.0) | 0;
        let y_offset = (height / -2.0) | 0;
        y_offset += 40; // Exclude the header
        this.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _dynamic_position: 50, _fixed_offset: x_offset });
        this.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: width });
        this.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _dynamic_position: 50, _fixed_offset: y_offset });
        this.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: height });

        this._key_down_callback = new ObjectCallback(this, "OnKeyDown");

        this._dialog_width = (width == 0) ? 450 : width;
        this._dialog_height = (height == 0) ? 280 : height;

        this.GetClientAreaStyle().borderBottomLeftRadius = corner_radius + "px";
        this.GetClientAreaStyle().borderBottomRightRadius = corner_radius + "px";
        //this.GetClientAreaStyle().opacity = "0.0";
    }

    Destroy()
    {
        OJLib.UnregisterForKeyDown(this._key_down_callback);
        this._key_down_callback.Destroy();
        super.Destroy();
        UI._current_modal_dialog_class_name = "";
    }

    ShortCutKeyRequiresAlt()
    {
        // Are any of our children text controls? If so, we have to 
        // require Alt otherwise typing the shortcut key directly
        // could close the dialog
        return this.ContainsTextControl(this);
    }

    OnKeyDown(event)
    {
        let alt_check = true;
        if (this.ShortCutKeyRequiresAlt())
            alt_check = event.altKey;

        if (this._enter_closes_dialog && event._key_code == 13)
        {
            // Return = OK

            if (event._source_element._window_element != null)
            {
                if (event._source_element._window_element._class_name == "OJTextControl")
                {
                    // If the text control has focus, go blur
                    if (document.activeElement == event._source_element)
                    {
                        event._source_element.blur();
                        OJLib.MarkEventAsHandled(event);
                        return;
                    }
                }
            }

            this.OnOK(event);
            OJLib.MarkEventAsHandled(event);
        }
        else if (event._key_code == 27)
        {
            // Escape = Cancel
            this.OnCancel(event);
            OJLib.MarkEventAsHandled(event);
        }
        else if (alt_check && (this._button_0_shortcut_key != null) && (event._key_code == this._button_0_shortcut_key.charCodeAt(0)))
        {
            this.OnCancel(event);
            OJLib.MarkEventAsHandled(event);
        }
        else if (alt_check && (this._button_1_shortcut_key != null) && (event._key_code == this._button_1_shortcut_key.charCodeAt(0)))
        {
            this.OnOK(event);
            OJLib.MarkEventAsHandled(event);
        }
    }

    Show()
    {
        // Make sure all child elements are marked as dialog elements
        UI.SetDialogFlag(this._client_area);
        UI.SetDialogFlag(this._window_header);
        this.SetDialogFlag();

        UI.SetStyleAttribute(OJServerLink.Get()._desktop_screen_grid.GetClientAreaStyle(), "filter", "brightness(0.8) contrast(0.5) grayscale(30%) blur(0.5px)");

        this.GetClientAreaStyle().zIndex = 1000; // On to of ServerDead overlay
        this.GetHeaderElement().style.zIndex = 1000;

        UI._current_modal_dialog_class_name = this._class_name;

        OJLib.RegisterForKeyDown(this._key_down_callback);

        let default_focus_item = this.GetDefaultFocusElement();
        if (default_focus_item != null)
        {
            default_focus_item.focus();

            if (default_focus_item.select != null)
                default_focus_item.select();
        }
    }

    GetDefaultFocusElement(element)
    {
        return null;
    }

    GetDialogResult()
    {
        return null;
    }

    OnCancel(event)
    {
        UI.SetStyleAttribute(OJServerLink.Get()._desktop_screen_grid.GetClientAreaStyle(), "filter", null);
        this.RemoveFromDOM();
        UI._current_modal_dialog_class_name = "";
        OJLib.UnregisterForKeyDown(this._key_down_callback);

        OJServerLink.Get().CloseDialogs();
    }

    OnOK(event)
    {
        UI.SetStyleAttribute(OJServerLink.Get()._desktop_screen_grid.GetClientAreaStyle(), "filter", null);
        this.RemoveFromDOM();
        UI._current_modal_dialog_class_name = "";
        OJLib.UnregisterForKeyDown(this._key_down_callback);

        OJServerLink.Get().CloseDialogs();
    }
}

export class OJMessageDialog extends OJModalDialog
{
    constructor(message, button_0_text, button_1_text,
                button_0_callback, button_1_callback,
                width, height)
    {
        // Base class constructor
        super(500, 300, UI._product_name);

        this._button_0_callback = button_0_callback;
        this._button_1_callback = button_1_callback;

        this._scrollable = new OJScrollable(null);
        this._scrollable.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 5 });
        this._scrollable.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -5 });
        this._scrollable.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 7 });
        this._scrollable.SetBottomAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -55 });

        this._message_area = new OJLabel(message);
        this.AddChild(this._scrollable);
        this._scrollable.AddScrollableChild(this._message_area);
        this._message_area.GetElement().focus();
        this._message_area.GetElement()._is_dialog_element = true;
        this._message_area._paragraph.style.maxHeight = "200px";
        this._message_area._paragraph.style.padding = "15px";

        this.GetElement()._is_dialog_element = true;

        this._button_0 = null;
        this._button_1 = null;

        this._button_0_shortcut_key = "y";
        this._button_1_shortcut_key = "n";

        if (button_0_text != "")
            this._button_0_shortcut_key = button_0_text.substr(0, 1);
        if (button_1_text != "")
            this._button_1_shortcut_key = button_1_text.substr(0, 1);

        let n_buttons = 0;
        if (button_0_text != "")
            n_buttons++;
        if (button_1_text != "")
            n_buttons++;

        this._button_panel = new OJGrid();
        this._button_panel.SetBackgroundColour("#ffffff");
        this._button_panel.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -55 });
        this.AddChild(this._button_panel);

        if (button_0_text != "")
        {
            this._button_0 = new OJTextButton(null, button_0_text, { _click_object: this, _click_callback: "OnCancel", _alignment: "centre" });
            this._button_0.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 10 });
            this._button_0.GetButtonElement()._is_dialog_element = true;
        }

        if (button_1_text != "")
        {
            this._button_1 = new OJTextButton(null, button_1_text, { _click_object: this, _click_callback: "OnOK", _alignment: "centre" });
            this._button_1.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 5 });
            this._button_1.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 10 });
            this._button_1.GetButtonElement()._is_dialog_element = true;
        }

        if (n_buttons == 1)
        {
            this._button_0.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -130 });
        }
        else if (n_buttons == 2)
        {
            this._button_0.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -260 });
            this._button_1.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -130 });
        }

        if (this._button_0)
            this._button_panel.AddChild(this._button_0);

        if (this._button_1)
            this._button_panel.AddChild(this._button_1);
    }

    OnCancel(event)
    {
        let dialog_result = this.GetDialogResult();

        // &#39;
        if (dialog_result != null)
            dialog_result = dialog_result.replace(/'/g, "&#39;");

        if (this._button_0_callback != null)
        {
            this._button_0_callback.Call(this._button_0_callback._user_data);
        }
        else
        {
            let param = (dialog_result == null) ? "0" : "0," + dialog_result;
            if (OJServerLink.Get() != null)
                OJServerLink.Get().ExecuteServerCommandWithParams("DialogResult", param);
        }

        super.OnCancel(event);
    }

    OnOK(event)
    {
        let dialog_result = this.GetDialogResult();

        if (this._button_1_callback != null)
        {
            this._button_1_callback.Call(this._button_1_callback._user_data);
        }
        else
        {
            let param = (dialog_result == null) ? "1" : "1," + dialog_result;
            if (OJServerLink.Get() != null)
                OJServerLink.Get().ExecuteServerCommandWithParams("DialogResult", param);
        }

        super.OnOK(event);
    }

    Resize(x, y, width, height)
    {
        super.Resize(x, y, width, height);

        if (this._have_dynamically_resized)
            return;

        let message_rect = UI.GetBoundingClientRect(this._message_area._paragraph);
        let message_width = message_rect.width;
        let message_height = message_rect.height;

        let use_width = width;
        let centre_x = (x + width / 2) | 0;
        let use_x = x;//(centre_x - use_width / 2) | 0;

        if (message_width > (width - 50))
        {
            use_width = message_width + 50;
            use_x = (centre_x - (use_width / 2)) | 0;
        }

        if (message_height > (height - 80))
        {
            // Resize again
            let max_height = 600;
            let use_height = message_height + 120;
            if (use_height > max_height)
            {
                // Require scrollbar
                use_height = max_height;
            }

            this._have_dynamically_resized = true;


            this.Resize(use_x, y, use_width, use_height);
            this._scrollable.ChildSizeChanged();
        }
    }

}

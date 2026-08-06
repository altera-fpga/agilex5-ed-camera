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
import { OJGrid } from "./OJL.js";
import { OJStack } from "./OJL.js";
import { OJTextControl, TEXT_CONTROL_TYPE } from "./OJL.js";
import { OJTextButton } from "./OJL.js";
import { OJLib, UI, ObjectCallback } from "./OJL.js";
import { OJDropDownList } from "./OJL.js";
import { OJWindowElement, ANCHOR_TYPE } from "./OJL.js";
import { OJLabel } from "./OJL.js";
import { OJSliderControl } from "./OJL.js";
import { OJOnOffButton } from "./OJL.js";
import { OJPicon } from "./OJL.js";
import { OJLogger } from "./OJL.js";

let control_height = 32;

export class OJControlItemBase extends OJGrid
{
    constructor(ui_element_json, opts)
    {
        // Base class constructor
        super(null, null, null, opts);
        this._unique_id = ui_element_json.unique_id;
        this._value_changed_cb = new ObjectCallback(this, "ValueChangedCB");
        this._server_callback_name = ui_element_json.server_callback_name;
        this._control_x = 220;
        if (ui_element_json.control_x > 0)
            this._control_x = ui_element_json.control_x;
        else if (ui_element_json.parent_fixed_width != null)
            this._control_x = (ui_element_json.parent_fixed_width / 2) | 0;

        this._full_width_value = ui_element_json.full_width_value;
        this._label = null;
        this._label_x = 0;
        this._label_text = ui_element_json.label || "";
        this._num_lines = ui_element_json.num_lines;
        this._enabled = ui_element_json.enabled;
        this._read_only = ui_element_json.read_only;
        this._tooltip = ui_element_json.tooltip;
        this._client_callback = null;
        this._client_callback_owner_id = 0;
        let show = ui_element_json.show;
        if (!show)
            this.Show(false);

        if (ui_element_json.client_callback_name != "")
        {
            this._client_callback_owner_id = ui_element_json.client_callback_owner_id;

            // Fetch this once all the controls are loaded if not found now
            let owner_item = OJServerLink.Get().GetControlItem(this._client_callback_owner_id);               
            this._client_callback = new ObjectCallback(owner_item, ui_element_json.client_callback_name);            
        }

        OJServerLink.Get().AddControlItem(this);

        if (!this._windowless)
        {
            if (ui_element_json.Style)
            {
                for (let style_item of ui_element_json.Style)
                {
                    let style_setting = style_item.setting;
                    let style_value = style_item.value;
                    UI.SetStyleAttribute(this._client_area.style, style_setting, style_value);
                }
            }
        }

        this.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: this.GetControlHeight() });
    }

    Destroy()
    {
        super.Destroy();
        if (this._client_callback != null)
        {
            this._client_callback.Destroy();
            this._client_callback = null;
        }
        this._value_changed_cb.Destroy();
        this._value_changed_cb = null;
        OJServerLink.Get().RemoveControlItem(this);
    }

    UpdateClientCallbackObject(server_link)
    {
        if (this._client_callback != null)
        {
            let owner_item = server_link.GetControlItem(this._client_callback_owner_id);
            this._client_callback.UpdateCallbackObject(owner_item);
        }
    }

    GetControlHeight()
    {
        if (this._num_lines == 1)
            return control_height;

        return (control_height - 8) * this._num_lines + 8;
    }

    CallClientCallback(value)
    {
        if (this._client_callback)
        {
            this._value = value;
            this._client_callback.Call(value);
        }
    }

    ValueChangedCB(value)
    {
        this.CallClientCallback(value);

        if (this._server_callback_name)
        {
            let params = "" + value;
            OJServerLink.Get().ExecuteServerCommandWithParams(this._server_callback_name, params);
        }
    }

    Enable(value)
    {
        this._enabled = value;
    }

    UpdateOwner(value)
    {
        if (this._client_callback)
            this._client_callback.Call(value);
    }

    RefreshOwner()
    {
        if (this._value != "undefined")
            this.UpdateOwner(this._value);
    }

    // Called on the server side value update
    UpdateValue(value)
    {
        this.UpdateOwner(value);
    }

    UpdateLabel(label)
    {
        this._label_text = label;
        if (this._label != null)
            this._label.SetValue(this._label_text);
    }

    LogAutomationDetails(index, settings_section_name)
    {
        let details = "";
        if (this._label_text != "")
        {
            let lc_label = this._label_text.toLowerCase().replace(/ /g, "_");
            details = `${lc_label}_control_id = web_api.GetControlID(\"${settings_section_name}\", \"${this._label_text}\")`;
        }
        else
            details = `control_id_${index} = web_api.GetControlID(\"${settings_section_name}\", \"<no-label>\")`;
        console.log(details);
    }
}

///////////////////////////////////////////////////////////////////////////////

class OJControlItemSlider extends OJControlItemBase
{
    constructor(ui_element_json, floating_point)
    {
        // Base class constructor
        super(ui_element_json);

        this._label = new OJLabel(this._label_text, { _alignment: "left" });
        this._label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._label_x });
        this._label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: this._control_x - this._label_x });
        this._label.SetBackgroundColour(UI._control_panel_background_colour);

        this.AddChild(this._label);
        this._floating_point = floating_point;
        this._show_decimal_places = ui_element_json.show_decimal_places;

        this._value = ui_element_json.value;
        this._minimum = ui_element_json.minimum;
        this._maximum = ui_element_json.maximum;

        let slider_opts = 
        {
            _update_callback: this._value_changed_cb,
            _min: this._minimum,
            _max: this._maximum,
            _floating_point: this._floating_point,
            _show_decimal_places: this._show_decimal_places,
            _tooltip : this._tooltip
        };

        if (ui_element_json.convert_value_to_display_owner_id != null)
        {
            let convert_value_to_display_owner_id = ui_element_json.convert_value_to_display_owner_id;
            if (convert_value_to_display_owner_id != 0)
            {
                let owner_item = OJServerLink.Get().GetControlItem(convert_value_to_display_owner_id);               
                this._convert_value_to_display_fn = new ObjectCallback(owner_item, ui_element_json.convert_value_to_display_fn_name);   
                slider_opts._convert_value_to_display_fn = this._convert_value_to_display_fn;
            }
        }

        if (ui_element_json.convert_display_to_value_owner_id != null)
        {
            let convert_display_to_value_owner_id = ui_element_json.convert_display_to_value_owner_id;
            if (convert_display_to_value_owner_id != 0)
            {
                let owner_item = OJServerLink.Get().GetControlItem(convert_display_to_value_owner_id);               
                this._convert_display_to_value_fn = new ObjectCallback(owner_item, ui_element_json.convert_display_to_value_fn_name);   
                slider_opts._convert_display_to_value_fn = this._convert_display_to_value_fn;
            }
        }        

        this._slider = new OJSliderControl(null, slider_opts);
        this._slider.SetToolTip(this._tooltip);

        this.UpdateValue(this._value);

        this._slider.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._control_x });
        this._slider.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: 0 });
        this._slider.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 0 });
        this._slider.SetBottomAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: 0 });

        this.AddChild(this._slider);

        if (ui_element_json.value_postfix != "")
        {
            this._prefix_label = new OJLabel(ui_element_json.value_postfix, { _alignment: "left" });
            this._prefix_label.SetLeftAnchor({ _type: ANCHOR_TYPE.SIBLING_FAR, _sibling: this._slider, _fixed_offset: 5 });
            this._prefix_label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 80 });
            this._prefix_label.SetBackgroundColour(UI._control_panel_background_colour);
            this.AddChild(this._prefix_label);
        }

        if (!this._enabled)
            this._slider.Enable(false);
    }

    UpdateValue(value)
    {
        this._value = value;
        super.UpdateValue(this._value);
        this._slider.SetValue(this._value);
    }

    Enable(value)
    {
        super.Enable(value);
        this._slider.Enable(this._enabled);
    }

    UpdateRange(min, max)
    {
        this._slider.UpdateRange(min, max);
    }
}

///////////////////////////////////////////////////////////////////////////////

class OJControlItemInteger extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);

        this._label = new OJLabel(this._label_text, { _alignment: "left" });
        this._label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._label_x });
        this._label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: this._control_x - this._label_x });
        this._label.SetBackgroundColour(UI._control_panel_background_colour);
        this.AddChild(this._label);

        if (ui_element_json.value_prefix != "")
        {
            this._prefix_label = new OJLabel(ui_element_json.value_prefix, { _alignment: "right" });
            this._prefix_label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._control_x - 55 });
            this._prefix_label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 50 });
            this._prefix_label.SetBackgroundColour(UI._control_panel_background_colour);
            this.AddChild(this._prefix_label);
        }

        this._value = ui_element_json.value;
        this._minimum = ui_element_json.minimum;
        this._maximum = ui_element_json.maximum;
        this._hex = ui_element_json.hex;
        this._increment = ui_element_json.increment;
        let control_type = this._hex ? TEXT_CONTROL_TYPE.TCT_HEX_INTEGER : TEXT_CONTROL_TYPE.TCT_INTEGER;
        this._text_edit = new OJTextControl(control_type, this._value, this._value_changed_cb,
                                            this._minimum, this._maximum, this._increment);
        this._text_edit.SetToolTip(this._tooltip);
        this._text_edit.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._control_x });
        this._text_edit.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: this._hex ? 100 : 80 });
        this.AddChild(this._text_edit);

        if (ui_element_json.value_postfix != "")
        {
            this._prefix_label = new OJLabel(ui_element_json.value_postfix, { _alignment: "left" });
            this._prefix_label.SetLeftAnchor({ _type: ANCHOR_TYPE.SIBLING_FAR, _sibling: this._text_edit, _fixed_offset: 5 });
            this._prefix_label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 80 });
            this._prefix_label.SetBackgroundColour(UI._control_panel_background_colour);
            this.AddChild(this._prefix_label);
        }

        if (!this._enabled)
            this._text_edit.Enable(false);

        if (this._read_only)
            this._text_edit.ReadOnly(true);
    }

    UpdateValue(value)
    {
        this._value = value;
        this._text_edit.SetValue(this._value);
        super.UpdateValue(this._value);
    }

    Enable(value)
    {
        super.Enable(value);
        this._text_edit.Enable(this._enabled);
    }
}

///////////////////////////////////////////////////////////////////////////////

class OJControlItemText extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);

        this._label = new OJLabel(this._label_text, { _alignment: "left" });
        this._label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._label_x });
        this._label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: this._control_x - this._label_x });
        this._label.SetBackgroundColour(UI._control_panel_background_colour);

        if (!this._full_width_value)
            this.AddChild(this._label);

        if (ui_element_json.value_prefix != "")
        {
            this._prefix_label = new OJLabel(ui_element_json.value_prefix, { _alignment: "right" });
            this._prefix_label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._control_x - 55 });
            this._prefix_label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 50 });
            this._prefix_label.SetBackgroundColour(UI._control_panel_background_colour);
            this.AddChild(this._prefix_label);
        }

        this._value = ui_element_json.value;
        this._text_edit = new OJTextControl(TEXT_CONTROL_TYPE.TCT_TEXT, this._value, this._value_changed_cb);
        this._text_edit.SetToolTip(this._tooltip);

        if (!this._full_width_value)
            this._text_edit.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._control_x });

        this.AddChild(this._text_edit);

        if (ui_element_json.value_postfix != "")
        {
            this._text_edit.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -50 });

            this._postfix_label = new OJLabel(ui_element_json.value_postfix, { _alignment: "left" });
            this._postfix_label.SetLeftAnchor({ _type: ANCHOR_TYPE.SIBLING_FAR, _sibling: this._text_edit, _fixed_offset: 5 });
            this._postfix_label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 40 });
            this._postfix_label.SetBackgroundColour(UI._control_panel_background_colour);
            this.AddChild(this._postfix_label);
        }

        if (!this._enabled)
            this._text_edit.Enable(false);

        if (this._read_only)
            this._text_edit.ReadOnly(true);            
    }

    UpdateValue(value)
    {
        this._value = value;
        this._text_edit.SetValue(this._value);
        super.UpdateValue(this._value);
    }

    Enable(value)
    {
        super.Enable(value);
        this._text_edit.Enable(this._enabled);
    }
}

///////////////////////////////////////////////////////////////////////////////

class OJControlItemLabel extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);

        let value_x = 0;
        let alignment = "center";

        if (this._label_text != "")
        {
            this._label = new OJLabel(this._label_text, { _alignment: "left" });
            this._label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._label_x });
            this._label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: this._control_x - this._label_x });
            this._label.SetBackgroundColour(UI._control_panel_background_colour);
            this.AddChild(this._label);
            value_x = this._control_x;
            alignment = "left";
        }

        this._value = ui_element_json.value;
        this._value_label = new OJLabel(this._value, { _alignment: alignment });
        this._value_label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: value_x });

        if (ui_element_json.value == "LFR")
        {
            this._monitor = true;
            this._value_label._monitor = true;
        }

        this.AddChild(this._value_label);

    }

    UpdateValue(value)
    {
        this._value = value;
        this._value_label.SetValue(this._value);
        super.UpdateValue(this._value);
    }
}

///////////////////////////////////////////////////////////////////////////////

class OJControlItemButton extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);

        if (this._label_text != "")
        {
            this._label = new OJLabel(this._label_text, { _alignment: "left" });
            this._label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._label_x });
            this._label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: this._control_x - this._label_x });
            this.AddChild(this._label);
        }

        let enabled = ui_element_json.enabled;
        let opts = { _click_object: this, _click_callback: "OnClick", _alignment: "centre", _enabled: enabled };
        this._text_button = new OJTextButton(null, ui_element_json.buttonLabel, opts);
        this._text_button.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 1 });
        this._text_button.SetBottomAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -1 });
        this._text_button.SetToolTip(this._tooltip);

        if ((ui_element_json.center != null) && ui_element_json.center)
        {
            this._text_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _dynamic_position: 50, _fixed_offset: -80 });
            this._text_button.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 160 });
        }
        else
        {                                         
            this._text_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._control_x });
            this._text_button.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: 0 });
        }

        this.AddChild(this._text_button);
    }

    OnClick()
    {
        this._value_changed_cb.Call(" ");
    }

    UpdateValue(value)
    {
        super.UpdateValue(value);
        this._text_button.SetLabel(value);
    }

    Enable(value)
    {
        super.Enable(value);
        this._text_button.Enable(this._enabled);
    }
}

///////////////////////////////////////////////////////////////////////////////

class OJControlItemPicture extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);

        let enabled = ui_element_json.enabled;
        this._picon = new OJPicon(null);
        this._picon.SetToolTip(this._tooltip);
        this.AddChild(this._picon);
    }

    SetImage(image_data)
    {
        this._picon.LoadFromImageData(image_data);
    }

    Resize(x, y, width, height)
    {
        let size_changed = super.Resize(x, y, width, height);
        return size_changed;
    }    
}


///////////////////////////////////////////////////////////////////////////////

class OJControlItemFloat extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);

        this._unique_id = ui_element_json.unique_id;
        this._value_changed_cb = new ObjectCallback(this, "ValueChangedCB");
        this._callback_name = ui_element_json.server_callback_name;

        this._label = new OJLabel(this._label_text, { _alignment: "left" });
        this._label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._label_x });
        this._label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: this._control_x - this._label_x });
        this._label.SetBackgroundColour(UI._control_panel_background_colour);
        this.AddChild(this._label);

        if (ui_element_json.value_prefix != "")
        {
            this._prefix_label = new OJLabel(ui_element_json.value_prefix, { _alignment: "right" });
            this._prefix_label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset : this._control_x - 55 });
            this._prefix_label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 50 });
            this._prefix_label.SetBackgroundColour(UI._control_panel_background_colour);
            this.AddChild(this._prefix_label);
        }

        this._value = ui_element_json.value;
        this._minimum = ui_element_json.minimum;
        this._maximum = ui_element_json.maximum;
        this._show_decimal_places = ui_element_json.show_decimal_places;
        this._increment = ui_element_json._increment;
        this._text_edit = new OJTextControl(TEXT_CONTROL_TYPE.TCT_FLOATING_POINT, this._value, this._value_changed_cb,
                                            this._minimum, this._maximum, this._increment);
        this._text_edit.SetDecimalPlaces(this._show_decimal_places);

        this._text_edit.SetToolTip(this._tooltip);
        this._text_edit.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._control_x });
        this._text_edit.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 80 });
        this.AddChild(this._text_edit);

        if (ui_element_json.value_postfix != "")
        {
            this._prefix_label = new OJLabel(ui_element_json.value_postfix, { _alignment: "left" });
            this._prefix_label.SetLeftAnchor({ _type: ANCHOR_TYPE.SIBLING_FAR, _sibling: this._text_edit, _fixed_offset: 5 });
            this._prefix_label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 80 });
            this._prefix_label.SetBackgroundColour(UI._control_panel_background_colour);
            this.AddChild(this._prefix_label);
        }

        if (!this._enabled)
            this._text_edit.Enable(false);

        if (this._read_only)
            this._text_edit.ReadOnly(true);            
    }

    UpdateValue(value)
    {
        this._value = value;
        this._text_edit.SetValue(this._value);
        super.UpdateValue(this._value);
    }

    Enable(value)
    {
        super.Enable(value);
        this._text_edit.Enable(this._enabled);
    }
}

///////////////////////////////////////////////////////////////////////////////

class OJControlItemEnum extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);

        this._label = new OJLabel(this._label_text, { _alignment: "left" });
        this._label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._label_x });
        this._label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: this._control_x - this._label_x });
        this._label.SetBackgroundColour(UI._control_panel_background_colour);

        this._value = ui_element_json.value;
        this._drop_down_list = new OJDropDownList(this._value, this._value_changed_cb, null, null, { _auto_select : true });
        this._drop_down_list.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._control_x });
        this._drop_down_list.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: 0 });
        this._drop_down_list.SetToolTip(this._tooltip);

        for (let option of ui_element_json.Options)
            this._drop_down_list.AddItem(option.string);

        this._drop_down_list.Create();

        this.UpdateValue(this._value);
        
        this.AddChild(this._drop_down_list);
        this.AddChild(this._label);

        if (!this._enabled)
            this._drop_down_list.Enable(false);
    }

    UpdateValue(value)
    {
        this._value = value;
        super.UpdateValue(this._value);
        this._drop_down_list.SetValue(this._value);
    }

    RemoveItem(index)
    {
        this._drop_down_list.RemoveItem(index);
    }

    AddItem(value) 
    {
        this._drop_down_list.AddItem(value);
    }

    Enable(value)
    {
        super.Enable(value);
        this._drop_down_list.Enable(this._enabled);
    }
}

///////////////////////////////////////////////////////////////////////////////

class OJControlItemBoolean extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);

        this._label = new OJLabel(this._label_text, { _alignment: "left" });
        this._label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._label_x });
        this._label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: this._control_x - this._label_x });
        this._label.SetBackgroundColour(UI._control_panel_background_colour);

        this._value = ui_element_json.value;
        this._on_off_button = new OJOnOffButton(this._value, this._value_changed_cb, ui_element_json._ledMode);
        this._on_off_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._control_x });
        this._on_off_button.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 40 });
        this._on_off_button.SetToolTip(this._tooltip);

        this.AddChild(this._on_off_button);
        this.AddChild(this._label);

        this.UpdateValue(ui_element_json.value);

        if (!this._enabled)
            this._on_off_button.Enable(false);

        if (this._read_only)
            this._on_off_button.ReadOnly(true);              
    }

    UpdateValue(value)
    {
        this._on_off_button.SetValue(value);
        this._value = this._on_off_button._on;

        super.UpdateValue(this._value);
    }

    Enable(value)
    {
        super.Enable(value);
        this._on_off_button.Enable(this._enabled);
    }
}

///////////////////////////////////////////////////////////////////////////////

class OJControlItemSeparator extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);

        this._separator = new OJWindowElement();
        this._separator.SetBackgroundColour("#d0d0d0");
        this._separator.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 6 });
        this._separator.SetBottomAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 7 });
        this._separator.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 0 });
        this._separator.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: 0 });
        this.AddChild(this._separator);
    }

    OnClick()
    {
    }

    UpdateValue(value)
    {
    }

    GetControlHeight()
    {
        return 10;
    }
}

///////////////////////////////////////////////////////////////////////////////

class OJControlItemEnumRadio extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);

        this._label = null;

        if (this._label_text)
        {
            this._label = new OJLabel(this._label_text, { _alignment: "left" });
            this._label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: this._label_x });
            this._label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: this._control_x - this._label_x });
            this._label.SetBackgroundColour(UI._dialog_background_colour);
        }

        this._buttons = [];

        this._value = ui_element_json.value;

        let sibling = this._label;
        let options = ui_element_json.Options;
        let num_buttons = options.length;
        let button_width = 62;
        let button_spacing = 12;
        let total_button_width = (button_width * num_buttons) + (button_spacing * (num_buttons - 1));
        let x_offset = total_button_width / -2;

        for (let i = 0; i < num_buttons; i++)
        {
            let option = options[i];
            let button = new OJTextButton(null, "", { _click_object: this, _click_callback: "OnButton", _user_data: i, _image_url: option.url });
            button.SetToolTip(option.string);

            if (this._label && sibling)
                button.SetLeftAnchor({ _type: ANCHOR_TYPE.SIBLING_FAR, _sibling: sibling, _fixed_offset: button_spacing });
            else
                button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _dynamic_position: 50, _fixed_offset: x_offset });
            
            button.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 62 });
            button.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 1 });
            button.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 52 });
            button._client_area.style.border = "3px solid transparent";
            this.AddChild(button);

            if (this._label)
                sibling = button;

            this._buttons.push(button);
            x_offset += (button_width + button_spacing);
        }

        this.UpdateValue(this._value);
        
        if (this._label)
            this.AddChild(this._label);

        if (!this._enabled)
            this._drop_down_list.Enable(false);
    }

    OnButton(event)
    {
        this._value = event._user_data;
        this._value_changed_cb.Call(this._value);
    }

    UpdateValue(value)
    {
        this._value = value;
        super.UpdateValue(this._value);

        for (let i = 0; i < this._buttons.length; i++)
        {
            let button = this._buttons[i];
            let border_style = "3px solid " + ((this._value == i) ? "#0068b5" : "transparent");
            button._client_area.style.border = border_style;        
        }
    }

    Enable(value)
    {
        super.Enable(value);
    }

    GetControlHeight()
    {
        return 56;
    }
}

///////////////////////////////////////////////////////////////////////////////

export class OJControlItemHiddenValue extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json, { _windowless: true });
        this.UpdateValue(ui_element_json.json_value);
    }

    UpdateValue (value)
    {
        this._value = value;
        super.UpdateValue(this._value);
    }
}

///////////////////////////////////////////////////////////////////////////////////

export class OJControlItemLogger extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);

        this._logger = new OJLogger("");

        this._logger.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 800 });
        this._logger.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 800 });

        this.AddChild(this._logger);

        this.UpdateValue(ui_element_json.value);
    }

    UpdateValue (value)
    {
        //this._value = value;
        //super.UpdateValue(this._value);

        this._logger.AddMessage(value);
    }
}

///////////////////////////////////////////////////////////////////////////////////

let custom_control_factory_functions = {};

export class OJControlContainer extends OJStack
{
    constructor(sibling_containers, ui_element_json)
    {
        let header_opts = {};
        let enable_border = ui_element_json.enable_border;
        let spacing = ui_element_json.spacing;

        let stack_opts = { _spacing: spacing, _margin: 12 };
        if (enable_border)
        {
            stack_opts._corner_radius = 8;
            stack_opts._square_top = true;
            header_opts._corner_radius = 8;
        }
        else
        {
            stack_opts._have_border = false;
        }

        // Add the label later once we've built up the details of the header buttons
        super(null, stack_opts, null);
        this._header_button_callbacks = [];
        this._transparent_panel = ui_element_json.transparent_panel;
        this._settings_section_name = ui_element_json.settings_section_name;

        if (ui_element_json.HeaderButtons)
        {
            let header_buttons = [];

            for (let json_button of ui_element_json.HeaderButtons)
            {
                 let user_data = { _server_callback_name: json_button.callback_name,
                                _client_callback_name: json_button.client_callback,
                                _client_callback_control_id: json_button.client_callback_control_id };
                let client_callback = new ObjectCallback(this, "HeaderButtonCallback", null, user_data);
                this._header_button_callbacks.push(client_callback);

                header_buttons.push({
                    _callback: client_callback,
                    _image: json_button.img,
                    _image_over: null,
                    _tool_tip: json_button.tooltip,
                    _width: 28,
                    _height: 28
                });            
            }

            header_opts._buttons = header_buttons;
        }
    
        if (!this._transparent_panel)//(ui_element_json.container_title != "") || (header_opts._buttons != null))
        {
            this.UseWindowHeader(ui_element_json.container_title, header_opts);	    
        }

        this._unique_id = ui_element_json.unique_id;
        this._controls = [];
        this._windowless_controls = [];
        this._fixed_width = 0;
        this._fixed_height = 0;
        this._original_parent = null;
        this._was_full_screen = UI.IsFullscreen();

        // If the parent is a stack (ie other OJControlContainer)
        // then it will be positioned according to the anchors
        // and not auto positioned in the usual stack way
        this._has_anchor_position = true;

        let show = ui_element_json.show;
        if (!show)
            this.Show(false);

        let bottom_anchor_params = { _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 0 };

        if (window.IsMobileDevice())
        {
            let width = window.innerWidth - 10;
            this.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 5 });
            this.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: width });
            if (sibling_containers.length > 0)
            {
                let sibling = sibling_containers[sibling_containers.length - 1];
                this.SetTopAnchor({ _type: ANCHOR_TYPE.SIBLING_FAR, _sibling: sibling, _fixed_offset: 10 });
            }
            else
            {
                this.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 10 });
            }
            this.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 300 });
        }
        else
        {
            let left_anchor_params = OJServerLink.Get().MakeAnchorParams(ui_element_json.LeftAttachment, sibling_containers);
            let top_anchor_params = OJServerLink.Get().MakeAnchorParams(ui_element_json.TopAttachment, sibling_containers);
            let right_anchor_params = OJServerLink.Get().MakeAnchorParams(ui_element_json.RightAttachment, sibling_containers);
            let temp_bottom_anchor_params = OJServerLink.Get().MakeAnchorParams(ui_element_json.BottomAttachment, sibling_containers);

            if ((right_anchor_params._type == ANCHOR_TYPE.FIXED_SIZE) && (right_anchor_params._fixed_size != 0))
                this._fixed_width = right_anchor_params._fixed_size; 

            if ((temp_bottom_anchor_params._type == ANCHOR_TYPE.FIXED_SIZE) && (temp_bottom_anchor_params._fixed_size != 0))
                this._fixed_height = temp_bottom_anchor_params._fixed_size;

            if ((temp_bottom_anchor_params._type != ANCHOR_TYPE.FIXED_SIZE) || (temp_bottom_anchor_params._fixed_size != 0))
                bottom_anchor_params = temp_bottom_anchor_params;

            this.SetLeftAnchor(left_anchor_params);
            this.SetRightAnchor(right_anchor_params);
            this.SetTopAnchor(top_anchor_params);
        }


        if (!this._transparent_panel)
            this.SetBackgroundColour(UI._control_panel_background_colour);

        let total_control_height = 0;

        if (ui_element_json.Controls != null)
        {
            for (let control_json of ui_element_json.Controls)
            {
                control_json.parent_fixed_width = this._fixed_width;
                let control = this.CreateControl(control_json);

                if (control._windowless)
                {
                    this._windowless_controls.push(control);
                }
                else
                {
                    total_control_height += control.GetControlHeight();
                    total_control_height += 4;
                    this._controls.push(control);
                    this.AddChild(control, false, false);
                }
            }
        }

        let height = this._fixed_height;
        if (height == 0) // Auto calculate
        {
            height = total_control_height + 54;
            this.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: height });
        }
        else
        {
            this.SetBottomAnchor(bottom_anchor_params);
        }
    }

    Destroy()
    {
        super.Destroy();

        OJLib.DestroyArray(this._header_button_callbacks);

        for (let control of this._windowless_controls)
            control.Destroy();
            
        this._windowless_controls.length = 0;
        this._controls.length = 0;
    }

    Resize(x, y, width, height)
    {
        this.RecalculateHeight(width, height);

        let size_changed = super.Resize(x, y, width, height);
        return size_changed;
    }

    GetControlHeight(pending_width, pending_height)
    {
        // Called when this OJControlContainer is a child of another OJControlContainer
        let my_contents_height = this.RecalculateHeight(pending_width, pending_height);
        return my_contents_height;
    }

    RecalculateHeight(pending_width, pending_height)
    {
        let total_control_height = 0;

        for (let control of this._controls)
        {
            total_control_height += control.GetControlHeight(pending_width, pending_height);
            total_control_height += this._spacing;
        }

        let height = this._fixed_height;
        if (height == 0) // Auto calculate
            height = total_control_height + 54;

        this.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: height });
        return height;
    }

    HeaderButtonCallback(event)
    {
        let params = "";
        let user_data = event._user_data;
        let server_callback_name = user_data._server_callback_name;
        let client_callback_name = user_data._client_callback_name;
        let client_callback_control_id = user_data._client_callback_control_id;

        if (client_callback_name != "")
        {
            // Call local client callback if supplied
            let owner_id = client_callback_control_id;
            let owner_item = (owner_id == 0) ? this : OJServerLink.Get().GetControlItem(owner_id);
            if (owner_item != null)
            {
                owner_item[client_callback_name](event);            
            }
        }

        // Call server callback if supplied
        if (server_callback_name != "")
        {
            OJServerLink.Get().ExecuteServerCommandWithParams(server_callback_name, params);
        }
    }

    RestoreAction() 
    {
        // Restore position
        OJServerLink.Get()._desktop_screen_grid.RemoveChild(this);

        this.SetTopAnchor(this._saved_top_anchor);
        this.SetBottomAnchor(this._saved_bottom_anchor);
        this.SetLeftAnchor(this._saved_left_anchor);
        this.SetRightAnchor(this._saved_right_anchor);

        this._original_parent.AddChild(this);
        OJServerLink.Get()._desktop_screen_grid.PositionChildren();
        this._original_parent = null;
        OJServerLink.Get()._full_screen_black_page.Show(false);

        if (!this._was_full_screen)
            UI.GoFullscreen(false);

        if (this._header_buttons.length > 0)
        {
            this._header_buttons[0].SwapImage(OJLib._maximise._src);
            this._header_buttons[0].SetToolTip("Maximize");
        }

        if (UI._fullscreen_restore_action)
        {
            UI._fullscreen_restore_action.Destroy();
            UI._fullscreen_restore_action = null;
        }
    }

    OnMaximize(event)
    {
        if (this._original_parent)
        {
            this.RestoreAction();
        }
        else
        {
            // Go full screen
            this._was_full_screen = UI.IsFullscreen();

            if (this._was_full_screen)
                this.MaximizeComplete();
            else
            {
                let after_fullscreen_action = new ObjectCallback(this, "MaximizeComplete");
                UI.GoFullscreen(true, after_fullscreen_action);
            }

            if (this._header_buttons.length > 0)
            {
                this._header_buttons[0].SwapImage(OJLib._restore._src);
                this._header_buttons[0].SetToolTip("Restore Down");
            }
        }
    }

    MaximizeComplete()
    {
        this._original_parent = this._parent_window_element;
        this._saved_top_anchor = this._top_anchor.Clone();
        this._saved_bottom_anchor = this._bottom_anchor.Clone();
        this._saved_left_anchor = this._left_anchor.Clone();
        this._saved_right_anchor = this._right_anchor.Clone();

        // Make us same size as parent
        this.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 0 });
        this.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: 0 });
        this.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 0 });
        this.SetBottomAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: 0 });

        // Re-parent us to the desktop screen grid
        this._parent_window_element.RemoveChild(this);
        OJServerLink.Get()._desktop_screen_grid.AddChild(this);
        OJServerLink.Get()._desktop_screen_grid.PositionChildren();
        OJServerLink.Get()._full_screen_black_page.Show(true);

        if (UI._fullscreen_restore_action)
            UI._fullscreen_restore_action.Destroy();

        UI._fullscreen_restore_action = new ObjectCallback(this, "RestoreAction");
    }

    static AddCustomControlFactory(control_name, factory_create_fn)
    {
        custom_control_factory_functions[control_name] = factory_create_fn;
    }

    CreateControl(ui_element_json)
    {
        let new_control = null;
        if (ui_element_json.element_type == "UiControlItemSlider")
        {
            new_control = new OJControlItemSlider(ui_element_json, false);
        }
        else if (ui_element_json.element_type == "UiControlItemFloatSlider")
        {
            new_control = new OJControlItemSlider(ui_element_json, true);
        }
        else if (ui_element_json.element_type == "UiControlItemInteger")
        {
            new_control = new OJControlItemInteger(ui_element_json);
        }
        else if (ui_element_json.element_type == "UiControlItemFloat")
        {
            new_control = new OJControlItemFloat(ui_element_json);
        }
        else if (ui_element_json.element_type == "UiControlItemEnum")
        {
            new_control = new OJControlItemEnum(ui_element_json);
        }
        else if (ui_element_json.element_type == "UiControlItemBoolean")
        {
            new_control = new OJControlItemBoolean(ui_element_json);
        }
        else if (ui_element_json.element_type == "UiControlItemText")
        {
            new_control = new OJControlItemText(ui_element_json);
        }
        else if (ui_element_json.element_type == "UiControlItemButton")
        {
            new_control = new OJControlItemButton(ui_element_json);
        }
        else if (ui_element_json.element_type == "UiControlItemPicture")
        {
            new_control = new OJControlItemPicture(ui_element_json);
        }        
        else if (ui_element_json.element_type == "UiControlSeparator")
        {
            new_control = new OJControlItemSeparator(ui_element_json);
        }
        else if (ui_element_json.element_type == "UiControlItemLabel")
        {
            new_control = new OJControlItemLabel(ui_element_json);
        }    
        else if (ui_element_json.element_type == "UiControlItemEnumRadio")
        {
            new_control = new OJControlItemEnumRadio(ui_element_json);
        }    
        else if (ui_element_json.element_type == "UiControlContainer")
        {
            new_control = new OJControlContainer([], ui_element_json);
        }
        else if (ui_element_json.element_type == "UiLogger")
        {
            //new_control = new OJLogger(ui_element_json);
            new_control = new OJControlItemLogger(ui_element_json);
        }        
        else
        {
            // Check for custom controls
            let factory_create_fn = custom_control_factory_functions[ui_element_json.element_type];
            if (factory_create_fn)
            {
                // The factory function can add items to either 
                // _control_containers, or _custom_controls
                new_control = factory_create_fn(ui_element_json.element_type, 
                                                ui_element_json);
            }

            if (new_control == null)
                OJLib.Trace("OJControlContainer.CreateControl no factory creator for " + ui_element_json.element_type);
        }

        return new_control;
    }

    LogAutomationDetails()
    {
        let index = 0;
        for (let control of this._controls)
        {
            if (this._base_header_label != "" )
                control.LogAutomationDetails(index++, this._base_header_label);
            else
                control.LogAutomationDetails(index++, this._settings_section_name);
        }
    }
}


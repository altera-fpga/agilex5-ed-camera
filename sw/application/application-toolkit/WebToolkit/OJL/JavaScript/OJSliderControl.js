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
import { OJLib, ObjectCallback, UI } from "./OJL.js";
import { OJGrid } from "./OJL.js";
import { OJLabel } from "./OJL.js";
import { OJTextControl, TEXT_CONTROL_TYPE } from "./OJL.js";

export class OJSlider extends OJWindowElement
{
    constructor(parent_element, opts)
    {
        super();
        this._class_name = "OJSlider";
        this.SetElementName("OJSlider");

        this._update_callback = opts._update_callback ? opts._update_callback : null;

        this._slider = document.createElement("input");
        this._slider.type = "range";
        this._slider.className = "input_slider";

        if ((opts._min != null) && (opts._max != null))
        {
            this._slider.min = opts._min;
            this._slider.max = opts._max;
            if ((opts._floating_point != null) && opts._floating_point)
                this._slider.step = 0.01;
        }

        var MakeUpdateFunctor = function(owner)
        {
            return function(event)
            {
                owner.SliderUpdateCB(event.srcElement ? event.srcElement.value : event.target.value);
            }
        }

        var on_slider_update = MakeUpdateFunctor(this);
        this._slider.oninput = on_slider_update;
        this._client_area.appendChild(this._slider);
    }

    GetValue()
    {
        return this._slider.value;
    }

    SetValue(value)
    {
        return this._slider.value = value;
    }

    SliderUpdateCB(value)
    {
        if (this._update_callback != null)
            this._update_callback.Call(value);
    }

    Enable(enable)
    {
        super.Enable(enable);
        this._slider.disabled = !enable;

        UI.SetStyleAttribute(this._slider.style, "borderColor", enable ? null : UI._disabled_text_colour);

        if (enable)
            this._slider.className = null;
        else
            this._slider.className = "input_slider_disabled";

        this._slider.id = enable ? null : "disabled_slider";
    }

    Resize (x, y, width, height)
    {
        // Call base to set the child sizes
        var size_changed = super.Resize(x, y, width, height);

        OJLib.SetElementPosition(this._slider, 0, 0, width, height);

        return size_changed;
    }
}

///////////////////////////////////////////////////////////////////////////////

export class OJSliderControl extends OJGrid
{
    constructor(parent_element, opts)
    {
        super();
        this._class_name = "OJSliderControl";
        this.SetElementName("OJSliderControl");

        this._update_callback = opts._update_callback ? opts._update_callback : null;
        this._floating_point = opts._floating_point ? opts._floating_point : false;
        this._show_decimal_places = opts._show_decimal_places ? opts._show_decimal_places : 0;

        // Label
        if (opts._label)
        {
            this._label = new OJLabel(opts._label);
            this._label.SetBackgroundColour("#181818");
            this._label.GetParagraphStyle().textAlign = "left";
            this._label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 0 });
            this._label.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 240 });
            this._label.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 30 });

            this.AddChild(this._label);
        }
        else
            this._label = null;

        this._convert_value_to_display_fn = null;
        this._convert_display_to_value_fn = null;

        if (opts._convert_value_to_display_fn)
            this._convert_value_to_display_fn = opts._convert_value_to_display_fn;

        if (opts._convert_display_to_value_fn)
            this._convert_display_to_value_fn = opts._convert_display_to_value_fn;

        // Text input
        this._text_update_callback = new ObjectCallback(this, "OnTextUpdateCB");
        if (this._floating_point)
        {
            this._text_input = new OJTextControl(TEXT_CONTROL_TYPE.TCT_FLOATING_POINT, 0, this._text_update_callback);
            this._text_input.SetDecimalPlaces(this._show_decimal_places);
        }
        else
            this._text_input = new OJTextControl(TEXT_CONTROL_TYPE.TCT_INTEGER, 0, this._text_update_callback);

        this._text_input.SetToolTip(opts._tooltip);
        this._text_input.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -60 });
        this._text_input.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: 0 });
        this._text_input.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 30 });
        if (this._label)
            this._text_input.SetTopAnchor({ _type: ANCHOR_TYPE.SIBLING_FAR, _sibling: this._label, _fixed_offset: 0 });

        // Slider
        var slider_opts = 
        {
            _update_callback: new ObjectCallback(this, "OnSliderUpdateCB"),
            _min: (opts._min != null) ? opts._min : null,
            _max: (opts._max != null) ? opts._max : null,
            _floating_point : this._floating_point
        };

        this._slider = new OJSlider(null, slider_opts);
        this._slider.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -68 });
        this._slider.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 30 });
        if (this._label)
            this._slider.SetTopAnchor({ _type: ANCHOR_TYPE.SIBLING_FAR, _sibling: this._label, _fixed_offset: 0 });

        this._slider.SetValue(0);

        this.AddChild(this._text_input);
        this.AddChild(this._slider);
    }

    Destroy()
    {
        this._text_update_callback.Destroy();
        super.Destroy();
    }

    OnSliderUpdateCB(value)
    {
        let the_value = value;
        let display_value = the_value;
        if (this._convert_value_to_display_fn != null)
            display_value = this._convert_value_to_display_fn.Call(the_value);

        if (this._floating_point)
            this._text_input.SetValue(display_value);
        else
            this._text_input.SetValue(display_value);

        if (this._update_callback)
            this._update_callback.Call(value);
    }

    OnTextUpdateCB(value)
    {
        let display_value = value;
        let the_value = display_value;

        if (this._convert_display_to_value_fn != null)
            the_value = this._convert_display_to_value_fn.Call(display_value);

        // Let the slider do the validation
        // the against range
        this._slider.SetValue(the_value);
        the_value = this._slider.GetValue();

        display_value = the_value;
        if (this._convert_value_to_display_fn != null)
            display_value = this._convert_value_to_display_fn.Call(the_value);

        this._text_input.SetValue(display_value);

        if (this._update_callback)
            this._update_callback.Call(the_value);
    }

    SetLabel(label)
    {
        if (this._label)
            this._label.SetValue(label);
    }

    GetValue()
    {
        return this._slider.GetValue();
    }

    SetValue(value)
    {
        var the_value;
        if (this._floating_point)
            the_value = parseFloat(value);
        else
            the_value = parseInt(value);
        
        this._slider.SetValue(the_value);

        let display_value = the_value;
        if (this._convert_value_to_display_fn != null)
            display_value = this._convert_value_to_display_fn.Call(the_value);

        this._text_input.SetValue(display_value);
    }

    Enable(enable)
    {
        if (this._label)
            this._label._paragraph.style.color = enable ? UI._text_colour : UI._disabled_text_colour;

        this._slider.Enable(enable);
        this._text_input.Enable(enable);
    }

    UpdateRange(min, max)
    {
        this._slider._slider.min = min;
        this._slider._slider.max = max;
    }
}

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
import { OJTextControl, TEXT_CONTROL_TYPE } from "./OJL.js";
import { ANCHOR_TYPE } from "./OJL.js";
import { UI, ObjectCallback } from "./OJL.js";

class OJPositionControl extends OJGrid
{
    constructor(parent_element, opts)
    {
        super();
        this._class_name = "OJPositionControl";
        this.SetElementName("OJPositionControl");

        this._update_callback = opts._update_callback ? opts._update_callback : null;
        this._floating_point = opts._floating_point ? opts._floating_point : false;

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

        // X and Y position text inputs
        this._xpos_update_callback = new ObjectCallback(this, "OnXPosUpdateCB");
        this._ypos_update_callback = new ObjectCallback(this, "OnYPosUpdateCB");
        var control_type = this._floating_point ? TEXT_CONTROL_TYPE.TCT_FLOATING_POINT : TEXT_CONTROL_TYPE.TCT_INTEGER;
        this._xpos = new OJTextControl(control_type, 0, this._xpos_update_callback);
        this._ypos = new OJTextControl(control_type, 0, this._ypos_update_callback);

        this._xpos.SetToolTip(opts._tooltip);
        this._xpos.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 0 });
        this._xpos.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 68 });
        this._xpos.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 30 });
        if (this._label)
            this._xpos.SetTopAnchor({ _type: ANCHOR_TYPE.SIBLING_FAR, _sibling: this._label, _fixed_offset: 0 });

        this._ypos.SetToolTip(opts._tooltip);
        this._ypos.SetLeftAnchor({ _type: ANCHOR_TYPE.SIBLING_FAR, _sibling:this._xpos, _fixed_offset: 10 });
        this._ypos.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 68 });
        this._ypos.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 30 });
        if (this._label)
            this._ypos.SetTopAnchor({ _type: ANCHOR_TYPE.SIBLING_FAR, _sibling: this._label, _fixed_offset: 0 });        


        this.AddChild(this._xpos);
        this.AddChild(this._ypos);
    }

    Destroy()
    {
        super.Destroy();
        this._xpos_update_callback.Destroy();
        this._ypos_update_callback.Destroy();
    }

    OnXPosUpdateCB(value)
    {
        let x = value;
        let y = this._floating_point ? parseFloat(this._ypos.GetValue()) : parseInt(this._ypos.GetValue());

        if (this._update_callback)
            this._update_callback._object[this._update_callback._method_name]([x,y]);
    }

    OnYPosUpdateCB(value)
    {
        var x = this._floating_point ? parseFloat(this._xpos.GetValue()) : parseInt(this._xpos.GetValue());
        var y = this._floating_point ? parseFloat(value) : parseInt(value);

        if (this._update_callback)
            this._update_callback._object[this._update_callback._method_name]([x,y]);
    }

    SetLabel(label)
    {
        if (this._label)
            this._label.SetValue(label);
    }

    GetValue()
    {
        var v = this._xpos.GetValue();
        var x = this._floating_point ? parseFloat(v) : parseInt(v);
        var v = this._ypos.GetValue();
        var y = this._floating_point ? parseFloat(v) : parseInt(v);
        return [x,y];
    }

    SetValue(value)
    {
        this._xpos.SetValue(value[0]);
        this._ypos.SetValue(value[1]);
    }

    Enable(enable)
    {
        if (this._label)
            this._label._paragraph.style.color = enable ? UI._text_colour : UI._disabled_text_colour;

        this._xpos.Enable(enable);
        this._ypos.Enable(enable);
    }

    SetIncrement(v)
    {
        this._xpos.SetIncrement(v);
        this._ypos.SetIncrement(v);
    }
}
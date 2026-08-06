/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJModalDialog } from "./OJL.js";
import { OJTextButton } from "./OJL.js";
import { OJGrid } from "./OJL.js";
import { ObjectCallback } from "./OJL.js";
import { ANCHOR_TYPE } from "./OJL.js";

export class OJSettingsDialog extends OJModalDialog
{
    constructor(settings)
    {
        // Base class constructor
        super(374, 290, "Settings");
        this._class_name = "OJSettingsDialog";
        this.SetElementName("OJSettingsDialog");

        this.SetBackgroundColour("#ffffff");

        this._button_panel = new OJGrid();
        this._button_panel.SetBackgroundColour("#eeeeee");
        this._button_panel.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -55 });
        this.AddChild(this._button_panel);

        var close_callback = new ObjectCallback(this, "OnOK");
        this._close_button = new OJTextButton(null, "Close", { _click_callback: close_callback, _alignment: "centre" });
        this._close_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -130 });
        this._close_button.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 10 });
        this._button_panel.AddChild(this._close_button);

        for (var control_container of settings) 
            this.AddChild(control_container, null, null, null, true);
    }
}

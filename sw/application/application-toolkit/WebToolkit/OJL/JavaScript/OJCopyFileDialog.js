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
import { OJServerLink } from "./OJL.js";
import { UI } from "./OJL.js";
import { ANCHOR_TYPE } from "./OJL.js";

export class OJCopyFileDialog extends OJGrid
{
    constructor(params)
    {
        // Base class constructor
        super();
        this._class_name = "OJCopyFileDialog";
        this._window_header_right_edge = 0;
        this.SetElementName("OJCopyFileDialog");
        this.UseWindowHeader("Copying file", { _dialog_header: true });
        this.GetClientAreaStyle().backgroundColor = UI.AddHue("#50");

        this.GetElement()._is_dialog_element = true;

        this._message_area = new OJLabel("Wait for copy");
        this._message_area.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 5 });
        this._message_area.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -5 });
        this._message_area.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 5 });
        this._message_area.SetBottomAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -55 });
        this.AddChild(this._message_area);
        this._message_area.GetElement().focus();
        this._message_area._paragraph.style.maxHeight = "200px";

        this._progress_bar = new OJProgressBarWindow(false);
        this.AddChild(this._progress_bar);
        this._progress_bar.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 5 });
        this._progress_bar.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -5 });
        this._progress_bar.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -55 });
        this._progress_bar.SetBottomAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -30 });

        //this._progress_bar.Show(false);

        this._percentage = new OJLabel("");
        this.AddChild(this._percentage);
        this._percentage.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 5 });
        this._percentage.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 70 });
        this._percentage.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -25 });
        this._percentage.SetBottomAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -5 });
        this._percentage.GetElement().focus();
        this._percentage.GetElement().style.backgroundColor = UI.AddHue("#50");
        this._percentage._paragraph.style.maxHeight = "20px";
        this._percentage.Show(false);
    }

    Destroy()
    {
        super.Destroy();
        UI._current_modal_dialog_class_name = "";
    }

    Show()
    {
        // Make sure all child elements are marked as dialog elements
        UI.SetDialogFlag(this._client_area);

        var body = document.getElementById("_body");
        body.appendChild(this._client_area);
        this.GetClientAreaStyle().zIndex = 1000; // On to of ServerDead overlay
        this.GetHeaderElement().style.zIndex = 1000;

        if (this._window_header != null)
            body.appendChild(this._window_header);

        var width = 600;
        var height = 200;
        if (UI._screen_width < width)
            width = UI._screen_width;
        if (UI._screen_height < height)
            height = UI._screen_height;

        this.Resize((UI._screen_width - width) / 2, (UI._screen_height - height) / 2 - 100, width, height);
        UI._current_modal_dialog_class_name = this._class_name;
    }

    Close(event)
    {
        this.RemoveFromDOM();
        OJServerLink.Get().ExecuteServerCommandWithParams("DialogResult", 0);
        UI._current_modal_dialog_class_name = "";
    }

    UpdateProgress(params)
    {
        let percentage = params._progress;
        this._progress_bar.UpdateProgress(percentage);
        this._progress_bar.Show(true);
        this._percentage.Show(true);
        this._percentage.SetLabel(percentage + "%");
    }
}

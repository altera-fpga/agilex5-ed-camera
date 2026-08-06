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
import { OJModalDialog } from "./OJL.js";
import { UI } from "./OJL.js";

export class OJTextViewDialog extends OJModalDialog
{
    constructor(params)
    {
        // Base class constructor
        super(1440 + 30, 810 + 115, params._title);
        this._class_name = "OJTextViewDialog";
        this.SetElementName("OJTextViewDialog");

        var initial_value = "";
        for (let text_obj of params.Lines)
        {
            var text = text_obj.toString();
            text = text.replaceAll("&amp;", "&");
            text = text.replaceAll("&lt;", "<");
            text = text.replaceAll("&gt;", ">");
            text = text.replaceAll("&quot;", "\"");
            text = text.replaceAll("&apos;", "'");
            initial_value += (text + "\n");
        }

        this._text = new OJTextArea(initial_value);
        this._text.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 0 });
        this._text.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: 0 });
        this._text.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 0 });
        this._text.SetBottomAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: 0 });
        this.AddChild(this._text);
    }
}

export class OJTextArea extends OJWindowElement
{
    constructor(input_text, opts)
    {
        // Base class constructor
        super();
        this._class_name = "OJTextArea";
        this.SetElementName("OJTextArea");
        var text = "" + input_text;

        // Set the class of the client area
        this._client_area.className = "oj_text_area_class";
        this._value = "";
        var class_set = false;
        this._button_callbacks = null;

        this._text_area = document.createElement("textarea");
        this._text_area.rows = 80;
        this._text_area.cols = 80;
        this._text_area.readOnly = true;
        this._text_area.spellcheck = false;
        this._text_area.className = "label_paragraph_class";
        this._text_area.textContent = text;

        var font = UI._console_font_name;
        if (opts != null)
        {
            if (opts._font != null)
            font = opts._font;
        }

        this._text_area.style.font = font;

        this._client_area.appendChild(this._text_area);
    }

    SetColour(colour)
    {
        UI.SetStyleAttribute(this._text_area.style, "color", colour);
    }

    SetBackgroundColour(colour)
    {
        UI.SetStyleAttribute(this._text_area.style, "backgroundColor", colour);
    }

    SetFont(font)
    {
        UI.SetStyleAttribute(this._text_area.style, "font", font);
    }

    Resize(x, y, width, height)
    {
        var size_changed = super.Resize(x, y, width, height);
        OJLib.SetElementPosition(this._text_area, 0, 0, width, height);
        return size_changed;
    }
}

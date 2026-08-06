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
import { OJToolTip } from "./OJL.js";
import { OJLib, UI } from "./OJL.js";

export class OJLabel extends OJWindowElement
{
    constructor(input_text, opts)
    {
        // Base class constructor
        super();
        this._class_name = "OJLabel";
        this.SetElementName("OJLabel");
        let text = "" + input_text;

        // Set the class of the client area
        this._client_area.className = "oj_label_class";
        this._single_line = false;
        this._value = "";
        //this._client_area.style.backgroundColor = background_colour;
        let class_set = false;
        this._button_callbacks = null;

        this._paragraph = document.createElement("p");

        if (opts == null)
            this._paragraph.className = "label_paragraph_class";
        else
        {
            if (opts._transparent_background != null)
                this._client_area.className = "oj_transparent_label_class";

            if (opts._single_line != null)
                this._single_line = opts._single_line;

            if (opts._column_header != null)
                this._client_area.className = "label_column_header";

            if (opts._indent != null)
                this._paragraph.style.textIndent = opts._indent + "px";

            if (opts._font != null)
                this._paragraph.style.font = opts._font;

            if (opts._font_size != null)
                this._paragraph.style.fontSize = opts._font_size + "px";

            if (opts._alignment != null)
            {
                if (opts._alignment == "left")
                {
                    this._paragraph.className = "label_paragraph_left_class";
                    this._client_area.style.textAlign = "left";
                    class_set = true;
                }
                else if (opts._alignment == "right")
                {
                    this._paragraph.className = "label_paragraph_right_class";
                    this._client_area.style.textAlign = "right";
                    class_set = true;
                }
                else if ((opts._alignment == "centre") || (opts._alignment == "center"))
                {
                    this._paragraph.className = "label_paragraph_centre_class";
                    this._client_area.style.textAlign = "center";
                    class_set = true;
                }
            }

            if (opts._background_colour != null)
            {
                let value = (opts._background_colour == "") ? null : opts._background_colour;
                UI.SetStyleAttribute(this._client_area.style, "backgroundColor", value);

                if (!class_set)
                    this._paragraph.className = "label_paragraph_class";

                if (value == null)
                    this._client_area.className = "oj_label_no_background_class";
            }

            if (opts._text_colour != null)
            {
                let value = (opts._text_colour == "") ? null : opts._text_colour;
                UI.SetStyleAttribute(this._client_area.style, "color", value);
            }
        }


        // For pre-formatted text, align to the left
        if ((text != null) && (text.substr(0, 5) == "{pre}"))
            this._client_area.style.textAlign = "left";

        // Special agreement, replace { with < and } with >
        // So we can tunnel html through XML and JSON 
        this.SetLabel(text);
        this._client_area.appendChild(this._paragraph);
    }

    Destroy()
    {
        if (this._button_callbacks != null)
            this._button_callbacks.Destroy();

        super.Destroy();
    }

    SetValue(text)
    {
        this.SetLabel(text);
    }

    GetValue()
    {
        return this._value;
    }

    SetLabel(text)
    {
        this._value = text;
        text = "" + text;
        text = text.replace(/{/g, "<");
        text = text.replace(/}/g, ">");
        this._paragraph.innerHTML = text;
    }

    SetColour(colour)
    {
        UI.SetStyleAttribute(this._paragraph.style, "color", colour);
    }

    SetBackgroundColour(colour)
    {
        UI.SetStyleAttribute(this._paragraph.style, "backgroundColor", colour);
    }

    SetFont(font)
    {
        UI.SetStyleAttribute(this._paragraph.style, "font", font);
    }

    GetParagraphStyle()
    {
        return this._paragraph.style;
    }

    Resize(x, y, width, height)
    {
        let size_changed = super.Resize(x, y, width, height);

        if (this._single_line)
            UI.SetStyleAttribute(this._client_area.style, "width", null);

        return size_changed;
    }

    ManualPositioning()
    {
        this._client_area.className = "";
        this._client_area.style.position = "absolute";
        this._client_area.style.overflow = "visible";
    }

    GetHeight()
    {
        let message_height = UI.GetBoundingClientRect(this._paragraph).height;
        return message_height;
    }

    SetToolTip(tip, show_time)
    {
        super.SetToolTip(tip, show_time);

        if (this._button_callbacks == null)
            this._button_callbacks = OJLib.RegisterButton(this, this._paragraph);
    }

    OnMouseOver(event)
    {
        // Show tool tip even if disabled
        if (this._tool_tip != null)
            OJToolTip.Start(this._tool_tip, this._paragraph);
    }

    OnMouseOut(event)
    {
        this.EndToolTip();
    }

    EndToolTip()
    {
        if (this._tool_tip != null)
            OJToolTip.End(this._tool_tip, this._paragraph);
    }
}

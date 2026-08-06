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


export class OJLogger extends OJWindowElement
{
    constructor(text, opts)
    {
        // Base class constructor
        super();
        this._class_name = "OJLogger";
        this.SetElementName("OJLogger");

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

    Destroy()
    {
        super.Destroy();
    }

    AddMessage(text)
    {
        this._text_area.textContent += text;

        //text = "" + text;
        //text = text.replace(/{/g, "<");
        //text = text.replace(/}/g, ">");
        //this._paragraph.innerHTML = text;
    }    
}
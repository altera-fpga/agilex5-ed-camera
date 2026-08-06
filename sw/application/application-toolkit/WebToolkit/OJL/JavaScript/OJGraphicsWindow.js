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
import { OJGraphics } from "./OJL.js";

export class OJGraphicsWindow extends OJWindowElement
{
    constructor(parent_element, canvas, opts) 
    {
        super(null, opts);
        this._class_name = "OJGraphicsWindow";
        this.SetElementName("OJGraphicsWindow");

        this._display = new OJGraphics(canvas);
        this._parent_element = parent_element;
        this._width = this._display._width;
        this._height = this._display._height;
        this._div = this.GetElement();
        this._div.appendChild(this._display._canvas);
        this._div.style.width = this._width + 'px';
        this._div.style.height = this._height + 'px';
    }

    Destroy()
    {
        super.Destroy();
        this._display.Destroy();
    }

    GetGraphics() 
    {
        return this._display;
    }

    GetDrawContext() 
    {
        return this._display._dc;
    }

    GetCanvas() 
    {
        return this._display._canvas;
    }
}
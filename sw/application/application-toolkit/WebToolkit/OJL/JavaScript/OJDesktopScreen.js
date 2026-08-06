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

export class OJDesktopScreen extends OJGrid
{
    constructor(body)
    {
        // Base class constructor
        super(body);

        this._class_name = "OJDesktopScreen";
        this.SetElementName("_desktop_screen_grid");

        var desktop_screen = this.GetElement();
        desktop_screen.className = "desktop_screen_class";

        if (window.IsMobileDevice())
            this._client_area.style.overflowY = "scroll";
    }

    Destroy()
    {
        super.Destroy();
        UI.RemoveFromParentElement(this._client_area);
    }

    Resize(x, y, width, height)
    {
        if (window.IsMobileDevice())
            width += 40;

        var size_changed = super.Resize(x, y, width, height);

        if (this._fade_div != null)
            OJLib.SetElementPosition(this._fade_div, 0, 0, width, 60);

        return size_changed;
    }
}

///////////////////////////////////////////////////////////////////////////////

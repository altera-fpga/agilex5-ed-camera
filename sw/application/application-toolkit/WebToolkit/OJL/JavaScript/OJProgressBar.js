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

class OJProgressBarWindow extends OJGrid
{
    constructor(use_border)
    {
        // Base class constructor
        super();
        this._class_name = "OJProgressBarWindow";

        this._bar = new OJProgressBar(this._client_area, "generic_progress_class");
        this._bar._progress_panel.style.display = "block";
        this._bar._label = "";
        this._bar._border = 0;

        if (!use_border)
            this._bar._text_colour = OJServerLink.Get()._medium_ui_colour;
        else
            this._bar._text_colour = UI._text_colour;

        this._bar._stroke_colour = UI._text_colour;
    }

    UpdateProgress(progress_percent)
    {
        this._bar.UpdateProgress(progress_percent);
    }
    Resize(x, y, width, height)
    {
        var size_changed = super.Resize(x, y, width, height);
        this._bar.SetSize(width, height);
        return size_changed;
    }

    SetLabel(label)
    {
        this._bar.SetLabel(label);
    }
}

///////////////

class OJProgressBar
{
    constructor(parent_element, use_class, text_colour, text_x)
    {
        var width = 201;
        var height = 30;

        this._progress_panel = document.createElement("canvas");
        if (use_class == null)
            this._progress_panel.className = "generator_progress_class";
        else
            this._progress_panel.className = use_class;

        this._progress_panel.width = width;
        this._progress_panel.height = height;
        this._progress_panel.style.width = width + "px";
        this._progress_panel.style.height = height + "px";
        this._progress_panel.style.display = "none";
        parent_element.appendChild(this._progress_panel);

        var dc = this._progress_panel.getContext("2d");
        dc.clearRect(0, 0, width, height);
        this._have_label = false;
        this._label = "";
        this._progress = 0;
        this._text_colour = "#000000";
        this._stroke_colour = "#505050";
        this._text_x = 7;
        this._border = 10;

        if (text_colour != null)
        {
            this._text_colour = text_colour;
            this._stroke_colour = text_colour;
        }

        if (text_x != null)
            this._text_x = text_x;
    }

    Destroy()
    {
        UI.RemoveFromParentElement(this._progress_panel);
        this._progress_panel = 0;
    }

    SetSize(width, height)
    {
        this._progress_panel.width = width;
        this._progress_panel.height = height;
        this._progress_panel.style.width = width + "px";
        this._progress_panel.style.height = height + "px";
    }

    SetColour(text_colour, stroke_colour)
    {
        this._text_colour = text_colour;
        this._stroke_colour = stroke_colour;
    }

    SetLabel(label)
    {
        if (label != this._label)
        {
            this._have_label = false;
            this._label = label;
            this.UpdateProgress(this._progress);
        }
    }

    UpdateProgress(progress_percent)
    {
        this._progress = progress_percent;
        var bar_offset = (this._label == "") ? 0 : 181;
        var dc = this._progress_panel.getContext("2d");
        dc.setTransform(1, 0, 0, 1, 0, 0); // Reset
        dc.beginPath();
        var panel_width = this._progress_panel.width - 1;
        var panel_height = this._progress_panel.height - 1;
        if ((progress_percent == 0) || (!this._have_label))
        {
            dc.clearRect(0, 0, this._progress_panel.width, this._progress_panel.height);
            this._have_label = false;

            if (progress_percent == 0)
                return;
        }

        dc.clearRect(bar_offset, 0, this._progress_panel.width - bar_offset, this._progress_panel.height);

        if (!this._have_label)
        {
            this._have_label = true;
            dc.font = UI._font_name;
            dc.fillStyle = this._text_colour;
            dc.fillText(this._label, this._text_x, 20);
        }

        var bar_height = panel_height - this._border;
        var bar_width = (panel_width - bar_offset - this._border);
        dc.translate(0.5, 0.5);
        dc.strokeStyle = this._text_colour;
        dc.strokeRect(bar_offset, this._border / 2, bar_width, bar_height);
        dc.stroke();

        dc.beginPath();
        dc.strokeStyle = this._stroke_colour;

        var y = this._border / 2 + 2;

        var marker_width = bar_width - 4;
        var marker_height = bar_height - 4;
        var progress_width = (marker_width * progress_percent) / 100;

        dc.setTransform(1, 0, 0, 1, 0, 0); // Reset
        dc.translate(0.5, 0);

        for (var x = 0; x <= progress_width ; x += 2)
        {
            dc.moveTo(x + bar_offset + 2, y);
            dc.lineTo(x + bar_offset + 2, y + marker_height + 1);
        }
        dc.stroke();
    }
}
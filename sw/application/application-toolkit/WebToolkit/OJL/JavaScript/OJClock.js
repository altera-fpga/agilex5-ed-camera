/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJToolTip } from "./OJL.js";
import { OJLib, UI } from "./OJL.js";

export class OJClock
{
    constructor(parent_element)
    {
        let clock_width = 45;
        let clock_height = 45;
        this._canvas = document.createElement("canvas");
        this._canvas.className = "clock_class";
        this._canvas.width = clock_width;
        this._canvas.height = clock_height;
        this._canvas.style.width = clock_width + "px";
        this._canvas.style.height = clock_height + "px";

        parent_element.appendChild(this._canvas);

        let time_secs = 13 * 60 * 60 + 50 * 60;
        this._button_callbacks = OJLib.RegisterButton(this, this._canvas, null);
        this._angle = 0;
        this._state = false;
    }

    Destroy()
    {
        this._button_callbacks.Destroy();
        UI.RemoveFromParentElement(this._canvas);
        this._canvas = null;
    }

    GetElement()
    {
        return this._canvas;
    }

    Show(state)
    {
        this._canvas.style.display = state ? "block" : "none";
    }

    OnMouseOver(event)
    {
        let now = new Date();
        this._tool_tip = now.toLocaleTimeString();
        OJToolTip.Start(this._tool_tip, this._canvas);
    }

    OnMouseOut(event)
    {
        OJToolTip.End(this._tool_tip, this._canvas);
    }

    TranslateMatrix(matrix, vector)
    {
        let translation_matrix = Matrix.Translation($V([vector[0], vector[1], vector[2]])).Ensure4x4();
        let new_matrix = matrix.x(translation_matrix);
        return new_matrix;
    }

    RotateMatrix(matrix, y_rotation, x_rotation)
    {
        let rotation_matrix = Matrix.RotationY(y_rotation).Ensure4x4();
        let new_matrix_1 = matrix.x(rotation_matrix);

        rotation_matrix = Matrix.RotationX(x_rotation).Ensure4x4();
        let new_matrix_2 = new_matrix_1.x(rotation_matrix);

        return new_matrix_2;
    }

    OnClick(event)
    {
        if (!event.ctrlKey)
            return;

        this._state = !this._state;

        let body = document.getElementById("_body");

        if (this._state)
        {
            body.className = "body_filter";

            // Need to invert the generator_pattern_picon (back to original)
            for (let i = 0; i < _all_picons.length; i++)
            {
                UI.SetStyleAttribute(_all_picons[i]._canvas.style, "filter", "invert(100%)");
                UI.SetStyleAttribute(_all_picons[i]._canvas.style, "WebkitFilter", "invert(100%)");
                UI.SetStyleAttribute(_picture_tile_instance._client_area.style, "filter", "invert(100%)");
                UI.SetStyleAttribute(_picture_tile_instance._client_area.style, "WebkitFilter", "invert(100%)");
            }
        }
        else
        {
            body.className = "";
            for (let i = 0; i < _all_picons.length; i++)
            {
                UI.SetStyleAttribute(_all_picons[i]._canvas.style, "filter", null);
                UI.SetStyleAttribute(_all_picons[i]._canvas.style, "WebkitFilter", null);
                UI.SetStyleAttribute(_picture_tile_instance._client_area.style, "filter", null);
                UI.SetStyleAttribute(_picture_tile_instance._client_area.style, "WebkitFilter", null);
            }
        }
    }

    SetTime(day_seconds)
    {
        this.Render(day_seconds);
    }

    Render(time_secs)
    {
        let dc = this._canvas.getContext("2d");
        dc.setTransform(1, 0, 0, 1, 0, 0); // Reset
        dc.translate(0.5, 0.5);
        dc.clearRect(0, 0, 45, 45);

        let centre_x = 20;
        let centre_y = 20;

        dc.beginPath();
        dc.strokeStyle = "#909090";
        dc.lineWidth = 3;
        dc.lineCap = 'round';
        dc.arc(centre_x, centre_y, 18, 0.0, 2 * Math.PI);
        dc.stroke();
        dc.beginPath();
        dc.lineWidth = 2;
        dc.arc(centre_x, centre_y, 1, 0.0, 2 * Math.PI);
        dc.stroke();


        dc.beginPath();
        dc.strokeStyle = "#808080";
        dc.lineWidth = 2;
        for (let m = 0; m < 60; m += 5)
        {
            dc.beginPath();
            dc.lineWidth = 1;
            let len = 4;
            if ((m == 0) || (m == 30) || (m == 15) || (m == 45))
            {
                len = 5;
                //dc.lineWidth = 2;
            }

            let angle = (Math.PI / 2) - (2 * Math.PI * m) / 60;
            let x1 = centre_x + (16 - len) * Math.cos(angle);
            let y1 = centre_y + (16 - len) * Math.sin(angle);
            let x2 = centre_x + 16 * Math.cos(angle);
            let y2 = centre_y + 16 * Math.sin(angle);

            if (m == 30)
            {
                dc.moveTo(x1 - 1, y1);
                dc.lineTo(x2 - 1, y2);
                dc.moveTo(x1 + 1, y1);
                dc.lineTo(x2 + 1, y2);
            }
            else
            {
                dc.moveTo(x1, y1);
                dc.lineTo(x2, y2);
            }
            dc.stroke();
        }

        let secs = time_secs;
        let int_hour = (time_secs / 3600) | 0;
        time_secs -= int_hour * 3600;
        let int_mins = (time_secs / 60) | 0;
        time_secs -= int_mins * 60;
        let int_secs = time_secs | 0;

        let hour_angle = (Math.PI / 2) - (2 * Math.PI * secs) / 43200;
        let mins_angle = (Math.PI / 2) - (Math.PI * 2 * (secs - int_hour * 3600) / 3600);
        let secs_angle = (Math.PI / 2) - (Math.PI * 2 * (secs - int_hour * 3600 - int_mins * 60) / 60);

        {
            dc.beginPath();
            dc.lineWidth = 3;
            let x1 = centre_x;
            let y1 = centre_y;
            let x2 = centre_x + 11 * Math.cos(hour_angle);
            let y2 = centre_y - 11 * Math.sin(hour_angle);
            dc.moveTo(x1, y1);
            dc.lineTo(x2, y2);
            dc.stroke();
        }
        {
            dc.beginPath();
            dc.lineWidth = 2.5;
            let x1 = centre_x;
            let y1 = centre_y;
            let x2 = centre_x + 15 * Math.cos(mins_angle);
            let y2 = centre_y - 15 * Math.sin(mins_angle);
            dc.moveTo(x1, y1);
            dc.lineTo(x2, y2);
            dc.stroke();
        }
        {
            dc.beginPath();
            dc.lineWidth = 1;
            let x1 = centre_x;
            let y1 = centre_y;
            let x2 = centre_x + 16 * Math.cos(secs_angle);
            let y2 = centre_y - 16 * Math.sin(secs_angle);
            dc.moveTo(x1, y1);
            dc.lineTo(x2, y2);
            dc.stroke();
        }
    }
}



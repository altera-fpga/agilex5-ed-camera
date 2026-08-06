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
import { UI } from "./OJL.js";

export class OJWaiting extends OJWindowElement
{
    constructor(json_picon)
    {
        // Base class constructor
        super();
        this._class_name = "OJWaiting";
        this._svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");
        this._view = document.createElementNS("http://www.w3.org/2000/svg", "g");
        this._svg.appendChild(this._view);
        this.GetElement().appendChild(this._svg);

        this._arms = [];
        this._client_area.style.animation = "spin 5s linear infinite";
    }

    Destroy()
    {
        super.Destroy();
        UI.RemoveFromParentElement(this._canvas);
    }

    Resize(x, y, width, height)
    {
        var size_changed = super.Resize(x, y, width, height);

        this._svg.setAttribute("width", width);
        this._svg.setAttribute("height", height);  

        for (let arm of this._arms)
            this._view.removeChild(arm);

        let stroke_width = (width / 13) | 0;
        let center_x = (width / 2) | 0;
        let center_y = (height / 2) | 0;
        let outer_radius = ((width / 2) | 0) - 10;
        let inner_radius = (outer_radius * 0.55) | 0;
        for (let d = 0; d < 360; d += 30)
        {
            let theta = (d * Math.PI) / 180;
            let cos_theta = Math.cos(theta);
            let sin_theta = Math.sin(theta);
            let outer_x = center_x + cos_theta * outer_radius;
            let outer_y = center_y + sin_theta * outer_radius;
            let inner_x = center_x + cos_theta * inner_radius;
            let inner_y = center_y + sin_theta * inner_radius;
            let arm = document.createElementNS("http://www.w3.org/2000/svg", "line");
            arm.setAttribute("x1", inner_x);
            arm.setAttribute("y1", inner_y);
            arm.setAttribute("x2", outer_x);
            arm.setAttribute("y2", outer_y);        
            arm.setAttribute("y2", outer_y);      
            arm.setAttribute("stroke", "#00c7fd");      
            arm.setAttribute("stroke-width", stroke_width);      
            arm.setAttribute("stroke-linecap", "round");      
            arm.setAttribute("opacity", 1 - (d / 480));      

            this._view.appendChild(arm);
        }

        return size_changed;
    }
}




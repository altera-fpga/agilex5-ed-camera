/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJServerLink } from "./OJLExtras.js";
import { OJWindowElement, ANCHOR_TYPE } from "./OJLExtras.js";
import { OJModalDialog } from "./OJLExtras.js";
import { OJGrid } from "./OJL.js"
import { OJControlContainer, OJControlItemBase } from "./OJLExtras.js";
import { OJLib } from "./OJLExtras.js";
import { wsvg } from "./OJLExtras.js";
import { OJControlItemHiddenValue } from "./OJLExtras.js";
import { OJScrollable } from "./OJLExtras.js";
import { OJLabel } from "./OJLExtras.js";
import { OJWebSocket } from "./OJL.js";


let results_min_height = 280;

export class OJAIResultsControl extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);
        this._ready = false;

        const svgNS = "http://www.w3.org/2000/svg";

        this._svg = document.createElementNS(svgNS, "svg");
        this._svg.setAttribute("width", "480");
        this._svg.setAttribute("height", "270");
        this._svg.setAttribute("viewBox", "0 0 960 540");
        this._svg.style.backgroundColor = "#d8d8de";
        this._svg.style.border = "#b7b7b7";
        this._svg.style.borderWidth = "1px";
        this._svg.style.borderStyle = "solid";
        this.GetElement().appendChild(this._svg);
        
        this._data_transfer_web_socket = new OJWebSocket(OJLib._websocket_server_url, "ai-results", this);
        this._svg.setAttribute("display", "inline");

        this._control_height = 280;

        this._data_transfer_web_socket = null;
        this._results = {};
        this._render_results = {};
        this._rendering = false;
        this._last_results_group = null;

        this._ready = true;
    }

    Destroy()
    {
        super.Destroy();
        this._destroyed = true;
        this._ready = false;
        this._results = {};
        this._render_results = {};
    }

    OnMessageWebSocket(web_socket, message)
    {
        let text = message._text_data;

        let json_object = JSON.parse(text);
        console.log(json_object);
        this._results = json_object;
        if(this._rendering == false)
        {
            this._render_results = this._results;
            this.DrawResults();
        }
    }

    // UpdateValue called by the framework when an update is received
    // from the server
    UpdateValue(value)
    {
        if(value.display == 1)
        {
            this._data_transfer_web_socket = new OJWebSocket(OJLib._websocket_server_url, "ai-results", this);
            this._svg.setAttribute("display", "inline");
        }
        else
        {
            if(this._data_transfer_web_socket != null)
            {
                this._data_transfer_web_socket.Close();
                this._data_transfer_web_socket = null;
            }
            this._svg.setAttribute("display", "none");
        }
    }

    GetHeight()
    {
        return this._control_height;
    }

    GetControlHeight(parent_width, parent_height)
    {
        if (parent_width == null)
            return 0;

        // Style border adds a pixels either side
        let outer_border = 13;
        let inner_border = 10;
        let control_width = parent_width - (2 * outer_border) - (2 * inner_border);
        let control_height = (control_width * 9) / 16 + (2 * inner_border);
        this._control_height = (control_height + 0.5) | 0;
        return this._control_height;
    }

    Resize(x, y, width, height)
    {
        let size_changed = super.Resize(x, y, width, height);

        // 1 Pixel border on either side
        this._svg.setAttribute("width", width - 2);
        this._svg.setAttribute("height", height - 2);

        this.DrawResults();

        return size_changed;
    }

    async DrawResults()
    {
        this._rendering = true;
        
        let width = this._svg.getAttribute("width");
        let height = this._svg.getAttribute("height");

        if(this._last_results_group != null)
        {
            this._svg.removeChild(this._last_results_group);
            this._last_results_group = null;
        }
        const svgNS = "http://www.w3.org/2000/svg";
        let results_group = document.createElementNS(svgNS, "g");

        let results = this._render_results;
        if(results != null)
        {
            if('items' in results)
            {
                if(results.networkTypeString == "YOLOV8N")
                {
                    let result_count = results.items.length;
                    for(let i = 0; i < result_count; i++)
                    {
                        let item = results.items[i];
                        let x = item.xMin;
                        let y = item.yMin;
                        let w = item.xMax - item.xMin;
                        let h = item.yMax - item.yMin;

                        let text = document.createElementNS(svgNS, "text");
                        text.setAttribute("x", x);
                        text.setAttribute("y", y - 2);
                        text.setAttribute("fill", item.colour);
                        text.setAttribute("font-size", "16");
                        text.textContent = `${item.categoryName} (${(item.score * 100).toFixed(1)}%)`;
                        results_group.appendChild(text);

                        let path = document.createElementNS(svgNS, "path");
                        let d = `M ${x} ${y} H ${x + w} V ${y + h} H ${x} Z`;
                        path.setAttribute("d", d);
                        path.setAttribute("fill", "none");
                        path.setAttribute("stroke", item.colour);
                        path.setAttribute("stroke-width", "1");
                        results_group.appendChild(path);
                    }
                }
                else if(results.networkTypeString == "YOLOV8N_POSE")
                {
                    let result_count = results.items.length;
                    for(let i = 0; i < result_count; i++)
                    {
                        let item = results.items[i];

                        if('keypoints' in item)
                        {
                            const skeleton_connections = [
                                [3, 1], [1, 0], [4, 2], [2, 0],
                                [0, 5], [0, 6], [5, 7], [6, 8],
                                [7, 9], [8, 10], [5, 11], [6, 12],
                                [11, 12], [11, 13], [12, 14],
                                [13, 15], [14, 16]
                            ];

                            for(let connection of skeleton_connections)
                            {
                                let first_keypoint = item.keypoints[connection[0]];
                                let second_keypoint = item.keypoints[connection[1]];
                                if((first_keypoint.visibility >= 0.5) &&
                                   (second_keypoint.visibility >= 0.5))
                                {
                                    let path = document.createElementNS(svgNS, "path");
                                    let d = `M ${first_keypoint.x} ${first_keypoint.y} L ${second_keypoint.x} ${second_keypoint.y}`;
                                    path.setAttribute("d", d);
                                    path.setAttribute("fill", "none");
                                    path.setAttribute("stroke", item.colour);
                                    path.setAttribute("stroke-width", "3");
                                    path.setAttribute("stroke-linecap", "round");
                                    results_group.appendChild(path);
                                }
                            }
                        }
                    }
                }
            }
        }

        this._svg.appendChild(results_group);

        this._last_results_group = results_group;

        this._rendering = false;
    }

    static CreateCustomControl(element_type, params)
    {
        if (element_type == "OJAIResultsControl")
        {
            return new OJAIResultsControl(params);
        }
    }
}

// Register our control factory fn for the OJAIResultsControl custom control
OJControlContainer.AddCustomControlFactory("OJAIResultsControl", OJAIResultsControl.CreateCustomControl);

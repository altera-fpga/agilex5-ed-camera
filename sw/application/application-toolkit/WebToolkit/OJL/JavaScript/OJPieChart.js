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
import { OJGrid } from "./OJL.js";
import { UI } from "./OJL.js";

class OJPieValue
{
    constructor(value, label, colour, used_value)
    {
        this._value = value;
        this._label = label;
        this._colour = colour;
        this._angle = 0;

        this._used_value = used_value;
        this._used_angle = 0;
    }
}

class OJPieChart extends OJWindowElement
{
    constructor(title)
    {
        // Base class constructor
        super();
        this._class_name = "OJPieChart";
        this._title = title;
        this._pie_canvas = document.createElement("canvas");
        this._client_area.appendChild(this._pie_canvas);
        this._values = [];
    }

    Destroy()
    {
        this._values.length = 0;
        super.Destroy();
        UI.RemoveFromParentElement(this._pie_canvas);
    }

    AddValue(value, label, colour, used_value)
    {
        this._values.push(new OJPieValue(value, label, colour, used_value));
    }

    ClearValues()
    {
        this._values.length = 0;
    }

    Resize(x, y, width, height)
    {
        var size_changed = super.Resize(x, y, width, height);

        this._pie_canvas.width = width;
        this._pie_canvas.height = height;
        this._pie_canvas.style.width = width + "px";
        this._pie_canvas.style.height = height + "px";

        this.Repaint();
        return size_changed;
    }

    Repaint()
    {
        if ((this._width * this._height) <= 0)
            return;

        var total = 0;
        for (var i = 0; i < this._values.length; i++)
            total += this._values[i]._value;

        for (var i = 0; i < this._values.length; i++)
        {
            var angle = 2 * Math.PI * (this._values[i]._value / total);
            this._values[i]._angle = angle;

            if (this._values[i]._used_value != null)
            {
                var angle = 2 * Math.PI * (this._values[i]._used_value / total);
                this._values[i]._used_angle = angle;
            }
        }

        var dest_dc = this._pie_canvas.getContext("2d");
        dest_dc.setTransform(1, 0, 0, 1, 0, 0); // Reset

        var centre_x = parseInt(this._width / 2);
        var centre_y = parseInt(this._height / 2);
        var radius = Math.min(centre_x - 10, centre_y - 10);
        centre_y += 20;

        dest_dc.clearRect(0, 0, this._width, this._height);

        dest_dc.fillStyle = UI._text_colour;
        dest_dc.font = UI._font_name;
        var text_width = dest_dc.measureText(this._title).width;
        var text_y = centre_y - radius - 20;
        dest_dc.fillText(this._title, centre_x - (text_width / 2), text_y);

        var start_angle = 0;
        for (var i = 0; i < this._values.length; i++)
        {
            var slice = this._values[i];

            var end_angle = start_angle + slice._angle;
            dest_dc.beginPath();
            dest_dc.moveTo(centre_x, centre_y);
            dest_dc.arc(centre_x, centre_y, radius, start_angle, end_angle, false);
            dest_dc.fillStyle = slice._colour;
            dest_dc.fill();
            dest_dc.closePath();

            if (slice._used_angle > 0)
            {
                dest_dc.beginPath();
                var used_end_angle = start_angle + slice._used_angle;
                //dest_dc.moveTo(centre_x, centre_y);
                dest_dc.arc(centre_x, centre_y, radius, start_angle, used_end_angle, false);
                dest_dc.arc(centre_x, centre_y, radius / 2, used_end_angle, start_angle, true);
                dest_dc.fillStyle = "rgba(255, 255, 255, 0.20)";
                dest_dc.fill();
                dest_dc.closePath();
            }

            dest_dc.beginPath();
            dest_dc.moveTo(centre_x, centre_y);
            dest_dc.arc(centre_x, centre_y, radius, start_angle, end_angle, false);
            dest_dc.lineWidth = 2;
            dest_dc.strokeStyle = UI._text_colour;
            dest_dc.closePath();
            dest_dc.stroke();

            start_angle = end_angle;
        }
    }
}

///////////////

class OJPieKey extends OJWindowElement
{
    constructor()
    {
        // Base class constructor
        super();
        this._class_name = "OJPieKey";
        this._pie_key_canvas = document.createElement("canvas");
        this._client_area.appendChild(this._pie_key_canvas);

        this._storage_used = 0;
        this._storage_available = 0;
        this._generator_allocated = 0;
        this._generator_used = 0;
        this._capture_allocated = 0;
        this._capture_used = 0;
    }

    Destroy()
    {
        super.Destroy();
        UI.RemoveFromParentElement(this._pie_key_canvas);
    }

    Resize(x, y, width, height)
    {
        var size_changed = super.Resize(x, y, width, height);

        this._pie_key_canvas.width = width;
        this._pie_key_canvas.height = height;
        this._pie_key_canvas.style.width = width + "px";
        this._pie_key_canvas.style.height = height + "px";

        this.Repaint();
        return size_changed;
    }

    Update(storage_available,
           storage_used,
           generator_allocated,
           generator_used,
           capture_allocated,
           capture_used)    
    {
        this._storage_used = storage_used;
        this._storage_available = storage_available;
        this._generator_allocated = generator_allocated;
        this._generator_used = generator_used;
        this._capture_allocated = capture_allocated;
        this._capture_used = capture_used;

        this.Repaint();
    };

    Repaint()
    {
        var dest_dc = this._pie_key_canvas.getContext("2d");
        dest_dc.setTransform(1, 0, 0, 1, 0, 0); // Reset

        dest_dc.clearRect(0, 0, this._width, this._height);
        var block_x = 10;
        var text_x = 55;
        var y = 25;
        var text_height = 22;
        var block_height = 14;

        var deets = [];

        dest_dc.fillStyle = UI._text_colour;
        dest_dc.font = "italic " +_font_name;
        dest_dc.fillText("Generator", text_x, y);
        y += text_height;

        dest_dc.font = UI._font_name;
        dest_dc.fillText("Allocated :" + this._generator_allocated + " MiB", text_x, y);
        deets.push({ _y: y, _colour: "#666728" })
        y += text_height;

        dest_dc.fillStyle = UI._text_colour;
        dest_dc.fillText("Used :" + this._generator_used + " MiB", text_x, y);
        deets.push({ _y: y, _colour: "#8e8f56" })
        y += text_height;

        y += 5;
        dest_dc.font = "italic " + _font_name;
        dest_dc.fillStyle = UI._text_colour;
        dest_dc.fillText("Capture", text_x, y);
        y += text_height;

        dest_dc.font = UI._font_name;
        dest_dc.fillStyle = UI._text_colour;
        dest_dc.fillText("Allocated :" + this._capture_allocated + " MiB", text_x, y);
        deets.push({ _y: y, _colour: "#355174" })
        y += text_height;

        dest_dc.fillStyle = UI._text_colour;
        dest_dc.fillText("Used :" + this._capture_used + " MiB", text_x, y);
        deets.push({ _y: y, _colour: "#5d738f" })
        y += text_height;

        y += 5;
        dest_dc.font = "italic " + _font_name;
        dest_dc.fillStyle = UI._text_colour;
        dest_dc.fillText("Storage", text_x, y);
        y += text_height;

        dest_dc.font = UI._font_name;
        dest_dc.fillStyle = UI._text_colour;
        dest_dc.fillText("Used :" + this._storage_used + " MiB", text_x, y);
        deets.push({ _y: y, _colour: "#507070" })
        y += text_height;

        dest_dc.fillStyle = UI._text_colour;
        dest_dc.fillText("Available :" + this._storage_available +" MiB", text_x, y);
        deets.push({ _y: y, _colour: "#505070" })
        y += text_height;

        dest_dc.lineWidth = 1;
        dest_dc.strokeStyle = UI._text_colour;
        dest_dc.translate(0.5, 0.5);
        for (var i = 0; i < deets.length; i++)
        {
            dest_dc.fillStyle = deets[i]._colour;
            dest_dc.fillRect(block_x, deets[i]._y - block_height, 30, block_height);
            dest_dc.strokeRect(block_x, deets[i]._y - block_height, 30, block_height);
        }

        deets.length = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

class OJPiesPanel extends OJGrid
{
    constructor(title)
    {
        // Base class constructor
        super();
        this._class_name = "OJPiesPanel";

        this.UseWindowHeader("Storage");

        var pie_height = 430;
        this.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: pie_height });

        this._storage_usage = new OJPieChart("Storage");
        this._memory_usage = new OJPieChart("Memory");
        this._pie_key = new OJPieKey();
        this._storage_usage.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 10 });
        this._storage_usage.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _dynamic_position: 50 });
        this._storage_usage.SetBottomAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 170 });
        this._memory_usage.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _dynamic_position: 50 });
        this._memory_usage.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -10 });
        this._memory_usage.SetBottomAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 170 });
        this._pie_key.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 170 });
        this._pie_key.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 30 });

        this.AddChild(this._storage_usage);
        this.AddChild(this._memory_usage);
        this.AddChild(this._pie_key);
    }

    Update(storage_used, storage_available,
           generator_allocated, generator_used,
           capture_allocated, capture_used)
    {
        this._storage_usage.ClearValues();
        this._storage_usage.AddValue(storage_used, "Used", "#507070");
        this._storage_usage.AddValue(storage_available, "Available", "#505070");
        this._storage_usage.Repaint();

        this._memory_usage.ClearValues();
        this._memory_usage.AddValue(generator_allocated, "Used", "#73742d", generator_used);
        this._memory_usage.AddValue(capture_allocated, "Available", "#355174", capture_used);
        this._memory_usage.Repaint();

        this._pie_key.Update(storage_available,
            storage_used,
            generator_allocated,
            generator_used,
            capture_allocated,
            capture_used);
    }
}
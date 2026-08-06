/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { UI } from "./OJL.js";

export class OJGraphics
{
    constructor(canvas)
    {
        this._canvas = canvas;
        this._dc = canvas.getContext("2d");
        this._dc.font = UI._font_name;
        this._line_colour = UI._text_colour;
        this._fill_colour = UI._text_colour;
        this._dx = 0;
        this._dy = 0;
        this._x_scale = 1;
        this._y_scale = 1;
        this._x_scale_centre = 0;
        this._y_scale_centre = 0;
        this.UpdateTransform();
        this._using_dash_pattern = null;
        this._width = this._canvas.width;
        this._height = this._canvas.height;
        this._rect_offset = 0.5;
        this._dash_supported = (this._dc.setLineDash != null);
    }

    Destroy()
    {
    }

    GetWidth()
    {
        return this._width;
    }

    GetHeight()
    {
        return this._height;
    }

    SetDirty(state)
    {
        this._canvas._dirty = state;
    }

    IsDirty()
    {
        if (this._canvas._dirty == null)
            return true;
        else
            return this._canvas._dirty;
    }

    UpdateTransform(dx, dy)
    {
        if (arguments.length == 0)
            this._dc.setTransform(this._x_scale, 0, 0, this._y_scale, this._dx - this._x_scale_centre + this._rect_offset, this._dy - this._y_scale_centre + this._rect_offset);
        else
            this._dc.setTransform(this._x_scale, 0, 0, this._y_scale, this._dx - this._x_scale_centre + dx, this._dy - this._y_scale_centre + dy);
    }

    Translate(dx, dy)
    {
        this._dx = (dx + 0.5) | 0;
        this._dy = (dy + 0.5) | 0;
        this.UpdateTransform();
    }

    Zoom(scale, centre_x, centre_y)
    {
        this._x_scale = scale;
        this._y_scale = scale;
        this._x_scale_centre = centre_x;
        this._y_scale_centre = centre_y;
        this.UpdateTransform();
    }

    ResetTransform()
    {
        this._dx = 0;
        this._dy = 0;
        this.UpdateTransform();
    }

    FillRectangle(x, y, width, height, colour)
    {
        let use_colour = (colour == null) ? this._fill_colour : colour;
        this._dc.fillStyle = use_colour;
        this.UpdateTransform(0, 0);
        this._dc.fillRect(x, y, width, height);
        this.UpdateTransform();
    }

    DrawRectangle(x, y, width, height, colour, lineWidth)
    {
        let use_colour = (colour == null) ? this._line_colour : colour;
        this._dc.strokeStyle = use_colour;
        this._dc.lineWidth = (lineWidth) ? lineWidth : 1;
        this._dc.strokeRect(x, y, width - 1, height - 1);
    }

    FillColour(colour)
    {
        this._dc.fillStyle = colour;
        this._dc.fillRect(0, 0, this._width, this._height);
    }

    ClearRectangle(x, y, width, height)
    {
        this._dc.clearRect(x, y, width, height);
    }

    Clear()
    {
        this._dc.clearRect(0, 0, this._width, this._height);
    }

    CreateImageData(width, height)
    {
        width = width || this._width;
        height = height || this._height;
        let image_data = this._dc.createImageData(width, height);
        return image_data;
    }

    GetImageData(width, height)
    {
        width = width || this._width;
        height = height || this._height;
        let image_data = this._dc.getImageData(0, 0, width, height);
        return image_data;
    }

    GetImageDataFrom(x, y, width, height)
    {
        width = width || this._width;
        height = height || this._height;
        let image_data = this._dc.getImageData(x, y, width, height);
        return image_data;
    }

    PutImageData(image_data, x, y)
    {
        x = x || 0;
        y = y || 0;
        this._dc.putImageData(image_data, x, y);
    }

    DrawLine(x1, y1, x2, y2, colour)
    {
        let set_transform = false;

        let from_x = (x1 + 0.5) | 0;
        let from_y = (y1 + 0.5) | 0;
        let to_x = (x2 + 0.5) | 0;
        let to_y = (y2 + 0.5) | 0;

        if (this._using_dash_pattern != null)
        {
            if (from_x == to_x)
                this.UpdateTransform(0.5, 0);
            else if (from_y == to_y)
                this.UpdateTransform(0, 0.5);

            set_transform = true;
        }

        let use_colour = (colour == null) ? this._line_colour : colour;
        this._dc.beginPath();
        this._dc.strokeStyle = use_colour;

        if (this._dash_supported || (this._using_dash_pattern == null))
        {
            if (this._dash_supported)
            {
                this._dc.moveTo(from_x, from_y);
                this._dc.lineTo(to_x, to_y);
            }
            else
            {
                // Bug in local WebKit, doesn't like drawing in all directions
                this._dc.moveTo(from_x, from_y);
                this._dc.lineTo(to_x, to_y);

                this._dc.moveTo(to_x, to_y);
                this._dc.lineTo(from_x, from_y);
            }
        }
        else
        {
            let start_x = Math.min(from_x, to_x);
            let end_x = Math.max(from_x, to_x);
            let start_y = Math.min(from_y, to_y);
            let end_y = Math.max(from_y, to_y);

            let dx = end_x - start_x;
            let dy = end_y - start_y;

            if (dx > 0)
            {
                for (let x = start_x; x < end_x;)
                {
                    this._dc.moveTo(x, start_y);
                    this._dc.lineTo(x + this._using_dash_pattern[0], start_y);

                    x += (this._using_dash_pattern[0] + this._using_dash_pattern[1]);
                }
            }
            else if (dy > 0)
            {
                for (let y = start_y; y < end_y;)
                {
                    this._dc.moveTo(start_x, y);
                    this._dc.lineTo(start_x, y + this._using_dash_pattern[0]);

                    y += (this._using_dash_pattern[0] + this._using_dash_pattern[1]);
                }
            }
            else
            {
                // Can't do diagonal
                this._dc.moveTo(start_x, start_y);
                this._dc.lineTo(end_x, end_y);
            }
        }

        this._dc.stroke();

        if (set_transform)
            this.UpdateTransform();
    }

    // "center|end|left|right|start"
    SetTextAlignment(alignment)
    {
        this._dc.textAlign = alignment;
    }

    // "alphabetic|top|hanging|middle|ideographic|bottom"
    SetLineAlignment(alignment)
    {
        this._dc.textBaseline = alignment;
    }

    SetFont(font)
    {
        this._dc.font = font;
    }

    DrawString(string, x, y, colour)
    {
        let use_colour = (colour == null) ? this._fill_colour : colour;
        this._dc.fillStyle = use_colour;
        this._dc.fillText(string, x, y);
    }

    DrawRotatedString(string, x, y, angle_degrees, colour)
    {
        let use_colour = (colour == null) ? this._fill_colour : colour;
        this._dc.save();
        this._dc.translate(x, y);
        this._dc.rotate((angle_degrees * Math.PI) / 180);
        this._dc.fillStyle = use_colour;
        this._dc.fillText(string, 0, 0);
        this._dc.restore();
    }

    MeasureString(string)
    {
        let size = UI.MeasureText(this._dc, string);
        return size;
    }

    SetLineColour(colour)
    {
        this._line_colour = colour;
    }

    SetFillColour(colour)
    {
        this._fill_colour = colour;
    }

    SetDashPattern(pattern)
    {
        if (pattern == null)
        {
            if (this._dc.setLineDash)
                this._dc.setLineDash([]);
        }
        else
        {
            if (this._dc.setLineDash)
                this._dc.setLineDash(pattern);
        }

        this._using_dash_pattern = pattern;
    }

    ToRadians(degrees)
    {
        return (degrees * Math.PI) / 180.0;
    }

    ToDegrees(radians)
    {
        return (180.0 * radians) / Math.PI;
    }

    DrawArc(rectangle, from_angle, sweep_angle) // In degrees
    {
        this._dc.beginPath();
        this._dc.strokeStyle = this._line_colour;

        let radius = Math.min(rectangle._width, rectangle._height) / 2;
        let centre_x = (rectangle._x + rectangle._width / 2 + 0.5) | 0;
        let centre_y = (rectangle._y + rectangle._height / 2 + 0.5) | 0;

        let to_angle = from_angle + sweep_angle;
        let from_radians = this.ToRadians(from_angle);
        let to_radians = this.ToRadians(to_angle);

        this._dc.arc(centre_x, centre_y, radius, from_radians, to_radians);
        this._dc.stroke();
    }

    FillArc(rectangle, from_angle, sweep_angle, colour) // In degrees
    {
        let use_colour = (colour == null) ? this._fill_colour : colour;
        this._dc.fillStyle = use_colour;

        this._dc.beginPath();
        this._dc.strokeStyle = use_colour;

        let radius = Math.min(rectangle._width, rectangle._height) / 2;
        let centre_x = (rectangle._x + rectangle._width / 2 + 0.5) | 0;
        let centre_y = (rectangle._y + rectangle._height / 2 + 0.5) | 0;

        let to_angle = from_angle + sweep_angle;
        let from_radians = this.ToRadians(from_angle);
        let to_radians = this.ToRadians(to_angle);

        this._dc.arc(centre_x, centre_y, radius, from_radians, to_radians);
        this._dc.stroke();
        this._dc.fill();
    }

    SetLineWidth(line_width)
    {
        this._dc.lineWidth = line_width;
    }

    // points is an array of PointF
    DrawLines(points, colour)
    {
        if (colour != null)
            this.SetLineColour(colour);

        let last_index = points.length - 1;
        for (let i = 0; i < last_index; i++)
        {
            let first = points[i];
            let second = points[i + 1];
            this.DrawLine(first._x, first._y, second._x, second._y);
        }
    }

    DrawEllipse(rectangle, colour)
    {
        if (colour != null)
            this.SetLineColour(colour);

        this.DrawArc(rectangle, 0, 360);
    }

    // points is an array of OJPointF
    FillPolygon(points, colour)
    {
        if (points.length < 3)
            return;
        let use_colour = (colour == null) ? this._fill_colour : colour;
        this._dc.fillStyle = use_colour;
        this._dc.beginPath();
        this._dc.moveTo(points[0]._x, points[0]._y);
        for (let i = 1; i < points.length; i++)
            this._dc.lineTo(points[i]._x, points[i]._y);
        this._dc.closePath();
        this._dc.fill();
    }

    SetBlendingMode(mode)
    {
        this._dc.globalCompositeOperation = mode;
    }

    Clip(x, y, width, height)
    {
        this._dc.save();
        this._dc.rect(x, y, width, height);
        this._dc.clip();
    }

    ClipRectangle(rectangle)
    {
        this._dc.save();
        this._dc.rect(rectangle._x, rectangle._y,
                    rectangle._width, rectangle._height);
        this._dc.clip();
    }

    ResetClip()
    {
        this._dc.restore();
    }

    DrawImage(image_canvas,
              destination_rect, // Use OJRect
              source_x, source_y,
              source_width, source_height)    
    {
        this._dc.drawImage(image_canvas, source_x, source_y,
                           source_width, source_height,
                           destination_rect._x,
                           destination_rect._y,
                           destination_rect._width,
                           destination_rect._height);
    };

    UpdateDimensions(width, height) 
    {
        let w = width || this._canvas.width;
        let h = height || this._canvas.height;

        this._canvas.width = w;
        this._canvas.height = h;

        this._width = w;
        this._height = h;
    }

    SetGlobalAlpha(alpha) 
    {
        this._dc.globalAlpha = alpha;
    }
}


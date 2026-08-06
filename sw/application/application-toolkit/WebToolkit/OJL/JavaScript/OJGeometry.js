/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
"use strict";

// OJRect - rectangle with integer coordinates and dimensions
export class OJRect
{
    constructor(x, y, width, height)
    {
        x = x || 0;
        y = y || 0;
        width = width || 0;
        height = height || 0;
        this._x = (x | 0);
        this._y = (y | 0);
        this._width = (width >= 0) ? (width | 0) : 0;
        this._height = (height >= 0) ? (height | 0) : 0;
    }

    Offset(dx, dy)
    {
        this._x += (dx | 0);
        this._y += (dy | 0);
    }

    Inflate(dx, dy)
    {
        this._x -= (dx | 0);
        this._y -= (dy | 0);
        this._width += ((dx | 0) * 2);
        this._height += ((dy | 0) * 2);
    }
    Deflate(dx, dy)
    {
        this._x += (dx | 0);
        this._y += (dy | 0);
        this._width  -= ((dx | 0) * 2);
        this._height -= ((dy | 0) * 2);
    }
    Area()
    {
        return (this._width * this._height);
    }

    IsEmpty()
    {
        var area = this.Area();
        return (area == 0);
    }

    Left()
    {
        return this._x;
    }

    Right()
    {
        return (this._x + this._width);
    }

    Top()
    {
        return this._y;
    }

    Bottom()
    {
        return (this._y + this._height);
    }

    Clone()
    {
        return new OJRect(this._x, this._y, this._width, this._height);
    }

    ToString()
    {
        return "(" + this._x + "," + this._y + " " + this._width + "x" + this._height + ")";
    }

    IntersectsWith(other_rectangle)
    {
        return (this.Left() < other_rectangle.Right() &&
                this.Top() < other_rectangle.Bottom() &&
                this.Right() > other_rectangle.Left() &&
                this.Bottom() > other_rectangle.Top());
    }

    Minus(other)
    {
        var rects = [];

        var b = other.Clone();

        // Clip top
        if (b._y < this._y)
        {
            var bottom = b.Bottom();
            b._y = this._y;
            b._height = bottom - b._y;
            if (b._height < 0)
                b._height = 0;
        }

        // Clip bottom
        if (b.Bottom() > this.Bottom())
        {
            var b_bottom = this.Bottom();
            b._height = b_bottom - b._y;
            if (b._height < 0)
                b._height = 0;
        }

        // Clip left
        if (b._x < this._x)
        {
            var right = b.Right();
            b._x = this._x;
            b._width = right - b._x;
            if (b._width < 0)
                b._width = 0;
        }

        // Clip right
        if (b.Right() > this.Right())
        {
            var b_right = this.Right();
            b._width = b_right - b._x;
            if (b._width < 0)
                b._width = 0;
        }

        if (b.IsEmpty())
        {
            if (!this.IsEmpty())
            {
                var clone = this.Clone();
                rects.push(clone);
            }
            return rects;
        }

        var rect_1 = new OJRect(this._x, this._y, this._width, b._y - this._y);
        if (!rect_1.IsEmpty())
            rects.push(rect_1);

        var rect_2 = new OJRect(this._x, b._y, b._x - this._x, b._height);
        if (!rect_2.IsEmpty())
            rects.push(rect_2);

        var rect_3 = new OJRect(b.Right(), b._y, this.Right() - b.Right(), b._height);
        if (!rect_3.IsEmpty())
            rects.push(rect_3);

        var rect_4 = new OJRect(this._x, b.Bottom(), this._width, this.Bottom() - b.Bottom());
        if (!rect_4.IsEmpty())
            rects.push(rect_4);

        return rects;
    }

    Contains(x, y)
    {
        var r = this.Right();
        var b = this.Bottom();
        return ((x >= this._x) && (x < r) && (y >= this._y) && (y < b));
    }
}

// OJRectF - rectangle with floating point coordinates and dimensions
export class OJRectF
{
    constructor(x, y, width, height)
    {
        x = x || 0;
        y = y || 0;
        width = width || 0;
        height = height || 0;
        this._x = x;
        this._y = y;
        this._width = (width >= 0) ? width : 0;
        this._height = (height >= 0) ? height : 0;
    }

    Offset(dx, dy)
    {
        this._x += dx;
        this._y += dy;
    }

    Inflate(dx, dy)
    {
        this._x -= dx;
        this._y -= dy;
        this._width += (dx * 2);
        this._height += (dy * 2);
    }

    Deflate(dx, dy)
    {
        this._x += dx;
        this._y += dy;
        this._width -= (dx * 2);
        this._height -= (dy * 2);
    }

    Area()
    {
        return (this._width * this._height);
    }

    IsEmpty()
    {
        var area = this.Area();
        return (area == 0);
    }

    Left()
    {
        return this._x;
    }

    Right()
    {
        return (this._x + this._width);
    }

    Top()
    {
        return this._y;
    }

    Bottom()
    {
        return (this._y + this._height);
    }

    Clone()
    {
        return new OJRect(this._x, this._y, this._width, this._height);
    }

    ToString()
    {
        return "(" + this._x + "," + this._y + " " + this._width + "x" + this._height + ")";
    }

    IntersectsWith(other_rectangle)
    {
        return (this.Left() < other_rectangle.Right() &&
                this.Top() < other_rectangle.Bottom() &&
                this.Right() > other_rectangle.Left() &&
                this.Bottom() > other_rectangle.Top());
    }
}

// OJPoint - point with integer coordinates;
export class OJPoint
{
    constructor(x, y)
    {
        x = x || 0;
        y = y || 0;
        this._x = (x | 0);
        this._y = (y | 0);
    }
}

// OJPointF - point with floating point coordinates;
export class OJPointF
{
    constructor(x, y)
    {
        x = x || 0;
        y = y || 0;
        this._x = x;
        this._y = y;
    }
}
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
import { OJLib } from "./OJL.js";
import { TEXT_CONTROL_TYPE } from "./OJL.js";
import { OJRect } from "./OJL.js";

export class OJGrid extends OJWindowElement
{
    constructor(parent_element, label, header_opts, opts)
    {
        // Base class constructor
        super(parent_element, opts);
        this._class_name = "OJGrid";
        this.SetElementName("OJGrid");

        this._h_border = 0;
        this._v_border = 0;
        this._circular_dependency = false;
        this._use_anchor_positions = true;

        if (label != null)
            this.UseWindowHeader(label, header_opts);

        if ((header_opts != null) && (header_opts._corner_radius != null))
        {
            let corner_radius = header_opts._corner_radius;
            this._client_area.style.borderBottomLeftRadius = corner_radius + "px";
            this._client_area.style.borderBottomRightRadius = corner_radius + "px";
        }		

        if ((opts != null) && (opts._corner_radius != null))
        {
            let corner_radius = opts._corner_radius;
            this._client_area.style.borderRadius = corner_radius + "px";
        }		
    }

    SetBorderWidth(h_border, v_border)
    {
        this._h_border = h_border;
        this._v_border = v_border;
    }

    SetBorders(left, top, right, bottom)
    {
        this._client_area.style.borderLeftWidth = left + "px";
        this._client_area.style.borderRightWidth = top + "px";
        this._client_area.style.borderTopWidth = right + "px";
        this._client_area.style.borderBottomWidth = bottom + "px";
        this._h_border = (left + right);
        this._v_border = (top + bottom);
        this._client_area.style.borderStyle = "solid";
        this._client_area.style.borderColor = "#c0c0c0";
    }

    Show(show)
    {
        let show_changed = (show != this._shown);
        super.Show(show);

        if (show_changed && (this._parent_window_element != null))
        {
            if ("PositionChildren" in this._parent_window_element)
                this._parent_window_element.PositionChildren();
        }
    }

    GetHeight()
    {
        if (!this._shown)
            return 0;

        if (this._bottom_anchor._type == ANCHOR_TYPE.FIXED_SIZE)
        {
            // We have a fixed height
            return this._bottom_anchor._fixed_size;
        }

        // If we are attached to our parent top and bottom, they this gives 
        // us our height, not the height of our children (which could be greater,
        // eg a scrollable child)
        let top_attached_to_parent = ((this._top_anchor._type == ANCHOR_TYPE.PARENT_NEAR) || (this._top_anchor._type == ANCHOR_TYPE.PARENT_FAR));
        let bottom_attached_to_parent = ((this._bottom_anchor._type == ANCHOR_TYPE.PARENT_NEAR) || (this._bottom_anchor._type == ANCHOR_TYPE.PARENT_FAR));

        if (top_attached_to_parent && bottom_attached_to_parent)
            return this._height;

        if (this._bottom_anchor._type != ANCHOR_TYPE.FIT_CHILDREN)
            return this._height;

        let largest_bottom = 0;
        for (let i = 0; i < this._children.length; i++)
        {
            // Clear flag for circular dependency detection
            this.ClearCircularFlags();

            let child = this._children[i];
            let rect = this.CalculateChildPosition(child);

            if (this._circular_dependency)
            {
                OJLib.Trace("Circular dependency");
                return 0;
            }

            if (rect.Bottom() > largest_bottom)
                largest_bottom = rect.Bottom();
        }
        return largest_bottom + this.GetHeaderHeight();
    }

    Resize(x, y, width, height)
    {
        // Call base
        let size_changed = super.Resize(x, y, width, height);
        this.PositionChildren();
        return size_changed;
    }

    AddChild(child_window, position_children)
    {
        child_window._parent = this;

        // Add to DOM if not already
        this.AddChildToDOM(child_window);

        this._children.push(child_window);
        child_window._parent_window_element = this;

        if ((position_children != null) && position_children)
            this.PositionChildren();
        
        return child_window;
    }

    // You might need to destroy the child after this
    RemoveChild(child_window)//, dont_position)
    {
        // Remove from DOM if not already
        this.RemoveChildFromDOM(child_window);
        child_window._parent = null;

        let i = this._children.indexOf(child_window);
        if (i >= 0)
            this._children.splice(i, 1);

        // if (dont_position == null)
        // 	this.PositionChildren();
    }

    ChildResized(child)
    {
        super.ChildResized(child);
    }

    RemoveAllChildren()
    {
        UI.RemoveAllChildElements(this._client_area);

        for (let i = 0; i < this._children.length; i++)
            this._children[i]._parent = null;

        this._children.length = 0;
    }

    PositionChildren()
    {
        this.PositionAttachedChildren();
    }

    // Return OJRect
    CalculateChildPosition(child)
    {
        let monitor = false;
        if (child._monitor != null)
            monitor = true;

        if (child._positioning)
        {
            OJLib.Trace("OJGrid circular dependency");
            this._circular_dependency = true;
            return new OJRect(0, 0, 0, 0);
        }

        child._positioning = true;

        let anchors = child.GetAnchors();
        let left_anchor = anchors._left;

        let x = 0;
        let y = 0;
        let width = 0;
        let height = 0;
        if (left_anchor._type == ANCHOR_TYPE.PARENT_NEAR)
        {
            x = left_anchor._fixed_offset;
            if (left_anchor._dynamic_position != null)
                x += ((left_anchor._dynamic_position * this._width) / 100);
        }
        else if (left_anchor._type == ANCHOR_TYPE.PARENT_FAR)
        {
            x = this._width + left_anchor._fixed_offset;
        }
        else if (left_anchor._type == ANCHOR_TYPE.SIBLING_NEAR)
        {
            let sibling = left_anchor._sibling;
            let sibling_rect = this.CalculateChildPosition(sibling);
            if (this._circular_dependency)
            {
                child._positioning = false;
                return new OJRect(0, 0, 0, 0);
            }

            x = sibling_rect._x + left_anchor._fixed_offset;
        }
        else if (left_anchor._type == ANCHOR_TYPE.SIBLING_FAR)
        {
            let sibling = left_anchor._sibling;
            let sibling_rect = this.CalculateChildPosition(sibling);
            if (this._circular_dependency)
            {
                child._positioning = false;
                return new OJRect(0, 0, 0, 0);
            }

            x = sibling_rect.Right() + left_anchor._fixed_offset;
        }
        else if (left_anchor._type == ANCHOR_TYPE.FIXED_SIZE)
        {
            width = left_anchor._fixed_size;
        }

        let right_anchor = anchors._right;
        if (right_anchor._type == ANCHOR_TYPE.PARENT_NEAR)
        {
            let anchor_point = 0;
            if (right_anchor._dynamic_position != null)
                anchor_point = ((right_anchor._dynamic_position * this._width) / 100);

            let x2 = anchor_point + right_anchor._fixed_offset;

            if (width > 0) // Fixed
                x = x2 - width;
            else
                width = x2 - x;
        }
        else if (right_anchor._type == ANCHOR_TYPE.PARENT_FAR)
        {
            let x2 = this._width + right_anchor._fixed_offset;

            if (width > 0) // Fixed
                x = x2 - width;
            else
                width = x2 - x;
        }
        else if (right_anchor._type == ANCHOR_TYPE.SIBLING_NEAR)
        {
            let sibling = right_anchor._sibling;
            let sibling_rect = this.CalculateChildPosition(sibling);
            if (this._circular_dependency)
            {
                child._positioning = false;
                return new OJRect(0, 0, 0, 0);
            }

            let x2 = sibling_rect._x + right_anchor._fixed_offset;

            if (width > 0) // Fixed
                x = x2 - width;
            else
                width = x2 - x;
        }
        else if (right_anchor._type == ANCHOR_TYPE.SIBLING_FAR)
        {
            let sibling = right_anchor._sibling;
            let sibling_rect = this.CalculateChildPosition(sibling);
            if (this._circular_dependency)
            {
                child._positioning = false;
                return new OJRect(0, 0, 0, 0);
            }

            let x2 = sibling_rect.Right() + right_anchor._fixed_offset;
            if (width > 0) // Fixed
                x = x2 - width;
            else
                width = x2 - x;
        }
        else if (right_anchor._type == ANCHOR_TYPE.FIXED_SIZE)
        {
            width = right_anchor._fixed_size;
        }
        else if (right_anchor._type == ANCHOR_TYPE.FIT_CHILDREN)
        {
            // Find the maximum child right position	
            let max_right = 0;
            for (let i = 0; i < child._children.length; i++)
            {
                let grandchild = child._children[i];
                let rect = child.CalculateChildPosition(grandchild);
                if (rect.Right() > max_right)
                    max_right = rect.Right();
            }

            width = max_right + right_anchor._fixed_offset;
        }

        let top_anchor = anchors._top;
        if (top_anchor._type == ANCHOR_TYPE.PARENT_NEAR)
        {
            y = top_anchor._fixed_offset;
            if (top_anchor._dynamic_position != null)
                y += ((top_anchor._dynamic_position * this._height) / 100);
        }
        else if (top_anchor._type == ANCHOR_TYPE.PARENT_FAR)
        {
            y = this._height + top_anchor._fixed_offset;
        }
        else if (top_anchor._type == ANCHOR_TYPE.SIBLING_NEAR)
        {
            let sibling = top_anchor._sibling;
            let sibling_rect = this.CalculateChildPosition(sibling);
            if (this._circular_dependency)
            {
                child._positioning = false;
                return new OJRect(0, 0, 0, 0);
            }

            y = sibling_rect._y + top_anchor._fixed_offset;
        }
        else if (top_anchor._type == ANCHOR_TYPE.SIBLING_FAR)
        {
            let sibling = top_anchor._sibling;
            let sibling_rect = this.CalculateChildPosition(sibling);
            if (this._circular_dependency)
            {
                child._positioning = false;
                return new OJRect(0, 0, 0, 0);
            }

            y = sibling_rect.Bottom() + top_anchor._fixed_offset;
        }

        let bottom_anchor = anchors._bottom;
        if (bottom_anchor._type == ANCHOR_TYPE.PARENT_NEAR)
        {
            let anchor_point = 0;
            if (bottom_anchor._dynamic_position != null)
                anchor_point = ((bottom_anchor._dynamic_position * this._height) / 100);

            let y2 = anchor_point + bottom_anchor._fixed_offset;
            height = y2 - y;
        }
        else if (bottom_anchor._type == ANCHOR_TYPE.PARENT_FAR)
        {
            let y2 = this._height + bottom_anchor._fixed_offset;
            height = y2 - y;
        }
        else if (bottom_anchor._type == ANCHOR_TYPE.SIBLING_NEAR)
        {
            let sibling = bottom_anchor._sibling;
            let sibling_rect = this.CalculateChildPosition(sibling);
            if (this._circular_dependency)
            {
                child._positioning = false;
                return new OJRect(0, 0, 0, 0);
            }

            let y2 = sibling_rect._y + bottom_anchor._fixed_offset;
            height = y2 - y;
        }
        else if (bottom_anchor._type == ANCHOR_TYPE.SIBLING_FAR)
        {
            let sibling = bottom_anchor._sibling;
            let sibling_rect = this.CalculateChildPosition(sibling);
            if (this._circular_dependency)
            {
                child._positioning = false;
                return new OJRect(0, 0, 0, 0);
            }

            let y2 = sibling_rect.Bottom() + bottom_anchor._fixed_offset;
            height = y2 - y;
        }
        else if (bottom_anchor._type == ANCHOR_TYPE.FIXED_SIZE)
        {
            height = bottom_anchor._fixed_size;
        }
        else if (bottom_anchor._type == ANCHOR_TYPE.FIT_CHILDREN)
        {
            // Find the maximum child bottom position	
            let max_bottom = 0;
            for (let i = 0; i < child._children.length; i++) 
            {
                let grandchild = child._children[i];
                let rect = child.CalculateChildPosition(grandchild);
                if (rect.Bottom() > max_bottom)
                    max_bottom = rect.Bottom();
            }

            height = max_bottom + bottom_anchor._fixed_offset;
        }

        child._positioning = false;
        return new OJRect(x, y, width, height);
    }

    ClearCircularFlags()
    {
        this._circular_dependency = false;
        for (let i = 0; i < this._children.length; i++)
        {
            let child = this._children[i];
            child._positioning = false;
        }
    }

    PositionAttachedChildren()
    {
        if ((this._width == 0) || (this._height == 0))
            return;

        for (let i = 0; i < this._children.length; i++)
        {
            // Clear flag for circular dependency detection
            this.ClearCircularFlags();

            let child = this._children[i];
            let rect = this.CalculateChildPosition(child);

            if (this._circular_dependency)
            {
                OJLib.Trace("Circular dependency");
                return;
            }

            child.Resize(rect._x, rect._y, rect._width, rect._height);
        }
    }

    HighlightItem(index, state, colour)
    {
        if (index < this._children.length)
        {
            let child = this._children[index];
            let element = child._text_div;

            if (state)
            {
                element.style.backgroundColor = colour;
            }
            else
            {
                // IE requirement to remove style attribute
                if (element.style.removeAttribute)
                    element.style.removeAttribute("backgroundColor");

                element.style.backgroundColor = null;
            }
        }
    }

    ContainsTextControl(oj_window)
    {
        if ((oj_window._class_name == "OJTextControl") &&
            (oj_window._control_type == TEXT_CONTROL_TYPE.TCT_TEXT))
        {
            return true;
        }

        for (let i = 0; i < oj_window._children.length; i++)
        {
            if (this.ContainsTextControl(oj_window._children[i]))
                return true;
        }

        return false;
    }
}

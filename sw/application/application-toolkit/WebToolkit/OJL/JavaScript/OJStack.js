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
import { OJLib } from "./OJL.js";
import { OJGrid } from "./OJL.js";
import { UI } from "./OJL.js";

export class OJStack extends OJWindowElement
{
    constructor(parent_element, opts, label, header_opts)
    {
        // Base class constructor
        super();
        this._class_name = "OJStack";
        this.SetElementName("OJStack");

        // Set the class of the client area
        this._client_area.className = "stack_class";

        this._spacing = 10;
        this._h_border_total = 2;
        this._v_border_total = 2;
        this._margin = 0;

        if (opts != null)
        {
            if ((opts._have_border != null) && !opts._have_border)
            {
                this._client_area.style.borderWidth = "0px";
                this._h_border_total = 0;
                this._v_border_total = 0;
                this._client_area.style.borderStyle = "none";
            }
            if ((opts._no_left_border_only != null) && opts._no_left_border_only)
            {
                this._client_area.style.borderLeftWidth = "1px";
                this._client_area.style.borderRightWidth = "1px";
                this._client_area.style.borderTopWidth = "1px";
                this._client_area.style.borderBottomWidth = "1px";
                this._h_border_total = 2;
                this._v_border_total = 2;
                this._client_area.style.borderStyle = "solid";
            }

            if (opts._spacing != null)
                this._spacing = opts._spacing;
            if (opts._margin != null)
                this._margin = opts._margin;

            if (opts._corner_radius != null)
            {
                var corner_radius = opts._corner_radius;
                this._client_area.style.borderBottomLeftRadius = corner_radius + "px";
                this._client_area.style.borderBottomRightRadius = corner_radius + "px";
                if ((label == null) && (opts._square_top !== true))
                {
                    this._client_area.style.borderTopLeftRadius = corner_radius + "px";
                    this._client_area.style.borderTopRightRadius = corner_radius + "px";
                }

                if (header_opts == null)
                    header_opts = {};
                header_opts._corner_radius = opts._corner_radius;
            }
        }
        else
        {
            this._client_area.style.borderWidth = "0px";
            this._client_area.style.borderStyle = "none";
        }
        
        if (label != null)
            this.UseWindowHeader(label, header_opts);	

        if (parent_element != null)
            parent_element.appendChild(this._client_area);

    }

    GetCount()
    {
        return this.GetNumChildren();
    }

    Resize(x, y, width, height)
    {
        var monitor = false;
        if (this._monitor)
            monitor = true;

        // If this stack is scrollable, then auto set the height
        if (this._parent_scrollable != null)
        {
            height = this.GetHeight();

            var size_changed = super.Resize(x, y,
                width - this._h_border_total, height - this._v_border_total);

            this.PositionChildren();
        }
        else
        {
            // Call base
            var size_changed = super.Resize(x, y,
                width - this._h_border_total, height - this._v_border_total);

            this.PositionChildren();
        }

        return size_changed;
    }

    AddChild(child_window, snap_bottom, position_children)
    {
        // Add to DOM if not already
        this.AddChildToDOM(child_window);

        if (snap_bottom != null)
            child_window._snap_bottom = snap_bottom;

        this._children.push(child_window);
        child_window._parent_window_element = this;

        var reposition_children = false;
        if (position_children != null)
            reposition_children = position_children;

        if (reposition_children)
            this.PositionChildren();
    }

    AddChildAtIndex(child_window, index)
    {
        // Add to DOM if not already
        this.AddChildToDOM(child_window);

        this._children.splice(index, 0, child_window);

        child_window._parent_window_element = this;
        this.PositionChildren();
    }

    ChildResized(child)
    {
        super.ChildResized(child);
        var old_height = this.GetHeight();
        this.PositionChildren();
        var new_height = this.GetHeight();

        // Propagate the child resize upwards if necessary
        //if (new_height != old_height)
            this.NotifyParentOfResize();
    }

    RemoveChild(child_index, reposition_children)
    {
        if ((this._children) && (child_index < this._children.length))
        {
            var child = this._children[child_index];
            this._children.splice(child_index, 1);

            child.RemoveFromDOM();
        }

        if ((reposition_children != null) && reposition_children)
            this.PositionChildren();
    }

    RemoveChildItem(child_item, reposition_children)
    {
        if (this._children == null)
            return -1;

        var i = this._children.indexOf(child_item);
        if (i < 0)
            return -1;

        var child = this._children[i];
        var scroll_position = 0;
        this._children.splice(i, 1);

        // If the child item is a scrollable, we will return the scroll position
        // so it can be restored
        if (child._scrollable != null)
        {
            scroll_position = child._scrollable.GetPosition();
        }

        child.RemoveFromDOM();

        if ((reposition_children != null) && reposition_children)
            this.PositionChildren();

        return scroll_position;
    }

    ReplaceChild(child_index, new_child)
    {
        if (child_index < this._children.length)
        {
            var old_child = this._children[child_index];
            if (old_child != null)
                old_child.RemoveFromDOM();
        }

        // Add to DOM if not already
        this.AddChildToDOM(new_child);

        this._children[child_index] = new_child;
        new_child._parent_window_element = this;
        this.PositionChildren();
    }

    RemoveAll(do_destroy, reposition_children)
    {
        var destroy = (do_destroy == null) ? true : do_destroy;

        for (var i = 0; i < this._children.length; i++)
        {
            var child = this._children[i];

            if (destroy)
                child.Destroy();
        }

        UI.RemoveAllChildElements(this._client_area);
        this._children.length = 0;

        var do_reposition = true;
        if (reposition_children != null)
            do_reposition = reposition_children;

        if (do_reposition)
            this.PositionChildren();
    }

    PositionChildren()
    {
        if ((this._width == 0) || (this._height == 0))
            return;
        
        var monitor = false;
        if (this._monitor)
            monitor = true;

        var current_bottom = this._height;

        var child_width = this._width - (2 * this._margin);
        var next_x = this._margin;
        var next_y = this._margin;
        var fill_space_child = null;

        for (var i = 0; i < this._children.length; i++)
        {
            var child = this._children[i];

            if ((child == null) || child._destroyed)
            {
                OJLib.Trace("OJStack contains a destroyed window");
                continue;
            }

            if (child == null)
                continue;

            if (child._has_anchor_position != null)
            {
                // Position the child using its anchors, it's not
                // part of the normal stack
                let rect = OJGrid.prototype.CalculateChildPosition.call(this, child);
                child.Resize(rect._x, rect._y, rect._width, rect._height);
                continue;
            }

            var child_height = child.GetHeight();

            // Only one child with _fill_space set is allowed
            if (child._fill_space)
            {
                fill_space_child = child;
                continue;
            }

            var use_x = next_x;
            var use_y = next_y;

            if (child._snap_bottom === true)
            {
                current_bottom -= child_height;
                use_y = current_bottom;
                current_bottom -= this._spacing;
            }

            child.Resize(use_x, use_y, child_width, child_height);

            // The child's width and height may be limited, so read back
            var child_height = child.GetHeight();

            if (child._snap_bottom === true)
            {
                child_height = 0;
            }
                
            if (child_height > 0)
                next_y += (child_height + this._spacing);
        }

        if (fill_space_child != null)
        {
            var child_height = current_bottom - next_y;
            if (child_height < 0)
                child_height = 5;
            fill_space_child.Resize(next_x, next_y, child_width, child_height);
        }
    }

    GetHeight()
    {
        if (!this._shown || this._destroyed)
            return 0;

        var height = this._margin * 2 + this.GetHeaderHeight() + this._v_border_total;

        var n_items = 0;
        for (var i = 0; i < this._children.length; i++)
        {
            if (this._children[i] == null)
                continue;

            var child_height = this._children[i].GetHeight();
            if (child_height > 0)
            {
                height += child_height;
                n_items++;
            }
        }

        if (n_items > 0)
            height += ((n_items - 1) * this._spacing);

        return height;
    }

    ForceMouseOut(index, event)
    {
        this._children[index].OnMouseOut(event);
        this._children[index].UnregisterForEvents()
    }
}
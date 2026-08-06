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
import { OJLib, UI } from "./OJL.js";

export class OJScrollableMouseWheelHandler
{
    constructor(scrollable)
    {
        this._scrollable = scrollable;
        this._button_callbacks = OJLib.RegisterButton(this, scrollable._client_area);
    }

    Destroy()
    {
        this._scrollable = null;
        this._button_callbacks.Destroy();
        this._button_callbacks = null;
    }

    OnMouseWheel(event)
    {
        if (this._scrollable)
            this._scrollable.MouseWheel(event);
    }
}

export class OJPageBar extends OJWindowElement
{
    constructor(parent_element, is_vertical)
    {
        // Base class constructor
        super(parent_element);
        this._class_name = "OJPageBar";
        this.SetElementName("OJPageBar");

        this.GetElement().style.backgroundColor = UI.AddHue("#90");

        this._is_vertical = is_vertical;
        this._aperture = 1;
        this._range = 1;
        this._position = 0;
        this._bar_centre_pixel = 0;
        this._bar_pixel = 0;

        this._bar_div = document.createElement("div");
        this._bar_div.className = this._is_vertical ? "vertical_page_bar_class" : "horizontal_page_bar_class";
        this._bar_div._window_element = this;

        this.GetElement().appendChild(this._bar_div);
    }

    Destroy()
    {
        if (this._destroyed)
            return;

        this._bar_div._window_element = null;
        this._bar_div = null;
        super.Destroy();
    }

    Resize(x, y, width, height)
    {
        let size_changed = super.Resize(x, y, width, height);
        this.RepositionBar();
        return size_changed;
    }

    SetRange(aperture, range)
    {
        this._aperture = aperture;
        this._range = range;
    }

    SetPosition(position)
    {
        this._position = position;
        this.RepositionBar();
    }

    GetControlLength()
    {
        if (this._is_vertical)
            return this._height;
        else
            return this._width;
    }

    RepositionBar()
    {
        let bar_size = ((this.GetControlLength() * this._aperture) / this._range) | 0;
        let bar_pixel = ((this._position * this.GetControlLength()) / this._range) | 0;

        if ((bar_pixel + bar_size) > this.GetControlLength())
        {
            // Last page
            bar_pixel = this.GetControlLength() - bar_size;
        }

        this._bar_pixel = bar_pixel;
        this._bar_centre_pixel = bar_pixel + (bar_size / 2);

        if (this._is_vertical)
        {
            this._bar_div.style.top = bar_pixel + "px";
            this._bar_div.style.height = bar_size + "px";
        }
        else
        {
            this._bar_div.style.left = bar_pixel + "px";
            this._bar_div.style.width = bar_size + "px";
        }
    }

    GetCentrePixel()
    {
        return this._bar_centre_pixel;
    }

    GetBarPixel()
    {
        return this._bar_pixel;
    }
}

export class OJScrollable extends OJGrid
{
    constructor(parent_element, opts)
    {
        // Base class constructor
        super(parent_element);
        this._class_name = "OJScrollable";
        this.SetElementName("OJScrollable.OJGrid");

        // Set the class of the client area
        this._client_area.className = "scrollable_class";

        this._h_border_total = 0;
        this._v_border_total = 0;
        //this._page_number = 0;
        this._horizontal_position = 0;
        this._vertical_position = 0;
        this._num_v_pages = -1;
        this._num_h_pages = -1;
        this._child_height = 0;
        this._aperture_height = 0;
        this._page_mode_only = false;
        this._button_down_pixel = 0;
        this._lbutton_down_position = 0;
        this._set_position_callback = null;
        this._auto_centre = false;

        if (opts != null)
        {
            if (opts._left_border != null)
            {
                this._client_area.style.borderLeftWidth = opts._left_border + "px";
            }

            if (opts._have_border != null)
            {
                if (opts._have_border)
                {
                    this._client_area.style.borderWidth = "1px";
                    this._h_border_total = 2;
                    this._v_border_total = 2;
                }
                else
                {
                    this._client_area.style.borderWidth = "0px";
                    this._h_border_total = 0;
                    this._v_border_total = 0;
                }
            }

            if (opts._update_position_callback != null)
                this._set_position_callback = opts._update_position_callback;

            if (opts._auto_centre != null)
                this._auto_centre = opts._auto_centre;
        }

        // Create the page-up, page-down and child-aperture elements
        this._page_button_size = 30;

        this._vertical_page_bar = new OJPageBar(this._client_area, true);
        this._horizontal_page_bar = new OJPageBar(this._client_area, false);
        this._child_aperture = new OJGrid(this._client_area);
        this._child_aperture.SetElementName("OJScrollableAperture.OJGrid");

        this._vertical_page_bar.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -this._page_button_size });
        this._horizontal_page_bar.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -this._page_button_size });
        this._child_aperture.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -this._page_button_size });

        this.AddChild(this._vertical_page_bar);
        this.AddChild(this._horizontal_page_bar);
        this.AddChild(this._child_aperture);

        this._button_callbacks = OJLib.RegisterButton(this, this._vertical_page_bar.GetElement());
        this._h_button_callbacks = OJLib.RegisterButton(this, this._horizontal_page_bar.GetElement());

        this._child = null;
        this._margin = 0;

        this._mouse_wheel_handler = new OJScrollableMouseWheelHandler(this);
        this._mouse_wheel_callbacks = OJLib.RegisterButton(this._mouse_wheel_handler, this._client_area);
    }

    Destroy()
    {
        this._child = null;
        this._mouse_wheel_callbacks.Destroy();
        this._mouse_wheel_handler.Destroy();
        this._button_callbacks.Destroy();
        this._h_button_callbacks.Destroy();
        this._vertical_page_bar.Destroy();
        this._horizontal_page_bar.Destroy();
        this._child = null;
        this._vertical_page_bar = null;
        this._horizontal_page_bar = null;
        this._child_aperture = null;

        super.Destroy();

        this._mouse_wheel_handler = null;
        this._mouse_wheel_callbacks = null;
        this._button_callbacks = null;
        this._h_button_callbacks = null;
    }

    // Not using OnMouseWheel, we come via the OJScrollableMouseWheelHandler
    MouseWheel(event)
    {
        let scroll = false;
        if (document.activeElement != null)
        {
            let window_element = document.activeElement._window_element;
            if (window_element != null)
            {
                scroll = (window_element._class_name != "OJTextControl");
            }
            else
                scroll = true;

            if (document.activeElement.id == "_body")
            {
                scroll = true;
            }
        }
        else
            scroll = true;

        if (scroll)
        {
            let e = window.event || event;
            let delta = Math.max(-1, Math.min(1, (e.wheelDelta || -e.detail)));
            let position = this._vertical_position - delta * 40;
            this.SetPosition(position, true);
        }
    }

    OnClick(event)
    {
        if (this.IsEnabled())
        {
            if (!this._page_mode_only)
                return;

            let is_vertical = true;
            if ((event._source_element == this._vertical_page_bar._bar_div) ||
                (event._source_element == this._vertical_page_bar._client_area))
            {
                is_vertical = true;
            }

            let pixel = is_vertical ? event._offset_y : event._offset_x;
            let page_bar = is_vertical ? this._vertical_page_bar : this._horizontal_page_bar;

            if (event._source_element == page_bar._bar_div)
                pixel += page_bar.GetBarPixel();

            let bar_centre_pixel = page_bar.GetCentrePixel();
            if (pixel > bar_centre_pixel)
                this.OnPageDown(event, is_vertical);
            else
                this.OnPageUp(event, is_vertical);

            OJLib.ReleaseMouse(event);
        }
    }

    OnLButtonDown(event)
    {
        let is_vertical = true;
        let page_bar = this._vertical_page_bar;
        let pixel = event._offset_y;
        let aperture_size = this._aperture_height;
        let child_size = this._child_height;

        if ((event._source_element == this._horizontal_page_bar._bar_div) ||
            (event._source_element == this._horizontal_page_bar._client_area))
        {
            is_vertical = false;
            page_bar = this._horizontal_page_bar;
            pixel = event._offset_x;
            aperture_size = this._aperture_width;
            child_size = this._child_width;
        }

        if (this.IsEnabled())
        {
            if ((event._is_touch_event == null) && (event._source_element == page_bar._bar_div))
            {
                pixel += page_bar.GetBarPixel();
            }
            else
            {
                let position = ((pixel * child_size / aperture_size) - aperture_size / 2) | 0;
                this.SetPosition(position, is_vertical);
            }

            this._button_down_pixel = pixel;
            this._lbutton_down_position = is_vertical ? this._vertical_position : this._horizontal_position;

            OJLib.CaptureMouse(page_bar._bar_div);
        }
    }

    OnLButtonUp(event)
    {
        if (this.IsEnabled())
            OJLib.ReleaseMouse(event);
    }

    OnMouseMove(event)
    {
        let is_vertical = true;
        let page_bar = this._vertical_page_bar;
        let pixel = event._offset_y;
        let aperture_size = this._aperture_height;
        let child_size = this._child_height;
        let num_pages = this._num_v_pages;

        if ((event._source_element == this._horizontal_page_bar._bar_div) ||
            (event._source_element == this._horizontal_page_bar._client_area))
        {
            is_vertical = false;
            page_bar = this._horizontal_page_bar;
            pixel = event._offset_x;
            aperture_size = this._aperture_width;
            child_size = this._child_width;
            num_pages = this._num_h_pages;
        }

        if (event._lbutton_down && this.IsEnabled())
        {
            if ((event._is_touch_event == null) && (event._source_element == page_bar._bar_div))
            {
                pixel += page_bar.GetBarPixel();
            }

            //let num_pages = this.CalculateNumPages();

            let page_size = aperture_size / num_pages;
            let page = (pixel / page_size) | 0;
            let position = 0;

            if (this._page_mode_only)
            {
                position = page * aperture_size;
            }
            else
            {
                let d_pixel = (pixel - this._button_down_pixel);
                position = this._lbutton_down_position +
                    (d_pixel * child_size / aperture_size) | 0;
            }

            this.SetPosition(position, is_vertical);
        }

        return true; // Don't propagate
    }

    OnPageUp(event, is_vertical)
    {
        if (is_vertical)
        {
            this._vertical_position -= this._aperture_height;
            if (this._vertical_position < 0)
                this._vertical_position = 0;
        }
        else
        {
            this._horizontal_position -= this._aperture_width;
            if (this._horizontal_position < 0)
                this._horizontal_position = 0;
        }

        this.JumpToPosition();
    }

    OnPageDown(event, is_vertical)
    {
        if (is_vertical)
        {
            if (this._num_v_pages == 0)
                return;

            this._vertical_position += this._aperture_height;

            if ((this._vertical_position + this._aperture_height) >= this._child_height)
            {
                this._vertical_position = this._child_height - this._aperture_height;
                if (this._vertical_position < 0)
                    this._vertical_position = 0;
            }
        }
        else
        {
            if (this._num_h_pages == 0)
                return;

            this._horizontal_position += this._aperture_width;

            if ((this._horizontal_position + this._aperture_width) >= this._child_width)
            {
                this._horizontal_position = this._child_width - this._aperture_width;
                if (this._horizontal_position < 0)
                    this._horizontal_position = 0;
            }
        }

        this.JumpToPosition();
    }

    SetPosition(position, vertical)
    {
        if (vertical)
        {
            if (this._num_v_pages == 0)
                return;

            this._vertical_position = position;

            if ((this._vertical_position + this._aperture_height) >= this._child_height)
                this._vertical_position = this._child_height - this._aperture_height;

            if (this._vertical_position < 0)
                this._vertical_position = 0;
        }
        else
        {
            if (this._num_h_pages == 0)
                return;

            this._horizontal_position = position;

            if ((this._horizontal_position + this._aperture_width) >= this._child_width)
                this._horizontal_position = this._child_width - this._aperture_width;

            if (this._horizontal_position < 0)
                this._horizontal_position = 0;
        }

        if (this._set_position_callback != null)
            this._set_position_callback.Call(position);

        this.JumpToPosition();
    }

    ScrollToBottom()
    {
        this.SetPosition(this._child_height - this._aperture_height, true);
    }

    // tell OJScrollable that it is on a dialog
    MakeDialog()
    {
        this._vertical_page_bar.GetElement()._is_dialog_element = true;
        this._vertical_page_bar._bar_div._is_dialog_element = true;
        this._horizontal_page_bar.GetElement()._is_dialog_element = true;
        this._horizontal_page_bar._bar_div._is_dialog_element = true;
    }

    GetPosition(is_vertical)
    {
        if (is_vertical == null)
            is_vertical = true;

        return is_vertical ? this._vertical_position : this._horizontal_position
    }

    CalculateNumPages()
    {
        let pages = { _horizontal: 0, _vertical: 0 }
        if (this._child == null)
            return pages;

        this._child_width = this._child.GetWidth() | 0;
        this._child_height = this._child.GetHeight() | 0;
        this._aperture_width = this._child_aperture.GetWidth();
        this._aperture_height = this._child_aperture.GetHeight();

        this._horizontal_page_bar.SetRange(this._aperture_width, this._child_width);
        this._vertical_page_bar.SetRange(this._aperture_height, this._child_height);

        if (this._child_height <= this._aperture_height)
            pages._vertical = 1;
        else
        {
            pages._vertical = (this._child_height / this._aperture_height) | 0;
            if ((this._child_height % this._aperture_height) > 0)
                pages._vertical++;
        }

        if (this._child_width <= this._aperture_width)
            pages._horizontal = 1;
        else
        {
            pages._horizontal = (this._child_width / this._aperture_width) | 0;
            if ((this._child_width % this._aperture_width) > 0)
                pages._horizontal++;
        }

        return pages;
    }

    ScrollbarRequiredForHeight(height)
    {
        if (this._child == null)
            return 0;

        this._child_height = this._child.GetHeight();
        let num_pages = (this._child_height / height) | 0;
        if ((this._child_height % height) > 0)
            num_pages++;
        return (num_pages > 1);
    }

    ScrollbarRequiredForWidth(width, child_width)
    {
        this._child_width = child_width;
        let num_pages = (this._child_width / width) | 0;
        if ((this._child_width % width) > 0)
            num_pages++;
        return (num_pages > 1);
    }

    JumpToPosition()
    {
        let x = this._horizontal_position;
        let y = this._vertical_position;

        if (this._child_height > this._aperture_height)
        {
            let bottom = y + this._aperture_height;
            if (bottom > this._child_height)
                y -= (bottom - this._child_height);
        }
        else
        {
            this._vertical_position = 0;

            if (this._auto_centre)
                y = ((this._child_height - this._aperture_height) / 2) | 0;
            else
                y = 0;
        }

        if (this._child_width > this._aperture_width)
        {
            let right = x + this._aperture_width;
            if (right > this._child_width)
                x -= (right - this._child_width);
        }
        else
        {
            this._horizontal_position = 0;

            if (this._auto_centre)
                x = ((this._child_width - this._aperture_width) / 2) | 0;
            else
                x = 0;
        }

        OJLib.SetElementLocation(this._child.GetElement(), -x, -y);

        if (typeof (this._child.OnScroll) === "function")
        {
            this._child.OnScroll(-x, -y);
        }

        this._horizontal_page_bar.SetPosition(this._horizontal_position);
        this._vertical_page_bar.SetPosition(this._vertical_position);
    }

    AddScrollableChild(child_window, dont_reposition)
    {
        if (child_window == null)
            return;

        this._child_aperture.AddChild(child_window, null, null, null, dont_reposition);
        this._child = child_window;
        child_window.SetScrollable(this);

        // This will reset the child positions and recalculate the scrollbar
        this.Resize(this._x, this._y, this._width, this._height);
        this.SetPosition(0, true);
    }

    RemoveScrollableChild()
    {
        this._child_aperture.RemoveAllChildren();

        if (this._child != null)
            this._child.SetScrollable(null);

        this._child = null;
        this.PositionChildren();
    }

    ChildSizeChanged()
    {
        this.Resize(this._x, this._y, this._width, this._height);
        this.JumpToPosition();
    }

    ChildResized(child)
    {
        this.ChildSizeChanged();
        super.ChildResized(child);
    }

    ResetScrollPages(x, y, width, height)
    {
        let size_changed = false;
        let num_pages = this.CalculateNumPages();
        let resize_again = false;

        if (num_pages._vertical != this._num_v_pages)
        {
            this._num_v_pages = num_pages._vertical;

            if (num_pages._vertical > 1)
            {
                // Need scrollbar area
                this._vertical_page_bar.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -this._page_button_size });
                this._child_aperture.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -this._page_button_size });
            }
            else
            {
                // Remove scrollbar controls
                this._vertical_page_bar.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: 0 });
                this._child_aperture.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: 0 });
            }

            resize_again = true;
        }

        if (num_pages._horizontal != this._num_h_pages)
        {
            this._num_h_pages = num_pages._horizontal;

            if (num_pages._horizontal > 1)
            {
                // Need scrollbar area
                this._horizontal_page_bar.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -this._page_button_size });
                this._child_aperture.SetBottomAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -this._page_button_size });
            }
            else
            {
                // Remove scrollbar controls
                this._horizontal_page_bar.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: 0 });
                this._child_aperture.SetBottomAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: 0 });
            }

            resize_again = true;
        }

        if (resize_again)
        {
            // Need to resize again if num pages has changed
            if (super.Resize(x, y, width, height))
                size_changed = true;
        }
        
        this.JumpToPosition();
        return size_changed;
    }

    Resize(x, y, width, height)
    {
        let monitor = false;
        if (this._monitor)
            monitor = true;

        // Call base to set the child sizes
        let size_changed = super.Resize(x, y, width, height);

        if (this.ResetScrollPages(x, y, width, height))
            size_changed = true;
        
        return size_changed;
    }

    ChildrenResized()
    {
        this._parent_window_element.RepeatResize();
    }

    SetActive(state)
    {
        // Call base
        super.SetActive(state);

        if (this._child != null)
            this._child.SetActive(state);

        this.ChildSizeChanged();
    }

    Show(state)
    {
        // Call base
        super.Show(state);

        if (this._child != null)
            this._child.Show(state);
    }

    SetDialogFlag()
    {
        super.SetDialogFlag();

        if (this._child != null)
            this._child.SetDialogFlag();
    }
}
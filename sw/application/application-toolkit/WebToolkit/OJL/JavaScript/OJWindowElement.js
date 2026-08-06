/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJLib } from "./OJL.js";
import { UI } from "./OJL.js";
import { OJPictureButton } from "./OJL.js";
import { ElementDragger } from "./OJL.js";

// LED Mode
export const LED_MODE_TYPE =
{
    GREEN_LED: "green",
    RED_LED: "red",
    BLUE_LED: "blue",
    NO_LED: "none",
};

// NB dynamic_position can only be used with parent_left or parent_top attachment,
// not applicable to right or bottom anchors

export const ANCHOR_TYPE =
{
    PARENT_NEAR: "parent_near",
    PARENT_FAR: "parent_far",
    SIBLING_NEAR: "sibling_near",
    SIBLING_FAR: "sibling_far",
    FIT_CHILDREN: "fit_children",
    FIXED_SIZE: "fixed_size"
};

export class OJAnchor
{
    constructor(type)
    {
        this._type = type; //ANCHOR_TYPE.PARENT_NEAR; // parent or SIBLING_NEAR_or_top/SIBLING_FAR_or_bottom
        this._sibling = null; // Sibling if anchor is "sibling"
        this._fixed_offset = 0; // In pixels
        this._fixed_size = null;
        this._dynamic_position = null; // or 50 , percent of parent dimension
    };

    Destroy()
    {
        // Buddy might result in circular references
        this._type = null;
        this._sibling = null;
        this._fixed_offset = null;
        this._fixed_size = null;
        this._dynamic_position = null;
    }

    Clone()
    {
        let clone = new OJAnchor();
        clone._type = this._type;
        clone._sibling = this._sibling;
        clone._fixed_offset = this._fixed_offset;
        clone._fixed_size = this._fixed_size;
        clone._dynamic_position = this._dynamic_position;
        return clone;
    }
}

let next_client_area_id = 1;
let next_window_element_index = 1;

const FULL_SCREEN_STATE =
{
    FSS_MULTI: 0,
    FSS_FULL: 1,
    FSS_SUPER_FULL: 2,
    FSS_DUMMY: 3
};

export class OJWindowElement
{
    constructor(parent_element, opts)
    {
        this._window_element_index = next_window_element_index++;

        // public
        this._class_name = "OJWindowElement";
        this._destroyed = false;
        this._parent_window_element = null;
        this._header_height = 34; // Constant
        this._index = 0;
        this._parent_element = parent_element;
        this._mouse_down = false;
        this._start_rectangle = null;
        this._mouse_down_x = 0;
        this._mouse_down_y = 0;
        this._corner_distance = 15;
        this._id_index = next_client_area_id++;

        this._windowless = false;
        if ((opts != null) && (opts._windowless != null) && opts._windowless)
            this._windowless = opts._windowless;

        if (this._windowless)
            this._client_area = {};
        else
            this._client_area = document.createElement("div");
        this._client_area.id = "OJWindowElement_client_area_" + this._id_index;
        this._client_area._window_element = this;
        this._client_area.className = "window_class";
        this._window_name = "OJWindowElement";
        this._x = 0;
        this._y = 0;
        this._width = 0;
        this._height = 0;
        this._window_header = null;
        this._window_header_label = null;
        this._use_window_header = false;
        this._window_header_right_edge = 0;
        this._base_header_label = "";
        this._active = false;
        this._shown = true;
        this._link_label = "";
        this._have_label = false;
        this._enabled = true;
        this._read_only = false;
        this._parent_scrollable = null;
        this._read_only = false;
        this._children = [];
        this._fullscreen_state = FULL_SCREEN_STATE.FSS_MULTI;
        this._has_focus = false;
        this._settings_button = null;
        this._fill_space = false; // Used by OJStack
        this._drop_callback = null;
        this._header_buttons = [];
        this._has_border = false;
        this._tool_tip = null;
        this._tool_tip_show_time = null;
        this._lbutton_down = false;
        this._lbutton_down_x = 0;
        this._lbutton_down_y = 0;
        this._drag_header_left = 0;
        this._drag_header_top = 0;
        this._drag_window_left = 0;
        this._drag_window_top = 0;
        this._total_window_width = 0;
        this._total_window_height = 0;
        this._draggable_window = false;

        if ((this._parent_element != null) && !this._windowless)
            this._parent_element.appendChild(this._client_area);

        this._left_anchor = new OJAnchor(ANCHOR_TYPE.PARENT_NEAR);
        this._right_anchor = new OJAnchor(ANCHOR_TYPE.PARENT_FAR);
        this._top_anchor = new OJAnchor(ANCHOR_TYPE.PARENT_NEAR);
        this._bottom_anchor = new OJAnchor(ANCHOR_TYPE.PARENT_FAR);

        if (opts != null)
        {
            if ((opts._border != null) && opts._border)
            {
                this._client_area.style.border = "1px";
                this._client_area.style.borderStyle = "solid";
                this._client_area.style.borderColor = "#838383";
                this._has_border = true;
            }
        }

        this.ResetAnchors();
    }

    Destroy()
    {
        if (this._destroyed)
        {
            //OJLib.Trace("Window element already destroyed!");
        }
        else
        {
            this._parent_window_element = null;
            this._left_anchor.Destroy();
            this._right_anchor.Destroy();
            this._top_anchor.Destroy();
            this._bottom_anchor.Destroy();
            this._left_anchor = null;
            this._right_anchor = null;
            this._top_anchor = null;
            this._bottom_anchor = null;
            this._parent_scrollable = null;

            if (this._settings_button != null)
                this._settings_button.Destroy();

            OJLib.DestroyArray(this._children);
            this._children = null;

            // Just is case, all children should have been removed by their
            // own Destroy above

            UI.RemoveAllChildElements(this._client_area);
            UI.RemoveAllChildElements(this._window_header);
            UI.RemoveAllChildElements(this._window_header_label);
            UI.RemoveFromParentElement(this._client_area);
            UI.RemoveFromParentElement(this._window_header);
            UI.RemoveFromParentElement(this._window_header_label);

            for (let i = 0; i < this._header_buttons.length; i++)
                this._header_buttons[i].Destroy();

            if (this._header_callback != null)
            {
                this._header_callback.Destroy();
                this._header_callback = null;
            }

            this._client_area._window_element = null;
            if (UI._captured_window_element == this)
                UI._captured_window_element = null;

            this._use_window_header = false;
            this._client_area = null;
            this._window_header = null;
            this._window_header_label = null;
            this._destroyed = true;
            this._parent_element = null;
        }
    }

    ResetAnchors()
    {
        // Set default attachment to fill parent
        this.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 0 });
        this.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: 0 });
        this.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 0 });
        this.SetBottomAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: 0 });
    }

    SetAbsolutePosition(x, y, width, height)
    {
        this.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: x });
        this.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: width });
        this.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: y });
        this.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: height });
    }

    GetNumChildren()
    {
        return this._children.length;
    }

    // If we are a child window of an OJScrollable, it will call us
    // so we have a reference to it. When we change size internally
    // (ie not via Resize, then we have to call scrollable.ChildSizeChanged()
    // so the OJScrollable can resize its scroll bars etc
    SetScrollable(scrollable)
    {
        this._parent_window_element = scrollable;
        this._parent_scrollable = scrollable;
    }

    Enable(state)
    {
        let changed = (this._enabled != state);
        this._enabled = state;

        if (changed)
        {
            UI.SetStyleAttribute(this.GetElement().style, "color", this._enabled ? null : UI._disabled_text_colour);
        }

        return changed;
    }

    IsEnabled()
    {
        if (this._parent_window_element != null)
        {
            // Don't reverse hierachy for property items
            if (this._parent_window_element._class_name == "OJProperty")
                return this._enabled;
        }

        if (!this._enabled)
            return false;
        else if (this._parent_window_element != null)
            return this._parent_window_element.IsEnabled();
        else
            return true;
    }

    ReadOnly(state)
    {
        let changed = (this._read_only != state);
        this._read_only = state;
        return changed;
    }

    SetElementName(name)
    {
        if (this._window_header != null)
        {
            this._window_header.id = name + "_window_header_" + this._id_index;
            if (this._window_header_label != null)
                this._window_header_label.id = name + "_window_header_label_" + this._id_index;
        }

        this._client_area.id = name + "_client_area_" + this._id_index;
        this._window_name = name;
    }

    GetElement()
    {
        return this._client_area;
    }

    GetHeaderElement()
    {
        return this._window_header;
    }

    GetHeaderLabelElement()
    {
        return this._window_header_label;
    }

    SetId(name)
    {
        this.GetElement().id = name;
    }

    GetAnchors()
    {
        return {
            _left: this._left_anchor,
            _right: this._right_anchor,
            _top: this._top_anchor,
            _bottom: this._bottom_anchor
        };
    }

    SetTopAnchor(opts)
    {
        this._top_anchor = this.SetAnchorOpts(this._top_anchor, opts);
    }

    SetBottomAnchor(opts)
    {
        this._bottom_anchor = this.SetAnchorOpts(this._bottom_anchor, opts);
    }

    SetLeftAnchor(opts)
    {
        this._left_anchor = this.SetAnchorOpts(this._left_anchor, opts);
    }

    SetRightAnchor(opts)
    {
        this._right_anchor = this.SetAnchorOpts(this._right_anchor, opts);
    }

    SetAnchorOpts(anchor, opts)
    {
        if (opts.constructor.name == "OJAnchor")
        {
            let new_anchor = opts;
            anchor.Destroy();
            anchor = new_anchor;
        }
        else
        {
            anchor._type = opts._type;
            if (opts._fixed_offset != null)
                anchor._fixed_offset = opts._fixed_offset;
            else
                anchor._fixed_offset = 0;

            if (opts._sibling != null)
                anchor._sibling = opts._sibling;
            else
                anchor._sibling = null;

            if (opts._dynamic_position != null)
                anchor._dynamic_position = opts._dynamic_position;
            else
                anchor._dynamic_position = null;

            if (opts._fixed_size != null)
                anchor._fixed_size = opts._fixed_size;
            else
                anchor._fixed_size = null;
        }

        return anchor;
    }

    AddChildToDOM(child_window)
    {
        if (child_window == null)
            return;

        let element = child_window.GetElement();
        if (element != null)
        {
            if (element.parentNode == null)
            {
                //this.GetElement().appendChild(element);
                var this_elem = this.GetElement();
                this_elem.appendChild(element);
            }

            let header_element = child_window.GetHeaderElement();
            if (header_element != null)
            {
                if (header_element.parentNode == null)
                    this.GetElement().appendChild(header_element);
            }
        }
    }

    RemoveChildFromDOM(child_window)
    {
        if (child_window == null)
            return;

        let element = child_window.GetElement();
        if (element != null)
        {
            UI.RemoveFromParentElement(element);

            let header_element = child_window.GetHeaderElement();
            if (header_element != null)
            {
                UI.RemoveFromParentElement(header_element);
            }
        }
    }

    UseWindowHeader(label, header_opts)
    {
        if (this._use_window_header || this._windowless)
            return;

        this._use_window_header = true;

        this._window_header = document.createElement("div");
        if (header_opts && (header_opts._fill_background == false))
            this._window_header.className = "simple_window_header_class";
        else if (header_opts && header_opts._dialog_header)
            this._window_header.className = "dialog_header_class";
        else
            this._window_header.className = this._active ? "active_window_header_class" : "inactive_window_header_class";

        if (header_opts && header_opts._height)
        {
            this._header_height = header_opts._height;
            this._window_header.style.height = this._header_height + "px";
            this._window_header.style.lineHeight = this._header_height + "px";
        }

        if (this._has_border)
        {
            this._window_header.style.border = "1px";
            this._window_header.style.borderStyle = "solid";
            this._window_header.style.borderColor = "#838383";
        }

        if (header_opts && header_opts._corner_radius)
        {
            let corner_radius = parseInt(header_opts._corner_radius);
            this._window_header.style.borderTopLeftRadius = corner_radius + "px";
            this._window_header.style.borderTopRightRadius = corner_radius + "px";
        }

        // Buttons required?
        if (header_opts && (header_opts._buttons != null))
        {
            let buttons_array = header_opts._buttons;
            for (let i = 0; i < buttons_array.length; i++)
            {
                let button_details = buttons_array[i];
                let button_width = 32;
                let button_height = 32;
                if (button_details._width != null)
                    button_width = button_details._width;
                if (button_details._height != null)
                    button_height = button_details._height;

                let button = new OJPictureButton(this._window_header, button_details._image, button_details._image_over,
                    {
                        _object: button_details._callback._object,
                        _object_callback: button_details._callback._method_name,
                        _user_data: button_details._callback._user_data
                    },
                    {
                        _width: button_width,
                        _height: button_height
                    });

                button.SetToolTip(button_details._tool_tip);
                button.ManualPositioning();
                let right = 55 + 38 * (buttons_array.length - 1 - i);
                button.GetElement().style.right = right + "px";
                button.GetElement().style.top = "3px";
                button.GetElement().style.display = "block";

                this._header_buttons.push(button);
            }
        }

        if (header_opts && (header_opts._background_colour != null))
        {
            this._window_header.style.backgroundColor = header_opts._background_colour;
        }

        if (header_opts && (header_opts._draggable_window != null))
            this._draggable_window = header_opts._draggable_window;

        this._base_header_label = label;

        let tile_label;
        if (this._link_label == "")
            tile_label = label;
        else
            tile_label = label + " - " + this._link_label;

        if (label != null)
        {
            this._window_header_label = document.createElement("div");
            this._window_header_label.className = "window_header_label_class";
            this._window_header.appendChild(this._window_header_label);
            this.SetHeaderLabel(tile_label);
        }

        let parent = this._client_area.parentNode;
        if (parent != null)
            parent.appendChild(this._window_header);

        this._header_callback = OJLib.RegisterButton(this, this._window_header);
    }

    SetHeaderLabel(label)
    {
        if (this._window_header_label != null)
            this._window_header_label.innerHTML = label;
    }

    GetClientAreaStyle()
    {
        return this._client_area.style;
    }

    // Override in your derived class
    SetActive(state)
    {
        this._active = state;

        if (this._active)
        {
            if (this._use_window_header)
                this._window_header.className = this._active ? "active_window_header_class" : "inactive_window_header_class";
        }
        else
        {
            if (this._use_window_header)
                this._window_header.className = this._active ? "active_window_header_class" : "inactive_window_header_class";
        }
    }

    IsActive()
    {
        return this._active;
    }

    SetParentElement(parent_element)
    {
        this._parent_element = parent_element;
        this._parent_element.appendChild(this._client_area);
    }

    RemoveFromDOM()
    {
        UI.RemoveFromParentElement(this._client_area);

        if (this._window_header != null)
            UI.RemoveFromParentElement(this._window_header);

        // Workaround the Firefox problem where onmouseout is not called on elements
        // that are removed from the DOM
        this.UnregisterForEvents();

    }

    UnregisterForEvents()
    {
        if (this._destroyed)
            return;

        for (let i = 0; i < this._children.length; i++)
            this._children[i].UnregisterForEvents();
    }

    FindDropCallback()
    {
        if (this._drop_callback)
            return this._drop_callback;
        else if (this._parent_window_element)
            return this._parent_window_element.FindDropCallback();
        else
            return null;
    }

    ChildrenResized()
    {
        if (this._parent_window_element)
            this._parent_window_element.ChildrenResized();
    }

    RepeatResize()
    {
        this.Resize(this._x, this._y, this._width, this._height);
    }
    // Return true if size changes
    Resize(x, y, width, height)
    {
        let empty = ((width == 0) || (height == 0));
        if (this._use_window_header && !empty && (this._fullscreen_state != FULL_SCREEN_STATE.FSS_SUPER_FULL))
        {
            let left_edge = 0; // 10;
            let right_edge = this._window_header_right_edge;

            // Header include 2 px border
            let h_border_total = 0;
            if (this._h_border_total)
                h_border_total = this._h_border_total;
            OJLib.SetElementPosition(this._window_header, x + left_edge, y, width - left_edge - right_edge + h_border_total, this._header_height);

            y = y + this._header_height;
            height -= this._header_height;
        }

        this._x = x;
        this._y = y;

        let size_changed = (width != this._width) || (height != this._height);

        this.AssignWidth(width);
        this.AssignHeight(height);

        this._client_area.style.left = x + "px";
        this._client_area.style.top = y + "px";
        this._client_area.style.width = width + "px";
        this._client_area.style.height = height + "px";
        if (this._have_label)
            this._client_area.style.lineHeight = height + "px";

        // Override if you need to
        this.WindowClientAreaResized(x, y, width, height);

        return size_changed;
    }

    // May need to merge with ClientAreaResize of the OJViewWindow
    WindowClientAreaResized(x, y, width, height)
    {
    }

    // You can override this (eg see OJStack)
    GetHeight()
    {
        if (this._shown)
            return this._height;
        else
            return 0;
    }

    GetHeaderHeight()
    {
        if (this._use_window_header)
            return this._header_height;
        else
            return 0;
    }

    GetWidth()
    {
        return this._width;
    }

    MouseMove(event)
    {
        //OJLib.Trace("DocumentMouseMove (" + event._client_x + ", " + event._client_y + ")");
        if (UI._captured_window_element != null)
        {
            UI._captured_window_element.OnBodyMouseMove(event);
        }
    }

    SetFocus()
    {
        this.GetElement().focus();
    }

    SetBackgroundColour(colour)
    {
        UI.SetBackgroundColour(this.GetElement(), colour);
    }

    SetColour(colour)
    {
        UI.SetColour(this.GetElement(), colour);
    }

    Reveal()
    {
        let element = this.GetElement();

        //element.style.webkitAnimation = "reveal 1000ms";
        //element.style.webkitAnimationFillMode = "both";
        //element.style.animation = "reveal 1000ms";
        //element.style.animationFillMode = "both";
        element.style.opacity = 1;
    }

    ChildResized(child)
    {
    }

    NotifyParentOfResize()
    {
        if (this._parent_window_element != null)
            this._parent_window_element.ChildResized(this);
    }

    Show(state)
    {
        this._shown = state;
        if (!this._windowless)
        {
            this._client_area.style.display = state ? "block" : "none";
            if (this._window_header != null)
                this._window_header.style.display = state ? "block" : "none";
        }
    }

    GetBoundingClientRect()
    {
        return UI.GetBoundingClientRect(this._client_area);
    }

    AssignWidth(width)
    {
        let is_nan = isNaN(width);

        if (is_nan)
            OJLib.Trace("Width NaN");
        else
            this._width = width;
    }

    AssignHeight(height)
    {
        let is_nan = isNaN(height);

        if (is_nan)
            OJLib.Trace("Height NaN");
        else
            this._height = height;
    }

    SetDialogFlag()
    {
        UI.SetDialogFlag(this.GetElement());

        if (this._window_header != null)
            this._window_header._is_dialog_element = true;

        if (this._window_header_label != null)
            this._window_header_label._is_dialog_element = true;

        for (let i = 0; i < this._children.length; i++)
            this._children[i].SetDialogFlag();
    }

    FillStackSpace()
    {
        // Used for a child window of a OJStack
        this._fill_space = true;
    }

    GetChild(index)
    {
        return this._children[index];
    }

    GetChildren(index)
    {
        return this._children;
    }

    IsChild(child)
    {
        if (this._children.indexOf(child))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    SetToolTip(tip, show_time)
    {
        if (tip === "")
        {
            this._tool_tip = null;
            this._tool_tip_show_time = null;
        }
        else
        {
            this._tool_tip = tip;
            this._tool_tip_show_time = show_time;
        }
    }

    OnLButtonDown(event)
    {
        if (this._draggable_window)
        {
            this._lbutton_down = true;
            this._lbutton_down_x = event._client_x;
            this._lbutton_down_y = event._client_y;
            this._drag_header_left = parseInt(this._window_header.style.left);
            this._drag_header_top = parseInt(this._window_header.style.top);
            this._drag_window_left = parseInt(this._client_area.style.left);
            this._drag_window_top = parseInt(this._client_area.style.top);

            this._total_window_width = parseInt(this._window_header.style.width);
            this._total_window_height = parseInt(this._client_area.style.height) + this._drag_window_top -
                                        this._drag_header_top;

            OJLib.CaptureMouse(event.target);
        }

        if (event.ctrlKey)
            this.LogAutomationDetails();
    }

    OnLButtonUp(event)
    {
        if (this._draggable_window)
        {
            OJLib.ReleaseMouse(event);
            this._lbutton_down = false;

            let dx = event._client_x - this._lbutton_down_x;
            let dy = event._client_y - this._lbutton_down_y;
            this.DragWindowPosition(dx, dy);
        }
    }

    OnMouseMove(event)
    {
        if (this._draggable_window)
        {
            if (this._lbutton_down)
            {
                let dx = event._client_x - this._lbutton_down_x;
                let dy = event._client_y - this._lbutton_down_y;

                this.DragWindowPosition(dx, dy);
            }
        }
    }

    DragWindowPosition(dx, dy)
    {
        let x = this._drag_header_left + dx;
        let y = this._drag_header_top + dy;
        if (x < 0)
            x = 0;
        if (y < 0)
            y = 0;
        
        let parent_width = parseInt(this._parent_window_element._client_area.style.width);
        let parent_height = parseInt(this._parent_window_element._client_area.style.height);

        if ((x + this._total_window_width) > parent_width)
            x = parent_width - this._total_window_width;
        if ((y + this._total_window_height) > parent_height)
            y = parent_height - this._total_window_height;

        this._window_header.style.left = x + "px";
        this._window_header.style.top = y + "px";
        this._client_area.style.left = x + "px";
        this._client_area.style.top = (y + this._drag_window_top - this._drag_header_top) + "px";
    }

    LogAutomationDetails()
    {
    }
}

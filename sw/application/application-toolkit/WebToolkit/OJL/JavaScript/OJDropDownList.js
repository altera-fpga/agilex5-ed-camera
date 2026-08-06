/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJStack } from "./OJL.js";
import { OJServerLink } from "./OJL.js";
import { OJTextButton, OJToolTip } from "./OJL.js";
import { OJLib, UI, ObjectCallback } from "./OJL.js";
import { OJScrollable } from "./OJL.js";


function RemoveFromOpenDropDownLists(drop_down_list)
{
    while (true)
    {
        let index = UI._drop_down_list_displayed.indexOf(drop_down_list);
        if (index < 0)
            return;
        UI._drop_down_list_displayed.splice(index, 1);
    }
}

export class OJDropDownList extends OJTextButton
{
    constructor(value, selection_changed_callback, shared_options_id,
                attach_picons_to_right, opts)
    {
        // Base class constructor
        if (opts == null)
            opts = { _colour_key: UI.AddHue("#35") };

        super(null, "<label>", opts);
        this._text_div.style.backgroundImage = OJLib._drop_marker.GetURL();
        this._text_div.style.backgroundRepeat = "no-repeat";
        this._text_div.style.backgroundPosition = "100px 13px";
        this.SetElementName("OJDropDownList");
        this._attach_picons_to_right = false;
        this._set_background_colour = null;
        this._auto_select = false;

        if (opts._set_background_colour != null)
            this.SetBackgroundColour(opts._set_background_colour);

        if (opts._font_size != null)
            this._text_div.style.fontSize = opts._font_size + "px";

        if (opts._auto_select != null)
            this._auto_select = opts._auto_select;

        if (attach_picons_to_right != null)
            this._attach_picons_to_right = attach_picons_to_right;

        this._selected_index = value;

        // Use the list entries of the shared options if required
        this._using_shared_options = (shared_options_id != null);

        if (this._using_shared_options)
        {
            this._list_entries = GetSharedOptions(shared_options_id)._list_entries;

            let item = this._list_entries[this._selected_index];
            if (item != null)
                this.SetLabel(item.GetText());
            else
                this.SetLabel("");
        }
        else
            this._list_entries = [];

        this._is_open = false;
        this._sub_menu_open = null;
        this._parent_menu = null;
        this._selection_changed_callback = selection_changed_callback;
        this._shared_options_id = shared_options_id;
        this._scrollable_panel = null;
        this._drop_list = null;

        // No drop list for shared options
        if (!this._using_shared_options)
        {
            this._scrollable_panel = new OJScrollable(null, { _left_border: 0 });
            this._drop_list = new OJStack(null, { _have_border: true, _spacing: 0 });
            this._scrollable_panel.AddScrollableChild(this._drop_list);
        }
        
        this._cancel_check_callback = new ObjectCallback(this, "OnCancelCheck");
        this._on_item_mouse_overs = [];
        this._on_item_mouse_outs = [];

        // Keeps references list items
        // Used to dynamically update item text
        this._item_list = [];
        this._colour_table = [];
    }

    Destroy()
    {
        this.Close(null, false);
        this.Clear();
        this._list_entries = null;
        this._sub_menu_open = null;
        this._parent_menu = null;

        OJLib.UnregisterForLButtonDown(this._cancel_check_callback);
        OJLib.UnregisterForRButtonDown(this._cancel_check_callback);
        this._cancel_check_callback.Destroy();
        this._cancel_check_callback = null;

        OJLib.DestroyArray(this._on_item_mouse_overs);
        OJLib.DestroyArray(this._on_item_mouse_outs);

        this._on_item_mouse_overs = null;
        this._on_item_mouse_outs = null;

        if (!this._using_shared_options)
        {
            this._scrollable_panel.Destroy();

            // drop_list is a child of scrollable panel
            this._drop_list.Destroy();
            this._scrollable_panel = null;
            this._drop_list = null;
        }

        super.Destroy();

        RemoveFromOpenDropDownLists(this);
    }

    OnClick(event)
    {
        if (this._tool_tip != null)
            OJToolTip.End(this._tool_tip, this._text_div);

        if (!this.IsEnabled())
            return;

        if (this._is_open)
            this.Close(null, true, event);
        else
            this.Open(event._client_x, event._client_y);
    }

    SetColourTable(colour_table)
    {
        this._colour_table = colour_table;
    }

    AddItem(item_text, server_command, picon, image_name, tag_colour)    
    {
        let index = this._drop_list.GetCount();
        let colour = tag_colour;

        if (OJServerLink.Get()._mono_ui)
            colour = "#505050";
        else
        {
            if ((tag_colour == null) || (tag_colour == ""))
                colour = this.GetKeyColour(index);
        }

        colour = "#e0e0e0";

        let height = 32;
        let item = null;

        if (picon != null)
        {
            let item_height = picon._height;// + 4;
            let height = Math.max(height, item_height);
            item = new OJTextButton(null, item_text,
            {
                _click_object: this,
                _click_callback: "OnItemSelected",
                _user_data: index,
                _height: height,
                _is_popup: true,
                _picon: picon,
                _attach_picons_to_right: this._attach_picons_to_right
            });
        }
        else if (image_name != null)
        {
            item = new OJTextButton(null, item_text,
            {
                _click_object: this,
                _click_callback: "OnItemSelected",
                _user_data: index,
                _height: height,
                _colour_key: colour,
                _is_popup: true,
                _image_url: image_name,
                _image_attachment: "right",
                _attach_picons_to_right: this._attach_picons_to_right
            });
        }
        else
        {
            // Create callbacks to propagate list item events
            let item_callbacks = null;

            if (this._propagate_events)
            {
                let on_item_mouse_over = new ObjectCallback(this, "OnItemMouseOver");
                let on_item_mouse_out = new ObjectCallback(this, "OnItemMouseOut");
                this._on_item_mouse_overs.push(on_item_mouse_over);
                this._on_item_mouse_outs.push(on_item_mouse_out);

                item_callbacks =
                    {
                        _onmouseover: on_item_mouse_over,
                        _onmouseout: on_item_mouse_out
                    };
            }
            
            item = new OJTextButton(null, item_text,
                    {
                        _click_object: this,
                        _click_callback: "OnItemSelected",
                        _user_data: index,
                        _height: height,
                        _colour_key: colour,
                        _is_popup: true,
                        _propagate_events: this._propagate_events,
                        _event_callbacks: item_callbacks
                    });

        }

        this._drop_list.AddChild(item);
        if (this._drop_list._client_area._is_dialog_element)
            UI.SetDialogFlag(item._client_area);

        this._list_entries.push(new OJDropDownListEntry(item_text, server_command, index, item));

        if (this._selected_index < 0)
            this._selected_index = 0;

        return this._list_entries.length - 1;
    }

    RemoveItem(index) 
    {
        this._list_entries.splice(index, 1);
        this._drop_list.RemoveChild(index);
    }

    AddSubMenu(item_index, sub_menu)
    {
        this._list_entries[item_index]._sub_menu = sub_menu;
    }

    GetKeyColour(index)
    {
        if ((this._colour_table.length > 0) && (index < this._colour_table.length))
            return this._colour_table[index];

        let hue = index / 8;
        while (hue > 1)
            hue -= 1;

        let rgb = OJLib.HSL_to_RGB(hue, 0.2, 0.4);
        let str = "rgb(" + (rgb.R | 0) + "," + (rgb.G | 0) + "," + (rgb.B | 0) + ")";
        return str;
    }

    OnItemSelected(event)
    {
        let selected_index = event._user_data;

        if (this._list_entries[selected_index] == null)
            return;

        // Is this a sub menu?
        if (this._list_entries[selected_index]._sub_menu != null)
        {
            if (this._sub_menu_open != null)
                this._sub_menu_open.Close();

            for (let i = 0; i < this._drop_list._children.length; i++)
            {
                if (i == selected_index)
                    this._drop_list._children[i].RestoreTagColour();
                else
                    this._drop_list._children[i].HideTagColour();
            }

            let selected_element = this._drop_list.GetChild(selected_index).GetElement();
            let rectangle = UI.GetBoundingClientRect(selected_element);

            this._sub_menu_open = this._list_entries[selected_index]._sub_menu;
            this._sub_menu_open.Open(rectangle.right, rectangle.top);
            this._sub_menu_open._parent_menu = this;
            return;
        }

        //OJLib.Trace("OJDropDownList OnItemSelected " + selected_index);
        this.Close(selected_index, true, event);

        // Call callback if we have one
        if (this._selection_changed_callback != null)
        {
            let entry = this._list_entries[selected_index];
            this._selection_changed_callback._object[this._selection_changed_callback._method_name](selected_index, entry);
        }

        if (this._auto_select)
            this.SetValue(selected_index);
    }

    // From server update
    SetValue(value)
    {
        this._selected_index = value;
        let item = this._list_entries[this._selected_index];
        if (item != null)
            this.SetLabel(item.GetText());
        else
            this.SetLabel("");
    }

    GetValue()
    {
        return this._selected_index;
    }

    Create()
    {
        if (this._list_entries.length == 0)
            return;

        if (this._selected_index >= 0)
        {
            let item = this._list_entries[this._selected_index];
            if (item != null)
                this.SetLabel(item.GetText());
            else
                this.SetLabel("");
        }
    }

    SetDialogFlag()
    {
        super.SetDialogFlag();

        if (this._scrollable_panel != null)
            this._scrollable_panel.SetDialogFlag();
    }

    IsOpen()
    {
        return this._is_open;
    }

    ProcessKey(event)
    {
    }

    GetScrollablePanel()
    {
        let scrollable_panel = null;

        if (this._shared_options_id == null)
            scrollable_panel = this._scrollable_panel;
        else
        {
            scrollable_panel = GetSharedOptions(this._shared_options_id)._scrollable_panel;
        }

        return scrollable_panel;
    }

    GetDropList()
    {
        let drop_list = null;

        if (this._shared_options_id == null)
            drop_list = this._drop_list;
        else
        {
            drop_list = GetSharedOptions(this._shared_options_id)._drop_list;
        }

        return drop_list;
    }

    RemapCallbacks(shared_combo)
    {
        for (let i = 0; i < shared_combo._drop_list._children.length; i++)
        {
            let text_button = shared_combo._drop_list._children[i];
            text_button._click_object = this;
        }
    }

    Open(x, y)
    {
        if (this._is_open)
            return;

        this._is_open = true;
        UI._drop_down_list_displayed.push(this);

        let shared_options = null;

        // If we are using a shared droplist then we must
        // reassign the callback objects to us
        if (this._shared_options_id != null)
        {
            shared_options = GetSharedOptions(this._shared_options_id);
            this.RemapCallbacks(shared_options);
        }

        let scrollable_panel = this.GetScrollablePanel();
        let scrollable_element = scrollable_panel.GetElement();

        let rectangle = UI.GetBoundingClientRect(this.GetElement());
        scrollable_element.style.zIndex = 999999;
        scrollable_element.style.position = "absolute";
        let body = document.getElementById("_body");

        let drop_list = this.GetDropList();
        let height = drop_list.GetHeight();
        let w = rectangle.right - rectangle.left;
        if (w > 0)
        {
            x = rectangle.left;
            y = rectangle.bottom;
        }
        let h = height;

        if ((y + h) > UI._screen_height)
        {
            y = UI._screen_height - h;
            if (y < 0)
            {
                // Need scroll bar
                y = 0;
                h = UI._screen_height;
            }
        }

        // Find maximum width
        let my_width = this.MeasureWidth(h);
        if (my_width > w)
            w = my_width;

        if ((x + w) > UI._screen_width)
            x = UI._screen_width - w;

        scrollable_panel.Resize(x, y, w, h);

        let selected_item_colour = OJServerLink.Get()._altera_background;
        selected_item_colour = "#f4f4f4";

        // Highlight current selection 
        for (let i = 0; i < drop_list._children.length; i++)
        {
            let item = drop_list._children[i];
            item.SetBackgroundColour(selected_item_colour);
            item.SetColour(OJServerLink.Get()._altera_text_colour);
            item.SetHoverColour(selected_item_colour);
        }

        // Scroll current selection into view
        if (this._selected_index >= 0)
            scrollable_panel.SetPosition(this._selected_index * (40 + 2));

        body.appendChild(scrollable_element);

        OJLib.RegisterForLButtonDown(this._cancel_check_callback);
        OJLib.RegisterForRButtonDown(this._cancel_check_callback);
    }

    MeasureWidth(container_height)
    {
        let drop_list = this.GetDropList();
        let rectangle = UI.GetBoundingClientRect(this.GetElement());
        let height = drop_list.GetHeight();
        let w = rectangle.right - rectangle.left;

        // Find maximum width
        if (drop_list != null)
        {
            for (let i = 0; i < drop_list._children.length; i++)
            {
                let item_width = drop_list._children[i].MeasureWidth() + 20;

                // If there is a sub menu, allow space for the > button
                if (this._list_entries[i]._sub_menu != null)
                    item_width += 25;

                if (item_width > w)
                    w = item_width;
            }
        }

        let scrollable_panel = this.GetScrollablePanel();
        if (scrollable_panel.ScrollbarRequiredForHeight(container_height))
            w += scrollable_panel._page_button_width; // For scrollbar

        return w;
    }

    Close(selected_index, close_parent, event)
    {
        RemoveFromOpenDropDownLists(this);

        if (!this._is_open)
            return;

        this._is_open = false;

        if (this._sub_menu_open != null)
            this._sub_menu_open.Close(null, false);

        if (close_parent && (this._parent_menu != null))
            this._parent_menu.Close(null, true);

        if (this._parent_menu != null)
            this._parent_menu._sub_menu_open = null;

        this._sub_menu_open = null;
        this._parent_menu = null;

        if (this._drop_list != null)
        {
            for (let i = 0; i < this._drop_list._children.length; i++)
                this._drop_list._children[i].RestoreTagColour();
        }

        // Workaround for IE and Firefox where
        // theres no mouse out event on zIndex change
        if (selected_index != null)
            this.GetDropList().ForceMouseOut(selected_index, event);

        // Remove from the DOM
        let scrollable_element = this.GetScrollablePanel().GetElement();
        scrollable_element.style.zIndex = 0;

        UI.RemoveFromParentElement(scrollable_element);

        OJLib.UnregisterForLButtonDown(this._cancel_check_callback);
        OJLib.UnregisterForRButtonDown(this._cancel_check_callback);

        this.OnMouseOut(event);

        UI.SetStyleAttribute(this._text_div.style, "backgroundColor", this._background_colour);
    }

    OnCancelCheck(event)
    {
        if ((this._sub_menu_open == null) && (UI._drop_down_list_displayed.length > 0))
        {
            // Check if event is outside of all drop downs
            let outside_drop_list = true;

            for (let d = 0; d < UI._drop_down_list_displayed.length; d++)
            {
                if (!UI._drop_down_list_displayed[d].EventOutsideWindow(event))
                {
                    outside_drop_list = false;
                    break;
                }
            }

            if (outside_drop_list)
            {
                // Close them all
                for (let d = 0; d < UI._drop_down_list_displayed.length; d++)
                    UI._drop_down_list_displayed[d].Close(null, false, event);

                return;
            }
        }

        let cancel = this.EventOutsideWindow(event);
        if (this._sub_menu_open != null)
            cancel = false;

        if (cancel)
        {
            // If clicked outside, close our drop list but not parent
            this.Close(null, false, event);
        }
    }

    EventOutsideWindow(event)
    {
        let x = event._client_x;
        let y = event._client_y;

        let scrollable_rectangle = UI.GetBoundingClientRect(this.GetScrollablePanel().GetElement());
        let button_rectangle = UI.GetBoundingClientRect(this.GetElement());
        let cancel = false;

        if ((x >= button_rectangle.left) && (x <= button_rectangle.right) &&
            (y >= button_rectangle.top) && (y <= button_rectangle.bottom))
        {
            return false;
        }

        if ((x < scrollable_rectangle.left) || (x > scrollable_rectangle.right))
            cancel = true;

        if ((y < scrollable_rectangle.top) || (y > scrollable_rectangle.bottom))
            cancel = true;

        return cancel;
    }

    Resize(x, y, width, height)
    {
        // Call base
        let size_changed = super.Resize(x, y, width, height);

        let by = (height - 5) / 2;
        let bx = 20;
        if (this._have_colour_key)
            bx = 28;

        this._text_div.style.backgroundPosition = (width - bx) + "px " + by + "px";

        return size_changed;
    }

    OnMouseOver(event)
    {
        if (UI._drop_down_list_displayed.length > 0)
            return;

        super.OnMouseOver(event);
    }

    OnMouseOut(event)
    {
        if (UI._drop_down_list_displayed.length > 0)
            return;

        super.OnMouseOut(event);
    }

    OnItemMouseOver(data)
    {	
        if (this._event_callbacks && this._event_callbacks._onitemmouseover)
        {
            let callback = this._event_callbacks._onitemmouseover;
            callback._object[callback._method_name]({_data:this._event_callbacks._user_data, _item_data:data});
        }
    }

    OnItemMouseOut(data)
    {	
        if (this._event_callbacks && this._event_callbacks._onitemmouseout)
        {
            let callback = this._event_callbacks._onitemmouseout;
            callback._object[callback._method_name]({_data:this._event_callbacks._user_data, _item_data:data});
        }	
    }

    UpdateItemText(idx, text)
    {	
        let entry = this._list_entries[idx];
        
        if (entry != null)
        {
            entry._text = text;
            entry._item.SetLabel(text);
            
            // Update main drop down text if needed
            if (idx == this._selected_index)
                this.SetLabel(entry.GetText());		
        }
    }

    GetItemText(idx)
    {
        if (idx == null)
            idx = this._selected_index;

        let entry = this._list_entries[idx];

        if (entry != null)
            return entry._text;
        else
            return "";
    }

    Clear()
    {
        if (this._drop_list != null)
            this._drop_list.RemoveAll();

        // Don't destroy if it's shared 
        if (!this._using_shared_options)
            OJLib.DestroyArray(this._list_entries);

        this._selected_index = -1;
    }
}

export class OJDropDownListEntry
{
    constructor(text, server_command, index, item)
    {
        this._text = text;
        this._server_command = server_command;
        this._index = index;
        this._item = item;
        this._sub_menu = null;
    }

    Destroy()
    {
        this._text = null;
        this._server_command = null;
        this._index = null;
        this._item = null;

        if (this._sub_menu != null)
            this._sub_menu.Destroy();

        this._sub_menu = null;
    }

    GetText()
    {
        return this._text;
    }
}


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
import { OJTextButton } from "./OJL.js";
import { OJLib, ObjectCallback, UI } from "./OJL.js";
import { OJScrollable } from "./OJL.js";

export class OJListBox extends OJScrollable
{
    constructor(value, selection_changed_callback, selection_double_clicked,
                add_border)
    {
        // Base class constructor
        super(null);

        if ((add_border != null) && add_border)
        {
            this._client_area.style.borderWidth = "1px";
            this._client_area.style.borderStyle = "solid";
        }

        this.SetElementName("OJListBox");
        this._list_entries = [];
        this._unselected_background_colour = null;
        this._selected_index = value;
        this._selected_indexes = [];
        this._is_open = false;
        this._selection_changed_callback = selection_changed_callback;
        this._selection_double_clicked = selection_double_clicked;
        this._scrollable_panel = null;
        this._items_list = null;

        this._items_list = new OJStack(null, { _have_border: false, _spacing: 2 });
        this.AddScrollableChild(this._items_list);

        this._cancel_check_callback = new ObjectCallback(this, "OnCancelCheck");
        this._on_item_mouse_overs = [];
        this._on_item_mouse_outs = [];

        // Keeps references list items
        // Used to dynamically update item text
        this._item_list = [];
        this._multiple_selection = false;
        this._last_index = 0;
    }

    Destroy()
    {
        this.Clear();
        OJLib.UnregisterForLButtonDown(this._cancel_check_callback);
        OJLib.UnregisterForRButtonDown(this._cancel_check_callback);
        this._cancel_check_callback.Destroy();
        this._cancel_check_callback = null;

        OJLib.DestroyArray(this._on_item_mouse_overs);
        OJLib.DestroyArray(this._on_item_mouse_outs);

        this._on_item_mouse_overs = null;
        this._on_item_mouse_outs = null;
        this._selected_indexes.length = 0;

        super.Destroy();
    }

    OnClick(event)
    {
    }

    AddItem(item, server_command, height_param, update_ui)
    {
        var do_update = true;
        if (update_ui != null)
            do_update = update_ui;

        var index = this._items_list.GetCount();
        var colour = this.GetKeyColour(index);
        var height = 40;

        // Is the given item a string or a window? (eg OJGrid)
        var is_string = (item._class_name == null);
        var item_text = "";
        if (is_string)
            item_text = item;

        if (height_param != null)
            height = height_param;

        var list_item_text = item_text; // defaults to name of file
        if (server_command)
            list_item_text = server_command;

        // Create callbacks to propagate list item events
        if (this._propagate_events)
        {

            var on_item_mouse_over = new ObjectCallback(this, "OnItemMouseOver");
            var on_item_mouse_out = new ObjectCallback(this, "OnItemMouseOut");
            this._on_item_mouse_overs.push(on_item_mouse_over);
            this._on_item_mouse_outs.push(on_item_mouse_out);

            var item_callbacks =
                {
                    _onmouseover: on_item_mouse_over,
                    _onmouseout: on_item_mouse_out
                };
        }

        if (is_string)
        {
            item = new OJTextButton(null, list_item_text,
                    {
                        _click_object: this,
                        _click_callback: "OnItemSelected",
                        _double_click_callback: "OnItemDoubleClicked",
                        _key_down_callback: "OnKeyDown",
                        _user_data: index,
                        _height: height,
                        _is_popup: false,
                        _propagate_events: this._propagate_events,
                        _event_callbacks: item_callbacks
                    });
        }

        this._items_list.AddChild(item);
        //item._text_div._is_dialog_element = this.GetElement()._is_dialog_element;

        this._list_entries.push(new OJListBoxEntry(item_text, server_command, index, item));

        if (this._selected_index < 0)
        {
            this._selected_index = 0;
            this._selected_indexes = [0];
        }

        if (do_update)
            this.ChildSizeChanged();

        return item;
    }

    RemoveItem(item)
    {
        // this._items_list._children
        var item_index = this._items_list._children.indexOf(item);

        if (item_index >= 0)
        {
            this._items_list.RemoveChild(item_index, true);
            this._list_entries.splice(item_index, 1);
        }
    }

    GetKeyColour(index)
    {
        var hue = index / 8;
        while (hue > 1)
        {
            hue -= 1;
        }
        var rgb = OJLib.HSL_to_RGB(hue, 0.2, 0.4);
        var str = "rgb(" + (rgb.R | 0) + "," + (rgb.G | 0) + "," + (rgb.B | 0) + ")";
        return str;
    }

    OnItemSelected(event)
    {
        var selected_index = event._user_data;
        this.SetValue(selected_index, event);  // select new item

        if (this._selection_changed_callback != null)
            CallCallback(this._selection_changed_callback, event);
    }

    OnItemDoubleClicked(event)
    {
        var selected_index = event._user_data;
        this.SetValue(selected_index, event);  // select new item

        if (this._selection_double_clicked != null)
            CallCallback(this._selection_double_clicked, event);
    }

    OnKeyDown(event)
    {
        var handled = false;
        if (event._key_code == 40)
        {
            // Down
            var n_items = this._items_list._children.length;
            var index = this._last_index + 1;
            if (index >= n_items)
                index = n_items - 1;
            this.SetValue(index, event);
            handled = true;
        }
        else if (event._key_code == 38)
        {
            // Up
            if (this._last_index > 0)
            {
                this.SetValue(this._last_index - 1, event);
            }
            handled = true;
        }
        else if (event.ctrlKey && ((event._key_code == "a".charCodeAt(0)) || (event._key_code == "A".charCodeAt(0))))
        {
            if (this._multiple_selection)
            {
                // Select all
                this._selected_indexes.length = 0;
                for (var i = 0; i < this._items_list._children.length; i++)
                {
                    this._selected_indexes.push(i);
                    sel_item = this._items_list._children[i];
                    if (sel_item != null)
                    {
                        sel_item.SetBackgroundColour("#3c646f");
                    }
                }
            }

            handled = true;
        }

        if (handled)
        {
            OJLib.MarkEventAsHandled(event);
        }

        return handled;
    }

    // From server update
    SetValue(value, event)
    {
        var index = value;
        var scroll_into_view = false;
        var sel_item = null;

        var control_key = false;
        var shift_key = false;

        if (event != null)
        {
            control_key = event.ctrlKey;
            shift_key = event.shiftKey;
        }

        if (this._multiple_selection)
        {
            if (control_key)
            {
                // Remove/Add index
                sel_item = this._items_list._children[index];
                var i = this._selected_indexes.indexOf(index);
                if (i < 0)
                {
                    this._selected_indexes.push(index);
                    sel_item.SetBackgroundColour("#3c646f");
                    scroll_into_view = true;
                }
                else
                {
                    this._selected_indexes.splice(index, 1);
                    sel_item.SetBackgroundColour(this._unselected_background_colour);
                }
            }
            else if (shift_key)
            {
                for (var i = 0; i < this._selected_indexes.length; i++)
                {
                    sel_item = this._items_list._children[this._selected_indexes[i]];
                    if (sel_item != null)
                        sel_item.SetBackgroundColour(this._unselected_background_colour);
                }

                var from = this._last_index;
                var to = index;
                if (from > to)
                {
                    var temp = from;
                    from = to;
                    to = temp;
                }

                this._selected_indexes.length = 0;
                for (var i = from; i <= to; i++)
                {
                    this._selected_indexes.push(i);
                    sel_item = this._items_list._children[i];
                    if (sel_item != null)
                    {
                        sel_item.SetBackgroundColour("#3c646f");
                        scroll_into_view = true;
                    }
                }
            }
            else
            {
                for (var i = 0; i < this._selected_indexes.length; i++)
                {
                    sel_item = this._items_list._children[this._selected_indexes[i]];
                    if (sel_item != null)
                    {
                        sel_item.SetBackgroundColour(this._unselected_background_colour);
                    }
                }

                this._selected_indexes = [index];
                sel_item = this._items_list._children[index];
                if (sel_item != null)
                {
                    sel_item.SetBackgroundColour("#3c646f");
                    scroll_into_view = true;
                }
            }
        }
        else
        {
            // deselect old value
            sel_item = this._items_list._children[this._selected_index];
            if (sel_item != null)
                sel_item.SetBackgroundColour(this._unselected_background_colour);

            this._selected_index = index;
            sel_item = this._items_list._children[this._selected_index];
            if (sel_item != null)
            {
                sel_item.SetBackgroundColour("#3c646f");
                scroll_into_view = true;
            }
        }

        if (scroll_into_view)
        {
            // Scroll into view
            var element = sel_item.GetElement();
            var rect = UI.GetBoundingClientRect(element);
            var source_y = parseInt(element.style.top);
            var source_height = parseInt(element.style.height);

            var aperture_height = this._aperture_height;
            var position = this.GetPosition();
            source_y -= position;
            var pattern_bottom = source_y + source_height;

            if (source_y < 0)
                this.SetPosition(source_y + position);
            else if (pattern_bottom > aperture_height)
                this.SetPosition(pattern_bottom + position - aperture_height);
        }

        if (!shift_key)
            this._last_index = index;
    }

    GetValue()
    {
        return this._selected_index;
    }

    GetMultipleSelections()
    {
        return this._selected_indexes;
    }

    GetSelectedButton()
    {
        return this._items_list._children[this._selected_index];
    }

    GetItem(index)
    {
        return this._items_list._children[index];
    }

    Create()
    {
        if (this._list_entries.length === 0)
        {
            return;
        }
        if (this._selected_index >= 0)
        {
    //		var item = this._list_entries[this._selected_index];
            this.SetLabel(item.GetText());
        }
    }

    ProcessKey(event)
    {
    }

    GetScrollablePanel()
    {
        var scrollable_panel = null;

        scrollable_panel = this._scrollable_panel;

        return scrollable_panel;
    }

    GetList()
    {
        return this._items_list;
    }

    GetCount()
    {
        return this._items_list.GetCount();
    }

    GetText(entry)
    {
        return this._list_entries[entry].item_text;
    }

    GetSelectedText()
    {
        return this._list_entries[this._selected_index]._text;
    }

    RemapCallbacks(shared_combo)
    {
        for (var i = 0; i < shared_combo._items_list._children.length; i++)
        {
            var text_button = shared_combo._items_list._children[i];
            text_button._click_object = this;
        }
    }

    OnCancelCheck(event)
    {
        if (!this._is_open)
        {
            return;
        }
        var x = event._client_x;
        var y = event._client_y;

        var scrollable_rectangle = UI.GetBoundingClientRect(this.GetElement());
        var button_rectangle = UI.GetBoundingClientRect(this.GetElement());
        var cancel = false;

        if ((x >= button_rectangle.left) && (x <= button_rectangle.right) &&
            (y >= button_rectangle.top) && (y <= button_rectangle.bottom))
        {
            return;
        }

        if ((x < scrollable_rectangle.left) || (x > scrollable_rectangle.right))
        {
            cancel = true;
        }
        if ((y < scrollable_rectangle.top) || (y > scrollable_rectangle.bottom))
        {
            cancel = true;
        }
        if (cancel)
        {
            this.Close();
        }
    }

    // Note the list main element events processed in the base class

    OnItemMouseOver(data)
    {	
        if (this._event_callbacks && this._event_callbacks._onitemmouseover)
        {
            var callback = this._event_callbacks._onitemmouseover;
            callback._object[callback._method_name]({_data:this._event_callbacks._user_data, _item_data:data});
        }
    }

    OnItemMouseOut(data)
    {	
        if (this._event_callbacks && this._event_callbacks._onitemmouseout)
        {
            var callback = this._event_callbacks._onitemmouseout;
            callback._object[callback._method_name]({_data:this._event_callbacks._user_data, _item_data:data});
        }	
    }

    UpdateItemText(idx, text)
    {	
        var entry = this._list_entries[idx];
        
        if (entry !== null)
        {
            entry._text = text;
            entry._item.SetLabel(text);
            
            // Update main drop down text if needed
            if (idx == this._selected_index)
            {
                this.SetLabel(entry.GetText());
            }
        }
    }

    Clear()
    {
        this._items_list.RemoveAll();
        this._list_entries.length = 0;
    }
}

export class OJListBoxEntry
{
    constructor(text, server_command, index, item)
    {
        this._text = text;
        this._server_command = server_command;
        this._index = index;
        this._item = item;
    }

    GetText()
    {
        return this._text;
    }
}


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
import { OJGrid } from "./OJL.js";
import { OJLib, UI } from "./OJL.js";
import { ANCHOR_TYPE } from "./OJL.js";

export class OJTreeView extends OJStack
{
    constructor(owner, folder, select_first)
    {
        // Base class constructor
        super(null, { _have_border: false });
        this._class_name = "OJTreeView";
        this._spacing = 2;
        this._item_height = 30;
        this._owner = owner;
        this._current_selected_item = null;

        this.SetElementName("OJTreeView");

        if (folder != null)
        {
            this.AddItems(folder, 0);

            if (select_first && (this._children.length > 0))
                this.SetSelection(this._children[0]);
        }

        // Set the collapsed children state
        for (let i = 0; i < this._children.length; i++)
        {
            if (this._children[i]._depth == 0)
                this.UpdateCollapseState(this._children[i]);
        }
    }

    Destroy()
    {
        this._owner = null;
        super.Destroy();
    }

    OnMouseOver(event)
    {
        for (let i = 0; i < this._children.length; i++)
            this._children[i].ShowOpenCloseIcons(true);
    }

    OnMouseOut(event)
    {
        for (let i = 0; i < this._children.length; i++)
            this._children[i].ShowOpenCloseIcons(false);
    }

    AddItems(folders, depth)
    {
        let can_collapse = (folders != null);
        let tree_item = new OJTreeItem(this, folders, depth, can_collapse);
        tree_item.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: this._item_height });
        this.AddChild(tree_item);

        if (can_collapse)
        {
            for (let sub_folder of folders)
                this.AddItems(sub_folder, depth + 1);
        }
    }

    IsCollapsible(property_item)
    {
        let item_depth = property_item._depth;
        let index = this._children.indexOf(property_item);
        if ((index >= 0) && ((index + 1) < this._children.length))
        {
            let next_child = this._children[index + 1];
            return (next_child._depth > item_depth);
        }

        return false;
    }

    ShowChildren(index, stop_depth, show_state)
    {
        for (; index._i < this._children.length; index._i++)
        {
            let child = this._children[index._i];
            if (child._depth <= stop_depth)
                return;

            child.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: show_state ? this._item_height : 0 });

            if (show_state && child._collapse_children)
            {
                index._i++;
                this.ShowChildren(index, child._depth, false);
                index._i--;
            }
        }
    }

    ToggleCollapsedState(item)
    {
        let index = this._children.indexOf(item);
        if (index >= 0)
        {

        }
    }

    UpdateCollapseState(property_item)
    {
        let item_depth = property_item._depth;
        let index = this._children.indexOf(property_item);
        if (index >= 0)
        {
            let i = { _i: index + 1 };

            if (property_item._collapse_children)
            {
                this.ShowChildren(i, item_depth, false);
            }
            else
            {
                // Open children
                this.ShowChildren(i, item_depth, true);
            }
            this.PositionChildren();

            // Tell our parent we have resized
            if (this._parent_window_element != null)
                this._parent_window_element.ChildResized(this);
        }
    }

    FindProperty(property_id)
    {
        for (let i = 0; i < this._children.length; i++)
        {
            let property = this._children[i];
            if (property._property_id == property_id)
                return property;

            property = property.FindAdditionalColumn(property_id);
            if (property != null)
                return property;
        }

        return null;
    }

    SetSelection(tree_item)
    {
        if (this._current_selected_item != null)
            UI.SetStyleAttribute(this._current_selected_item._client_area.style, "backgroundColor", null);

        this._current_selected_item = tree_item;

        if (this._current_selected_item != null)
            UI.SetStyleAttribute(this._current_selected_item._client_area.style, "backgroundColor", "#3c646f");

        this._owner.ItemSelected(tree_item);
    }

    ItemDoubleClicked(tree_item)
    {
        this._owner.ItemDoubleClicked(tree_item);
    }
}

/////////////////

class OJTreeItem extends OJGrid
{
    constructor(owner, folder, depth, can_collapse)
    {
        // Base class constructor
        super();
        this._class_name = "OJTreeItem";
        this._depth = depth;
        this._owner = owner;
        this._collapse_children = false;
        this._selected_index = 0;
        this._folder_name = folder._name;

        let indent = 22 + 30 * this._depth;

        if (can_collapse)
        {
            let up_icon_name = this._collapse_children ? _open_up.src : _close_up.src;
            let over_icon_name = this._collapse_children ? _open_over.src : _close_over.src;
            this._open_close_button = new OJPictureButton(null, up_icon_name, over_icon_name, { _object: this, _object_callback: "OnOpenClose" }, { _width: 13, _height: 13 });
            this._open_close_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: indent - 16 });
            this._open_close_button.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: indent - 3 });
            this.AddChild(this._open_close_button);
        }

        let title = folder._name.replace("/media/usb/", "USB/");

        // Remove trailing slash
        if (title.charAt(title.length - 1) == "/")
            title = title.substr(0, title.length - 1);

        let slash = title.lastIndexOf("/");
        let short_title = title.substr(slash + 1);

        this._label_item = new OJWindowElement();
        this._label_item._client_area.innerHTML = short_title;
        this._label_item._have_label = true;
        this._label_item.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: indent });

        this.AddChild(this._label_item);

        this._button_callbacks = OJLib.RegisterButton(this, this._client_area, null);

        this._files = [];
        if (folder.Files != null)
        {
            for (let file of folder.Files)
                this._files.push(file._filename);
        }
    }

    Destroy()
    {
        this._owner = null;
        this._files.length = 0;
        this._button_callbacks.Destroy();

        super.Destroy();
    }

    ShowOpenCloseIcons(state)
    {
        if (this._open_close_button != null)
        {
            if (this._collapse_children)
                state = true;

            UI.SetStyleAttribute(this._open_close_button.GetElement().style, "display", state ? "block" : "none");
        }
    }

    OnOpenClose(event)
    {
        this._collapse_children = !this._collapse_children;
        let up_icon_name = this._collapse_children ? OJLib._open_up._src : OJLib._close_up._src;
        let over_icon_name = this._collapse_children ? OJLib._open_over._src : OJLib._close_over._src;
        this._open_close_button.SwapImage(up_icon_name, over_icon_name);

        this._owner.UpdateCollapseState(this);
    }

    OnClick(event)
    {
        this._owner.SetSelection(this);
    }

    OnDoubleClick(event)
    {
        this._owner.ItemDoubleClicked(this);
    }
}

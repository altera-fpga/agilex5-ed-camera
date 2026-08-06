/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJGrid, ANCHOR_TYPE } from "./OJL.js";
import { OJTextButton } from "./OJL.js";
import { OJLib, UI, ObjectCallback } from "./OJL.js";

export class OJTab extends OJGrid
{
    // oj_window is the content that is displayed when the tab is active
    constructor(oj_window, tab_name, tab_id, tool_tip)
    {
        super();
        this._oj_window = oj_window;
        this._select_tab_cb = new ObjectCallback(this, "OnSelect");
        let button_options = { _click_callback: this._select_tab_cb, _alignment: "centre" };
        this._tab_button = new OJTextButton(null, tab_name, button_options);
        this._tab_button.FitText();
        this._tab_button.SetToolTip(tool_tip);
        this._tab_name = tab_name;
        this._tab_button.GetElement()._tab = this;
        this.AddChild(this._tab_button);

        this.SetRightAnchor({ _type: ANCHOR_TYPE.FIT_CHILDREN });

        this._selected_indicator = new OJGrid();
        this._selected_indicator.SetTopAnchor({ _type: ANCHOR_TYPE.SIBLING_FAR, _sibling: this._tab_button, _fixed_offset: 2 });
        this._selected_indicator.SetLeftAnchor({ _type: ANCHOR_TYPE.SIBLING_NEAR, _sibling: this._tab_button, _fixed_offset: 0 });
        this._selected_indicator.SetRightAnchor({ _type: ANCHOR_TYPE.SIBLING_FAR, _sibling: this._tab_button, _fixed_offset: -3 });
        this._selected_indicator.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 4 });
        this.AddChild(this._selected_indicator);
        this._selected_indicator.SetBackgroundColour(UI._selected_background_colour);

        this._active = false;
        this._selected = false;
        this._manual_width = 0;
        this._tab_control = null;
        this._index = 0;
        this._tab_id = tab_id;
    }

    Destroy()
    {
        this._tab_button.GetElement()._tab = null;
        this._tab_control = null;
        this._select_tab_cb.Destroy();
    }

    SetIndex(index, tab_control)
    {
        this._index = index;
    }

    OnSelect(event)
    {
        if (this._tab_control != null)
            this._tab_control.SetTab(this._index, true);
    }

    GetClientAreaStyle()
    {
        return this._oj_window.GetClientAreaStyle();
    }

    SetActive(state)
    {
        this._active = state;
        this._oj_window.SetActive(state);
    }

    SetSelected(state)
    {
        this._selected = state;
        let tab_style = this.GetClientAreaStyle();

        this._oj_window.Show(this._selected);
        this._selected_indicator.Show(this._selected);
    }
}

class OJTabHeaderBar extends OJGrid
{
    constructor()
    {
        // Base class constructor
        super();
        this.GetClientAreaStyle().borderTopWidth = "0px";
        this.GetClientAreaStyle().borderBottomWidth = "1px";
        this.GetClientAreaStyle().borderLeftWidth = "0px";
        this.GetClientAreaStyle().borderRightWidth = "0px";
        this.GetClientAreaStyle().borderStyle = "solid";
        this.GetClientAreaStyle().borderColor = UI._selected_background_colour;
        this.SetBottomAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 41 });
    }
};

class OJTabContainer extends OJGrid
{
    constructor()
    {
        // Base class constructor
        super();
        this.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 50 });
    }
};

export class OJTabControl extends OJGrid
{
    constructor(options)
    {
        // Base class constructor
        super();
        this._class_name = "OJTabControl";
        this.SetElementName("OJTabControl");

        let container = this.GetElement();

        this._tabs = [];
        container.className = "tab_control_container_class"

        this._width = 300;
        this._height = 800;
        this._select_tab_callback = null;
        this._context_menu = null;
        this._hide_tab_buttons = false;
        this._tab_header_bar = null;

        this._tile_page_container = new OJTabContainer();
        this.AddChild(this._tile_page_container);

        this._tab_header_bar = new OJTabHeaderBar();
        this.AddChild(this._tab_header_bar);
        this._index = -1;
    }

    Destroy()
    {
        OJLib.DestroyArray(this._tabs);
        this._tabs = null;

        super.Destroy();
    }

    SetSelectTabCallback(callback)
    {
        this._select_tab_callback = callback;
    }

    Reveal()
    {
        this.SetTab(this._index, false);
        this.GetElement().style.opacity = 1;
    }

    HideTabButtons(state)
    {
        this._hide_tab_buttons = state;

        let show = state ? "none" : "block";

        if (this._warning_label)
            this._warning_label.style.display = show;
    }

    // OJTab new_tab. The tab is a grid that contains the tab selector button and indicator.
    // It also hold the _oj_window which is the window contents for that tab
    AddTab(new_tab)
    {
        let tab_index = this._tabs.length;
        new_tab.SetIndex(tab_index);

        this._tabs.push(new_tab);
        this._tile_page_container.AddChild(new_tab._oj_window);

        this._tab_header_bar.AddChild(new_tab);
        if (this._tab_header_bar.GetNumChildren() > 1)
        {
            // Attach the new button to the right of the previous one
            let previous_button = this._tab_header_bar.GetChild(this._tab_header_bar.GetNumChildren() - 2);
            new_tab.SetLeftAnchor({ _type : ANCHOR_TYPE.SIBLING_FAR, _sibling: previous_button, _fixed_offset : 2 });
        }

        new_tab._index = this._tabs.length - 1;
        new_tab._tab_control = this;

        new_tab.SetSelected(false);
    }

    CheckNumberOfTabs()
    {
        if (this._tabs.length <= 1)
        {
            // Only need tab header if more than 1 tab
            this._tab_header_bar.Show(false);
            this._tile_page_container.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 0 });
        }
    }

    SetTab(index, activate)
    {
        let do_activate = (activate == null) ? false : activate;

        if (this._tabs.length == 0)
            return;

        let current_tab = null;
        let deselect_index = this._index;

        if (this._index >= 0)
            current_tab = this._tabs[this._index];

        if (index >= this._tabs.length)
            index = 0;

        this._index = index;

        let active_tab = null;
        if (this._index >= 0)
            active_tab = this._tabs[this._index];

        if ((current_tab != null) && (current_tab === active_tab))
        {
            // Turn on current tab again
            active_tab.SetSelected(true);

            if (do_activate)
                active_tab.SetActive(true);
        }
        else
        {
            // Turn off current tab
            if (current_tab != null)
            {
                if (do_activate)
                    current_tab.SetActive(false);

                current_tab.SetSelected(false);
            }

            // Turn on current tab
            if (active_tab != null)
            {
                active_tab.SetSelected(true);

                if (do_activate)
                    active_tab.SetActive(true);
            }

            // Don't call callback is same tab is reselected
            let enabledTabParams = `${this._index},${deselect_index}`;
            OJLib.ServerExecuteWithParams("TopLevelTabEnabled", enabledTabParams);
        }
    }

    GetSelectedTabIndex()
    {
        return this._index;
    }
}

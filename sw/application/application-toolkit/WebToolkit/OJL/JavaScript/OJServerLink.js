/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJAlphaNumericKeypad } from "./OJL.js"
import { OJNumericKeypad } from "./OJL.js";
import { OJColourPickerDialog } from "./OJL.js"
import { OJControlContainer } from "./OJL.js"
import { OJCommandController } from "./OJL.js";
import { OJMessageDialog } from "./OJL.js";
import { OJPictureDialog } from "./OJL.js";
import { OJPictureInPictureDialog } from "./OJL.js";
import { OJSettingsDialog } from "./OJL.js";
import { OJWarpFullControlDialog } from "./OJL.js";
import { OJTmoRoiDialog } from "./OJL.js";
import { OJTextViewDialog } from "./OJL.js";
import { OJLib, UI } from "./OJL.js";
import { OJWebSocket } from "./OJL.js";
import { OJFileUploader } from "./OJL.js";
import { OJDesktopScreen } from "./OJL.js";
import { OJImage } from "./OJL.js";
import { OJGrid } from "./OJL.js";
import { OJLabel } from "./OJL.js";
import { OJPictureButton } from "./OJL.js";
import { OJScrollable } from "./OJL.js";
import { ANCHOR_TYPE } from "./OJL.js";
import { TEXT_CONTROL_TYPE } from "./OJL.js";
import { OJWaiting } from "./OJL.js";
import { OJImageResizer } from "./OJL.js";
import { OJTabControl, OJTab } from "./OJL.js";
import { OJLogger } from "./OJL.js";

export class OJServerLink
{
    static _server_link;
    static _control_factory_functions = {};

    // Dialog factories
    static _dialog_factories = {
        "PictureDialog"     : (params, context) => new OJPictureDialog(),
        "PictureInPicture"  : (params, context) => new OJPictureInPictureDialog(params),
        "SettingsDialog"    : (params, context) => new OJSettingsDialog(context._settings_containers),
        "WarpControls"      : (params, context) => new OJWarpFullControlDialog(params),
        "TmoRoiDialog"      : (params, context) => new OJTmoRoiDialog(params),
        "TextViewDialog"    : (params, context) => new OJTextViewDialog(params),
        "MessageDialog"     : (params, context) => {
            if (params.message != null) {
                return new OJMessageDialog(params.message,
                                          params.button_0,
                                          params.button_1,
                                          null,
                                          null,
                                          params.width,
                                          params.height);
            }
            return null;
        },
        "ColourPicker"     : (params, context) => new OJColourPickerDialog(params)
    };    

    constructor(websocket_server_url)
    {
        this._open = false;
        this._show_in_commands = false;
        this._show_out_commands = false;
        this._command_controllers = [];
        this._control_items = {};
        this._pip_update = null;
        this._modal_dialog = null;
        this._ui_loaded = false;
        this._numeric_keypad = null;
        this._client_id = 0;
        this._control_containers = {};
        this._custom_controls = {};
        this._settings_containers = [];
        this._service_unavailable = false;
        this._websocket_server_url = websocket_server_url;
        this._hover_ui_colour = "#5fc8ef";
        this._startup_info = "";
        this._pause_timeout = null;
        this._uploader = null;
        this._first_ping = true;
        this._debugging = false;
        this._ping_counter = 0;
        this._last_ping_check = 9999;
        this._server_dead = false;
        this._image_resizer = null;
        OJServerLink._server_link = this;
    }

    Destroy()
    {
        this._top_level_screen.Destroy();
        this._top_level_screen = null;

        this._numeric_keypad.Destroy();
        this._numeric_keypad = null;

        this._alphanumeric_keypad.Destroy();
        this._alphanumeric_keypad = null;

        // this._softkeys_bar = null;
        this.CloseDialogs();
        this._ui_loaded = false;

        this._ui_web_socket.Destroy();
        this._ui_web_socket = null;

        if (this._image_resizer != null)
            this._image_resizer.Destroy();

        this._control_containers = {};
        this._custom_controls = {};
        this._settings_containers.length = 0;
        this._command_controllers.length = 0;
        OJServerLink._server_link = null;
    }

    static Get() { return OJServerLink._server_link; }

    AddDialog(dialog_name, factory_function)
    {
        OJServerLink._dialog_factories[dialog_name] = factory_function;
    }    

    StartUI()
    {
        this._ui_web_socket = new OJWebSocket(this._websocket_server_url, "UiCommandProcessor", this);
    }

    OnWebSocketServiceUnavailable(reason)
    {
        // The web socket connected but the service didn't start up. See 'reason'
        let startup_text = document.getElementById("_start_text");

        startup_text.innerHTML = reason;
        this._service_unavailable = true;
        OJLib.Trace(reason);
    }

    // Called once the private protocol has been negotiated
    OnWebSocketReady(event)
    {
        this._open = true;

        // We can now create our file uploaders
        this.ResetUploaders();

        // Now fetch the UI from the server
        this.GetMainUserInterface();
    }

    WebSocketDestroyed(event)
    {
        if (!this._service_unavailable)
            OJLib.ServerDead(true);
    }

    OnMessageWebSocket(web_socket, message)
    {
        // Async message received from server intermediate layer.
        // The message is XML as one string
        if (message._is_binary)
            return; // Not handled

        let text = message._text_data;
        if (text == "")
            return;

        if (this._show_in_commands)
        {
            OJLib.Trace("Receive: " + text, false);
        }

        let text_length = text.length;
        let json_object = JSON.parse(text);

        let json_main_ui = json_object.MainUserInterface;
        if (json_main_ui != null)
        {
            this.LoadMainUiFromJson(json_main_ui);
            return;
        }

        if (!this._ui_loaded)
            return;

        let json_ui_update = json_object.UiUpdate;
        if (json_ui_update != null)
        {
            this.UiUpdate(json_ui_update);
            return;
        }
    }

    ContinueReveal()
    {
        let element = this._top_level_screen.GetElement();

        element.style.opacity = "1.0";
        let animation_time = " 300ms";
        element.style.animationFillMode = "both";
        element.style.webkitAnimationFillMode = "both";
        element.style.animation = "logo_start " + animation_time;
        element.style.webkitAnimation = "logo_start " + animation_time;

        this._top_level_screen.Show(true);
    }

    // Find the tile, remove it from the list and return it to the caller
    RemoveTile(tile_id)
    {
        if (this._view_window != null)
            return this._view_window.RemoveTile(tile_id);
    }

    AddUiUpdateCommands()
    {
        this._ui_update_commands = [];

        this.AddCommandHandler(new OJCommandController("PiPUpdate", this, "PiPUpdateCB"));
        this.AddCommandHandler(new OJCommandController("ModalDialog", this, "ModalDialogCB"));
        this.AddCommandHandler(new OJCommandController("NewInfoLabel", this, "NewInfoLabelCB"));
        this.AddCommandHandler(new OJCommandController("UiElementValueUpdate", this, "UiElementValueUpdateCB"));
        this.AddCommandHandler(new OJCommandController("UiElementLabelUpdate", this, "UiElementLabelUpdateCB"));
        this.AddCommandHandler(new OJCommandController("UiElementEnabledUpdate", this, "UiElementEnabledUpdateCB"));
        this.AddCommandHandler(new OJCommandController("UiControlContainerShow", this, "UiControlContainerShowCB"));
        this.AddCommandHandler(new OJCommandController("UiControlsUpdate", this, "UiControlsUpdateCB"));
        this.AddCommandHandler(new OJCommandController("UiControlEnumValueRemove", this, "UiControlEnumRemoveCB"));
        this.AddCommandHandler(new OJCommandController("UiControlUpdateRange", this, "UiControlUpdateRangeCB"));
        this.AddCommandHandler(new OJCommandController("UiControlEnumValueAdd", this, "UiControlEnumAddCB"));
        this.AddCommandHandler(new OJCommandController("ImportFile", this, "ImportFileCB"));
        this.AddCommandHandler(new OJCommandController("ExportFile", this, "ExportFileCB"));
        this.AddCommandHandler(new OJCommandController("CallFunctionOnControl", this, "CallFunctionOnControlCB"));
        this.AddCommandHandler(new OJCommandController("UiElementShow", this, "UiElementShowCB"));
        this.AddCommandHandler(new OJCommandController("Ping", this, "PingCB"));
        this.AddCommandHandler(new OJCommandController("ShowWaiting", this, "ShowWaitingCB"));
        this.AddCommandHandler(new OJCommandController("LoadImageFile", this, "LoadImageFileCB"));
    }

    AddCommandHandler(command_handler)
    {
        this._ui_update_commands.push(command_handler);
    }

    RemoveCommandHandler(command_handler)
    {
        let index = this._ui_update_commands.indexOf(command_handler);
        this._ui_update_commands.splice(index, 1);
    }

    CloseDialogs()
    {
        if (this._modal_dialog)
        {
            this._top_level_screen.RemoveChild(this._modal_dialog);
            this._modal_dialog.Destroy();
            this._modal_dialog = null;
        }
    }

    DestroyPictureDialog()
    {
        if (this._picture_dialog != null)
        {
            this._top_level_screen.RemoveChild(this._picture_dialog);
            this._picture_dialog.Destroy();
            this._picture_dialog = null;
        }
    }

    ModalDialogCB(params)
    {
        this.CloseDialogs();
        UI.SetStyleAttribute(this._desktop_screen_grid._client_area.style, "filter", null);

        if (params.dialog_type == "CloseDialogs")
        {
            return;
        }

        const factory = OJServerLink._dialog_factories[params.dialog_type];

        if (factory)
        {
            this._modal_dialog = factory(params, this);
        }

        if (this._modal_dialog != null)
        {
            this._top_level_screen.AddChild(this._modal_dialog, true);
            this._modal_dialog.Show();
        }
    }

    ExportFileCB(params)
    {
        let file_name = params.file_name;
        let download_url = params.url;
        let save = document.createElement("a");
        save.href = download_url;
        save.download = params.file_name;

        let event = document.createEvent("MouseEvents");
        event.initMouseEvent(
            "click", true, false, window, 0, 0, 0, 0, 0
            , false, false, false, false, 0, null
        );
        save.dispatchEvent(event);
    }

    AddControlItem(control_item)
    {
        let unique_id_str = "" + control_item._unique_id;
        this._control_items[unique_id_str] = control_item;
    }

    RemoveControlItem(control_item)
    {
        let unique_id_str = "" + control_item._unique_id;
        delete this._control_items[unique_id_str];
    }

    UiElementValueUpdateCB(params)
    {
        let unique_id = params.id;
        let value = params.value;

        let control_item = this._control_items[unique_id];
        if (control_item != null)
        {
            // UpdateValue will parse the string value into its _value param
            control_item.UpdateValue(value);
        }
    }

    CallFunctionOnControlCB(params)
    {
        let unique_id = params.control_id;
        let function_name = params.function_name;
        let value = params.parameter;

        let control_item = this._control_items[unique_id];
        if (control_item != null)
        {
            if (function_name in control_item)
                control_item[function_name](value);
        }
    }

    UiControlEnumRemoveCB(params)
    {
        let unique_id = params.id;
        let value = params.value;

        let control_item = this._control_items[unique_id];
        if (control_item != null)
        {
            control_item.RemoveItem(value);
        }
    }

    UiControlEnumAddCB(params)
    {
        let unique_id = params.id;
        let value = params.value;

        let control_item = this._control_items[unique_id];
        if (control_item != null)
        {
            control_item.AddItem(value);
        }
    }

    UiControlUpdateRangeCB(params)
    {
        let unique_id = params.id;
        let min = params.min;
        let max = params.max;

        let control_item = this._control_items[unique_id];

        if (control_item != null)
        {
            if(typeof control_item.UpdateRange === 'function')
            {
                control_item.UpdateRange(min, max);
            }
        }
    }

    UiElementLabelUpdateCB(params)
    {
        let unique_id = params.id;
        let label = params.label;

        let control_item = this._control_items[unique_id];
        if (control_item != null)
        {
            // UpdateValue will parse the string value into its _value param
            control_item.UpdateLabel(label);
        }
    }

    UiElementEnabledUpdateCB(params)
    {
        let unique_id = params.id;
        let enabled = params.enabled;
        let control_item = this._control_items[unique_id];

        if (control_item != null)
        {
            control_item.Enable(enabled);
        }
    }

    UiElementShowCB(params)
    {
        let unique_id = params.id;
        let show = params.show;
        let control_item = this._control_items[unique_id];

        if (control_item != null)
        {
            control_item.Show(show);
        }
    }

    ShowWaitingCB(params)
    {
        let show = params.show;
        this.ShowWaiting(show);
    }

    LoadImageFileCB(params)
    {
        if (this._image_resizer != null)
            this._image_resizer.Destroy();

        this._image_resizer = new OJImageResizer(params);
    }

    PingCB(params)
    {
        this._ping_counter = params.counter;

        if (this._first_ping && !this._debugging)
        {
            this._first_ping = false;
            setInterval(this.CheckPing.bind(this), 5000);
        }
    }

    CheckPing()
    {
        if (this._ping_counter == this._last_ping_check)
        {
            if (!this._server_dead)
            {
                OJLib.ShowServerDeadOverlay(true);
                this._server_dead = true;
            }
        }
        else if (this._server_dead)
        {
            // It's back
            OJLib.ShowServerDeadOverlay(false);
            this._server_dead = false;
        }

        this._last_ping_check = this._ping_counter;
    }

    UiControlContainerShowCB(params)
    {
        let unique_id = params.id;
        let show = params.show;
        let container = this._control_containers[unique_id];

        if (container)
        {
            container.Show(show);
            if (show)
                UI.FadeIn(container.GetElement(), 1000);
        }
    }

    GetControlItem(unique_id)
    {
        let control_item = this._control_items[unique_id];
        if (control_item == null)
            control_item = this._custom_controls[unique_id];
        return control_item;
    }

    UiControlsUpdateCB(params)
    {
        this._control_containers = {};
        this._custom_controls = {};
        this._control_page.RemoveAllChildren();
        this.LoadControls(params.Controls, this._control_page);
    }

    UiUpdate(json_ui_update)
    {
        for (let i = 0; i < this._command_controllers.length; i++)
        {
            let command_controller = this._command_controllers[i];
            if (command_controller.Process(json_ui_update))
                return;
        }
    }

    RegisterCommandController(command_controller)
    {
        OJLib.AddObserver(this._command_controllers, command_controller);
    }

    UnregisterCommandController(command_controller)
    {
        OJLib.RemoveObserver(this._command_controllers, command_controller);
    }

    ShowKeypad(text_control)
    {
        this._numeric_keypad_text_control = text_control;

        let keypad = null;
        if (text_control._control_type == TEXT_CONTROL_TYPE.TCT_TEXT)
            keypad = this._alphanumeric_keypad;
        else
            keypad = this._numeric_keypad;

        // Copy the control type
        keypad.SetControlType(text_control._control_type);

        // Copy the value
        keypad.SetValue(text_control._input_control.value);

        let control_element = text_control.GetElement();
        let rectangle = UI.GetBoundingClientRect(control_element);

        let x = rectangle.left;
        let y = rectangle.top;
        let width = keypad._width;
        let height = keypad._height;

        if ((y + height) > UI._screen_height)
            y = UI._screen_height - (height + 10);
        if ((x + width) > UI._screen_width)
            x = UI._screen_width - (width + 10);

        keypad.Resize(x, y, width, height);
        keypad.GetElement().style.display = "block";

        UI._current_modal_dialog_class_name = keypad._class_name;
    }

    HideKeypad(new_value)
    {
        if (new_value != null)
        {
            this._numeric_keypad_text_control.SetFromKeyPad(new_value);
        }

        this._numeric_keypad.GetElement().style.display = "none";
        this._alphanumeric_keypad.GetElement().style.display = "none";
        UI._current_modal_dialog_class_name = "";
    }

    LoadMainUiFromJson(json_ui)
    {
        let body = document.getElementById("_body");

        this.LoadUiSettings(json_ui.UiSettings);

        // Update the font family
        UI._font_family = json_ui.font_family;
        UI._font_name = UI._font_height.toFixed(0) + "px " + UI._font_family;
        UI._small_font_name = UI._small_font_height.toFixed(0) + "px " + UI._font_family;
        UI._large_font_name = UI._large_font_height.toFixed(0) + "px " + UI._font_family;
        UI._extra_large_font_name = UI._extra_large_font_height.toFixed(0) + "px " + UI._font_family;

        this._product_name = json_ui.product_name;
        this._hostname = json_ui.hostname;
        this._os_name = json_ui.os_name;
        this._build_date = json_ui.build_date;
        this._build_time = json_ui.build_time;
        this._software_version = json_ui.software_version;
        this._debugging = json_ui.debugging;

        UI._product_name = this._product_name;
        UI._fixed_window_width = json_ui.window_width;
        UI._fixed_window_height = json_ui.window_height;

        document.title = this._product_name;

        UI.SetBodyScale(false);

        let product_tooltip = this._product_name + " - running on host " + this._hostname + ". OS: " + this._os_name;
        product_tooltip += ". Software version: V" + this._software_version;

        this._top_level_screen = new OJDesktopScreen(body);
        this._top_level_screen.Show(false);

        this._desktop_screen_grid = new OJGrid();
        this._desktop_screen_grid.SetBackgroundColour("#ffffff");
        this._top_level_screen.AddChild(this._desktop_screen_grid);

        let border_height = 1;
        this._header_grid = new OJGrid();
        this._header_grid.SetBorders(0, 0, 0, border_height);
        this._header_grid.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 80 });
        this._desktop_screen_grid.AddChild(this._header_grid);

        this._fade_div = null;
        this._logo = new OJImage(OJLib._logo_image._src, true);
        this._logo.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 80 });
        this._logo.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 80 });

        this._header_grid.AddChild(this._logo);
        this._header_grid.SetBackgroundColour("#00377c");
        this._header_grid.SetColour("#ffffff");

        this._header_label = new OJLabel(this._product_name, { _alignment: "left", _text_color: "#0071c5", _font_size: 23 });
        this._header_label.SetToolTip(product_tooltip);
        this._header_label.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 80 });
        this._header_label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 100 });
        this._header_grid.AddChild(this._header_label);

        let settings_button_details = { _object: this, _object_callback: "OnSettings" };
        let settings_button_size= { _width: 48, _height: 48 };
        this._settings_button = new OJPictureButton(null, OJLib._settings48_button._src, null, settings_button_details, settings_button_size);
        this._settings_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -84 });
        this._settings_button.SetToolTip("Settings");

        this._header_grid.AddChild(this._settings_button);

        this._control_page = new OJGrid();
        this._control_page.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 80 });
        this._desktop_screen_grid.AddChild(this._control_page);

        // Full screen black page used as background when a child window is made full screen
        // So any corners are black
        this._full_screen_black_page = new OJGrid();
        this._full_screen_black_page.SetBackgroundColour("#000000");
        this._full_screen_black_page.Show(false);
        this._desktop_screen_grid.AddChild(this._full_screen_black_page);

        this._numeric_keypad = new OJNumericKeypad(body);
        this._alphanumeric_keypad = new OJAlphaNumericKeypad(body);

        let info_labels = json_ui.InfoLabels;
        this._startup_info = "";
        if (info_labels != null)
        {
            for (let i = 0; i < info_labels.length; i++)
            {
                let info_label_json = info_labels[i];
                this._startup_info += info_label_json.label;
                if (i != (info_labels.length - 1))
                    this._startup_info += "<br/>";
            }
        }

        this.AddControlFactory("UiControlContainer", this.CreateControlContainer);

        this.LoadTabs(json_ui.Tabs, this._control_page);

        this._settings = json_ui.Settings;

        this._h_margin_left = 0;
        this._h_margin_right = 0;
        this._v_margin_top = 0;
        this._v_margin_bottom = 0;

        let desktop_grid_width = UI._screen_width - this._h_margin_left - this._h_margin_right;
        let desktop_grid_height = UI._screen_height - this._v_margin_top - this._v_margin_bottom;

        this._top_level_screen.Resize(this._h_margin_left, this._v_margin_top, desktop_grid_width, desktop_grid_height);

        this._ui_loaded = true;
        this._hide_ui_timeout = setTimeout(HideLogoTimeout, 1500, this);
    }

    ShowWaiting(state)
    {
        if (state)
        {
            this._waiting = new OJWaiting();
            this._waiting.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _dynamic_position: 50, _fixed_offset: -60 });
            this._waiting.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 120 });
            this._waiting.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _dynamic_position: 50, _fixed_offset: -60 });
            this._waiting.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 120 });
            this._desktop_screen_grid.AddChild(this._waiting, true);
            this._waiting.Show(true);
        }
        else if (this._waiting != null)
        {
            this._desktop_screen_grid.RemoveChild(this._waiting);
            this._waiting.Destroy();
            this._waiting = null;
        }
    }

    CreateControlContainer(control_containers_list,
                           custom_controls_list,
                           control_page,
                           params)
    {
        let control_container = new OJControlContainer(control_containers_list, params);
        control_containers_list[control_container._unique_id] = control_container;
        control_page.AddChild(control_container, null, null, null, true);

        return { _control_container: control_container };
    };

    AddControlFactory(control_name, factory_create_fn)
    {
        OJServerLink._control_factory_functions[control_name] = factory_create_fn;
    }

    LoadTabs(tabsJSON, parent_window)
    {
        this._tab_control = new OJTabControl(true, null);
        this._tab_control.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 10 });
        this._tab_control.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 10 });
        this._tab_control.SetRightAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -10 });
        this._tab_control.SetBottomAnchor({ _type: ANCHOR_TYPE.PARENT_FAR, _fixed_offset: -10 });

        parent_window.AddChild(this._tab_control);

        let tabIndex = 0;

        for (let tabJSON of tabsJSON)
        {
            let tab_control_grid = new OJGrid();
            tab_control_grid.SetRightAnchor({ _type: ANCHOR_TYPE.FIT_CHILDREN, _fixed_offset: 0 });
            tab_control_grid.SetBottomAnchor({ _type: ANCHOR_TYPE.FIT_CHILDREN, _fixed_offset: 0 });

            let scrollable_control_page = new OJScrollable(null, { _auto_centre: true } );
            scrollable_control_page.AddScrollableChild(tab_control_grid);

            let tab_id = tabIndex;
            let tab = new OJTab(scrollable_control_page, tabJSON.title, tab_id, tabJSON.tooltip, "")

            this._tab_control.AddTab(tab);

            // Find control page from tab
            this.LoadControls(tabJSON.Controls, tab_control_grid);        

            tabIndex++;
        }

        this._tab_control.CheckNumberOfTabs();
        this._tab_control.SetTab(0, true);
    }

    LoadControls(controls, parent_window)
    {
        let new_controls = { _control_containers: [], _custom_controls: [] };
        if (controls == null)
            return new_controls;

        let new_control_containers = [];
        let new_custom_controls = [];

        for (let ui_element of controls)
        {
            let factory_create_fn = OJServerLink._control_factory_functions[ui_element.element_type];
            if (factory_create_fn)
            {
                // The factory function can add items to either
                // _control_containers, or _custom_controls
                let new_items = factory_create_fn(this._control_containers,
                                                this._custom_controls,
                                                parent_window,
                                                ui_element);

                if (new_items._control_container)
                    new_control_containers.push(new_items._control_container);
                if (new_items._custom_control)
                    new_custom_controls.push(new_items._custom_control);
            }
        }

        for (let custom_control of new_custom_controls)
        {
            if (custom_control.LoadComplete)
                custom_control.LoadComplete();
        }

        new_controls._control_containers = new_control_containers;
        new_controls._custom_controls = new_custom_controls;

        // Assign the objects to each client callback now that all the controls are loaded
        this.UpdateClientCallbacks();

        return new_controls;
    }

    RemoveControls(controls)
    {
        let control_containers = controls._control_containers;
        let custom_controls = controls._custom_controls;

        for (let control_container of control_containers)
            delete this._control_containers[control_container._unique_id];

        for (let custom_control of custom_controls)
            delete this._custom_controls[custom_control._unique_id];
    }

    UpdateClientCallbacks()
    {
        for (let control_item_id in this._control_items)
        {
            let control_item = this._control_items[control_item_id];
            control_item.UpdateClientCallbackObject(this);
        }
    }

    MakeAnchorParams(attachment_details,
                     sibling_containers)
    {
        let type = attachment_details.type;
        let sibling_id = attachment_details.sibling_id;
        let size_or_offset = attachment_details.size_or_offset;
        let dynamic_position = attachment_details.dynamic_position;

        let anchor_params = { _type: type };
        if (type == ANCHOR_TYPE.FIXED_SIZE)
        {
            anchor_params._fixed_size = size_or_offset;
        }
        else
        {
            anchor_params._fixed_offset = size_or_offset;
            if (dynamic_position != 9999.0)
                anchor_params._dynamic_position = dynamic_position;
        }

        if (sibling_id != 0)
        {
            let sibling = sibling_containers[sibling_id];
            anchor_params._sibling = sibling;
        }

        return anchor_params;
    };

    PositionCustomControl(control, siblings, params)
    {
        let left_anchor_params = this.MakeAnchorParams(params.LeftAttachment, siblings);
        let top_anchor_params = this.MakeAnchorParams(params.TopAttachment, siblings);
        let right_anchor_params = this.MakeAnchorParams(params.RightAttachment, siblings);
        let bottom_anchor_params = this.MakeAnchorParams(params.BottomAttachment, siblings);

        control.SetLeftAnchor(left_anchor_params);
        control.SetTopAnchor(top_anchor_params);
        control.SetRightAnchor(right_anchor_params);
        control.SetBottomAnchor(bottom_anchor_params);
    }

    LoadSettings(settings)
    {
        this._settings_containers.length = 0;

        if (settings == null)
            return;

        for (let ui_element of settings)
        {
            if (ui_element.element_type == "UiControlContainer")
            {
                let control_container = new OJControlContainer(this._settings_containers, ui_element);
                this._settings_containers.push(control_container);
            }
        }
    }

    AddInfoLabel(label_text)
    {
        let label_height = 16;

        let i = this._info_label_panel.GetNumChildren();
        if (i > 100)
        {
            this._info_label_panel.RemoveAll(true, true);
            this._info_scrollable.ChildSizeChanged();
        }

        let label = new OJLabel(label_text, { _alignment: "left", _font: "16px courier" });
        label.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: 10 });
        label.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: label_height });
        this._info_label_panel.AddChild(label, false, true);
        this._info_scrollable.ChildSizeChanged();
        this._info_scrollable.ScrollToBottom();
    }

    ButtonCallback(event)
    {
        this.ExecuteServerCommandWithParams(event._user_data, "");
    }

    ClearTraceLog(event)
    {
        this._info_label_panel.RemoveAll(true, true);
        this._info_scrollable.ChildSizeChanged();
    }

    NewInfoLabelCB(new_info_json)
    {
        let label = new_info_json.label;
        OJLib.Trace(label);
    }

    LoadUiSettings(ui_settings)
    {
        this._altera_light_blue = ui_settings.altera_light_blue;
        this._altera_dark_blue = ui_settings.altera_dark_blue;
        this._altera_background = ui_settings.altera_background;
        this._altera_child_window_background = ui_settings.altera_child_window_background;
        this._altera_text_colour = ui_settings.altera_text_colour;
        this._clicked_colour = ui_settings.clicked_colour;
        this._mouse_over_colour = ui_settings.mouse_over_colour;
    }

    Reveal()
    {
    }

    UiTrace(message_val)
    {
        let message = "" + message_val;
        let no_commas_message = message.replace(/,/g, ";");
        this.ExecuteServerCommandWithParams("UiTrace", no_commas_message);
    }

    SendCommand(json_command)
    {
        if (this._show_out_commands)
            OJLib.Trace("Send: " + json_command, false);

        this._ui_web_socket.Send(json_command);
    }

    ExecuteServerCommand(server_command)
    {
        let command_object = { Command: { value_with_params: server_command } };
        let json_string = JSON.stringify(command_object);
        this.SendCommand(json_string);
    }

    ExecuteServerCommandWithParams(server_command, parameters)
    {
        let command_object = { Command: { value: server_command, params: parameters } };
        let json_string = JSON.stringify(command_object);
        this.SendCommand(json_string);
    }

    ExecuteServerCommandJson(command_object)
    {
        let json_string = JSON.stringify(command_object);
        this.SendCommand(json_string);
    }

    GetMainUserInterface()
    {
        console.log("GetMainUserInterface");
        this.ExecuteServerCommand("GetMainUserInterface");
    }

    ShutdownServer()
    {
        OJLib._shutdown_from_ui = true;
        this.ExecuteServerCommand("ShutdownServer");
    }

    RebootServer()
    {
        this.ExecuteServerCommand("RebootServer");
    }

    ShowCommands(in_state, out_state)
    {
        if (out_state == null)
            out_state = in_state;

        this.ExecuteServerCommandWithParams("ShowCommands", in_state + "," + out_state);

        this._show_in_commands = in_state;
        this._show_out_commands = out_state;
    }

    ResetUiSize()
    {
        if (UI.SetBodyScale(true))
        {
            let desktop_grid_width = UI._screen_width - this._h_margin_left - this._h_margin_right;
            let desktop_grid_height = UI._screen_height - this._v_margin_top - this._v_margin_bottom;
            this._top_level_screen.Resize(this._h_margin_left, this._v_margin_top, desktop_grid_width, desktop_grid_height);
        }

        this.CallOnResizeAction();
    }

    CallOnResizeAction()
    {
        if (this._on_resize_action != null)
        {
            this._on_resize_action.Call();
            this._on_resize_action.Destroy();
            this._on_resize_action = null;
        }
    }

    NetworkSelectedCB(param)
    {
        let index = param._index;

        this._networks_tab.SetTab(index, true);
        this._networks_tab.current_index = index;
    }

    OnSettings()
    {
        this.LoadSettings(this._settings);
        this.ModalDialogCB({ dialog_type: "SettingsDialog" });
    }

    ImportFileCB(params)
    {
        let open_dialog_title = params.open_dialog_title;
        let destination_path = params.destination;
        let extension = params.extension;
        let multiple_files = params.multiple_files;

        if (this._uploader)
            this._uploader.SelectFiles("ImportFile", open_dialog_title, multiple_files, extension);
    }

    ResetUploaders()
    {
        if (this._uploader != null)
            this._uploader.Destroy();

        this._uploader = null;
        this._uploader = new OJFileUploader(this._websocket_server_url);
    }
}

function HideLogoTimeout(server_link)
{
    let startup_logo = document.getElementById("_start_logo");
    let startup_text = document.getElementById("_start_text");
    let startup_text_2 = document.getElementById("_start_text_2");

    let animation_time = " 700ms";

    if (startup_logo != null)
    {
        startup_logo.style.animationFillMode = "both";
        startup_logo.style.animation = "hide_logo " + animation_time;
        startup_logo.style.opacity = "0.0";
    }

    if (startup_text != null)
    {
        startup_text.style.animationFillMode = "both";
        startup_text.style.animation = "hide_logo " + animation_time;
        startup_text.style.opacity = "0.0";
    }

    if (startup_text_2 != null)
    {
        startup_text_2.style.animationFillMode = "both";
        startup_text_2.style.animation = "hide_logo " + animation_time;
        startup_text_2.style.opacity = "0.0";
    }

    this._show_ui_timeout = setTimeout(ShowUiTimeout, 400, server_link);
}

function ShowUiTimeout(server_link)
{
    let startup_logo = document.getElementById("_start_logo");
    let startup_text = document.getElementById("_start_text");
    let startup_text_2 = document.getElementById("_start_text_2");
    let startup_background = document.getElementById("_startup_background");

    if (startup_background != null)
    {
        startup_background.style.display = "none";
        UI.RemoveFromParentElement(startup_background);
    }

    if (startup_logo != null)
    {
        startup_logo.style.display = "none";
        UI.RemoveFromParentElement(startup_logo);
    }

    if (startup_text != null)
    {
        startup_text.style.display = "none";
        UI.RemoveFromParentElement(startup_text);
    }

    if (startup_text_2 != null)
    {
        startup_text_2.style.display = "none";
        UI.RemoveFromParentElement(startup_text_2);
    }

    UI.SetBodyScale(true);

    OJServerLink.Get().ContinueReveal();
}


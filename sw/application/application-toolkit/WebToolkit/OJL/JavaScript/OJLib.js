/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";
import { OJServerLink } from "./OJL.js";
import { OJPanel } from "./OJL.js";
import { TEXT_CONTROL_TYPE } from "./OJL.js";
import { OJRect } from "./OJL.js";
import { OJLibExtrasInit } from "./OJLibExtras.js";

let oj_lib = null;

// Main entry point, called from HTML when loaded
document.addEventListener('DOMContentLoaded', function() 
{
    // Instantiate OJLib library
    oj_lib = new OJLib();
});

export class UI
{
    static SetBodyScale(set_scale)
    {
        let body = document.getElementById("_body");

        let device_pixel_ratio = window.devicePixelRatio;

        UI._device_pixel_scale = device_pixel_ratio;
        let use_fixed_window = (UI._fixed_window_width * UI._fixed_window_height) != 0;

        let use_width = 0;
        let use_height = 0;

        if (use_fixed_window)
        {
            use_width = UI._fixed_window_width;
            use_height = UI._fixed_window_height;
        }
        else
        {
            use_width = window.innerWidth;
            use_height = window.innerHeight;
        }

        if (window.IsMobileDevice())
        {
            use_width = window.innerWidth;
            use_height = window.innerHeight;
            UI._screen_width = use_width;
            UI._screen_height = use_height;
            UI._body_scale = 1;
            body.style.transform = "scale(" + UI._body_scale + ")";
            body.style.transformOrigin = "0% 0%";
            body.style.WebkitTransform = "scale(" + UI._body_scale + ")";
            body.style.WebkitTransformOrigin = "0% 0%";
            return;
        }

        let resized = (use_width != UI._screen_width) || (use_height != UI._screen_height);

        UI._screen_width = use_width;
        UI._screen_height = use_height;
        let previous_body_scale = UI._body_scale;

        let large = false; // (UI._screen_height > UI._fixed_window_height) && (UI._screen_width > UI._fixed_window_width);

        if ((UI._screen_height < UI._fixed_window_height) || (UI._screen_width < UI._fixed_window_width) || large)
        {
            let ratio = use_width / use_height;

            let h_scale = UI._screen_height / UI._fixed_window_height;
            let v_scale = UI._screen_width / UI._fixed_window_width;

            let scale = h_scale;
            if (h_scale < v_scale)
            {
                scale = h_scale;
                UI._screen_height = UI._fixed_window_height;
                UI._screen_width = UI._screen_height * ratio;
            }
            else
            {
                scale = v_scale;
                UI._screen_width = UI._fixed_window_width;
                UI._screen_height = UI._screen_width / ratio;
            }

            UI._body_scale = scale;

            if (set_scale)
            {
                body.style.transform = "scale(" + UI._body_scale + ")";
                body.style.transformOrigin = "0% 0%";
                body.style.WebkitTransform = "scale(" + UI._body_scale + ")";
                body.style.WebkitTransformOrigin = "0% 0%";
            }
        }
        else
        {
            UI.SetStyleAttribute(body.style, "transformOrigin", null);
            UI.SetStyleAttribute(body.style, "transform", null);
            UI.SetStyleAttribute(body.style, "WebkitTransform", null);
            UI.SetStyleAttribute(body.style, "WebkitTransformOrigin", null);
            UI._body_scale = 1;
        }

        if (UI._body_scale != previous_body_scale)
        {
            resized = true;
        }

        return resized;
    }

    static SetBackgroundColour(element, colour)
    {
        UI.SetStyleAttribute(element.style, "backgroundColor", colour);
    }
    
    static SetColour(element, colour)
    {
        UI.SetStyleAttribute(element.style, "color", colour);
    }

    static SetStyleAttribute(style, attribute_name, value)
    {
        // Required for style removal in IE
        if (value == null)
        {
            style[attribute_name] = ""; // Edge support
            if (style.removeAttribute)
            {
                style.removeAttribute(attribute_name);
            }
        }
    
        style[attribute_name] = value;
    }    

    static RemoveFromParentElement(element)
    {
        if (element != null)
        {
            if (element.parentNode != null)
                element.parentNode.removeChild(element);
        }
    }    

    static AddHue(luma)
    {
        let luma_value = 0;
    
        if (luma[0] == '#')
            luma_value = parseInt(luma.substr(1), 16);
        else
            luma_value = parseInt(luma);
    
        let red = luma_value;
        let green = luma_value;
        let blue = luma_value;
    
        let rgb = new RgbColor(red, green, blue);
        if (UI._ui_saturation > 0)
            rgb.Colourise(UI._ui_hue, UI._ui_saturation);
    
        let str_colour = rgb.GetCssString();
        return str_colour;
    }

    static IsFullscreen()
    {
        if (document.fullscreenElement)
            return true;
    
        return ((window.innerWidth == window.screen.width) &&
                (window.innerHeight == window.screen.height));
    }

    static GoFullscreen(state, resize_complete_action)
    {
        let element = document.documentElement;

        if (resize_complete_action != null)
            OJServerLink.Get()._on_resize_action = resize_complete_action;

        if (state)
        {
            if (element.requestFullscreen != null)
                element.requestFullscreen();
            else if (element.mozRequestFullScreen != null)
                element.mozRequestFullScreen();
            else if (element.msRequestFullscreen != null)
            {
                element = document.getElementById('_body');
                element.msRequestFullscreen();
            }
            else if (element.webkitRequestFullscreen != null)
                element.webkitRequestFullscreen();
        }
        else
        {
            if (document.exitFullscreen != null)
                document.exitFullscreen();
            else if (document.mozCancelFullScreen != null)
                document.mozCancelFullScreen();
            else if (document.msExitFullscreen != null)
                document.msExitFullscreen();
            else if (document.webkitExitFullscreen != null)
                document.webkitExitFullscreen();
        }
    }

    static ReadFontHeight(font)
    {
        let new_font = font.replace("bold", "");
        let font_2 = new_font.replace("italic", "");
        return parseFloat(font_2);
    }
    
    static MeasureText(dc, text)
    {
        let size = dc.measureText(text);
        size.height = UI.ReadFontHeight(dc.font);
        size.height = (size.height + 0.5) | 0;
        return size;
    }
    
    static MeasureSmallText(text)
    {
        if (UI._small_text_canvas_dc == null)
        {
            let temp_canvas = document.createElement("canvas");
            UI._small_text_canvas_dc = temp_canvas.getContext('2d');
            UI._small_text_canvas_dc.font = _small_font_name;
        }
    
        let size = UI._small_text_canvas_dc.measureText(text);
        size.height = UI.ReadFontHeight(UI._small_text_canvas_dc.font);
        size.height = (size.height + 0.5) | 0;
        return size;
    }
    
    static MeasureRegularText(text)
    {
        if (UI._regular_text_canvas_dc == null)
        {
            let temp_canvas = document.createElement("canvas");
            UI._regular_text_canvas_dc = temp_canvas.getContext('2d');
            UI._regular_text_canvas_dc.font = UI._font_name;
        }
    
        let size = UI._regular_text_canvas_dc.measureText(text);
        size.height = UI.ReadFontHeight(UI._regular_text_canvas_dc.font);
        size.height = (size.height + 0.5) | 0;
    
        return size;
    }
    
    static MeasureLargeText(text)
    {
        if (UI._large_text_canvas_dc == null)
        {
            let temp_canvas = document.createElement("canvas");
            UI._large_text_canvas_dc = temp_canvas.getContext('2d');
            UI._large_text_canvas_dc.font = UI._large_font_name;
        }
    
        let size = UI._large_text_canvas_dc.measureText(text);
        size.height = UI.ReadFontHeight(UI._large_text_canvas_dc.font);
        size.height = (size.height + 0.5) | 0;
        return size;
    }
    
    static MeasureExtraLargeText(text)
    {
        if (UI._extra_large_text_canvas_dc == null)
        {
            let temp_canvas = document.createElement("canvas");
            UI._extra_large_text_canvas_dc = temp_canvas.getContext('2d');
            UI._extra_large_text_canvas_dc.font = UI._extra_large_font_name;
        }
    
        let size = UI._extra_large_text_canvas_dc.measureText(text);
        size.height = UI.ReadFontHeight(UI._extra_large_text_canvas_dc.font);
        size.height = (size.height + 0.5) | 0;
        return size;
    }
    
    static MeasureTextWithFont(text, font_name)
    {
        let temp_canvas = document.createElement("canvas");
        let dc = temp_canvas.getContext('2d');
        dc.font = font_name;
    
        let size = dc.measureText(text);
        size.height = UI.ReadFontHeight(font_name);
        size.height = (size.height + 0.5) | 0;
        return size;
    }

    static GetBoundingClientRect(element)
    {
        let box = UI.GetOffsetRect(element);
    
        let t = Math.round(box.top / UI._body_scale);
        let l = Math.round(box.left / UI._body_scale);
        let r = Math.round(box.right / UI._body_scale);
        let b = Math.round(box.bottom / UI._body_scale);
    
        let rectangle = new OJRect(l, t, (r - l), (b - t));
    
        // Append left/top/right/bottom for compatibility
        rectangle.left = l;
        rectangle.top = t;
        rectangle.width = rectangle._width;
        rectangle.height = rectangle._height;
        rectangle.right = rectangle.Right();
        rectangle.bottom = rectangle.Bottom();
    
        return rectangle;
    }
    
    static GetOffsetRect(elem)
    {
        let box = elem.getBoundingClientRect();
        let body = document.body;
        let docElem = document.documentElement;
        let scrollTop = window.pageYOffset || docElem.scrollTop || body.scrollTop;
        let scrollLeft = window.pageXOffset || docElem.scrollLeft || body.scrollLeft;
        let clientTop = docElem.clientTop || body.clientTop || 0;
        let clientLeft = docElem.clientLeft || body.clientLeft || 0;
        let top = box.top + scrollTop - clientTop;
        let left = box.left + scrollLeft - clientLeft;

        return { top: Math.round(top), left: Math.round(left), right: box.right, bottom: box.bottom };
    }

    static SetDialogFlag(element)
    {
        element._is_dialog_element = true;
    
        let child_nodes = element.childNodes;
        for (let i = 0; i < child_nodes.length; i++)
        {
            UI.SetDialogFlag(child_nodes[i]);
        }
    }    

    static RemoveAllChildElements(object)
    {
        if (object != null)
        {
            while (object.firstChild)
                object.removeChild(object.firstChild);
        }
    }

    static FadeIn(element, fade_time_ms)
    {
        let fade_time = parseInt(fade_time_ms);
        element.style.opacity = "1.0";
        let animation_time = " " + fade_time + "ms";
        element.style.animationFillMode = "both";
        element.style.animation = "logo_start " + animation_time;
        setTimeout(UI.CancelAnimation, fade_time + 200, element);
    }    

    static CancelAnimation(element)
    {
        UI.SetStyleAttribute(element.style, "animation", null);
    }    

    static GetKeyCode(event)
    {
        let key_code = event.keyCode || event.charCode;
        return key_code;
    }

    static _small_font_height = 14;
    static _font_height = 18;
    static _large_font_height = 20;
    static _extra_large_font_height = 22;
    static _font_family = "GT Walsheim Regular";
    static _font_name = UI._font_height.toFixed(0) + "px " + UI._font_family;
    static _small_font_name = UI._small_font_height.toFixed(0) + "px " + UI._font_family;
    static _large_font_name = UI._large_font_height.toFixed(0) + "px " + UI._font_family;
    static _extra_large_font_name = UI._extra_large_font_height.toFixed(0) + "px " + UI._font_family;
    static _console_font_name = UI._small_font_height.toFixed(0) + "px " + "Consolas";
    static _invisible_input = null;
    static _current_modal_dialog_class_name = "";
    static _selected_background_colour = "#3c646f";
    static _lbutton_down = false;
    static _mbutton_down = false;
    static _rbutton_down = false;
    static _small_text_canvas_dc = null;
    static _regular_text_canvas_dc = null;
    static _large_text_canvas_dc = null;
    static _extra_large_text_canvas_dc = null;
    static _pending_mouse_overs = [];
    static _pending_mouse_outs = [];
    static _screen_width = 1;
    static _screen_height = 1;
    static _body_scale = 1;
    static _device_pixel_scale = 1;
    static _ui_hue = 0;
    static _ui_saturation = 0;
    static _all_images = [];
    static _main_application = null;
    static _product_name = "";
    static _fullscreen_restore_action = null;
    static _text_colour = "#b0b0b8";
    static _disabled_text_colour = "#a0a0a0";
    static _capture_mouse_element = null;
    static _captured_window_element = null;
    static _server_dead_overlay = null;
    static _dialog_background_colour = "#ffffff";
    static _control_panel_background_colour = "#f0f0f6"; 
    static _drop_down_list_displayed = [];
    static _fixed_window_width = 1920;
    static _fixed_window_height = 1080;
}

function OnFullScreenChange()
{
    if (!UI.IsFullscreen())
    {
        if (UI._fullscreen_restore_action)
            UI._fullscreen_restore_action.Call();
    }
}

function ShowStartupLogo(state)
{
    let startup_logo = document.getElementById("_start_logo");
    let startup_text = document.getElementById("_start_text");
    let startup_text_2 = document.getElementById("_start_text_2");

    if (startup_logo != null)
        startup_logo.style.display = state ? "block" : "none";

    if (startup_text != null)
        startup_text.style.display = state ? "block" : "none";

    if (startup_text_2 != null)
        startup_text_2.style.display = state ? "block" : "none";
}

window.IsMobileDevice = function()
{
    let user_agent = window.navigator.userAgent;
    let is_mobile = /mobi/i.test(user_agent);
    return is_mobile;
};

export class OJLImage
{
    constructor(oj_lib, url_title)
    {
        this._oj_lib = oj_lib;
        this._src = "OJL/Images/" + url_title;
        this._image = new Image();
        this._image.src = this._src;
        this._oj_lib._images.push(this);
    }

    Destroy()
    {
        this._oj_lib = null;
        this._image = null;
    }

    Get()
    {
        let image_copy = new Image();
        image_copy.src = this._src;
        return image_copy;
    }

    GetURL()
    {
        let url = "url('" + this._src + "')";
        return url;
    }
}

/////////////
export class OJLib
{
    constructor()
    {
        this.CreateSplashScreen();

        document.body.setAttribute("orient", "landscape");
        window.addEventListener("resize", OnBodyResize, false);
        window.addEventListener("fullscreenchange", OnFullScreenChange);

        let url = document.URL;
        OJLib._websocket_server_url = url.replace("http", "ws");
        OJLib.Trace("WebSocket URL " + OJLib._websocket_server_url);

        this._comms_ready_callback = new ObjectCallback(this, "CommsReady");
        this._the_server_link = new OJServerLink(OJLib._websocket_server_url);
        this._the_server_link.AddUiUpdateCommands();
        this._the_server_link.StartUI();

        this.LoadImages();
        this.OJInit();
        OJLibExtrasInit();
    }

    CreateSplashScreen()
    {
        let body = document.getElementById("_body");
        let startup_backgroup = document.createElement("div");
        startup_backgroup.id = "_startup_background";
        startup_backgroup.className = "startup_background_style";
        body.appendChild(startup_backgroup);
    
        let logo = document.createElement("img");
        logo.id = "_start_logo";
        logo.className = "logo_style";
        logo.alt = "Altera";
        logo.src = "OJL/Images/LogoStartup.png";
        logo.addEventListener("load", ShowStartupLogo);
        startup_backgroup.appendChild(logo);
    
        let start_text = document.createElement("div");
        start_text.id = "_start_text";
        start_text.className = "start_text_style";
        startup_backgroup.appendChild(start_text);
    }    

    static _x2js;
    static _logo_image;
    static _minimise;
    static _maximise;
    static _restore;
    static _close;
    static _upload;
    static _upload_over;
    static _download;
    static _download_over;
    static _settings_image;
    static _radio_button;
    static _radio_button_selected;
    static _open_up;
    static _close_up;
    static _open_over;
    static _close_over;
    static _expand_gray_22x22;
    static _collapse_gray_22x22;
    static _image_place_holder;
    static _delete_image;
    static _delete_over_image;
    static _settings_handle_image;
    static _maximise_arrow;
    static _maximise_arrow_over;
    static _restore_arrow;
    static _restore_arrow_over;
    static _close_keypad;
    static _drop_marker;
    static _off_image;
    static _on_image;
    static _off_image_mono;
    static _on_image_mono;
    static _green_led;
    static _red_led;
    static _blue_led;
    static _grey_led;
    static _split_panel_image;
    static _combine_panel_image;
    static _settings_grey_22x22;
    static _info_button;
    static _reset_button;
    static _warp_fixed_img;
    static _warp_corners_img;
    static _warp_arbitrary_img;
    static _warp_fisheye_img;
    static _settings32_button;
    static _settings48_button;
    static _settings64_button;
    static _convert_mesh_button;

    static _websocket_server_url = null;
    static _dead = false;
    static _file_select_callback = null;

    CloseUI()
    {
        this.DestroyImages();

        UI._captured_window_element = null;
        UI._capture_mouse_element = null;
        OJLib._websocket_server_url = null;

        if (OJLib._file_select_callback != null)
        {
            OJLib._file_select_callback.Destroy();
            OJLib._file_select_callback = null;
        }
        UI._current_modal_dialog_class_name = "";
        UI._selected_background_colour = "#3c646f";
        UI._lbutton_down = false;
        UI._mbutton_down = false;
        UI._rbutton_down = false;
        UI._small_text_canvas_dc = null;
        UI._regular_text_canvas_dc = null;
        UI._large_text_canvas_dc = null;
        UI._extra_large_text_canvas_dc = null;
        global_shared_options.length = 0;
        global_shared_options = null;
    }

    DestroyImages()
    {
        for (let i = 0; i < this._images.length; i++)
            this._images[i].Destroy();

        this._images.length = 0;
    }

    LoadImages()
    {
        this._images = [];
        OJLib._logo_image = new OJLImage(this, "LogoTitle80.png");
        OJLib._minimise = new OJLImage(this, "Minimise.png");
        OJLib._maximise = new OJLImage(this, "Maximise.png");
        OJLib._restore = new OJLImage(this, "Restore.png");
        OJLib._close = new OJLImage(this, "Close.png");
        OJLib._upload = new OJLImage(this, "UploadIcon.png");
        OJLib._upload_over = new OJLImage(this, "UploadIconOver.png");
        OJLib._download = new OJLImage(this, "DownloadIcon.png");
        OJLib._download_over = new OJLImage(this, "DownloadIconOver.png");
        OJLib._settings_image = new OJLImage(this, "Settings.png");
        OJLib._radio_button = new OJLImage(this, "RadioButton.png");
        OJLib._radio_button_selected = new OJLImage(this, "RadioButtonSelected.png");
        OJLib._open_up = new OJLImage(this, "OpenUp.png");
        OJLib._close_up = new OJLImage(this, "CloseUp.png");
        OJLib._open_over = new OJLImage(this, "OpenOver.png");
        OJLib._close_over = new OJLImage(this, "CloseOver.png");
        OJLib._expand_gray_22x22 = new OJLImage(this, "ExpandGrey22x22.png");
        OJLib._collapse_gray_22x22 = new OJLImage(this, "CollapseGrey22x22.png");
        OJLib._image_place_holder = new OJLImage(this, "ImagePlaceHolder.png");
        OJLib._delete_image = new OJLImage(this, "Delete.png");
        OJLib._delete_over_image = new OJLImage(this, "DeleteOver.png");
        OJLib._settings_handle_image = new OJLImage(this, "SettingsHandle.png");
        OJLib._maximise_arrow = new OJLImage(this, "MaximiseArrow.png");
        OJLib._maximise_arrow_over = new OJLImage(this, "MaximiseArrowOver.png");
        OJLib._restore_arrow = new OJLImage(this, "RestoreArrow.png");
        OJLib._restore_arrow_over = new OJLImage(this, "RestoreArrowOver.png");
        OJLib._close_keypad = new OJLImage(this, "CloseKeypad.png");
        OJLib._drop_marker = new OJLImage(this, "DropMarker.png");
        OJLib._off_image = new OJLImage(this, "Off.png");
        OJLib._on_image = new OJLImage(this, "On.png");
        OJLib._off_image_mono = new OJLImage(this, "OffMono.png");
        OJLib._on_image_mono = new OJLImage(this, "OnMono.png");
        OJLib._green_led = new OJLImage(this, "greenLed-small.png");
        OJLib._red_led = new OJLImage(this, "redLed-small.png");
        OJLib._blue_led = new OJLImage(this, "blueLed-small.png");
        OJLib._grey_led = new OJLImage(this, "greyLed-small.png");
        OJLib._split_panel_image = new OJLImage(this, "SplitScreen.png");
        OJLib._combine_panel_image = new OJLImage(this, "CombineScreen.png");
        OJLib._settings_grey_22x22 = new OJLImage(this, "SettingsGrey22x22.png");
        OJLib._info_button = new OJLImage(this, "InfoButton.png");
        OJLib._reset_button = new OJLImage(this, "Reset.png");
        OJLib._warp_fixed_img = new OJLImage(this, "WarpFixed.png");
        OJLib._warp_corners_img = new OJLImage(this, "WarpCorners.png");
        OJLib._warp_arbitrary_img = new OJLImage(this, "WarpArbitrary.png");
        OJLib._warp_fisheye_img = new OJLImage(this, "WarpFisheye.png");
        OJLib._settings32_button = new OJLImage(this, "Settings32.png");
        OJLib._settings48_button = new OJLImage(this, "Settings48.png");
        OJLib._settings64_button = new OJLImage(this, "Settings64.png");
        OJLib._convert_mesh_button = new OJLImage(this, "ConvertMesh.png");
    }

    static _entity_map = 
    {
        //"&": "&amp;",
        "<": "&lt;",
        ">": "&gt;",
        '"': '&quot;',
        "'": '&#39;',
        "/": '&#x2F;'
    };

    static EscapeHtml(string)
    {
        if (string == null)
            return null;

        return String(string).replace(/[<>"'\/]/g, function(s)
        {
            return OJLib._entity_map[s];
        });
    }

    static Trace(message, notify_server)
    {
        if (window.console != null)
            console.log(message);

        if (OJServerLink.Get() != null)
        {
            if ((notify_server == null) || notify_server)
                OJServerLink.Get().UiTrace(message);
        }
    }

    static ServerExecute(command)
    {
        OJServerLink.Get().ExecuteServerCommand(command);
    } 

    static ServerExecuteWithParams(command, params)
    {
        OJServerLink.Get().ExecuteServerCommandWithParams(command, params);
    }    

    StopScrolling(touch_event)
    {
        touch_event.preventDefault();
    }

    DocumentDragEnter(event)
    {
        event.preventDefault();
        event.stopPropagation();
        event.dataTransfer.setData('text/plain', 'dummy');
    }

    DocumentDragLeave(event)
    {
        event.preventDefault();
    }

    DocumentDragOver(event)
    {
        let source_element = event.srcElement || event.originalTarget;

        let window_element = source_element._window_element;
        let drop_callback = source_element._drop_callback;

        let drop_effect = "none";
        event.preventDefault();

        if ((drop_callback == null) && (window_element != null))
            drop_callback = window_element.FindDropCallback();

        if (drop_callback != null)
            drop_effect = "copy";

        event.dataTransfer.dropEffect = drop_effect;
    }

    DocumentDrop(event)
    {
        event.stopPropagation();
        event.preventDefault();

        let source_element = event.srcElement || event.originalTarget;

        let window_element = source_element._window_element;
        let drop_callback = source_element._drop_callback;

        if ((drop_callback == null) && (window_element != null))
            drop_callback = window_element.FindDropCallback();

        if (drop_callback != null)
            CallCallback(drop_callback, event);

        return false;
    }

    DocumentDragEnd(event)
    {
        event.stopPropagation();
        event.preventDefault();
        return false;
    }

    OJInit()
    {
        let body = document.getElementById('_body');
        document.addEventListener("touchmove", this.StopScrolling, false);
        document.onmousemove = DocumentMouseMove;
        document.onmouseup = DocumentMouseUp;
        document.onmousedown = DocumentMouseDown;
        document.onkeydown = DocumentOnKeyDown;
        document.oncontextmenu = DocumentOnContextMenu;
        document.ondblclick = DocumentOnDoubleClick;
        document.addEventListener("mousewheel", DocumentMouseWheel, false);
        document.addEventListener("DOMMouseScroll", DocumentMouseWheel, false);
        document.addEventListener("touchstart", TouchStart, false);
        document.addEventListener("touchmove", TouchMove, false);
        document.addEventListener("touchend", TouchEnd, false);
        document.addEventListener("dragenter", this.DocumentDragEnter, false);
        document.addEventListener("dragleave", this.DocumentDragLeave, false);
        document.addEventListener("dragover", this.DocumentDragOver, false);
        document.addEventListener("drop", this.DocumentDrop, false);
        document.addEventListener("dragend", this.DocumentDragEnd, false);

        // Add an 'input' element for file selection
        UI._invisible_input = document.createElement("input");
        UI._invisible_input.type = "file";
        UI._invisible_input.addEventListener('change', this.HandleFileSelect, false);
        UI._invisible_input.style.display = "none";
        body.appendChild(UI._invisible_input);
    };

    static IsTrue(str)
    {
        return (str == "true");
    }

    static SetCanvasSize(canvas, width, height)
    {
        canvas.width = width;
        canvas.height = height;
        canvas.style.width = width + "px";
        canvas.style.height = height + "px";
    }

    HandleFileSelect(event)
    {
        if (OJLib._file_select_callback != null)
        {
            let files = event.target.files;
            OJLib._file_select_callback.Call(files);
        }
    }

    static SetElementPosition(element, x, y, width, height)
    {
        element.style.left = (x | 0) + "px";
        element.style.top = (y | 0) + "px";
        element.style.width = (width | 0) + "px";
        element.style.height = (height | 0) + "px";
    }

    static SetElementLocation(element, x, y)
    {
        element.style.left = (x | 0) + "px";
        element.style.top = (y | 0) + "px";
    }


    static AddObserver(observer_array, observer, insert_position)
    {
        // Make sure not registered multiple times
        let index = observer_array.indexOf(observer);
        if (index == -1)
        {
            if (insert_position == null)
                observer_array.push(observer);
            else
            {
                observer_array.splice(insert_position, 0, observer);
            }
        }
    }
    
    static RemoveObserver(observer_array, observer)
    {
        while (true)
        {
            let index = observer_array.indexOf(observer);
            if (index < 0)
                return;
            observer_array.splice(index, 1);
        }
    }
    
    // Click events happen on mouse-up so we can distinguish press-and-hold
    static RegisterForClick(observer, insert_position)
    {
        OJLib.AddObserver(_click_observers, observer, insert_position);
    }

    static UnregisterForClick(observer)
    {
        OJLib.RemoveObserver(_click_observers, observer);
    }

    static RegisterForDoubleClick(observer, insert_position)
    {
        OJLib.AddObserver(_double_click_observers, observer, insert_position);
    }

    static UnregisterForDoubleClick(observer)
    {
        OJLib.RemoveObserver(_double_click_observers, observer);
    }

    // Context menu is either right click from mouse or
    // press and hold from tablet
    static RegisterForContextMenu(observer, insert_position)
    {
        OJLib.AddObserver(_context_menu_observers, observer, insert_position);
    }

    static UnregisterForContextMenu(observer)
    {
        OJLib.RemoveObserver(_context_menu_observers, observer);
    }

    static RegisterForLButtonDown(observer, insert_position)
    {
        OJLib.AddObserver(_lbutton_down_observers, observer, insert_position);
    }

    static UnregisterForLButtonDown(observer)
    {
        OJLib.RemoveObserver(_lbutton_down_observers, observer);
    }

    static RegisterForLButtonUp(observer, insert_position)
    {
        OJLib.AddObserver(_lbutton_up_observers, observer, insert_position);
    }

    static UnregisterForLButtonUp(observer)
    {
        OJLib.RemoveObserver(_lbutton_up_observers, observer);
    }

    static RegisterForRButtonDown(observer, insert_position)
    {
        OJLib.AddObserver(_rbutton_down_observers, observer, insert_position);
    }

    static UnregisterForRButtonDown(observer)
    {
        OJLib.RemoveObserver(_rbutton_down_observers, observer);
    }

    static RegisterForRButtonUp(observer, insert_position)
    {
        OJLib.AddObserver(_rbutton_up_observers, observer, insert_position);
    }

    static UnregisterForRButtonUp(observer)
    {
        OJLib.RemoveObserver(_rbutton_up_observers, observer);
    }

    static RegisterForMButtonDown(observer, insert_position)
    {
        OJLib.AddObserver(_mbutton_down_observers, observer, insert_position);
    }

    static UnregisterForMButtonDown(observer)
    {
        OJLib.RemoveObserver(_mbutton_down_observers, observer);
    }

    static RegisterForMButtonUp(observer, insert_position)
    {
        OJLib.AddObserver(_mbutton_up_observers, observer, insert_position);
    }

    static UnregisterForMButtonUp(observer)
    {
        OJLib.RemoveObserver(_mbutton_up_observers, observer);
    }

    static RegisterForMouseMove(observer, insert_position)
    {
        OJLib.AddObserver(_mouse_move_observers, observer, insert_position);
    }

    static UnregisterForMouseMove(observer)
    {
        OJLib.RemoveObserver(_mouse_move_observers, observer);
    }

    static RegisterForMouseWheel(observer, insert_position)
    {
        OJLib.AddObserver(_mouse_wheel_observers, observer, insert_position);
    }

    static UnregisterForMouseWheel(observer)
    {
        OJLib.RemoveObserver(_mouse_wheel_observers, observer);
    }

    static RegisterForKeyDown(observer, insert_position)
    {
        OJLib.AddObserver(_key_down_observers, observer, insert_position);
    }

    static UnregisterForKeyDown(observer)
    {
        OJLib.RemoveObserver(_key_down_observers, observer);
    }    

    static RegisterButton(owner, element, user_data, insert_position)
    {
        let button_callback_container = new ButtonCallbackContainer(owner, element, user_data, insert_position);
        return button_callback_container;
    }

    static HueToRGB(p, q, t)
    {
        if (t < 0) 
            t += 1;
        if (t > 1) 
            t -= 1;
        if (t < 1 / 6) 
            return p + (q - p) * 6 * t;
        if (t < 1 / 2)
            return q;
        if (t < 2 / 3)
            return p + (q - p) * (2 / 3 - t) * 6;
        return p;
    }
    
    static HSL_to_RGB(h, s, l)
    {
        let r, g, b;
    
        if (s == 0)
        {
            r = g = b = l; // achromatic
        }
        else
        {
            let q = l < 0.5 ? l * (1 + s) : l + s - l * s;
            let p = 2 * l - q;
            r = OJLib.HueToRGB(p, q, h + 1 / 3);
            g = OJLib.HueToRGB(p, q, h);
            b = OJLib.HueToRGB(p, q, h - 1 / 3);
        }
    
        return { R: r * 255, G: g * 255, B: b * 255 };
    }    

    static _dead_interval_id = null;
    static _shutdown_from_ui = false;
    
    static ShowServerDeadOverlay(state)
    {
        let body = document.getElementById('_body');

        if (state)
        {
            if (UI._server_dead_overlay == null)
            {
                UI._server_dead_overlay = new OJPanel("#101020", body);
                UI._server_dead_overlay.Resize(0, 0, UI._screen_width, UI._screen_height);
                UI._server_dead_overlay._client_area.className = "server_connection_lost_class";
                UI._server_dead_overlay._client_area.style.zIndex = 999;
                UI._server_dead_overlay._client_area.innerHTML = ""; // "Server connection lost";

                let element = UI._server_dead_overlay._client_area;
                let animation_time = " 1000ms";
                element.style.animationFillMode = "both";
                element.style.webkitAnimationFillMode = "both";
                element.style.animation = "server_lost_keyframes " + animation_time;
                element.style.webkitAnimation = "server_lost_keyframes " + animation_time;
            }
            if (OJServerLink.Get())
                OJServerLink.Get().CloseDialogs();
        }
        else
        {
            if (UI._server_dead_overlay != null)
            {
                UI._server_dead_overlay.Destroy();
                UI._server_dead_overlay = null;
            }
        }
    }

    static ServerDead(state)
    {
        if (!OJLib._dead)
        {
            OJLib._dead = true;
    
            if (OJLib._dead_interval_id != null)
            {
                clearInterval(OJLib._dead_interval_id);
                OJLib._dead_interval_id = null;
            }
    
            OJLib._dead_interval_id = setInterval(ReloadPage, 2000);
    
            if (!OJLib._shutdown_from_ui)
            {
                OJLib.ShowServerDeadOverlay(true);
            }
        }
    }
    
    static CaptureMouse(element)
    {
        UI._capture_mouse_element = element;
        AddPendingMouseOut(element);
    }
    
    static ReleaseMouse(event)
    {
        FixMouseOvers(event)
    
        let over_element = document.elementFromPoint(event.clientX, event.clientY);
        UI._capture_mouse_element = null;
    
        if (over_element != null)
        {
            if (over_element.onmouseover == null)
                over_element = over_element.parentNode;
    
            if ((over_element != null) && (over_element.onmouseover != null))
                over_element.onmouseover(event);
        }
    }    

    static DestroyArray(array)
    {
        if (array != null)
        {
            for (let i = 0; i < array.length; i++)
            {
                if ((array[i] != null) && (array[i].Destroy != null))
                    array[i].Destroy();
            }
            array.length = 0;
        }
    }    

    static MarkEventAsHandled(event)
    {
        event._handled = 1;
        event.preventDefault();
    }    
}

export class ObjectCallback
{
    constructor(object, method_name, element, user_data)
    {
        this._object = object;
        this._method_name = method_name;
        this._element = element;
        this._user_data = user_data;
        this._is_destroyed = false;
    }

    Destroy()
    {
        if (this._is_destroyed)
            return;

        this._is_destroyed = true;
        this._object = null;
        this._method_name = null;
        this._element = null;
        this._user_data = null;
    }

    UpdateCallbackObject(new_object)
    {
        this._object = new_object;
    }

    Call(param)
    {
        if (this._object != null)
        {
            if (this._user_data != null)
            {
                if (param == null)
                    param = {};

                param._user_data = this._user_data;
            }

            return this._object[this._method_name](param);
        }
    }

    Clone()
    {
        let clone = new ObjectCallback(this._object, this._method_name, this._element, this._user_data);
        return clone;
    }

    Matches(other)
    {
        return ((this._object === other._object) && (this._method_name === other._method_name));
    }
}

let _lbutton_down_observers = [];
let _lbutton_up_observers = [];
let _rbutton_down_observers = [];
let _rbutton_up_observers = [];
let _mbutton_down_observers = [];
let _mbutton_up_observers = [];
let _mouse_move_observers = [];
let _mouse_wheel_observers = [];
let _key_down_observers = [];
let _click_observers = [];
let _double_click_observers = [];
let _context_menu_observers = [];

function FixWhichButton(event)
{
    if (!event.which && event.button)
    {
        if (event.button & 1)
            event.which = 1      // Left
        else if (event.button & 4)
            event.which = 2 // Middle
        else if (event.button & 2)
            event.which = 3 // Right
    }
}

function ObserverMatch(observer, observers, i)
{
    if (i == 0)
        return false;

    for (let j = 0; j < i; j++)
    {
        if (observer.Matches(observers[j]))
            return true;
    }
}

function CallCallback(observer, event)
{
    if (observer._user_data != null)
        event._user_data = observer._user_data;
    return observer._object[observer._method_name](event);
}

function CallCallbacks(observers, event)
{
    // If a callback is registered multiple times, 
    // only call it once.
    for (let i = 0; i < observers.length; i++)
    {
        let observer = observers[i];

        if (ObserverMatch(observer, observers, i))
            continue;

        let stop_further_callbacks = CallCallback(observer, event);

        if ((stop_further_callbacks != null) && (stop_further_callbacks))
            return;
    }
}

let _press_and_hold_handle = null;
let _press_and_hold_event = null;

function CheckModalDialog(event)
{
    if (event == null)
        return;

    if (UI._current_modal_dialog_class_name != "")
    {
        // Only propagate the event if it is from an element
        // in the modal dialog
        let source_element = event._source_element;
        if (source_element != null)
        {
            if (source_element.id == "_body")
                return true;

            if (source_element._is_dialog_element == null)
            {
                if (event.preventDefault != null)
                    event.preventDefault();
                return false;
            }
        }
    }

    return true;
}

function DocumentMouseDown(event)
{
    ModifyMouseCoordinates(event, _lbutton_down_observers);

    FixWhichButton(event);

    let which = 1;
    if (event.which)
        which = event.which;

    if (which == 1)
        UI._lbutton_down = true;
    else if (which == 2)
        UI._mbutton_down = true;
    else if (which == 3)
        UI._rbutton_down = true;

    SetMouseButtonStates(event);

    if (!CheckModalDialog(event))
    {
        //OJLib.Trace("Not Dialog element " + event._source_element);
        return;
    }

    if (which == 1)
    {
        // Left button
        CallCallbacks(_lbutton_down_observers, event);

        _press_and_hold_handle = window.setTimeout(PressAndHold, 500);
        _press_and_hold_event = event;
    }
    else if (which == 2)
    {
        // Middle button
        CallCallbacks(_mbutton_down_observers, event);
    }
    else if (which == 3)
    {
        // Right button
        CallCallbacks(_rbutton_down_observers, event);
    }
}

function DocumentMouseUp(event)
{
    let was_capture_mouse_element = UI._capture_mouse_element;
    let mouse_was_captured = (UI._capture_mouse_element != null);

    ModifyMouseCoordinates(event, _lbutton_up_observers);

    FixWhichButton(event);

    let which = 1;
    if (event.which)
        which = event.which;

    if (which == 1)
        UI._lbutton_down = false;
    else if (which == 2)
        UI._mbutton_down = false;
    else if (which == 3)
        UI._rbutton_down = false;

    SetMouseButtonStates(event);

    if (!CheckModalDialog(event))
    {
        window.clearTimeout(_press_and_hold_handle);
        return;
    }

    if (which == 1)
    {
        // Left button
        CallCallbacks(_lbutton_up_observers, event);

        // If press and hold was triggered, then don't call the click_observers
        if (!_press_and_hold_triggered)
            CallCallbacks(_click_observers, event);

        window.clearTimeout(_press_and_hold_handle);
        _press_and_hold_event = null;
        _press_and_hold_triggered = false;
    }
    else if (which == 2)
    {
        // Middle button
        CallCallbacks(_mbutton_up_observers, event);
    }
    else if (which == 3)
    {
        // Right button
        CallCallbacks(_rbutton_up_observers, event);
    }

    if (UI._capture_mouse_element != null)
    {
        FixMouseOvers(event);
        UI._capture_mouse_element = null;
    }
}

function FixMouseOvers(event)
{
    UnregisterAllButtonCallbacks();

    for (let i = 0; i < UI._pending_mouse_overs.length; i++)
    {
        //OJLib.Trace("Calling UI._pending_mouse_overs " + UI._pending_mouse_overs[i].id);
        if (UI._pending_mouse_overs[onmouseout] != null)
            UI._pending_mouse_overs[onmouseout](event);
    }

    for (let i = 0; i < UI._pending_mouse_outs.length; i++)
    {
        //OJLib.Trace("Calling UI._pending_mouse_outs " + UI._pending_mouse_outs[i].id);
        if (UI._pending_mouse_outs[onmouseover] != null)
            UI._pending_mouse_outs[onmouseover](event);
    }

    UI._pending_mouse_overs.length = 0;
    UI._pending_mouse_outs.length = 0;
}

function AddPendingMouseOver(element)
{
    //OJLib.Trace("AddPendingMouseOver " + element.id);
    let index = UI._pending_mouse_outs.indexOf(element);
    if (index >= 0)
    {
        UI._pending_mouse_outs.splice(index, 1);
    }
    else
    {
        UI._pending_mouse_overs.push(element);
    }
}

function AddPendingMouseOut(element)
{
    let index = UI._pending_mouse_overs.indexOf(element);
    if (index >= 0)
    {
        UI._pending_mouse_overs.splice(index, 1);
    }
    else
    {
        UI._pending_mouse_outs.push(element);
    }
}

function DocumentOnDoubleClick(event)
{
    let was_capture_mouse_element = UI._capture_mouse_element;
    let mouse_was_captured = (UI._capture_mouse_element != null);

    ModifyMouseCoordinates(event, _double_click_observers);

    FixWhichButton(event);
    CallCallbacks(_double_click_observers, event);
}

function DocumentOnContextMenu(event)
{
    ModifyMouseCoordinates(event, _context_menu_observers);

    if (!CheckModalDialog(event))
        return;

    //OJLib.Trace("DocumentOnContextMenu");
    //if (!CheckContextMenus(event))
    CallCallbacks(_context_menu_observers, event);

    return false;
}

let _press_and_hold_triggered = false;

function PressAndHold()
{
    _press_and_hold_triggered = true;
    CallCallbacks(_context_menu_observers, _press_and_hold_event);
}

let _touch_count = 1;

function GetMouseOverElement(start_element)
{
    if (typeof (start_element.onmouseover) === "function")
        return start_element;

    let parent = start_element.parentNode;
    if (parent == null)
        return start_element;

    if (typeof (parent.onmouseover) === "function")
        return parent;

    let grand_parent = parent.parentNode;
    if (grand_parent == null)
        return parent;

    if (typeof (grand_parent.onmouseover) === "function")
        return grand_parent;

    return null;
}

function GetMouseOutElement(start_element)
{
    if (typeof (start_element.onmouseout) === "function")
        return start_element;

    let parent = start_element.parent;
    if (typeof (parent.onmouseout) === "function")
        return parent;

    let grand_parent = parent.parent;
    if (typeof (grand_parent.onmouseout) === "function")
        return grand_parent;

    return null;
}

let _touch_event = null;
let _touch_element = null;
let _touch_hover_element = null;
let _show_touch_traces = false;
let _touch_start = 1;
let _touch_end = 1;
let _double_touch = false;
let _last_touch_client_x = -1;
let _last_touch_client_y = -1;

let _last_touch_start = 0;
let double_touch_threshold = 400;

function StopEventProcessing(event)
{
    if (event.preventDefault)
        event.preventDefault();
    if (event.stopPropagation)
        event.stopPropagation();

    event.cancelBubble = true;
}

function TouchStart(event)
{
    //if (!UI.IsFullscreen())
    //{
    //	UI.GoFullscreen(true);
    //}

    let now = window.performance.now();

    _double_touch = ((now - _last_touch_start) < double_touch_threshold);
    _last_touch_start = now;

    let touch_event = event.touches[0];
    let target_element = touch_event.target;
    touch_event._is_touch_event = true;

    let mouse_over_element = GetMouseOverElement(target_element);

    touch_event._source_element = mouse_over_element;

    if (!CheckModalDialog(touch_event))
    {
        StopEventProcessing(event);
        return;
    }

    if (mouse_over_element)
    {
        _touch_event = touch_event;
        _touch_element = mouse_over_element;
        _touch_hover_element = _touch_element;

        if (typeof (mouse_over_element.onmouseover) === "function")
            mouse_over_element.onmouseover(touch_event);

        if (_show_touch_traces)
            OJLib.Trace("TouchStart " + _touch_element);

        let offset_point = GetMouseClientCoordinates(mouse_over_element, touch_event);
        touch_event._offset_x = offset_point._x;
        touch_event._offset_y = offset_point._y;
        touch_event._client_x = Math.round(touch_event.clientX / UI._body_scale);
        touch_event._client_y = Math.round(touch_event.clientY / UI._body_scale);
        _last_touch_client_x = touch_event._client_x;
        _last_touch_client_y = touch_event._client_y;

        DocumentMouseDown(touch_event);
    }

    if (touch_event._allow_default_action == null)
        StopEventProcessing(event);
}

function TouchMove(event)
{
    let touch_event = event.touches[event.touches.length - 1];
    let target_element = touch_event.target;
    _touch_event = touch_event;

    //let mouse_over_element = GetMouseOverElement(target_element);
    let mouse_over_element = document.elementFromPoint(touch_event.clientX, touch_event.clientY);

    // If we've gone over a difference element, we need to simulate
    // mouse over event
    if (mouse_over_element != _touch_hover_element)
        ReplaceTouchHoverElement(mouse_over_element);

    touch_event._source_element = target_element;
    touch_event._is_touch_event = true;

    if (!CheckModalDialog(touch_event))
    {
        if (touch_event._allow_default_action == null)
            StopEventProcessing(event);
        return;
    }

    if (_touch_element != null)
    {
        let offset_point = GetMouseClientCoordinates(_touch_element, touch_event);
        touch_event._offset_x = offset_point._x;
        touch_event._offset_y = offset_point._y;

        touch_event._client_x = Math.round(touch_event.clientX / UI._body_scale);
        touch_event._client_y = Math.round(touch_event.clientY / UI._body_scale);

        if (touch_event._offset_y < 0)
        {
            touch_event._offset_y = 0;
        }
    }

    if ((touch_event._client_x == _last_touch_client_x) && (touch_event._client_y == _last_touch_client_y))
    {
        StopEventProcessing(event);
        return;
    }

    _last_touch_client_x = touch_event._client_x;
    _last_touch_client_y = touch_event._client_y;

    if (_show_touch_traces)
        OJLib.Trace("TouchMove" + touch_event);

    DocumentMouseMove(touch_event);

    if (touch_event._allow_default_action == null)
        StopEventProcessing(event);
}

function TouchEnd(event)
{
    _last_touch_client_x = -1;
    _last_touch_client_y = -1;

    let source_element = event.srcElement || event.originalTarget;

    // If a dialog was popped up as a result of the touch start but before the touch
    // end, we still have to fake the onmouseout to unregister the original button
    let dont_check_dialog = (source_element === _touch_element);

    // Use the last _touch_event, since there is no location of a touch end
    if (!dont_check_dialog && !CheckModalDialog(_touch_event))
    {
        if ((_touch_event != null) && (_touch_event._allow_default_action == null))
            StopEventProcessing(event);

        return;
    }

    let do_double_touch = _double_touch;
    _double_touch = false;

    UI._lbutton_down = false;
    UI._mbutton_down = false;
    UI._rbutton_down = false;

    if (_touch_element != null)
    {
        if (_show_touch_traces)
            OJLib.Trace("TouchEnd" + _touch_element);

        if (do_double_touch)
            DocumentOnDoubleClick(_touch_event);

        DocumentMouseUp(_touch_event);

        //CallCallbacks(_lbutton_up_observers, _touch_event);

        //if (!_touch_context_invoked)
        //	CallCallbacks(_click_observers, _touch_event);

        if (typeof (_touch_element.onmouseout) === "function")
            _touch_element.onmouseout(_touch_event);

        if (_touch_hover_element && _touch_hover_element != _touch_element)
        {
            if (typeof (_touch_hover_element.onmouseout) === "function")
                _touch_hover_element.onmouseout(_touch_event);
        }
    }
    else
    {
        if (_show_touch_traces)
            OJLib.Trace("TouchEnd - no element");
    }

    if ((_touch_event != null) && (_touch_event._allow_default_action == null))
        StopEventProcessing(event);

    _touch_element = null;
    _touch_event = null;
    return;
}

// If you have popped up a window which needs to receive
// the events after the touch event has started
function ReplaceTouchElement(new_element)
{
    if (_touch_element != null)
    {
        if (typeof (_touch_element.onmouseout) === "function")
            _touch_element.onmouseout(_touch_event);
    }

    _touch_element = new_element;

    if (_touch_element != null)
    {
        if (typeof (_touch_element.onmouseover) === "function")
            _touch_element.onmouseover(_touch_event);
    }
}

function ReplaceTouchHoverElement(new_element)
{
    if (_touch_hover_element != null)
    {
        if (typeof (_touch_hover_element.onmouseout) === "function")
            _touch_hover_element.onmouseout(_touch_event);
    }

    _touch_hover_element = new_element;

    if (_touch_hover_element != null)
    {
        if (typeof (_touch_hover_element.onmouseover) === "function")
            _touch_hover_element.onmouseover(_touch_event);
    }
}

let _loitering_cursor_timeout = null;
let _cursor_is_moving = false;
let _postponed_callbacks = [];

function LoiteringCursor()
{
    _cursor_is_moving = false;
    CallCallbacks(_postponed_callbacks);
    _postponed_callbacks.length = 0;
}

function CursorIsMoving(postponed_callback)
{
    if (OJServerLink.Get() == null)
        return false;

    if (!OJServerLink.Get()._is_local_connection)
        return false;

    if (_cursor_is_moving)
        OJLib.AddObserver(_postponed_callbacks, postponed_callback);

    return _cursor_is_moving;
}

function DocumentMouseMove(event)
{
    _cursor_is_moving = true;

    if (_loitering_cursor_timeout != null)
        clearTimeout(_loitering_cursor_timeout);

    _loitering_cursor_timeout = setTimeout(LoiteringCursor, 200);

    ModifyMouseCoordinates(event, _mouse_move_observers);

    //OJLib.Trace("Doc Move (" + event._client_x + "," + event._client_y + ") (" + event._offset_x + "," + event._offset_y + ")");

    if (!CheckModalDialog(event))
        return;

    event = event || window.event;
    FixWhichButton(event);

    event._lbutton_down = UI._lbutton_down;
    event._mbutton_down = UI._mbutton_down;
    event._rbutton_down = UI._rbutton_down;

    CallCallbacks(_mouse_move_observers, event);
}


function DocumentMouseWheel(event)
{
    ModifyMouseCoordinates(event, _mouse_wheel_observers);

    //OJLib.Trace("Doc Move (" + event._client_x + "," + event._client_y + ") (" + event._offset_x + "," + event._offset_y + ")");

    if (!CheckModalDialog(event))
        return;

    event = event || window.event;
    FixWhichButton(event);

    event._lbutton_down = UI._lbutton_down;
    event._mbutton_down = UI._mbutton_down;
    event._rbutton_down = UI._rbutton_down;

    CallCallbacks(_mouse_wheel_observers, event);
}

function AppDocumentOnKeyDown(event)
{
    let trap_special_keys = true;
    let focussed_element = document.activeElement;

    if ((focussed_element != null) && focussed_element._text_control != null)
    {
        if ((focussed_element._text_control._control_type == TEXT_CONTROL_TYPE.TCT_TEXT) ||
            (focussed_element._text_control._control_type == TEXT_CONTROL_TYPE.TCT_HEX_INTEGER))
            trap_special_keys = false;
    }

    if (UI._current_modal_dialog_class_name != "")
        trap_special_keys = false;

    if (trap_special_keys && !event.ctrlKey)
    {
    }

    let handled = false;
    if (_key_down_observers.length > 0)
    {
        CallCallbacks(_key_down_observers, event);

        if (event._handled != null)
            handled = event._handled;
    }

    if (!handled)
    {
        let is_escape = (event.which == 27);
        if (is_escape)
        {
            // Cancel context menus
            let handled = false;

            // Close drop down lists
            if (UI._drop_down_list_displayed.length > 0)
            {
                for (let d = 0; d < UI._drop_down_list_displayed.length; d++)
                    UI._drop_down_list_displayed[d].Close(null, false);
                handled = true;
            }

            if (!handled)
            {
            // CancelPopUpWindows
            }
        }
        else
        {
            //etc
        }
    }
}

let resize_done_timeout_id = null;
let x_scale = 1.0;
let y_scale = 1.0;
let start_width = 0;
let start_height = 0;
let resizing = false;

function OnBodyResize()
{
    if (window.IsMobileDevice())
        return;

    if (!resizing)
    {
        start_width = window.innerWidth;
        start_height = window.innerHeight;
        resizing = true;
    }
    else
    {
        x_scale = UI._body_scale * window.innerWidth / start_width;
        y_scale = UI._body_scale * window.innerHeight / start_height;

        let body = document.getElementById("_body");
        body.style.transform = "scale(" + x_scale + "," + y_scale + ")";
        body.style.WebkitTransform = "scale(" + x_scale + "," + y_scale + ")";
    }

    if (resize_done_timeout_id != null)
        clearTimeout(resize_done_timeout_id);

    resize_done_timeout_id = setTimeout(OnResizeDone, 150);
}

function DocumentOnKeyDown(event)
{
    ModifyMouseCoordinates(event, _key_down_observers);
    event = event || window.event;
    event._key_code = UI.GetKeyCode(event);

    AppDocumentOnKeyDown(event);
}

function GetMouseClientCoordinates(element, event)
{
    let rectangle = UI.GetBoundingClientRect(element);

    let x = (event.clientX / UI._body_scale) - rectangle.left;
    let y = (event.clientY / UI._body_scale) - rectangle.top;

    let result = { _x: x, _y: y };
    return result;
}

function AssignSourceElement(event)
{
    if (event == null)
        return;
    // event.srcElement is read-only, so we use _source_element
    let source_element = null;
    if (event.srcElement)
        source_element = event.srcElement;
    else if (event.target)
        source_element = event.target;

    event._source_element = source_element;
}

export function ModifyMouseCoordinates(event, callback_list)
{
    if (event._is_touch_event != null)
        return;

    AssignSourceElement(event);

    // Convert to virtual screen coordinates (if there is a scale transform
    // on the body element)
    event._client_x = Math.round(event.clientX / UI._body_scale);
    event._client_y = Math.round(event.clientY / UI._body_scale);

    // The offsets already incorporate the scaling factor ...
    event._offset_x = event.offsetX;
    event._offset_y = event.offsetY;

    if (event._offset_x == null)
    {
        // Firefox?
        let rect = event._source_element.getBoundingClientRect();
        let offset_x = event.clientX - rect.left;
        let offset_y = event.clientY - rect.top;

        event._offset_x = offset_x;
        event._offset_y = offset_y;
    }

    if (UI._capture_mouse_element == null)
        return;

    if (callback_list == null)
        return;

    if (callback_list.length == 0)
        return;

    let captured_rectangle = UI.GetBoundingClientRect(UI._capture_mouse_element);

    let x = event._client_x - captured_rectangle.left;
    let y = event._client_y - captured_rectangle.top;

    // Now relative to the captured rectangle
    event._offset_x = x;
    event._offset_y = y;
    event._source_element = UI._capture_mouse_element;
};

function OnErrorWebSocket(event)
{
    OJLib.Trace("OnErrorWebSocket Error occured: " + event.data);
}

function StringToXmlDocument(xml_string)
{
    let xml_document = null;

    if (window.DOMParser)
    {
        let parser = new DOMParser();
        xml_document = parser.parseFromString(xml_string, "text/xml");
    }
    else // Internet Explorer
    {
        let xml_document = new ActiveXObject("Microsoft.XMLDOM");
        xml_document.async = false;
        xml_document.loadXML(xml_string);
    }

    return xml_document;
}

////// Utility classes

// IntRef is for passing an integer value by reference
function IntRef(value)
{
    this._value = value;
};

IntRef.prototype.Increment = function()
{
    this._value++;
};

IntRef.prototype.Decrement = function()
{
    this._value--;
};

IntRef.prototype.SetValue = function(int_value)
{
    this._value = int_value;
};

IntRef.prototype.GetValue = function()
{
    return this._value;
};

let _new_url = "";
let _test_ws = null;
let _retry_server = false;
let _reboot_start_time_ms = 0;

function ServerIsOnline()
{
    try
    {
        if (!OJLib._dead)
        {
            if (_test_ws != null)
            {
                _test_ws.close();
                _test_ws = null;
            }
            return true;
        }

        let url = document.URL;
        if (_new_url != "")
        {
            // If we've waited more than one minute, try the original url
            let time_now = new Date().getTime();
            let delta_time_since_reboot = time_now - _reboot_start_time_ms;

            if (delta_time_since_reboot < 60000)
                url = _new_url
            else
                _new_url = "";
        }

        let websocket_server_url = url.replace("http", "ws");

        if ((_test_ws != null) && !_retry_server)
            return false;

        if (_test_ws)
        {
            _test_ws.close();
            _test_ws = null;
            _retry_server = false;
        }

        _test_ws = new WebSocket(websocket_server_url);
        _test_ws.onopen = function(event)
        {
            OJLib._dead = false;
        };

        _test_ws.onerror = function(event)
        {
            OJLib._dead = true;
            _retry_server = true;
        }

        return !OJLib._dead;
    }
    catch (error)
    {
        return false;
    }
}

function OnResizeDone()
{
    clearTimeout(resize_done_timeout_id);
    resize_done_timeout_id = null;

    if (window.IsMobileDevice())
        return;

    let server_link = OJServerLink.Get();
    if (server_link != null)
    {
        if (!server_link._ui_loaded)
            return;
        server_link.ResetUiSize();
    }
    else
    {
        // Reload the page now
        location.reload(true);
    }

    clearTimeout(resize_done_timeout_id);
    resize_done_timeout_id = null;
    resizing = false;
}

function ReloadPage()
{
    if (ServerIsOnline())
    {
        clearInterval(OJLib._dead_interval_id);
        OJLib._dead_interval_id = null;

        if (_new_url == "")
            location.reload(true);
        else
            ConnectToServer(_new_url);
    }
}

function ConnectToServer(url)
{
    location.replace(url);
}

///////////////

let all_button_callbacks = [];

function UnregisterAllButtonCallbacks()
{
    for (let i = 0; i < all_button_callbacks.length; i++)
        all_button_callbacks[i].UnregisterForEvents();
}

function SetMouseButtonStates(event)
{
    event._lbutton_down = UI._lbutton_down;
    event._mbutton_down = UI._mbutton_down;
    event._rbutton_down = UI._rbutton_down;
}


// Implement any of OnClick, OnLButtonDown, OnLButtonUp, OnMouseOver, OnMouseOut
// OnRButtonDown, OnRButtonUp
export class ButtonCallbackContainer
{
    constructor(owner, element, user_data, insert_position)
    {
        all_button_callbacks.push(this);
        this._owner = owner;
        this._element = element;
        this._call_click_on_mouse_down = false;
        this._registered_for_events = false;
        this._is_destroyed = false;
        this._monitor = false;
        this._insert_position = insert_position;

        if (typeof (owner.OnClick) === "function")
            this._click_callback = new ObjectCallback(owner, "OnClick", element, user_data);
        else
            this._click_callback = null;

        if (typeof (owner.OnDoubleClick) === "function")
            this._double_click_callback = new ObjectCallback(owner, "OnDoubleClick", element, user_data);
        else
            this._double_click_callback = null;

        if (typeof (owner.OnLButtonDown) === "function")
            this._lbutton_down_callback = new ObjectCallback(owner, "OnLButtonDown", element, user_data);
        else
            this._lbutton_down_callback = null;

        if (typeof (owner.OnLButtonUp) === "function")
            this._lbutton_up_callback = new ObjectCallback(owner, "OnLButtonUp", element, user_data);
        else
            this._lbutton_up_callback = null;

        if (typeof (owner.OnRButtonDown) === "function")
            this._rbutton_down_callback = new ObjectCallback(owner, "OnRButtonDown", element, user_data);
        else
            this._rbutton_down_callback = null;

        if (typeof (owner.OnRButtonUp) === "function")
            this._rbutton_up_callback = new ObjectCallback(owner, "OnRButtonUp", element, user_data);
        else
            this._rbutton_up_callback = null;

        if (typeof (owner.OnContextMenu) === "function")
            this._context_menu_callback = new ObjectCallback(owner, "OnContextMenu", element, user_data);
        else
            this._context_menu_callback = null;

        if (typeof (owner.OnMouseMove) === "function")
            this._mouse_move_callback = new ObjectCallback(owner, "OnMouseMove", element, user_data);
        else
            this._mouse_move_callback = null;

        if (typeof (owner.OnMouseWheel) === "function")
            this._mouse_wheel_callback = new ObjectCallback(owner, "OnMouseWheel", element, user_data);
        else
            this._mouse_wheel_callback = null;

        if (typeof (owner.OnKeyDown) === "function")
            this._key_down_callback = new ObjectCallback(owner, "OnKeyDown", element, user_data);
        else
            this._key_down_callback = null;

        element._button_callback_container = this;

        element.onmouseover = function(event)
        {
            AssignSourceElement(event);
            SetMouseButtonStates(event);

            if (typeof (owner.OnActualMouseOver) === "function")
                this._button_callback_container._owner.OnActualMouseOver(event);

            if (UI._capture_mouse_element != null)
            {
                AddPendingMouseOver(this);
                return;
            }

            this._button_callback_container.RegisterForEvents();

            if (CheckModalDialog(event))
            {
                if (typeof (owner.OnMouseOver) === "function")
                    this._button_callback_container._owner.OnMouseOver(event);
            }
        };

        element.onmouseout = function(event)
        {
            AssignSourceElement(event);
            SetMouseButtonStates(event);

            if (typeof (owner.OnActualMouseOut) === "function")
                this._button_callback_container._owner.OnActualMouseOut(event);

            if (UI._capture_mouse_element != null)
            {
                AddPendingMouseOut(this);
                return;
            }

            //OJLib.Trace("UnregisterForEvents " + this._button_callback_container._owner);
            this._button_callback_container.UnregisterForEvents();

            if (CheckModalDialog(event))
            {
                if (typeof (owner.OnMouseOut) === "function")
                    this._button_callback_container._owner.OnMouseOut(event);
            }
        };
    }

    Destroy()
    {
        if (this._is_destroyed)
            return;

        this._is_destroyed = true;
        this.UnregisterForEvents();
        let my_index = all_button_callbacks.indexOf(this);
        if (my_index >= 0)
            all_button_callbacks.splice(my_index, 1);

        this._owner = null;
        this._element._button_callback_container = null;
        this._element.onmouseover = null;
        this._element.onmouseout = null;
        this._element = null;

        if (this._click_callback != null)
        {
            this._click_callback.Destroy();
            this._click_callback = null;
        }

        if (this._double_click_callback != null)
        {
            this._double_click_callback.Destroy();
            this._double_click_callback = null;
        }

        if (this._lbutton_down_callback != null)
        {
            this._lbutton_down_callback.Destroy();
            this._lbutton_down_callback = null;
        }

        if (this._lbutton_up_callback != null)
        {
            this._lbutton_up_callback.Destroy();
            this._lbutton_up_callback = null;
        }

        if (this._rbutton_down_callback != null)
        {
            this._rbutton_down_callback.Destroy();
            this._rbutton_down_callback = null;
        }

        if (this._rbutton_up_callback != null)
        {
            this._rbutton_up_callback.Destroy();
            this._rbutton_up_callback = null;
        }

        if (this._context_menu_callback != null)
        {
            this._context_menu_callback.Destroy();
            this._context_menu_callback = null;
        }

        if (this._mouse_move_callback != null)
        {
            this._mouse_move_callback.Destroy();
            this._mouse_move_callback = null;
        }

        if (this._mouse_wheel_callback != null)
        {
            this._mouse_wheel_callback.Destroy();
            this._mouse_wheel_callback = null;
        }

        if (this._key_down_callback != null)
        {
            this._key_down_callback.Destroy();
            this._key_down_callback = null;
        }
    }

    RegisterForEvents()
    {
        if (this._monitor)
            OJLib.Trace("-> RegisterForEvents");

        if (!this._registered_for_events)
        {
            if (this._click_callback)
            {
                // Normally OnClick is called on the mouse up event, but
                // you can override this by setting _call_click_on_mouse_down
                if (this._call_click_on_mouse_down)
                    OJLib.RegisterForLButtonDown(this._click_callback, this._insert_position);
                else
                    OJLib.RegisterForClick(this._click_callback, this._insert_position);
            }

            if (this._double_click_callback != null)
                OJLib.RegisterForDoubleClick(this._double_click_callback, this._insert_position);

            if (this._lbutton_down_callback != null)
                OJLib.RegisterForLButtonDown(this._lbutton_down_callback, this._insert_position);

            if (this._lbutton_up_callback != null)
                OJLib.RegisterForLButtonUp(this._lbutton_up_callback, this._insert_position);

            if (this._rbutton_down_callback != null)
                OJLib.RegisterForRButtonDown(this._rbutton_down_callback, this._insert_position);

            if (this._rbutton_up_callback != null)
                OJLib.RegisterForRButtonUp(this._rbutton_up_callback, this._insert_position);

            if (this._context_menu_callback != null)
                OJLib.RegisterForContextMenu(this._context_menu_callback, this._insert_position);

            if (this._mouse_move_callback != null)
                OJLib.RegisterForMouseMove(this._mouse_move_callback, this._insert_position);

            if (this._mouse_wheel_callback != null)
                OJLib.RegisterForMouseWheel(this._mouse_wheel_callback, this._insert_position);

            if (this._key_down_callback != null)
                OJLib.RegisterForKeyDown(this._key_down_callback, this._insert_position);

            this._registered_for_events = true;
        }
    }

    UnregisterForEvents()
    {
        if (this._monitor)
            OJLib.Trace("-> UnregisterForEvents");

        if (this._registered_for_events)
        {
            if (this._click_callback)
            {
                if (this._call_click_on_mouse_down)
                    OJLib.UnregisterForLButtonDown(this._click_callback);
                else
                    OJLib.UnregisterForClick(this._click_callback);
            }

            if (this._double_click_callback != null)
                OJLib.UnregisterForDoubleClick(this._double_click_callback);

            if (this._lbutton_down_callback != null)
                OJLib.UnregisterForLButtonDown(this._lbutton_down_callback);

            if (this._lbutton_up_callback != null)
                OJLib.UnregisterForLButtonUp(this._lbutton_up_callback);

            if (this._rbutton_down_callback != null)
                OJLib.UnregisterForRButtonDown(this._rbutton_down_callback);

            if (this._rbutton_up_callback != null)
                OJLib.UnregisterForRButtonUp(this._rbutton_up_callback);

            if (this._context_menu_callback != null)
                OJLib.UnregisterForContextMenu(this._context_menu_callback);

            if (this._mouse_move_callback != null)
                OJLib.UnregisterForMouseMove(this._mouse_move_callback);

            if (this._mouse_wheel_callback != null)
                OJLib.UnregisterForMouseWheel(this._mouse_wheel_callback);

            if (this._key_down_callback != null)
                OJLib.UnregisterForKeyDown(this._key_down_callback);

            this._registered_for_events = false;
        }
    }
}

/////////////////////////

// _user_data will be attached to the event at callback


let _base64_lookup = null;

function BuildBase64LookupTable()
{
    if (_base64_lookup != null)
        return;

    _base64_lookup = [];
    let alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (let i = 0; i < 64; i++)
    {
        _base64_lookup[alphabet[i]] = i;
    }
};

function Decode64BitString(data)
{
    BuildBase64LookupTable();

    let length = data.length;

    // Length should always be in multiples of 4
    if (length < 4)
    {
        if (typeof Uint8ClampedArray == "undefined")
            return new Uint8Array(0);
        else
            return new Uint8ClampedArray(0);
    }

    let len = ((data.length / 4) * 3) | 0;
    let double_eq = data.substr(length - 2);
    let single_eq = data.substr(length - 1);
    let handle_tail = true;
    if (double_eq == "==")
        len -= 2;
    else if (single_eq == "=")
        len -= 1;
    else
        handle_tail = false;

    let bytes = null;
    if (typeof Uint8ClampedArray == "undefined")
        bytes = new Uint8Array(len);
    else
        bytes = new Uint8ClampedArray(len);

    let out_i = 0;
    let loop_end = handle_tail ? (length - 4) : length;
    for (let i = 0; i < loop_end; i += 4)
    {
        let a = _base64_lookup[data[i]];
        let b = _base64_lookup[data[i + 1]];
        let c = _base64_lookup[data[i + 2]];
        let d = _base64_lookup[data[i + 3]];

        bytes[out_i++] = (a * 4) | ((b & 0x30) >> 4);
        bytes[out_i++] = ((b & 0xf) << 4) | ((c & 0x3c) >> 2);
        bytes[out_i++] = ((c & 0x3) << 6) | d;
    }

    if (handle_tail)
    {
        // Special case for the last 4 characters
        let a = _base64_lookup[data[loop_end]];
        let b = _base64_lookup[data[loop_end + 1]];
        let c = _base64_lookup[data[loop_end + 2]];
        let d = _base64_lookup[data[loop_end + 3]];

        bytes[out_i++] = a * 4 + (b & 0x30) >> 4;
        if (out_i < len)
        {
            bytes[out_i++] = (b & 0xf) << 4 + (c & 0x3c) >> 2;
            if (out_i < len)
                bytes[out_i++] = (c & 0x2) << 6 + d;
        }
    }

    return bytes;
}

let _current_context_menu = null;

function ShowContextMenu(context_menu, x, y)
{
    if (_current_context_menu != null)
        _current_context_menu.CloseIt();

    _current_context_menu = context_menu;
    _current_context_menu.OpenIt(x, y);
}

function ParseIntArray(value)
{
    let array = [];
    if (value == "")
        return array;

    let start = 0;
    while (true)
    {
        let comma = value.indexOf(",", start);
        if (comma < 0)
        {
            array.push(parseInt(value.substring(start)));
            break;
        }
        else
        {
            array.push(parseInt(value.substring(start, comma)));
            start = comma + 1;
        }
    }

    return array;
}

function ParseFloatArray(value)
{
    let array = [];
    if (value == "")
        return array;

    let start = 0;
    while (true)
    {
        let comma = value.indexOf(",", start);
        if (comma < 0)
        {
            array.push(parseFloat(value.substring(start)));
            break;
        }
        else
        {
            array.push(parseFloat(value.substring(start, comma)));
            start = comma + 1;
        }
    }

    return array;
}

// Return string array, values separated by strings
function ParseArray(value)
{
    let array = [];
    if (value == "")
        return array;

    let start = 0;
    while (true)
    {
        let comma = value.indexOf(",", start);
        if (comma < 0)
        {
            array.push(value.substring(start));
            break;
        }
        else
        {
            array.push(value.substring(start, comma));
            start = comma + 1;
        }
    }

    return array;
}

// Add helper functions to canvas.getContext
function ClearLineDashes(dc)
{
    if (dc.setLineDash)
        dc.setLineDash([]);

    if (dc.webkitLineDash)
        dc.webkitLineDash = [];
};

function DrawSolidLine(dc, x1, y1, x2, y2)
{
    dc.beginPath();
    ClearLineDashes(dc);
    dc.moveTo(x1, y1);
    dc.lineTo(x2, y2);
    dc.stroke();
};

function DrawDottedLine(dc, x1, y1, x2, y2, spacing)
{
    dc.beginPath();
    let have_dots = false;
    if (dc.setLineDash)
    {
        dc.setLineDash([1, spacing]);
        have_dots = true;
    }

    if (dc.webkitLineDash)
    {
        dc.webkitLineDash = [1, spacing];
        have_dots = true;
    }

    if (have_dots)
    {
        dc.moveTo(x1, y1);
        dc.lineTo(x2, y2);
    }
    else
    {
        spacing++;
        if (x1 == x2)
        {
            let y_min = Math.min(y1, y2);
            let y_max = Math.max(y1, y2);

            for (let y = y_min; y <= y_max; y += spacing)
            {
                dc.moveTo(x1, y);
                dc.lineTo(x1, y + 1);
            }
        }
        else if (y1 == y2)
        {
            let x_min = Math.min(x1, x2);
            let x_max = Math.max(x1, x2);

            for (let x = x_min; x <= x_max; x += spacing)
            {
                dc.moveTo(x, y1);
                dc.lineTo(x + 1, y1);
            }
        }
        else
        {
            // Not horizontal or vertical, just draw solid line
            dc.moveTo(x1, y1);
            dc.lineTo(x2, y2);
        }
    }
    dc.stroke();

    if (dc.setLineDash)
        dc.setLineDash([]);
    if (dc.webkitLineDash)
        dc.webkitLineDash = [];
};

function FillRotatedText(dc, x, y, angle, text)
{
    let sin_angle = Math.sin(angle);
    let cos_angle = Math.cos(angle);
    dc.textAlign = "center";
    dc.setTransform(1, 0, 0, 1, x, y);
    dc.transform(cos_angle, sin_angle, -sin_angle, cos_angle, 0, 0);
    dc.transform(1, 0, 0, 1, -x, -y);
    dc.transform(1, 0, 0, 1, 0.5, 0.0);
    dc.fillText(text, x, y);
    dc.setTransform(1, 0, 0, 1, 0, 0); // Reset
};

function FillRotatedTextDegrees(dc, x, y, angle, text)
{
    let angle_rads = angle * Math.PI / 180;
    FillRotatedText(dc, x, y, angle_rads, text);
};

function WarningMessage(message)
{
    let params = 
    { 
        dialog_type: "MessageDialog",
        message: message,
        button_0: "Close",
        button_1: "",
        width: 200,
        height: 100
    };
        
    OJServerLink.Get().ModalDialogCB(params);
}

let _known_touch_device = false;
let _is_touch_device = false;

function IsTouchDevice()
{
    if (!_known_touch_device)
    {
        _known_touch_device = true;
        if ("ontouchstart" in window)
        {
            if (navigator.userAgent.indexOf("basicweb") >= 0)
                _is_touch_device = false;
            else
                _is_touch_device = true;
        }
        else if (navigator.MaxTouchPoints > 0)
            _is_touch_device = true;
        else if (navigator.msMaxTouchPoints > 0)
            _is_touch_device = true;
        else
            _is_touch_device = false;
    }

    return _is_touch_device;
}

export class HslColor
{
    constructor(h, s, l)
    {
        this._h = h;
        this._s = s;
        this._l = l;
    }

    HueConvert(p, q, t)
    {
        if (t < 0.0)
            t += 1.0;
        if (t > 1.0)
            t -= 1.0;
        if (t < 1.0 / 6.0)
            return p + (q - p) * 6.0 * t;
        if (t < 1.0 / 2.0)
            return q;
        if (t < 2.0 / 3.0)
            return p + (q - p) * (2.0 / 3.0 - t) * 6.0;

        return p;
    }

    ToRgb()
    {
        let r;
        let g;
        let b;

        if (this._s == 0)
        {
            r = g = b = this._l; // achromatic
        }
        else
        {
            let q = this._l < 0.5 ? this._l * (1.0 + this._s) : this._l + this._s - this._l * this._s;
            let p = 2 * this._l - q;
            r = this.HueConvert(p, q, this._h + 1.0 / 3.0);
            g = this.HueConvert(p, q, this._h);
            b = this.HueConvert(p, q, this._h - 1.0 / 3.0);
        }

        let r_int = Math.min(255, (r * 255.0)) | 0;
        let g_int = Math.min(255, (g * 255.0)) | 0;
        let b_int = Math.min(255, (b * 255.0)) | 0;

        return new RgbColor(r_int, g_int, b_int);
    }
}

///

export class RgbColor
{
    constructor(red, green, blue)
    {
        this._red = red;
        this._green = green;
        this._blue = blue;
    }

    Scale(scale)
    {
        let red = (this._red * scale) | 0;
        let green = (this._green * scale) | 0;
        let blue = (this._blue * scale) | 0;

        if (red > 255)
            red = 255;
        if (green > 255)
            green = 255;
        if (blue > 255)
            blue = 255;

        return new RgbColor(red, green, blue);
    }

    Colourise(hue, saturation)
    {
        let hsl_colour = this.ToHsl();
        hsl_colour._h += hue;
        if (hsl_colour._h > 1.0)
            hsl_colour._h -= 1.0;

        hsl_colour._s += saturation;

        let rgb = hsl_colour.ToRgb();
        this._red = rgb._red;
        this._green = rgb._green;
        this._blue = rgb._blue;
    }

    GetCssString()
    {
        let str_colour = "#" + this._red.toString(16) + this._green.toString(16) + this._blue.toString(16);
        return str_colour;
    }

    ToHsl()
    {
        let r = this._red / 255.0;
        let g = this._green / 255.0;
        let b = this._blue / 255.0;

        let maximum = Math.max(r, g);
        maximum = Math.max(maximum, b);

        let minimum = Math.min(r, g);
        minimum = Math.min(minimum, b);

        let h = 0.0;
        let s = 0.0;
        let l = (maximum + minimum) / 2.0;

        if (maximum == minimum)
        {
            // achromatic
        }
        else
        {
            let d = maximum - minimum;
            s = (l > 0.5) ? d / (2.0 - maximum - minimum) : d / (maximum + minimum);

            if (maximum == r)
                h = (g - b) / d + (g < b ? 6.0 : 0.0);
            else if (maximum == g)
                h = (b - r) / d + 2.0;
            else if (maximum == b)
                h = (r - g) / d + 4.0;

            h /= 6.0;
        }

        return new HslColor(h, s, l);
    }
}

// String utilities
String.prototype.Left = function(length)
{
    return this.substr(0, length);
};

String.prototype.Mid = function(start, length)
{
    return this.substr(start, length);
};

String.prototype.Right = function(length)
{
    return this.substr(this.length - length);
};

String.prototype.GetLength = function()
{
    return this.length;
};

String.prototype.IsEmpty = function()
{
    return (this.length == 0);
};

String.prototype.ReverseFind = function(search)
{
    return (this.lastIndexOf(search));
};

String.prototype.Find = function(search)
{
    return (this.indexOf(search));
};

///////////////////////////////////////////////////////////////////////////////

export class ElementDragger
{
    constructor(callback_object, element, user_data)
    {
        this._callback_object = callback_object;
        this._button_callback = new ButtonCallbackContainer(this, element, user_data);
        this._lbutton_down = false;
        this._lbutton_down_x = 0;
        this._lbutton_down_y = 0;
        this._drag_start_cb = null
        this._drag_callback = null;
        this._drag_end_cb = null;
        this._drag_mouse_over_cb = null;
        this._drag_mouse_out_cb = null;

        if (typeof (this._callback_object.DragStart) === "function")
            this._drag_start_cb = new ObjectCallback(this._callback_object, "DragStart", element, user_data);

        if (typeof (this._callback_object.Drag) === "function")
            this._drag_callback = new ObjectCallback(this._callback_object, "Drag", element, user_data);

        if (typeof (this._callback_object.DragEnd) === "function")
            this.property = new ObjectCallback(this._callback_object, "DragEnd", element, user_data);

        if (typeof (this._callback_object.DragMouseOver) === "function")
            this._drag_mouse_over_cb = new ObjectCallback(this._callback_object, "DragMouseOver", element, user_data);

        if (typeof (this._callback_object.DragMouseOut) === "function")
            this._drag_mouse_out_cb = new ObjectCallback(this._callback_object, "DragMouseOut", element, user_data);
    }

    Destroy()
    {
        this._button_callback.Destroy();
        this._drag_callback = null;
        this._callback_object = null;

        if (this._drag_start_cb != null)
            this._drag_start_cb.Destroy();

        if (this._drag_callback != null)
            this._drag_callback.Destroy();

        if (this._drag_end_cb != null)
            this._drag_end_cb.Destroy();

        if (this._drag_mouse_over_cb != null)
            this._drag_mouse_over_cb.Destroy();
            
        if (this._drag_mouse_out_cb != null)
            this._drag_mouse_out_cb.Destroy();
    }

    OnLButtonDown(event)
    {
        this._lbutton_down = true;
        let user_data = event._user_data;
        this._lbutton_down_x = event._client_x;
        this._lbutton_down_y = event._client_y;
        if (this._drag_start_cb != null)
            this._drag_start_cb.Call({ _event: event });
        OJLib.CaptureMouse(event.target);
    }

    OnLButtonUp(event)
    {
        OJLib.ReleaseMouse(event);
        this._lbutton_down = false;

        let dx = event._client_x - this._lbutton_down_x;
        let dy = event._client_y - this._lbutton_down_y;

        if (this._drag_end_cb != null)
            this._drag_end_cb.Call({ _dx: dx, _dy: dy, _event: event });
    }

    OnMouseMove(event)
    {
        if (this._lbutton_down)
        {
            let dx = event._client_x - this._lbutton_down_x;
            let dy = event._client_y - this._lbutton_down_y;
            if (this._drag_callback != null)
                this._drag_callback.Call({ _dx: dx, _dy: dy, _event: event });
        }
    }

    OnMouseOver(event)
    {
        if (this._drag_mouse_over_cb != null)
            this._drag_mouse_over_cb.Call({ _event: event });
    }

    OnMouseOut(event)
    {
        if (this._drag_mouse_out_cb != null)
            this._drag_mouse_out_cb.Call({ _event: event });
    }
}




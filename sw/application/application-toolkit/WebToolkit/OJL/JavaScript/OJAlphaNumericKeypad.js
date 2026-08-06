/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
"use strict";

import { OJGrid } from "./OJL.js";
import { OJServerLink } from "./OJL.js";
import { OJTextButton } from "./OJL.js";
import { OJLib } from "./OJL.js";
import { ANCHOR_TYPE } from "./OJL.js";
import { OJPictureButton } from "./OJL.js";

export class OJAlphaNumericKeypad extends OJGrid
{
    constructor(parent_element)
    {
        // Base class constructor
        super(parent_element);
        this.GetElement().id = "OJAlphaNumericKeypad";
        this.GetElement().style.backgroundColor = "#3d5f9a";
        this.GetElement().style.display = "none";
        this.GetElement().style.zIndex = 1001; // Above OJModalDialog
        this._spacing = 10;
        this._command = new Array(8);
        this._objects = new Array(8);
        this._object_callbacks = new Array(8);
        for (let i = 0; i < 8; i++)
            this._command[i] = "";

        this._value = "";
        this._shift = false;
        this._first_button_press = true;
        let border = 1;
        let size_y = 60;
        let size_x = 70;
        let text_control_height = 35;

        this._height = 6 * border + 5 * size_y + border + text_control_height;
        this._width = 980;

        this._text_control = document.createElement("div");
        this._text_control.className = "alpha_keypad_input_control_class";
        this.GetElement().appendChild(this._text_control);

        this._close_button = new OJPictureButton(this.GetElement(), OJLib._close_keypad._src, null,
            { _object: this, _object_callback: "OnCloseKeypad" }, { _width: 44, _height: 27 });
        this._close_button.ManualPositioning();
        this._close_button.GetElement().style.right = "49px";
        this._close_button.GetElement().style.top = "0px";
        this._close_button._image._is_dialog_element = true;

        let keys = ["1!", '2"', "3£", "4$", "5%", "6^", "7&", "8*", "9(", "0)", "-_", "=+",
                    "qQ", "wW", "eE", "rR", "tT", "yY", "uU", "iI", "oO", "pP", "[{", "]}",
                    "aA", "sS", "dD", "fF", "gG", "hH", "jJ", "kK", "lL", ";:", "'@", "#~",
                    "\|", "zZ", "xX", "cC", "vV", "bB", "nN", "mM", ",<", ".>", "/?"];
        let i = 0;
        for (let y = 0; y < 4; y++)
        {
            for (let x = 0; x < 12; x++)
            {
                if (keys[i] == null)
                {
                    x = 12;
                    y = 4;
                    break;
                }

                let label = keys[i].substr(0, 1);
                if ((label >= "a") && (label <= "z"))
                {
                    //label = keys[i].substr(1);
                }
                else
                    label = keys[i].substr(1, 1) + "<br>" + keys[i].substr(0, 1);

                let button = new OJTextButton(this.GetElement(), label,
                    {
                        _click_object: this,
                        _click_callback: "ButtonClickedCB",
                        _allow_html: true,
                        _line_height: 30,
                        //_colour_key: "#353535",
                        _user_data: keys[i]
                    });

                button._text_div.className = "multiline_text_button_class";
                button._text_div.style.paddingLeft = "10px";

                let x_pixel = border + (size_x + border) * x;
                let y_pixel = border + (size_y + border) * y + border + text_control_height;
                if (y == 1)
                    x_pixel += 35;
                else if (y == 2)
                    x_pixel += (35 + 18);
                else if (y == 3)
                    x_pixel += 15;

                        // 18, 52
                button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: x_pixel });
                button.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: y_pixel });
                button.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: size_x });
                button.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: size_y });

                //button.GetElement().className = "softkey_button";//+ i;

                this.AddChild(button);

                if (keys[i] == ".")
                    this._point_button = button;
                i++;
            }
        }

        let bs_button = new OJTextButton(this.GetElement(), "&laquo;",
            {
                _click_object: this,
                _click_callback: "ButtonClickedCB",
                _colour_key: "#323662",
                _user_data: "Backspace"
            });

        let x_pixel = 853;
        let y_pixel = border + border + text_control_height;
        bs_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: x_pixel });
        bs_button.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: y_pixel });
        bs_button.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 126 });
        bs_button.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: size_y });
        this.AddChild(bs_button);

        let space_button = new OJTextButton(this.GetElement(), "",
            {
                _click_object: this,
                _click_callback: "ButtonClickedCB",
                _user_data: " "
            });

        x_pixel = 200;
        y_pixel = border + (size_y + border) * 4 + border + text_control_height;
        space_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: x_pixel });
        space_button.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: y_pixel });
        space_button.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 500 });
        space_button.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: size_y });
        this.AddChild(space_button);

        let enter_button = new OJTextButton(this.GetElement(), "Enter",
            {
                _click_object: this,
                _click_callback: "ButtonClickedCB",
                _colour_key: "#336238",
                _user_data: "Enter"
            });

        x_pixel = 888;
        y_pixel = border + (size_y + border) * 1 + border + text_control_height;
        enter_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: x_pixel });
        enter_button.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: y_pixel });
        enter_button.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 91 });
        enter_button.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: size_y });
        this.AddChild(enter_button);

        let shift_button = new OJTextButton(this.GetElement(), "Shift",
            {
                _click_object: this,
                _click_callback: "ButtonClickedCB",
                _colour_key: "#616234",
                _user_data: "Shift"
            });

        x_pixel = 797;
        y_pixel = border + (size_y + border) * 3 + border + text_control_height;
        shift_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: x_pixel });
        shift_button.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: y_pixel });
        shift_button.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: 130 });
        shift_button.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: size_y });
        this.AddChild(shift_button);

        for (let i = 0; i < this._children.length; i++)
            this._children[i].GetButtonElement()._is_dialog_element = true
    }

    Destroy()
    {
        this._close_button.Destroy();
        super.Destroy();
    }

    ButtonClickedCB(event)
    {
        if (event._user_data == "Enter")
        {
            OJServerLink.Get().HideKeypad(this._value);
            return;
        }

        if (event._user_data == "Shift")
        {
            this.ToggleShift();
            return;
        }

        if (this._first_button_press)
        {
            if (event._user_data != "Backspace")
                this._value = "";

            this._first_button_press = false;
        }

        if (event._user_data == " ")
            this._value += " ";
        else if (event._user_data == "Backspace")
            this._value = this._value.substr(0, this._value.length - 1);
        else
        {
            if (this._shift)
                this._value += event._user_data.substr(1, 1);
            else
                this._value += event._user_data.substr(0, 1);
        }

        this.UpdateValue();
    }

    SetValue(value)
    {
        this._value = value;
        this.UpdateValue();
    }

    UpdateValue()
    {
        let str = this._value.replace(/ /g, "&nbsp;");
        this._text_control.innerHTML = str;
    }

    OnCloseKeypad()
    {
        OJServerLink.Get().HideKeypad();
    }

    SetControlType(control_type)
    {
        this._control_type = control_type;
        this._first_button_press = true;
    }

    ToggleShift()
    {
        this._shift = !this._shift;

        for (let i = 0; i < this._children.length; i++)
        {
            let child = this._children[i];
            let user_data = child._user_data;
            if (user_data.length == 2)
            {
                let label = this._shift ? user_data.substr(1, 1) : user_data.substr(0, 1);

                let ch = user_data.substr(0, 1);
                if ((ch >= "a") && (ch <= "z"))
                    child.SetLabel(label);
                else
                {
                    if (this._shift)
                        label = user_data.substr(0, 1) + "<br>" + user_data.substr(1, 1);
                    else
                        label = user_data.substr(1, 1) + "<br>" + user_data.substr(0, 1);
                    child.SetLabel(label);
                }
            }
        }
    }
}

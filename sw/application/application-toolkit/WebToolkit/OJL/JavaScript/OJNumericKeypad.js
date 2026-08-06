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
import { OJPictureButton } from "./OJL.js";
import { OJLib } from "./OJL.js";
import { ANCHOR_TYPE } from "./OJL.js";
import { TEXT_CONTROL_TYPE } from "./OJL.js";

export class OJNumericKeypad extends OJGrid
{
    constructor(parent_element)
    {
        // Base class constructor
        super(parent_element);
        this.GetElement().id = "OJNumericKeypad";
        this.GetElement().style.backgroundColor = "#3d5f9a";
        this.GetElement().style.display = "none";
        this.GetElement().style.zIndex = 200;
        this._spacing = 10;
        this._command = new Array(8);
        this._objects = new Array(8);
        this._object_callbacks = new Array(8);
        for (var i = 0; i < 8; i++)
            this._command[i] = "";

        this._control_type = TEXT_CONTROL_TYPE.TCT_FLOATING_POINT;
        this._point_button = null;
        this._first_button_press = true;
        var border = 1;
        var size_y = 60;
        var size_x = 70;
        var text_control_height = 35;

        this._height = 5 * border + 4 * size_y + border + text_control_height;
        this._width = 5 * border + 4 * size_x;

        this._text_control = document.createElement("div");
        this._text_control.className = "keypad_input_control_class";
        this.GetElement().appendChild(this._text_control);

        this._close_button = new OJPictureButton(this.GetElement(), OJLib._close_keypad._src, null,
            { _object: this, _object_callback: "OnCloseKeypad" }, { _width: 44, _height: 27 });
        this._close_button.ManualPositioning();
        this._close_button.GetElement().style.right = "49px";
        this._close_button.GetElement().style.top = "0px";
        this._close_button._image._is_dialog_element = true;

        var keys = ["7", "8", "9", "4", "5", "6", "1", "2", "3", "0", "+/&#8211;", "."]
        var colours = ["#353535", "#353535", "#353535",
                    "#353535", "#353535", "#353535",
                    "#353535", "#353535", "#353535",
                    "#353535", "#252525", "#252525"];
        var i = 0;
        for (var y = 0; y < 4; y++)
        {
            for (var x = 0; x < 3; x++)
            {
                var button = new OJTextButton(this.GetElement(), keys[i],
                    {
                        _click_object: this,
                        _click_callback: "ButtonClickedCB",
                        _colour_key: colours[i],
                        _user_data: keys[i]
                    });

                //button.SetBackgroundColour("#303038");

                var x_pixel = border + (size_x + border) * x;
                var y_pixel = border + (size_y + border) * y + border + text_control_height;
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

        var clear_button = new OJTextButton(this.GetElement(), "C",
            {
                _click_object: this,
                _click_callback: "ButtonClickedCB",
                _colour_key: "#616234",
                _user_data: 10
            });

        var x_pixel = border + (size_x + border) * 3;
        var y_pixel = border + (size_y + border) * 0 + border + text_control_height;
        clear_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: x_pixel });
        clear_button.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: y_pixel });
        clear_button.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: size_x });
        clear_button.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: size_y });
        this.AddChild(clear_button);

        var delete_button = new OJTextButton(this.GetElement(), "&laquo;",
            {
                _click_object: this,
                _click_callback: "ButtonClickedCB",
                _colour_key: "#323662",
                _user_data: 11
            });

        var x_pixel = border + (size_x + border) * 3;
        var y_pixel = border + (size_y + border) * 1 + border + text_control_height;
        delete_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: x_pixel });
        delete_button.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: y_pixel });
        delete_button.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: size_x });
        delete_button.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: size_y });
        this.AddChild(delete_button);

        var enter_button = new OJTextButton(this.GetElement(), "Enter",
            {
                _click_object: this,
                _click_callback: "ButtonClickedCB",
                _colour_key: "#336238",
                _user_data: 12
            });

        x_pixel = border + (size_x + border) * 3;
        y_pixel = border + (size_y + border) * 2 + border + text_control_height;
        enter_button.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: x_pixel });
        enter_button.SetTopAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: y_pixel });
        enter_button.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: size_x });
        enter_button.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: size_y * 2 + border });
        this.AddChild(enter_button);

        for (var i = 0; i < this._children.length; i++)
            this._children[i].GetButtonElement()._is_dialog_element = true
    }

    ButtonClickedCB(event)
    {
        if (this._first_button_press)
        {
            if ((event._user_data != 11) && (event._user_data != 12))
                this._text_control.innerHTML = "0";
            this._first_button_press = false;
        }

        if ((event._user_data >= 0) && (event._user_data <= 9))
        {
            if (this._text_control.innerHTML == "0")
                this._text_control.innerHTML = event._user_data;
            else
                this._text_control.innerHTML += event._user_data;
        }
        else if (event._user_data == 10)
        {
            this._text_control.innerHTML = "0";
        }
        else if (event._user_data == 11)
        {
            var text = this._text_control.innerHTML;
            this._text_control.innerHTML = text.substr(0, text.length - 1);
            if (this._text_control.innerHTML == "")
                this._text_control.innerHTML = "0";
        }
        else if (event._user_data == "+/&#8211;")
        {
            if (this._text_control.innerHTML.indexOf("-") == -1)
            {
                if (this._text_control.innerHTML == "0")
                    this._text_control.innerHTML = "-";
                else
                    this._text_control.innerHTML = "-" + this._text_control.innerHTML;
            }
            else
                this._text_control.innerHTML = this._text_control.innerHTML.substr(1);
        }
        else if ((event._user_data == ".") && (this._control_type == TEXT_CONTROL_TYPE.TCT_FLOATING_POINT))
        {
            if (this._text_control.innerHTML == "")
                this._text_control.innerHTML = "0.";
            else if (this._text_control.innerHTML.indexOf(".") == -1)
                this._text_control.innerHTML += ".";
        }
        else if (event._user_data == 12)
        {
            // Enter
            OJServerLink.Get().HideKeypad(this._text_control.innerHTML);
        }
    }

    SetValue(value)
    {
        this._text_control.innerHTML = value;
    }

    OnCloseKeypad()
    {
        OJServerLink.Get().HideKeypad();
    }

    SetControlType(control_type)
    {
        this._control_type = control_type;
        this._point_button.Enable(this._control_type == TEXT_CONTROL_TYPE.TCT_FLOATING_POINT);
        this._first_button_press = true;
    }
}

/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJColourContainer } from "./OJL.js";
import { OJModalDialog } from "./OJL.js";
import { OJTextControl, TEXT_CONTROL_TYPE } from "./OJL.js";
import { OJTextButton } from "./OJL.js";
import { OJGraphicsWindow } from "./OJL.js";
import { ANCHOR_TYPE } from "./OJL.js";
import { OJLib, UI, ObjectCallback } from "./OJL.js";
import { OJOnOffButton } from "./OJL.js";
import { OJLabel } from "./OJL.js";
import { OJGrid } from "./OJL.js";
import { RgbColor } from "./OJL.js";

export class OJColourPickerDialog extends OJModalDialog
{
    constructor(params)
    {
        const colour_max = 255;
        const colour_min = 0;

        // Base class constructor
        super(430, 750, params._label);

        this._originalColour = { R: params.red, G: params.green, B: params.blue };
        this._class_name = "OJColourPickerDialog";
        this.SetElementName("OJColourPickerDialog");

        this._canvas_r_label = new OJLabel("Red: ", {
            _alignment : "left",
            _single_line : true
        });

        this._canvas_g_label = new OJLabel("Green: ", {
            _alignment : "left",
            _single_line : true
        });

        this._canvas_b_label = new OJLabel("Blue: ", {
            _alignment : "left",
            _single_line : true
        });

        this._canvas_h_label = new OJLabel("Hue: ", {
            _alignment : "left",
            _single_line : true
        });
        this._canvas_s_label = new OJLabel("Saturation: ", {
            _alignment : "left",
            _single_line : true
        });
        this._canvas_l_label = new OJLabel("Luma: ", {
            _alignment : "left",
            _single_line : true
        });

        let r_callback = new ObjectCallback(this, "SetRValue");
        this.r_entry_box = new OJTextControl(TEXT_CONTROL_TYPE.TCT_INTEGER, 0, r_callback, colour_min, colour_max);

        let g_callback = new ObjectCallback(this, "SetGValue");
        this.g_entry_box = new OJTextControl(TEXT_CONTROL_TYPE.TCT_INTEGER, 0, g_callback, colour_min, colour_max);

        let b_callback = new ObjectCallback(this, "SetBValue");
        this.b_entry_box = new OJTextControl(TEXT_CONTROL_TYPE.TCT_INTEGER, 0, b_callback, colour_min, colour_max);

        let h_callback = new ObjectCallback(this, "SetHValue");
        this.h_entry_box = new OJTextControl(TEXT_CONTROL_TYPE.TCT_INTEGER, 0, h_callback, 0, 360);

        let s_callback = new ObjectCallback(this, "SetSValue");
        this.s_entry_box = new OJTextControl(TEXT_CONTROL_TYPE.TCT_INTEGER, 0, s_callback, 0, 100);

        let l_callback = new ObjectCallback(this, "SetLValue");
        this.l_entry_box = new OJTextControl(TEXT_CONTROL_TYPE.TCT_INTEGER, 0, l_callback, 0, 100);

        this._colourPicker = new OJColourContainer(this, this._originalColour);
        this.AddChild(this._colourPicker);

        this._control_container = new OJGrid(null, null, { _corner_radius: 8 });
        this._control_container.SetBackgroundColour(UI._control_panel_background_colour);
        this.AddChild(this._control_container);

        let OKCallBack = new ObjectCallback(this, "OKButtonPressed");
        let CancelCallBack = new ObjectCallback(this, "CancelButtonPressed");
        this._OKButton = new OJTextButton(null, "OK", {_click_object : this._control_container.GetElement(), _click_callback : OKCallBack, _double_click_callback : null,
                                            _key_down_callback : null, _context_callback: null, _alignment: "centre"});
        this._CancelButton = new OJTextButton(null, "Cancel", {_click_object : this._control_container.GetElement(), _click_callback : CancelCallBack,
                                            _double_click_callback : null,
                                            _key_down_callback : null, _context_callback: null, _alignment: "centre"});
        this._CancelButton.GetElement().style.width = 200;
        this._CancelButton.GetElement().style.height = 80;

        this._canvas_preview_label = new OJLabel("Preview: ", {
            _alignment : "left",
            _single_line : true
        });

        let preview_callback = new ObjectCallback(this, "PreviewCheckboxChanged");
        this._preview_checkbox = new OJOnOffButton(false, preview_callback);
        let show_preview = params.preview;
        if (show_preview)
        {
            this._control_container.AddChild(this._canvas_preview_label);
            this._control_container.AddChild(this._preview_checkbox);
        }

        this._control_container.AddChild(this._OKButton);
        this._control_container.AddChild(this._CancelButton);

        this._control_container.AddChild(this._canvas_r_label);
        this._control_container.AddChild(this._canvas_g_label);
        this._control_container.AddChild(this._canvas_b_label);
        this._control_container.AddChild(this._canvas_h_label);
        this._control_container.AddChild(this._canvas_s_label);
        this._control_container.AddChild(this._canvas_l_label);

        this._control_container.AddChild(this.r_entry_box);
        this._control_container.AddChild(this.g_entry_box);
        this._control_container.AddChild(this.b_entry_box);
        this._control_container.AddChild(this.h_entry_box);
        this._control_container.AddChild(this.s_entry_box);
        this._control_container.AddChild(this.l_entry_box);

        let colourSquare = document.createElement('canvas');
        colourSquare.id = "ColourPreviewSquare";
        colourSquare.width = 100;
        colourSquare.height = 100;

        this._colourPreview = new OJGraphicsWindow(this, colourSquare, {_border: true});
        this.RGBUpdate(this._originalColour);
        this.UpdateRGBBoxes(this._originalColour);

        this._control_container.AddChild(this._colourPreview);
        this.SetUpAnchors();
    };

    UpdateColourChoice(hslColour)
    {
        this._colourPreview.GetDrawContext().clearRect(0,0, this._colourPreview._width, this._colourPreview._height);
        let rgb = OJLib.HSL_to_RGB(hslColour.h, hslColour.s, hslColour.l);

        this.ColourSquareFill(rgb)

        this.UpdateRGBBoxes(rgb);

        this.UpdateHSLBoxes(hslColour);

        if (this._preview_checkbox.GetValue())
            this.UpdateServer(rgb);
    }

    UpdateServer(rgbColour)
    {
        let colour = `${rgbColour.R},${rgbColour.G},${rgbColour.B}`;
        OJLib.ServerExecuteWithParams("ColourPickerUpdate", colour);
    }

    UpdateRGBBoxes(rgb)
    {
        this.r_entry_box.SetValue(rgb.R);
        this.g_entry_box.SetValue(rgb.G);
        this.b_entry_box.SetValue(rgb.B);
    }

    UpdateHSLBoxes(hslColour)
    {
        this.h_entry_box.SetValue(parseInt(hslColour.h * 360));
        this.s_entry_box.SetValue(parseInt(hslColour.s * 100));
        this.l_entry_box.SetValue(parseInt(hslColour.l * 100));
    }

    ColourSquareFill(rgb)
    {
        var imageData = this._colourPreview.GetDrawContext().createImageData(this._colourPreview._width, this._colourPreview._height);

        for (var i = 0; i < imageData.data.length; i+=4)
        {
            imageData.data[i] = rgb.R;
            imageData.data[i+1] = rgb.G;
            imageData.data[i+2] = rgb.B;
            imageData.data[i+3] = 255;
        }
        this._colourPreview.GetGraphics().PutImageData(imageData, 0,0);
    }

    RGBUpdate(rgb)
    {
        this._colourPicker.RGBChange(rgb);
        this.ColourSquareFill(rgb);
        if (this._preview_checkbox.GetValue())
            this.UpdateServer(rgb);
        let colour = new RgbColor(rgb.R, rgb.G, rgb.B);
        let hsl = colour.ToHsl()
        this.UpdateHSLBoxes({h:hsl._h, s:hsl._s, l:hsl._l});
    }

    SetRValue(r)
    {
        if (r > 255)
        {
            r = 255;
        }
        else if (r < 0)
        {
            r = 0;
        }
        this.r_entry_box.SetValue(r);
        let rgb =  {R:r, G:this.g_entry_box.GetValue(), B:this.b_entry_box.GetValue()}
        this.RGBUpdate(rgb);
    }

    SetGValue(g)
    {
        if (g > 255) {
            g = 255;
        } else if (g < 0) {
            g = 0;
        }
        this.g_entry_box.SetValue(g);
        let rgb = {R:this.r_entry_box.GetValue(), G:g, B:this.b_entry_box.GetValue()};
        this.RGBUpdate(rgb);
    }

    SetBValue(b)
    {
        if (b > 255)
        {
            b = 255;
        } else if (b < 0)
        {
            b = 0;
        }
        this.b_entry_box.SetValue(b);
        let rgb = {R:this.r_entry_box.GetValue(), G:this.g_entry_box.GetValue(), B:b};
        this.RGBUpdate(rgb);
    }

    SetHValue(h)
    {
        if (h > 360)
        {
            h = 360;
        }
        else if (h < 0)
        {
            b = 0;
        }
        this._colourPicker.HSLChange({h:h, s:this.s_entry_box.GetValue(), l:this.l_entry_box.GetValue()});
    }

    SetSValue(s)
    {
        if (s > 100)
        {
            s = 100;
        }
        else if (s < 0)
        {
            s = 0;
        }
        this._colourPicker.HSLChange({h:this.h_entry_box.GetValue(), s:s, l:this.l_entry_box.GetValue()});
    }

    SetLValue(l)
    {
        if (l > 100)
        {
            l = 100;
        }
        else if (l < 0)
        {
            l = 0;
        }
        this._colourPicker.HSLChange({h:this.h_entry_box.GetValue(), s:this.s_entry_box.GetValue(), l:l});
    }

    CancelButtonPressed(event)
    {
        this.UpdateServer(this._originalColour);

        this.OnCancel(event);
    }

    OKButtonPressed(event)
    {
        let hsl = this._colourPicker.getSelectedColour();
        let rgb = OJLib.HSL_to_RGB(hsl.h, hsl.s, hsl.l);
        this.UpdateServer(rgb);

        this.OnCancel(event);
    }

    PreviewCheckboxChanged(value)
    {
        let onOff = value ? "true" : "false"
        this._preview_checkbox.SetValue(onOff);
        if (value)
        {
            let hsl = this._colourPicker.getSelectedColour();
            let rgb = OJLib.HSL_to_RGB(hsl.h, hsl.s, hsl.l);
            this.UpdateServer(rgb)
        }
        else
        {
            let colour = `${parseInt(this._originalColour.R)},${parseInt(this._originalColour.G)},${parseInt(this._originalColour.B)}`;
            OJLib.ServerExecuteWithParams("ColourPickerUpdate", colour);
        }
    }

    SetUpAnchors()
    {
        let label_height = 30;
        let input_width = 60;
        let margin = 20;
        let spacing = 15;

        this._colourPicker.SetBottomAnchor ({
            _type         : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size   : 480
        });
        this._colourPicker.SetTopAnchor ({
            _type         : ANCHOR_TYPE.PARENT_NEAR,
        });
        this._colourPicker.SetLeftAnchor ({
            _type         : ANCHOR_TYPE.PARENT_NEAR,
        });
        this._colourPicker.SetRightAnchor({
            _type         : ANCHOR_TYPE.PARENT_FAR,
        })

        this._control_container.SetLeftAnchor({
            _type         : ANCHOR_TYPE.PARENT_NEAR,
        });
        this._control_container.SetRightAnchor({
            _type         : ANCHOR_TYPE.PARENT_FAR,
        });
        this._control_container.SetBottomAnchor({
            _type         : ANCHOR_TYPE.PARENT_FAR,
            _fixed_offset : margin
        });
        this._control_container.SetTopAnchor({
            _type         : ANCHOR_TYPE.SIBLING_FAR,
            _sibling      : this._colourPicker,
            _fixed_offset : spacing
        });

        this._canvas_preview_label.SetTopAnchor({
            _type         : ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset : spacing
        });
        this._canvas_preview_label.SetBottomAnchor({
            _type         : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size   : label_height
        });
        this._canvas_preview_label.SetLeftAnchor({
            _type         : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size   : 80
        });
        this._canvas_preview_label.SetRightAnchor({
            _type         : ANCHOR_TYPE.SIBLING_NEAR,
            _sibling      : this._preview_checkbox
        });
        this._preview_checkbox.SetTopAnchor({
            _type         : ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset : spacing
        });
        this._preview_checkbox.SetBottomAnchor({
            _type         : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size   : label_height
        });
        this._preview_checkbox.SetLeftAnchor({
            _type         : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size   : 225
        });

        this._canvas_r_label.SetLeftAnchor({
            _type			: ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset	: margin
        });
        this._canvas_r_label.SetRightAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size   	: 30
        });
        this._canvas_r_label.SetTopAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._preview_checkbox,
            _fixed_offset   : spacing
        });
        this._canvas_r_label.SetBottomAnchor({
            _type           : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size     : label_height
        });
        this.r_entry_box.SetTopAnchor({
            _type           : ANCHOR_TYPE.SIBLING_NEAR,
            _sibling        : this._canvas_r_label
        });
        this.r_entry_box.SetBottomAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._canvas_r_label
        });
        this.r_entry_box.SetLeftAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._canvas_r_label,
            _fixed_offset   : spacing + 5
        });
        this.r_entry_box.SetRightAnchor({
            _type           : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size     : input_width
        });

        this._canvas_g_label.SetTopAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._canvas_r_label,
            _fixed_offset   : 4
        });
        this._canvas_g_label.SetBottomAnchor({
            _type           : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size     : label_height
        });
        this._canvas_g_label.SetLeftAnchor({
            _type			: ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset	: margin
        });
        this._canvas_g_label.SetRightAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size   	: 30
        });
        this.g_entry_box.SetTopAnchor({
            _type           : ANCHOR_TYPE.SIBLING_NEAR,
            _sibling        : this._canvas_g_label
        });
        this.g_entry_box.SetBottomAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._canvas_g_label
        });
        this.g_entry_box.SetLeftAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._canvas_g_label,
            _fixed_offset   : spacing + 5
        });
        this.g_entry_box.SetRightAnchor({
            _type           : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size     : input_width
        });

        this._canvas_b_label.SetTopAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._canvas_g_label,
            _fixed_offset   : 4
        });
        this._canvas_b_label.SetBottomAnchor({
            _type           : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size     : label_height
        });
        this._canvas_b_label.SetLeftAnchor({
            _type			: ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset	: margin
        });
        this._canvas_b_label.SetRightAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size   	: 30
        });
        this.b_entry_box.SetTopAnchor({
            _type           : ANCHOR_TYPE.SIBLING_NEAR,
            _sibling        : this._canvas_b_label
        });
        this.b_entry_box.SetBottomAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._canvas_b_label
        });
        this.b_entry_box.SetLeftAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._canvas_b_label,
            _fixed_offset   : spacing + 5
        });
        this.b_entry_box.SetRightAnchor({
            _type           : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size     : input_width
        });

        this._canvas_h_label.SetLeftAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size   	: 30
        });
        this._canvas_h_label.SetRightAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling        : this.h_entry_box,
            _fixed_offset   : -2
        });
        this._canvas_h_label.SetTopAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._preview_checkbox,
            _fixed_offset   : spacing
        });
        this._canvas_h_label.SetBottomAnchor({
            _type           : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size     : label_height
        });
        this.h_entry_box.SetTopAnchor({
            _type           : ANCHOR_TYPE.SIBLING_NEAR,
            _sibling        : this._canvas_r_label
        });
        this.h_entry_box.SetBottomAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._canvas_r_label
        });
        this.h_entry_box.SetLeftAnchor({
            _type           : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size     : input_width
        });
        this.h_entry_box.SetRightAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this.s_entry_box
        });

        this._canvas_s_label.SetTopAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._canvas_r_label,
            _fixed_offset   : 4
        });
        this._canvas_s_label.SetBottomAnchor({
            _type           : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size     : label_height
        });
        this._canvas_s_label.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this.g_entry_box,
            _fixed_offset	: margin
        });
        this._canvas_s_label.SetRightAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size   	: 65
        });
        this.s_entry_box.SetTopAnchor({
            _type           : ANCHOR_TYPE.SIBLING_NEAR,
            _sibling        : this._canvas_s_label
        });
        this.s_entry_box.SetBottomAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._canvas_s_label
        });
        this.s_entry_box.SetLeftAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._canvas_s_label,
            _fixed_offset   : spacing
        });
        this.s_entry_box.SetRightAnchor({
            _type           : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size     : input_width
        });

        this._canvas_l_label.SetTopAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._canvas_s_label,
            _fixed_offset   : 4
        });
        this._canvas_l_label.SetBottomAnchor({
            _type           : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size     : label_height
        });
        this._canvas_l_label.SetLeftAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size   	: 30
        });
        this._canvas_l_label.SetRightAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling        : this.l_entry_box,
            _fixed_offset   : -15
        });
        this.l_entry_box.SetTopAnchor({
            _type           : ANCHOR_TYPE.SIBLING_NEAR,
            _sibling        : this._canvas_b_label
        });
        this.l_entry_box.SetBottomAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._canvas_b_label
        });
        this.l_entry_box.SetLeftAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._canvas_s_label,
            _fixed_offset   : spacing
        });
        this.l_entry_box.SetRightAnchor({
            _type           : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size     : input_width
        });

        this._OKButton.SetTopAnchor({
            _type         : ANCHOR_TYPE.SIBLING_FAR,
            _sibling      : this._canvas_l_label,
            _fixed_offset : 10
        });
        this._OKButton.SetRightAnchor({
            _type         : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size   : 100
        });
        this._OKButton.SetBottomAnchor({
            _type         : ANCHOR_TYPE.PARENT_FAR,
            _fixed_offset : -margin * 2
        });
        this._OKButton.SetLeftAnchor({
            _type         : ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset : 100
        });

        this._CancelButton.SetTopAnchor({
                _type        : ANCHOR_TYPE.SIBLING_NEAR,
                _sibling     : this._OKButton
        });
        this._CancelButton.SetBottomAnchor({
            _type         : ANCHOR_TYPE.SIBLING_FAR,
            _sibling      : this._OKButton
        });
        this._CancelButton.SetLeftAnchor({
                _type         : ANCHOR_TYPE.SIBLING_FAR,
                _sibling      : this._OKButton,
                _fixed_offset : 50
        });
        this._CancelButton.SetRightAnchor({
                _type         : ANCHOR_TYPE.FIXED_SIZE,
                _fixed_size   : 100
        });

        this._colourPreview.SetTopAnchor({
            _type          : ANCHOR_TYPE.SIBLING_NEAR,
            _sibling       : this.h_entry_box
        });
        this._colourPreview.SetBottomAnchor({
            _type          : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size    : 100
        });
        this._colourPreview.SetLeftAnchor({
                _type         : ANCHOR_TYPE.SIBLING_FAR,
                _sibling      : this.h_entry_box,
                _fixed_offset : margin
        });
        this._colourPreview.SetRightAnchor({
            _type          : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size    : 100
        });
    }}
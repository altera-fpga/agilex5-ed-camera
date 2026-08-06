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
import { OJGraphicsWindow } from "./OJL.js";
import { ANCHOR_TYPE } from "./OJL.js";
import { OJLib } from "./OJL.js";
import { RgbColor } from "./OJL.js";

export class OJColourContainer extends OJGrid
{
    constructor(parent, start_colour) 
    {
        super();
        this._class_name = "OJColourContainer";
        this.SetElementName("OJColourContainer");
        this._parent = parent;

        let _hue_bar = document.createElement('canvas');
        _hue_bar.id = "HueSelectorBar";
        _hue_bar.width = 400;
        _hue_bar.height = 50;
        this._hue_selector = new OJGraphicsWindow(this, _hue_bar, {_border: true});

        let _sat_lum_box = document.createElement('canvas');
        _sat_lum_box.id = "SaturationLuminationController";
        _sat_lum_box.width = 400;
        _sat_lum_box.height = 400;
        this._colour_selector = new OJGraphicsWindow(this, _sat_lum_box, {_border: true});

        let rgb = new RgbColor(start_colour.R, start_colour.G, start_colour.B);
        let hsl = rgb.ToHsl();
        this._cursorHueBox = {x:hsl._h * this._hue_selector._width, y:this._hue_selector._height/2, width:10, height:10}
        this._cursorSatLumBox = {x:hsl._s * this._colour_selector._width, y:hsl._l * this._colour_selector._height, width:10, height: 10}

        this.DrawSatLumBox();
        this.DrawHueBar();

        let self = this;
        this._mouseDownHue = false;
        this._mouseDownSatLum = false;

        var canvas = this._hue_selector.GetCanvas();
        canvas.onpointerdown = function(event) 
        {
            self._mouseDownHue = true;
            document.getElementById("HueSelectorBar").setPointerCapture(event.pointerId);
            self.OnMouseDownHue(event);
        }

        canvas.onpointerup = function(event) 
        {
            self._mouseDownHue = false;
            document.getElementById("HueSelectorBar").releasePointerCapture(event.pointerId);
        }

        canvas.onpointermove = function(event) 
        {
            if (self._mouseDownHue) 
            {
                let pos = self.getMousePos(event);
                self._cursorHueBox.x = pos.x;
                let col = self.getSelectedColour();
                self.ReDraw(col);
            }
        }

        canvas = this._colour_selector.GetCanvas();
        
        canvas.onpointerdown = function(event) 
        {
            self._mouseDownSatLum = true;
            document.getElementById("SaturationLuminationController").setPointerCapture(event.pointerId);
            self.OnMouseDownColour(event);
        }

        canvas.onpointerup = function(event) 
        {
            self._mouseDownSatLum = false;
            document.getElementById("SaturationLuminationController").releasePointerCapture(event.pointerId);
        }

        canvas.onpointermove = function(event) 
        {
            if (self._mouseDownSatLum) 
            {
                let pos = self.getMousePos(event);
                self._cursorSatLumBox.x = pos.x;
                self._cursorSatLumBox.y = pos.y;
                let col = self.getSelectedColour();
                self.ReDraw(col);
            }
        }
        
        this.AddChild(this._colour_selector);
        this.AddChild(this._hue_selector);
        this.SetUpAnchors();
    }

    DrawHueBar() 
    {
        var s = 1;
        var l = 0.5;
        var _dc = this._hue_selector.GetDrawContext()
        _dc.clearRect(0,0, this._hue_selector._width, this._hue_selector._height);
        var imageData = _dc.createImageData(this._hue_selector._width, this._hue_selector._height);

        for (var x = 0; x < this._hue_selector._width; x++) 
        {
            var hue = (x/this._hue_selector._width) * 360;
            var rgb = OJLib.HSL_to_RGB((hue/360), s, l);
            for (var y = 0; y < this._hue_selector._height; y++) 
            {
                imageData.data[((y*this._hue_selector._width)+x)*4] = rgb.R;
                imageData.data[((y*this._hue_selector._width)+x)*4+1] = rgb.G;
                imageData.data[((y*this._hue_selector._width)+x)*4+2] = rgb.B;
                imageData.data[((y*this._hue_selector._width)+x)*4+3] = 255;
            }
        }
        this._hue_selector.GetGraphics().PutImageData(imageData, 0, 0);

        _dc.beginPath();
        _dc.rect(this._cursorHueBox.x-(this._cursorHueBox.width/2), this._cursorHueBox.y-(this._cursorHueBox.height/2),
                                        this._cursorHueBox.width, this._cursorHueBox.height);
        _dc.stroke();
    }

    DrawSatLumBox() 
    {
        var h = (this._cursorHueBox.x / this._hue_selector._width);
        var _dc = this._colour_selector.GetDrawContext()
        _dc.clearRect(0,0, this._colour_selector._width, this._colour_selector._height);
        var imageData = this._colour_selector.GetDrawContext().createImageData(this._colour_selector._width, this._colour_selector._height);
        for (var s = 0; s < this._colour_selector._width; s ++) 
        {
            var sVal = s/this._colour_selector._width;
            for (var l = 0; l < this._colour_selector._height; l++) 
            {
                var lVal = l/this._colour_selector._height;
                var rgb = OJLib.HSL_to_RGB(h, sVal, lVal);
                imageData.data[((l*this._colour_selector._width)+s)*4] = rgb.R;
                imageData.data[((l*this._colour_selector._width)+s)*4+1] = rgb.G;
                imageData.data[((l*this._colour_selector._width)+s)*4+2] = rgb.B;
                imageData.data[((l*this._colour_selector._width)+s)*4+3] = 255;
            }
        }
        this._colour_selector.GetGraphics().PutImageData(imageData, 0,0);
        _dc.beginPath();
        _dc.rect(this._cursorSatLumBox.x-(this._cursorSatLumBox.width/2), this._cursorSatLumBox.y-(this._cursorSatLumBox.height/2), 
                                this._cursorSatLumBox.width, this._cursorSatLumBox.height);
        _dc.stroke();
    }

    getMousePos(event) 
    {
        let box = event.target.getBoundingClientRect();
        let cPos = 
        {
            x: (event.clientX - box.left) * (event.target.width / box.width),
            y: (event.clientY - box.top) * (event.target.height / box.height)
        }
        cPos.x = (cPos.x > 0) ? cPos.x : 0;
        cPos.y = (cPos.y > 0) ? cPos.y : 0;
        cPos.x = (cPos.x > event.target.width) ? event.target.width : cPos.x;
        cPos.y = (cPos.y > event.target.height) ? event.target.height : cPos.y;
        return cPos;
    }

    OnMouseDownColour(event) 
    {
        let clickPos = this.getMousePos(event);
        this._cursorSatLumBox.x = clickPos.x;
        this._cursorSatLumBox.y = clickPos.y;

        let colour = this.getSelectedColour();

        this.ReDraw(colour);
    }

    OnMouseDownHue(event) 
    {
        let clickPos = this.getMousePos(event);
        this._cursorHueBox.x = clickPos.x;
        
        let colour = this.getSelectedColour();

        this.ReDraw(colour);
    }

    getSelectedColour() 
    {
        let colour = {h:0, s:0, l:0}
        colour.h = this._cursorHueBox.x / this._hue_selector._width;
        colour.s = this._cursorSatLumBox.x / this._colour_selector._width;
        colour.l = this._cursorSatLumBox.y / this._colour_selector._height;
        return colour;
    }

    RGBChange(rgbValues) 
    {
        let rgb = new RgbColor(rgbValues.R, rgbValues.G, rgbValues.B);
        let hsl = rgb.ToHsl();

        this._cursorHueBox.x = hsl._h * this._hue_selector._width;
        this._cursorSatLumBox.x = hsl._s * this._colour_selector._width;
        this._cursorSatLumBox.y = hsl._l * this._colour_selector._height;

        this.DrawHueBar();
        this.DrawSatLumBox();
    }

    HSLChange(hslValues) 
    {
        this._cursorHueBox.x = (hslValues.h /360) * this._hue_selector._width;
        this._cursorSatLumBox.x = (hslValues.s / 100) * this._colour_selector._width;
        this._cursorSatLumBox.y = (hslValues.l / 100) * this._colour_selector._height;

        this.ReDraw({h: (hslValues.h /360), s:(hslValues.s / 100), l:(hslValues.l / 100)});
    }

    ReDraw(colour) 
    {
        this.DrawHueBar();
        this.DrawSatLumBox();
        this._parent.UpdateColourChoice(colour);
    }

    SetUpAnchors() 
    {
        let margin = 15;
        let spacing = 10;
        
        this._colour_selector.SetTopAnchor({
            _type           : ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset   : spacing
        });
        this._colour_selector.SetBottomAnchor({
            _type           : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size     : 400
        });
        this._colour_selector.SetLeftAnchor({
            _type           : ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset   : margin
        });
        this._colour_selector.SetRightAnchor({
            _type           : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size     : 400 
        });

        this._hue_selector.SetTopAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._colour_selector,
            _fixed_offset   : spacing
        });
        this._hue_selector.SetBottomAnchor({
            _type           : ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size     : 50
        });
        this._hue_selector.SetLeftAnchor({
            _type           : ANCHOR_TYPE.SIBLING_NEAR,
            _sibling        : this._colour_selector
        });
        this._hue_selector.SetRightAnchor({
            _type           : ANCHOR_TYPE.SIBLING_FAR,
            _sibling        : this._colour_selector
        })
    }

}
/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJModalDialog } from "./OJL.js";
import { OJTextControl, TEXT_CONTROL_TYPE } from "./OJL.js";
import { OJTextButton } from "./OJL.js";
import { OJSliderControl } from "./OJL.js";
import { OJGrid } from "./OJL.js";
import { OJPictureInPictureLayerContainer } from "./OJL.js";
import { OJDropDownList } from "./OJL.js";
import { ANCHOR_TYPE } from "./OJL.js";
import { OJLib, UI, ObjectCallback } from "./OJL.js";

export class OJPictureInPictureDialog extends OJModalDialog
{
    constructor(params)
    {
        let max_width = 1440;
        let max_height = 810;

        // Base class constructor
        super(max_width + 320, max_height + 78, "Video Resizer");
        this._class_name = "OJPictureInPictureDialog";
        this.SetElementName("OJPictureInPictureDialog");

        this._capture_width = 0;
        this._capture_height = 0;
        this._output_width = params.output_width;
        this._output_height = params.output_height;

        this._pip_container = new OJPictureInPictureLayerContainer(this, max_width, max_height, 
                                                                   this._output_width, this._output_height);

        this._layers_dropdown_label = new OJLabel("Layer: ", {
            _alignment		: "left",
            _single_line	: true
        });

        this._canvas_x_label = new OJLabel("X: ", {
            _alignment		: "left",
            _single_line	: true
        });

        this._canvas_y_label = new OJLabel("Y: ", {
            _alignment		: "left",
            _single_line	: true
        });

        this._canvas_z_label = new OJLabel("Z: ", {
            _alignment		: "left",
            _single_line	: true
        });

        this._canvas_width_label = new OJLabel("Width: ", { 
            _alignment 		: "left", 
            _single_line	: true
        });

        this._canvas_height_label = new OJLabel("Height: ", {
            _alignment		: "left",
            _single_line	: true
        });

        this._canvas_alpha_label = new OJLabel("Alpha: ", {
            _alignment		: "left",
            _single_line	: true
        });

        let layers_dropdown_callback = new ObjectCallback(this, "DropdownSetCanvas");
        this._layers_dropdown = new OJDropDownList(0, layers_dropdown_callback, null, null, {
            _auto_select	: true
        });
        let x_callback = new ObjectCallback(this, "SetCanvasX");
        this._canvas_x = new OJTextControl(TEXT_CONTROL_TYPE.TCT_INTEGER, 0, x_callback, 0, this._output_width);

        let y_callback = new ObjectCallback(this, "SetCanvasY");
        this._canvas_y = new OJTextControl(TEXT_CONTROL_TYPE.TCT_INTEGER, 0, y_callback, 0, this._output_height);

        let z_callback = new ObjectCallback(this._pip_container, "SetCurrentZIndex");
        this._canvas_z = new OJTextControl(TEXT_CONTROL_TYPE.TCT_INTEGER, 0, z_callback, 0, 255);

        let width_callback = new ObjectCallback(this, "SetCanvasWidth");
        this._canvas_width = new OJTextControl(TEXT_CONTROL_TYPE.TCT_INTEGER, this._capture_width, width_callback, 1, this._capture_width);

        let height_callback = new ObjectCallback(this, "SetCanvasHeight");
        this._canvas_height = new OJTextControl(TEXT_CONTROL_TYPE.TCT_INTEGER, this._capture_height, height_callback, 1, this._capture_height);

        let alpha_callback = new ObjectCallback(this._pip_container, "SetAlpha");
        this._canvas_alpha = new OJSliderControl(this, {
            _min				: 0,
            _max				: 255,
            _update_callback	: alpha_callback
        });

        this.AddChild(this._pip_container);

        this._control_container = new OJGrid(null, "Layer Settings", { _corner_radius: 8 });
        this._control_container.SetBackgroundColour(UI._control_panel_background_colour);
        this.AddChild(this._control_container);

        this._canvas_x_label.SetBackgroundColour(UI._control_panel_background_colour);
        this._canvas_y_label.SetBackgroundColour(UI._control_panel_background_colour);
        this._canvas_z_label.SetBackgroundColour(UI._control_panel_background_colour);
        this._canvas_width_label.SetBackgroundColour(UI._control_panel_background_colour);
        this._canvas_height_label.SetBackgroundColour(UI._control_panel_background_colour);

        this._control_container.AddChild(this._layers_dropdown_label);
        this._control_container.AddChild(this._canvas_x_label);
        this._control_container.AddChild(this._canvas_y_label);
        this._control_container.AddChild(this._canvas_z_label);
        this._control_container.AddChild(this._canvas_width_label);
        this._control_container.AddChild(this._canvas_height_label);
        this._control_container.AddChild(this._canvas_alpha_label);

        this._control_container.AddChild(this._layers_dropdown);
        this._control_container.AddChild(this._canvas_x);
        this._control_container.AddChild(this._canvas_y);
        this._control_container.AddChild(this._canvas_z);
        this._control_container.AddChild(this._canvas_width);
        this._control_container.AddChild(this._canvas_height);
        this._control_container.AddChild(this._canvas_alpha);

        this._zoom_slider_label = new OJLabel("Zoom: ",  {
            _alignment		: "left",
            _single_line	: true
        });

        let zoom_callback = new ObjectCallback(this, "SetZoom");
        this._zoom_slider = new OJSliderControl(this, {
            _floating_point		: true,
            _min				: 0.25,
            _max				: 1,
            _update_callback	: zoom_callback
        });

        this._global_settings_container = new OJGrid(null, "Global Settings", { _corner_radius: 8 });
        this._global_settings_container.SetBackgroundColour(UI._control_panel_background_colour);
        this.AddChild(this._global_settings_container);

        this._global_settings_container.AddChild(this._zoom_slider_label);
        this._global_settings_container.AddChild(this._zoom_slider);

        this._pip_container.LinkLabels(this._layers_dropdown, this._canvas_x, this._canvas_y, this._canvas_z, this._canvas_width, this._canvas_height, this._canvas_alpha, this._zoom_slider);

        this._cached_dimensions = [];
        for (let layer of params.Layers)
        {
            this._pip_container.AddLayer(layer);
            this._cached_dimensions.push(layer);
        }

        let ok_callback = new ObjectCallback(this, "OnOK");
        this._ok_button = new OJTextButton(this.GetElement(), "OK", { 
            _click_callback	: ok_callback,
            _alignment		: "centre"
        });

        let cancel_callback = new ObjectCallback(this, "CancelChanges");
        this._cancel_button = new OJTextButton(this.GetElement(), "Cancel", {
            _click_callback	: cancel_callback,
            _alignment		: "centre"
        });

        this.AddChild(this._ok_button);
        this.AddChild(this._cancel_button);

        let div = this.GetElement();
        let pip_container = this._pip_container;

        div.onwheel = function(event)
        {
            if (pip_container)
                pip_container.OnWheel(event);
        };
        div.onkeydown = function(event)
        {
            if (pip_container)
                pip_container.OnKeyDown(event);
        };
        div.onkeyup = function(event)
        {
            if (pip_container)
                pip_container.OnKeyUp(event);
        };

        this.SetupAnchors();
    }

    Close(event)
    {
    }

    Resize(x, y, width, height)
    {
        var size_changed = super.Resize(x, y, width, height);

        this.SetCanvasSize();

        return size_changed;
    }

    SetCanvasSize(x, y, width, height)
    {
        if (typeof x === "number" || typeof y === "number" || typeof width === "number" || typeof height === "number")
            this._pip_container.CurrentCanvasSetDimensions(x, y, width, height);
    }

    SetCanvasX(x)
    {
        this.SetCanvasSize(parseInt(x), null, null, null);
    }

    SetCanvasY(y)
    {
        this.SetCanvasSize(null, parseInt(y), null, null);
    }

    SetCanvasWidth(width)
    {
        this.SetCanvasSize(null, null, parseInt(width), null);
    }

    SetCanvasHeight(height)
    {
        this.SetCanvasSize(null, null, null, parseInt(height));
    }

    SetZoom(zoom)
    {
        if (this._pip_container) {
            this._pip_container._scale = parseFloat(zoom);
            this._pip_container.ApplyScale();
        }
    }

    DropdownSetCanvas(index, entry)
    {
        if (this._pip_container) 
        {
            let layer = this._pip_container.GetCanvasByName(entry.GetText());
            if (layer !== null)
                this._pip_container.SetCurrentCanvas(layer);
        }
    }

    OnCanvasUpdate()
    {
        if (this._pip_container) 
        {
            let attr = this._pip_container.GetCurrentCanvasAttributes();
            let params = `${attr.unique_id},${attr.x},${attr.y},${attr.z},${attr.width},${attr.height},${attr.alpha}`;
            OJLib.ServerExecuteWithParams("PictureInPictureLayerUpdate", params);
        }
    }

    CancelChanges()
    {
        let z = 0;
        for (let layer of this._cached_dimensions) 
        {
            let params = `${layer._index},${layer._x},${layer._y},${z++},${layer._width},${layer._height},${layer._alpha}`;
            OJLib.ServerExecuteWithParams("PictureInPictureLayerUpdate", params);
        }
        this.OnOK();
    }

    SetupAnchors() 
    {
        let label_height = 30;
        let input_width = 60;
        let margin = 20;
        let spacing = 15;

        this._pip_container.SetLeftAnchor({
            _type			: ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset	: margin
        });
        this._pip_container.SetRightAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size		: 1440
        });
        this._pip_container.SetTopAnchor({
            _type			: ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset	: margin
        });
        this._pip_container.SetBottomAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size		: 810
        });

        this._control_container.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._pip_container,
            _fixed_offset	: 20
        });
        this._control_container.SetRightAnchor({
            _type			: ANCHOR_TYPE.PARENT_FAR,
            _fixed_offset	: -20
        });
        this._control_container.SetTopAnchor({
            _type			: ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset	: 20
        });
        this._control_container.SetBottomAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size		: 390
        });


        this._layers_dropdown_label.SetLeftAnchor({
            _type			: ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset	: margin
        });
        this._layers_dropdown_label.SetRightAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_offset	: 120
        });
        this._layers_dropdown_label.SetTopAnchor({
            _type			: ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset	: spacing
        });
        this._layers_dropdown_label.SetBottomAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size		: label_height
        });

        this._canvas_x_label.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._layers_dropdown_label
        });
        this._canvas_x_label.SetRightAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._layers_dropdown_label
        });
        this._canvas_x_label.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._layers_dropdown_label,
            _fixed_offset	: spacing
        });
        this._canvas_x_label.SetBottomAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size		: label_height
        });

        this._canvas_y_label.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._canvas_x_label
        });
        this._canvas_y_label.SetRightAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_x_label
        });
        this._canvas_y_label.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_x_label,
            _fixed_offset	: spacing
        });
        this._canvas_y_label.SetBottomAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size		: label_height
        });

        this._canvas_z_label.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._canvas_x_label
        });
        this._canvas_z_label.SetRightAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_y_label
        });
        this._canvas_z_label.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_y_label,
            _fixed_offset	: spacing
        });
        this._canvas_z_label.SetBottomAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size		: label_height
        });

        this._canvas_width_label.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._canvas_x_label
        });
        this._canvas_width_label.SetRightAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_z_label
        });
        this._canvas_width_label.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_z_label,
            _fixed_offset	: 40
        });
        this._canvas_width_label.SetBottomAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size		: label_height
        });

        this._canvas_height_label.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._canvas_x_label
        });
        this._canvas_height_label.SetRightAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_width_label,
        });
        this._canvas_height_label.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_width_label,
            _fixed_offset	: spacing
        });
        this._canvas_height_label.SetBottomAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size		: label_height
        });

        this._canvas_alpha_label.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._canvas_x_label
        });

        this._canvas_alpha_label.SetRightAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_height_label
        });

        this._canvas_alpha_label.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_height_label,
            _fixed_offset	: spacing
        });

        this._canvas_alpha_label.SetBottomAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size		: label_height
        });


        this._layers_dropdown.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._layers_dropdown_label,
            _fixed_offset	: input_width
        });
        this._layers_dropdown.SetRightAnchor({
            _type			: ANCHOR_TYPE.PARENT_FAR,
            _fixed_offset	: -margin
        });
        this._layers_dropdown.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._layers_dropdown_label
        });
        this._layers_dropdown.SetBottomAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._layers_dropdown_label
        });

        this._canvas_x.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_x_label,
            _fixed_offset	: input_width
        });
        this._canvas_x.SetRightAnchor({
            _type			: ANCHOR_TYPE.PARENT_FAR,
            _fixed_offset	: -margin
        });
        this._canvas_x.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._canvas_x_label
        });
        this._canvas_x.SetBottomAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_x_label
        });

        this._canvas_y.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_y_label,
            _fixed_offset	: input_width
        });
        this._canvas_y.SetRightAnchor({
            _type			: ANCHOR_TYPE.PARENT_FAR,
            _fixed_offset	: -margin
        });
        this._canvas_y.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._canvas_y_label
        });
        this._canvas_y.SetBottomAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_y_label
        });

        this._canvas_z.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_z_label,
            _fixed_offset	: input_width
        });
        this._canvas_z.SetRightAnchor({
            _type			: ANCHOR_TYPE.PARENT_FAR,
            _fixed_offset	: -margin
        });
        this._canvas_z.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._canvas_z_label
        });
        this._canvas_z.SetBottomAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_z_label
        });

        this._canvas_width.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_width_label,
            _fixed_offset	: input_width
        });
        this._canvas_width.SetRightAnchor({
            _type			: ANCHOR_TYPE.PARENT_FAR,
            _fixed_offset	: -margin
        });
        this._canvas_width.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._canvas_width_label
        });
        this._canvas_width.SetBottomAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_width_label,
        });

        this._canvas_height.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._canvas_width
        });
        this._canvas_height.SetRightAnchor({
            _type			: ANCHOR_TYPE.PARENT_FAR,
            _fixed_offset	: -margin
        });
        this._canvas_height.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._canvas_height_label,
        });
        this._canvas_height.SetBottomAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._canvas_height_label
        });

        this._canvas_alpha.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._canvas_height,
        });
        this._canvas_alpha.SetRightAnchor({
            _type			: ANCHOR_TYPE.PARENT_FAR,
            _fixed_offset	: -margin
        });
        this._canvas_alpha.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._canvas_alpha_label,
        });
        this._canvas_alpha.SetBottomAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size		: label_height
        });


        this._global_settings_container.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._control_container
        });
        this._global_settings_container.SetRightAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._control_container
        });
        this._global_settings_container.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._control_container,
            _fixed_offset	: margin
        });
        this._global_settings_container.SetBottomAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size		: 90
        });
        
        this._zoom_slider_label.SetLeftAnchor({
            _type			: ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset	: margin
        });
        this._zoom_slider_label.SetRightAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size		: 120
        });
        this._zoom_slider_label.SetTopAnchor({
            _type			: ANCHOR_TYPE.PARENT_NEAR,
            _fixed_offset	: spacing
        });
        this._zoom_slider_label.SetBottomAnchor({
            _type			: ANCHOR_TYPE.FIXED_SIZE,
            _fixed_size		: label_height
        });
        
        this._zoom_slider.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._zoom_slider_label,
            _fixed_offset	: -input_width
        });
        this._zoom_slider.SetRightAnchor({
            _type			: ANCHOR_TYPE.PARENT_FAR,
            _fixed_offset	: -margin
        });
        this._zoom_slider.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._zoom_slider_label,
        });
        this._zoom_slider.SetBottomAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._zoom_slider_label
        });
        
        let button_width = input_width + 30;
        let button_height = label_height + 20;
        let button_spacing = 3 - spacing;
        this._cancel_button.SetLeftAnchor({
            _type			: ANCHOR_TYPE.PARENT_FAR,
            _fixed_offset	: button_spacing - button_width
        });
        this._cancel_button.SetRightAnchor({
            _type			: ANCHOR_TYPE.PARENT_FAR,
            _fixed_offset	: button_spacing
        });
        this._cancel_button.SetTopAnchor({
            _type			: ANCHOR_TYPE.PARENT_FAR,
            _fixed_offset	: button_spacing - button_height
        });
        this._cancel_button.SetBottomAnchor({
            _type			: ANCHOR_TYPE.PARENT_FAR,
            _fixed_offset	: button_spacing
        });
        this._ok_button.SetLeftAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._cancel_button,
            _fixed_offset	: button_spacing - button_width
        });
        this._ok_button.SetRightAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._cancel_button,
            _fixed_offset	: button_spacing
        });
        this._ok_button.SetTopAnchor({
            _type			: ANCHOR_TYPE.SIBLING_NEAR,
            _sibling		: this._cancel_button
        });
        this._ok_button.SetBottomAnchor({
            _type			: ANCHOR_TYPE.SIBLING_FAR,
            _sibling		: this._cancel_button
        });
    }
}
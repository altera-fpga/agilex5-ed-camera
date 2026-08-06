/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJServerLink } from "./OJL.js"
import { OJWindowElement, ANCHOR_TYPE } from "./OJL.js"
import { OJModalDialog } from "./OJL.js"
import { OJWarpWireframe } from "./OJL.js"
import { OJGrid } from "./OJL.js"
import { OJControlContainer, OJControlItemBase } from "./OJL.js"
import { OJLib } from "./OJL.js"
import { wsvg } from "./OJL.js"
import { OJControlItemHiddenValue } from "./OJL.js"

let WarpMode = 
{
    FIXED:		0,
    CORNERS:	1,
    ARBITRARY:	2,
    FISHEYE:    3
};

let FisheyeMapping = 
{
    PANORAMA:   0,
    EQUIRECT:   1,
    UNITY:      2
};

export class OJWarpFullControlDialog extends OJModalDialog
{
    constructor(params)
    {
        let width = 1708;
        let height = 910;
        
        // Base class constructor
        super(width, height, "Warp");
        this._warp_control_panels = OJServerLink.Get().LoadControls(params.Controls, this);
    }

    Destroy()
    {
        // Remove the control panels
        OJServerLink.Get().RemoveControls(this._warp_control_panels);

        super.Destroy();
        this._warp_full_control = null;

        // The custom 'controls' are not necessarily children windows of this
        // dialog so might not have been destroyed by OJModalDialog.prototype.Destroy
        let custom_controls = this._warp_control_panels._custom_controls;
        for (let custom_control of custom_controls)
            custom_control.Destroy();

        this._warp_control_panels = {};
    }
}

export class OJWarpFullControl extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json);
        this._ready = false;

        // Calculated when resized
        this._wireframe_height = 1;

        // Warp output resolution
        let output_width = ui_element_json.output_width;
        let output_height = ui_element_json.output_height;
        this._oh_range = output_width - 1;
        this._ov_range = output_height - 1;
        this._mode = ui_element_json.mode;
        this._num_arbitrary_knots = 3;
        this._bypass_warp = false;
        this._arbitrary_mesh_warning = OJLib.IsTrue(ui_element_json.mesh_warning);

        // Wireframe widget
        this._wireframe = new OJWarpWireframe();
        this.UpdateMeshWarning();
        this.AddChild(this._wireframe);

        this._points = new OJWarpControlPoints();
        this._points.SetCallback((idx, pos) => { this.ControlPointsCB(idx, pos); });
        this.AddChild(this._points);

        this._show_points = ((this._mode == WarpMode.CORNERS) || (this._mode == WarpMode.ARBITRARY));
        if (this._points && !this._bypass_warp)
            this._points.Show(this._show_points);

        // Overlay to disable controls if bypass is turned on
        this._bypass_overlay = new OJGrid();
        this._bypass_overlay.Show(false);
        this.AddChild(this._bypass_overlay);

        // Forward control point updates based on the current mode(corners/arbitrary)
        this._points_forward_cb = null;

        this._mode_callbacks = [];

        this._ready = true;

        for (let callback of this._mode_callbacks)
            callback(this._mode);

        this.SetMode(this._mode);
    }

    Destroy()
    {
        super.Destroy();
        this._destroyed = true;
        this._ready = false;
        this._points = null;
        this._mode_callbacks.length = 0;
    }

    GetHeight()
    {
        return this._wireframe_height;
    }

    GetControlHeight(parent_width, parent_height)
    {
        // Called before Resize, which depends on our return value
        if (parent_width != null)
        {
            let parent_client_height = parent_height - this._header_height;
            this._wireframe_height = parent_client_height - (56 + (3 * 12));
        }

        return this._wireframe_height;
    }

    Resize(x, y, width, height)
    {
        // Calculate the wireframe width based on the height
        let handle_radius = 8;
        let inner_height = height - (2 * handle_radius);
        let inner_width = ((inner_height * 16) / 9) | 0;
        let wireframe_width = inner_width + (2 * handle_radius);
        let offset = ((width - wireframe_width) / 2) | 0;
        this._points.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: offset });
        this._points.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: wireframe_width });
        this._wireframe.SetLeftAnchor({ _type: ANCHOR_TYPE.PARENT_NEAR, _fixed_offset: offset });
        this._wireframe.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: wireframe_width });

        let size_changed = super.Resize(x, y, width, height);

        return size_changed;
    }

    RegisterModeCallback(cb)
    {
        this._mode_callbacks.push(cb);
    }

    GetPointsControl()
    {
        return this._points;
    }

    ControlPointsCB(idx, pos)
    {
        if (this._points_forward_cb)
        {
            let x = pos[0].toFixed(6);
            let y = pos[1].toFixed(6);	
            this._points_forward_cb(idx, [x,y]);	
        }
    }

    SetControlPointsForwardCB(callback)
    {
        this._points_forward_cb = callback;
    }

    ShowAlignmentGuideCB(state)
    {
        this._wireframe.ShowGuide(state);
    }

    MeshScaleCB(view_scale)
    {
        this._wireframe.SetViewScale(view_scale);
        this._points.SetViewScale(view_scale);
    }

    RotationCB(v)
    {
        if (this._mode == WarpMode.FIXED)
            this._wireframe.SetRotation(Number(v));
    }

    ZoomCB(v)
    {
        if (this._mode == WarpMode.FIXED)
            this._wireframe.SetZoom(Number(v));
    }

    HorizontalOffsetCB(v)
    {
        if (this._mode == WarpMode.FIXED)
            this._wireframe.SetHorizonalOffset(Number(v) / this._oh_range);
    }

    VerticalOffsetCB(v)
    {
        if (this._mode == WarpMode.FIXED)
            this._wireframe.SetVerticalOffset(Number(v) / this._ov_range);
    }

    HorizontalKeystoneCB(v)
    {
        if (this._mode == WarpMode.FIXED)
            this._wireframe.SetHorizontalKeystone(Number(v));
    }

    VerticalKeystoneCB(v)
    {
        if (this._mode == WarpMode.FIXED)
            this._wireframe.SetVerticalKeystone(Number(v));
    }
    HorizontalMirrorCB(v)
    {
        if (this._mode == WarpMode.FIXED)
            this._wireframe.SetHorizontalMirror(v);
    }

    VerticalMirrorCB(v)
    {
        if (this._mode == WarpMode.FIXED)
            this._wireframe.SetVerticalMirror(v);
    }

    FixedRadialCB(v)
    {
        if (this._mode == WarpMode.FIXED)
            this._wireframe.SetRadial(Number(v), false);
    }

    CornerRadialCB(v)
    {
        if (this._mode == WarpMode.CORNERS)
            this._wireframe.SetRadial(Number(v), true);
    }

    CornerCB(v)
    {
        if (this._mode == WarpMode.CORNERS)
        {
            this._wireframe.SetCornerPoint(v._idx, v._pos);
            this._points.SetPoint(v._idx, v._pos);
        }
    }

    ArbitraryKnotsNumCB(param)
    {
        this._num_arbitrary_knots = param._value;
    }

    ArbitraryKnotsPosCB(v)
    {
        if (this._mode == WarpMode.ARBITRARY)
        {
            this._wireframe.SetArbitraryPoint(v._idx, v._pos);
            this._points.SetPoint(v._idx, v._pos);
        }
    }
    
    BypassWarpCB(value)
    {
        this._bypass_warp = value;

        var show_warp_mesh = (this._bypass_warp == false);

        this._wireframe.ShowTransformMesh(show_warp_mesh);
        if (this._show_points)
            this._points.Show(show_warp_mesh);
        this._bypass_overlay.Show(!show_warp_mesh);
    }

    UpdatePoints(num_rows, num_columns, values)
    {
        let indexed_values = [];

        for (let i = 0; i < values.length; i++)
            indexed_values.push({ _idx: i, _pos: values[i] });

        if ((num_rows > 1) && (num_columns > 1))
        {
            this._wireframe.SetArbitraryKnotsNum(num_columns);

            if (this._mode == WarpMode.CORNERS)
                this._wireframe.GeneratePerspectiveGrid();
            else if (this._mode == WarpMode.ARBITRARY)
                this._wireframe.GenerateArbitraryGrid();

            this._points.ResetPoints(num_columns);
        }

        for (let item of indexed_values)
        {
            if (this._mode == WarpMode.CORNERS)
                this._wireframe.SetCornerPoint(item._idx, item._pos);
            else if (this._mode == WarpMode.ARBITRARY)
                this._wireframe.SetArbitraryPoint(item._idx, item._pos);

            this._points.SetPoint(item._idx, item._pos);
        }
    }

    /////////////////////////

    FisheyeLensFovCB(v)
    {
        if (this._mode == WarpMode.FISHEYE)
        {
            this._wireframe.SetFisheyeLensFov(Number(v));
        }        
    }

    FtpRadiusCB(v)
    {
        if (this._mode == WarpMode.FISHEYE)
        {
            this._wireframe.SetFtpRadius(Number(v));
        }   
    }

    FisheyeModeCB(v)
    {
        this._wireframe.SetFisheyeMapping((v == 0? FisheyeMapping.PANORAMA : (v == 1 ? FisheyeMapping.EQUIRECT : FisheyeMapping.UNITY)));
    }

    FtpLatitudeMinCB(v)
    {
        if (this._mode == WarpMode.FISHEYE)
        {
            this._wireframe.SetFtpLatitudeMin(Number(v));
        }        
    }

    FtpLatitudeMaxCB(v)
    {
        if (this._mode == WarpMode.FISHEYE)
        {
            this._wireframe.SetFtpLatitudeMax(Number(v));
        }        
    }

    FtpLongitudeCB(v)
    {
        if (this._mode == WarpMode.FISHEYE)
        {
            this._wireframe.SetFtpLongitude(Number(v));
        }        
    }

    FtpHorizontalFovCB(v)
    {
        if (this._mode == WarpMode.FISHEYE)
        {
            this._wireframe.SetFtpHorizontalFov(Number(v));
        }        
    }

    FtpInsideOutCB(v)
    {
        if (this._mode == WarpMode.FISHEYE)
        {
        }        
    }

    EquirectVFovCB(v)
    {
        if (this._mode == WarpMode.FISHEYE)
        {
            this._wireframe.SetEquirectVfov(Number(v));
        }        
    }

    EquirectHFovCB(v)
    {
        if (this._mode == WarpMode.FISHEYE)
        {
            this._wireframe.SetEquirectHfov(Number(v));
        }        
    }

    /////////////////////////

    ModeChangeStartCB(params)
    {
        if (!this._ready)
            return;

        this.SetMode(params._value);

        for (let callback of this._mode_callbacks)
            callback(params._value);
    }

    ModeChangeCompleteCB(params)
    {
        if (!this._ready)
            return;

        this._show_points = ((this._mode == WarpMode.CORNERS) || (this._mode == WarpMode.ARBITRARY));
        if (this._points && !this._bypass_warp)
            this._points.Show(this._show_points);
    }

    SetMode(mode)
    {
        if (mode != this._mode)
        {
            this._mode = mode;
            this._wireframe.Reset();

            if ((this._mode == WarpMode.FIXED) ||
                (this._mode == WarpMode.CORNERS))
            {
                this._wireframe.GeneratePerspectiveGrid();
            }
            else if (this._mode == WarpMode.ARBITRARY)
            {
                this._wireframe.GenerateArbitraryGrid();
            }
            else if (this._mode == WarpMode.FISHEYE)
            {
                this._wireframe.DrawFisheyeDiagram();
            }            
        }
    }

    ResolutionCB(v)
    {
        this._oh_range = v._x - 1;
        this._ov_range = v._y - 1;
    }

    UpdateMeshWarning()
    {
        this._wireframe.ShowWarning(this._arbitrary_mesh_warning);
    }

    SetMeshWarning(value)
    {
        this._arbitrary_mesh_warning = OJLib.IsTrue(value);
        this.UpdateMeshWarning();
    }

    WarpPointsValueToDisplay(value)
    {
        let display_value = 2;
        for (let i = 0; i < value; i++)
            display_value += (display_value - 1);
        return display_value;
    }
    
    WarpPointsDisplayToValue(display_value)
    {
        let display = 2;
        for (let i = 0; i < 5; i++)
        {
            if (display == display_value)
                return i;
            display += (display - 1);
            if (display > display_value)
                return i;
        }
    
        return 0;
    }

    static CreateCustomControl(element_type, params)
    {
        if (element_type == "OJWarpFullControl")
        {
            return new OJWarpFullControl(params);
        }
        else if (element_type == "UiControlItemHiddenValue")
        {
            return new OJControlItemHiddenValue(params);
        }
        else if (element_type == "UiControlItemWarpCorners")
        {
            return new OJControlItemWarpCorners(params);
        }
        else if (element_type == "UiControlItemWarpArbitrary")
        {
            return new OJControlItemWarpArbitrary(params);
        }
        // else if (element_type == "UiControlItemWarpFisheye")
        // {
        //     return new OJControlItemWarpFisheye(params);
        // }
    }
}

// Register our control factory fn for the OJWarpFullControl custom controls
OJControlContainer.AddCustomControlFactory("OJWarpFullControl", OJWarpFullControl.CreateCustomControl);
OJControlContainer.AddCustomControlFactory("UiControlItemHiddenValue", OJWarpFullControl.CreateCustomControl);
OJControlContainer.AddCustomControlFactory("UiControlItemWarpCorners", OJWarpFullControl.CreateCustomControl);
OJControlContainer.AddCustomControlFactory("UiControlItemWarpArbitrary", OJWarpFullControl.CreateCustomControl);

///////////////////////////////////////////////////////////////////////////////

function UpdateWarpPoints(value, json_value)
{
    // Update can be either a single point or the entire array 
    // A single point is updated in the existing array.
    // Entire array update happens when number of control knots
    // is changed so the array will have different size
    if (json_value.points.length > 1)
    {
        value = [];
        value.length = json_value.points.length;
    }

    for (let point of json_value.points)
    {
        let index = point.i;
        let x = point.x;
        let y = point.y;
        value[index] = [x, y];
    }

    return value;
}

export class OJWarpControlPoints extends OJWindowElement
{
    constructor()
    {
        // Base class constructor
        super();

        this._handle_radius = 8;
        
        // Set by Resize
        let width = 320;
        let height = 180;
        this._svg_width = width;
        this._img_width = this._svg_width - 2 * this._handle_radius;

        let square_size = this._img_width / 16;

        this._img_height = square_size * 9;
        this._svg_height = this._img_height + 2 * this._handle_radius;
        
        this._svg = wsvg.create("svg");
        this._svg.setAttribute("width", this._svg_width);
        this._svg.setAttribute("height", this._svg_height);
        this._view_scale = 1;

        let view = wsvg.create("g");
        this._svg.appendChild(view);
        this._view = view;

        let img = wsvg.create("g");
        view.appendChild(img);
        this._img = img;

        this._handles = [];
        this._on_change_cb = null;
        this._defaultPoints = [];
        this._targetPoints = [];
        this._num_points = 2;

        this.ResetPoints(2);

        this.GetElement().appendChild(this._svg);
    }

    Resize(x, y, width, height)
    {
        // Call base
        let size_changed = super.Resize(x, y, width, height);

        this._svg_width = width;
        this._img_width = this._svg_width - 2 * this._handle_radius - 4;

        let square_size = this._img_width / 16;

        this._img_height = square_size * 9;
        this._svg_height = this._img_height + 2 * this._handle_radius;

        this._svg.setAttribute("width", this._svg_width);
        this._svg.setAttribute("height", this._svg_height);   

        this.ResetPoints();

        // Reapply transforms for new size
        for (let i = 0; i < this._targetPoints.length; ++i)
        {
            this._targetPoints[i][0] = this._normalised_points[i][0] * this._img_width;
            this._targetPoints[i][1] = this._normalised_points[i][1] * this._img_height;
        
            this._handles[i].setAttribute("transform", this.GetTransform(i));
        }

        return size_changed;
    }

    ResetPoints(num)
    {
        let num_points_changed = false;
        if (num != null)
        {
            num_points_changed = (num != this._num_points);
            this._num_points = num;
        }
        this._handles.forEach(h => { this._img.removeChild(h); });
        this._handles.length = 0;

        // Control knots
        let v_num = this._num_points;
        let h_num = this._num_points;
        let img_width = this._img_width;
        let img_height = this._img_height;
        let v_step = img_height / (v_num - 1);
        let h_step = img_width / (v_num - 1);
        let v_range = wsvg.range(0, img_height + 1, v_step);
        let h_range = wsvg.range(0, img_width + 1, h_step);

        this._defaultPoints = v_range.flatMap(v => h_range.map(function(h) {return [h,v];}));
        this._targetPoints = v_range.flatMap(v => h_range.map(function(h) {return [h,v];}));

        if ((this._normalised_points == null) || num_points_changed)
            this._normalised_points = v_range.flatMap(v => h_range.map(function(h) {return [h,v];}));
        
        let drag = wsvg.drag();

        let handle_owner = this;

        let make_callback = function(warp_control_points, idx)
        {
            return function(elem)
            {
                elem.setAttribute("transform", warp_control_points.GetTransform(idx));

                if (handle_owner._on_change_cb)
                    handle_owner._on_change_cb(idx, [elem._d[0]/img_width, elem._d[1]/img_height]);
            };
        };

        let get_scale = function(warp_control_points)
        {
            return function()
            {
                return warp_control_points._view_scale;
            }
        };

        for(let i = 0; i < this._targetPoints.length; ++i)
        {
            let handle = wsvg.create("circle");
            let d = this._targetPoints[i];
            handle._d = d;
            handle.setAttribute("transform", this.GetTransform(i));
            handle.setAttribute("r", this._handle_radius - 1);
            handle.setAttribute("stroke", "#00aeef");
            handle.setAttribute("fill", "#00aeef");

            this._img.appendChild(handle);

            // Add dragging
            drag.add_svg(handle, make_callback(this, i), get_scale(this));

            handle.onmouseover = function(event)
            {
                this.setAttribute("stroke", "#0078a7");
                this.setAttribute("fill", "#0078a7");            
            };
            handle.onmouseout = function(event)
            {
                this.setAttribute("stroke", "#00aeef");
                this.setAttribute("fill", "#00aeef");            
            };
            handle.onmouseup = function(e)
            {
                this.setAttribute("stroke", "#00aeef");
                this.setAttribute("fill", "#00aeef");            
            };

            this._handles.push(handle);
        }	
    }

    GetTransform(index)
    {
        let use_radius = this._handle_radius + 1;
        let radius_offset = use_radius + (1 - this._view_scale) * use_radius;
        let dx = (((1 - this._view_scale) * this._img_width) / 2) + radius_offset;
        let dy = (((1 - this._view_scale) * this._img_height) / 2) + radius_offset;
        dx += this._targetPoints[index][0] * this._view_scale;
        dy += this._targetPoints[index][1] * this._view_scale;

        // Center lines in middle of pixel
        dx -= 0.5;
        dy -= 0.5;

        let transform = "translate(" + dx + " " + dy + ")";
        return transform;
    }

    SetViewScale(view_scale)
    {
        this._view_scale = view_scale;
        for (let i = 0; i < this._targetPoints.length; ++i)
        {
            this._handles[i].setAttribute("transform", this.GetTransform(i));
        }
    }

    SetPoint(idx, pos)
    {
        this._targetPoints[idx][0] = pos[0] * this._img_width;
        this._targetPoints[idx][1] = pos[1] * this._img_height;
        this._normalised_points[idx][0] = pos[0];
        this._normalised_points[idx][1] = pos[1];
        
        this._handles[idx].setAttribute("transform", this.GetTransform(idx));
    }

    SetCallback(callback)
    {
        this._on_change_cb = callback;
    }
}

////////////////////////////////////////////////////////////////

export class OJControlItemWarpCorners extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json, { _windowless: true });

        this._active_point_idx = 0;
        this._value = [];

        let warp_control_id = ui_element_json.warp_control_id;
        this._warp_control = OJServerLink.Get().GetControlItem(warp_control_id);
        this._points = this._warp_control.GetPointsControl();

        this.UpdateValue(ui_element_json.value);

        let on_mode_change_cb = (mode) => 
        {
            if (mode == WarpMode.CORNERS)
            {
                this._points.ResetPoints(this._rows);
                this._warp_control.SetControlPointsForwardCB((idx, pos) => { this.PointControlCB(idx, pos); });		
                this._warp_control.UpdatePoints(this._rows, this._columns, this._value);
            }
        };

        this._warp_control.RegisterModeCallback(on_mode_change_cb);
    }

    Destroy()
    {
        this._warp_control = null;
        super.Destroy();
    }

    // UpdateValue called by the framework when an update is received
    // from the server
    UpdateValue(json_value)
    {
        // Update can be either a single point or the entire array 
        // A single point is updated in the existing array.
        // Entire array update happens when number of control knots
        // is changed so the array will have different size
        this._value = UpdateWarpPoints(this._value, json_value);
        this._rows = json_value.rows;
        this._columns = json_value.columns;

        if (this._warp_control._mode == WarpMode.CORNERS)
        {
            this._warp_control.UpdatePoints(this._rows, this._columns, this._value);
        }
    }

    RefreshOwner()
    {   
        this._value.forEach((v, idx) => {
            this.UpdateOwner({_idx:idx, _pos:v});
        }); 
    }

    PointControlCB(idx, pos)
    {
        this._active_point_idx = idx;
        this._value[idx] = pos;
        this.NotifyValueUpdate({_idx:idx, _pos:pos});
    }

    PositionControlCB(value)
    {
        this.NotifyValueUpdate({_idx:this._active_point_idx, _pos:value});
    }

    NotifyValueUpdate(v)
    {
        this._client_callback.Call(v);
        let server_value = v._idx + " " + v._pos[0] + " " + v._pos[1];
        OJServerLink.Get().ExecuteServerCommandWithParams(this._server_callback_name, server_value);
    }

    Enable(value)
    {
        super.Enable(value);
    }

    CallClientCallback(value)
    {
        if (this._client_callback)
        {
            this._value = value;

            for (let i = 0; i < this._value.length; i++)
            {
                let single_point_pos = { _idx: i, _pos: this._value[i] };
                this._client_callback.Call(single_point_pos);
            }
        }
    }
}

////////////////////////////////////////////////////////////////

export class OJControlItemWarpArbitrary extends OJControlItemBase
{
    constructor(ui_element_json)
    {
        // Base class constructor
        super(ui_element_json, { _windowless: true });

        this._active_point_idx = 0;
        this._value = [];

        let warp_control_id = ui_element_json.warp_control_id;
        this._warp_control = OJServerLink.Get().GetControlItem(warp_control_id);
        this._points = this._warp_control.GetPointsControl();

        this.UpdateValue(ui_element_json.value);

        let on_mode_change_cb = (mode) => 
        {
            if (mode == WarpMode.ARBITRARY)
            {
                this._points.ResetPoints(this._rows);
                this._warp_control.SetControlPointsForwardCB((idx, pos) => { this.PointControlCB(idx, pos); });
                this._warp_control.UpdatePoints(this._rows, this._columns, this._value);
            }
        };	

        this._warp_control.RegisterModeCallback(on_mode_change_cb);
    }

    Destroy()
    {
        this._warp_control = null;
        super.Destroy();
    }

    // UpdateValue called by the framework when an update is received
    // from the server
    UpdateValue(json_value)
    {
        // Update can be either a single point or the entire array 
        // A single point is updated in the existing array.
        // Entire array update happens when number of control knots
        // is changed so the array will have different size
        this._value = UpdateWarpPoints(this._value, json_value);
        this._rows = json_value.rows;
        this._columns = json_value.columns;

        if (this._warp_control._mode == WarpMode.ARBITRARY)
        {
            this._warp_control.UpdatePoints(this._rows, this._columns, this._value);
        }
    }

    RefreshOwner()
    {   
        this._value.forEach((v, idx) => {
            this.UpdateOwner({_idx:idx, _pos:v});
        }); 
    }

    PointControlCB(idx, pos)
    {
        this._active_point_idx = idx;

        this._value[idx] = pos;
        this.NotifyValueUpdate({_idx:idx, _pos:pos});
    }

    PositionControlCB(value)
    {
        let idx = this._active_point_idx;

        this._value[idx] = value;
        this.NotifyValueUpdate({_idx:idx, _pos:value});
    }

    NotifyValueUpdate(v)
    {
        this._client_callback.Call(v);
        let server_value = v._idx + " " + v._pos[0] + " " + v._pos[1];
        OJServerLink.Get().ExecuteServerCommandWithParams(this._server_callback_name, server_value);
    }

    CallClientCallback(value)
    {
        if (this._client_callback)
        {
            this._value = value;

            for (let i = 0; i < this._value.length; i++)
            {
                let single_point_pos = { _idx: i, _pos: this._value[i] };
                this._client_callback.Call(single_point_pos);
            }
        }
    }
}

////////////////////////////////////////////////////////////////

// export class OJControlItemWarpFisheye extends OJControlItemBase
// {
//     constructor(ui_element_json)
//     {
//         // Base class constructor
//         super(ui_element_json, { _windowless: true });

//         this._value = [];

//         let warp_control_id = ui_element_json.warp_control_id;
//         this._warp_control = OJServerLink.Get().GetControlItem(warp_control_id);

//         this.UpdateValue(ui_element_json.value);

//         let on_mode_change_cb = (mode) => 
//         {
//             if (mode == WarpMode.FISHEYE)
//             {
//             }
//         };	

//         this._warp_control.RegisterModeCallback(on_mode_change_cb);
//     }

//     Destroy()
//     {
//         this._warp_control = null;
//         super.Destroy();
//     }

//     // UpdateValue called by the framework when an update is received
//     // from the server
//     UpdateValue(json_value)
//     {
//         if (this._warp_control._mode == WarpMode.FISHEYE)
//         {
//         }
//     }

//     RefreshOwner()
//     { 
//     }

//     NotifyValueUpdate(v)
//     {
//     }

//     CallClientCallback(value)
//     {
//         if (this._client_callback)
//         {
//             // this._value = value;
//         }
//     }
// }

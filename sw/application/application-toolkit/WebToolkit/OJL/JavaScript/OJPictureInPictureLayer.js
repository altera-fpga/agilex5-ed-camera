/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";
import { OJWindowElement } from "./OJL.js";
import { OJServerLink } from "./OJL.js";
import { ModifyMouseCoordinates } from "./OJL.js";

let CANVAS_EDGE = 
{
    LEFT	: false,
    RIGHT	: false,
    TOP		: false,
    BOTTOM	: false
};

export class OJPictureInPictureLayerContainer extends OJWindowElement
{
    constructor(parent, width, height,
                output_width, output_height)
    {

        // Base class constructor
        super();
        this._parent = parent;
        this._class_name = "OJPictureInPictureLayerContainer";
        this.SetElementName("OJPictureInPictureLayerContainer");

        this._layers = [];

        this._current_layer = null;

        this._parent_layers_dropdown = null;
        this._parent_x_label = null;
        this._parent_y_label = null;
        this._parent_z_label = null;
        this._parent_width_label = null;
        this._parent_height_label = null;
        this._parent_alpha_label = null;
        this._output_width = output_width;
        this._output_height = output_height;
        this._canvas_to_image_ratio = this._output_width / width;
        
        this._div = this.GetElement();
        this._div.className += " pip_layer_container_class";

        //this._div.style.width = width + "px";
        //this._div.style.height = height + "px";
        this._div.tabIndex = 0;

        this._div.style.border = "1px solid #000000";
        this._div.style.outline = "none";
        this._div.style.overflow = "visible"

        this._scroll_zoom = false;
        this._scroll_z = false;

        this._scale = 1;
        this.ApplyScale();
        
        this._track_canvas_edge = true;
        this._transforming = false;
        this._resizing = false;
        this._mouse_down_x = 0;
        this._mouse_down_y = 0;
        this._mouse_down_details =
        {
            x				: 0,
            y				: 0,
            canvas_left		: 0,
            canvas_top		: 0,
            canvas_right	: 0,
            canvas_bottom	: 0
        };

        let self_obj = this;

        this._div.onmousedown = function(event)
        {
            self_obj.OnMouseDown(event);
        };

        this._div.onkeydown = function(event)
        {
            self_obj.OnKeyDown(event);
        };

        this._div.onkeyup = function(event)
        {
            self_obj.OnKeyUp(event);
        };

        this._div.onwheel = function(event)
        {
            self_obj.OnWheel(event);
        }

        this._div.addEventListener("touchstart", function(event) 
        { self_obj.TouchHandler(event); });
        this._div.addEventListener("touchmove", function(event) 
        { self_obj.TouchHandler(event); });
        this._div.addEventListener("touchend", function(event) 
        { self_obj.TouchHandler(event); });
        this._div.addEventListener("touchcancel", function(event) 
        { self_obj.TouchHandler(event); });
    };

    Destroy()
    {
        this._current_layer = null;
        for (let layer of this._layers)
            layer.Destroy();

        this._parent_layers_dropdown = null;
        this._parent_x_label = null;
        this._parent_y_label = null;
        this._parent_z_label = null;
        this._parent_width_label = null;
        this._parent_height_label = null;
        this._parent_alpha_label = null;
        super.Destroy();
    }

    CanvasToImage(canvas_pixels)
    {
        let image_pixels = ((canvas_pixels * this._canvas_to_image_ratio) + 0.5) | 0;
        return image_pixels;
    }

    ImageToCanvas(image_pixels)
    {
        let canvas_pixels = ((image_pixels / this._canvas_to_image_ratio) + 0.5) | 0;
        return canvas_pixels;
    }

    AddLayer(layer_params)
    {
        let new_layer = new OJPictureInPictureLayer(this, layer_params);
        if (new_layer) 
        {
            this._layers.push(new_layer);

            if (this._parent_layers_dropdown)
                this._parent_layers_dropdown.AddItem(new_layer.GetName());

            this._div.appendChild(new_layer.GetCanvas());
            this.SetCurrentCanvas(new_layer);
            this.UpdateZIndices();
        }
    }

    DeleteCurrentCanvas()
    {
        if (this._current_layer) 
        {
            this._current_layer.Destroy();
            this._layers.splice(this._layers.indexOf(this._current_layer), 1);

            this._current_layer = null;
            this.UpdateZIndices();
        }
    }

    ApplyScale()
    {
        if (typeof this._scale === "number" && !isNaN(this._scale)) 
        {
            this._scale = Math.round((this._scale + Number.EPSILON) * 100) / 100;
            if (this._scale < 0.25)
                this._scale = 0.25;
            if (this._scale > 1)
                this._scale = 1;
            this._div.style.transform = `scale(${this._scale}, ${this._scale})`;
            
            this.RefreshLabels();
        }
    }

    LinkLabels(layers_dropdown, x_label, y_label, z_label, width_label, height_label, alpha_label, zoom_label)
    {
        this._parent_layers_dropdown = layers_dropdown;
        this._parent_x_label = x_label;
        this._parent_y_label = y_label;
        this._parent_z_label = z_label;
        this._parent_width_label = width_label;
        this._parent_height_label = height_label;
        this._parent_alpha_label = alpha_label;
        this._parent_zoom_label = zoom_label;
    }

    RefreshLabels()
    {
        if (this._current_layer == null)
            return;

        if (this._parent_x_label)
            this._parent_x_label.SetValue(this._current_layer.GetX());

        if (this._parent_y_label)
            this._parent_y_label.SetValue(this._current_layer.GetY());

        if (this._parent_z_label)
            this._parent_z_label.SetValue(this._current_layer.GetZ());

        if (this._parent_width_label)
            this._parent_width_label.SetValue(this._current_layer.GetWidth());

        if (this._parent_height_label)
            this._parent_height_label.SetValue(this._current_layer.GetHeight());

        if (this._parent_alpha_label)
            this._parent_alpha_label.SetValue(this._current_layer.GetAlpha());

        if (this._parent_zoom_label)
            this._parent_zoom_label.SetValue(this._scale);
    }

    SetCurrentZIndex(new_z)
    {
        if (this._current_layer && new_z >= 0) 
        {
            if (new_z > this._layers.length)
                new_z = this._layers.length;
            let old_index = this._layers.indexOf(this._current_layer);
            this._layers.splice(new_z, 0, this._layers.splice(old_index, 1)[0]);
        }
        this.UpdateZIndices();
    }

    GetCanvasByName(name)
    {
        for (let layer of this._layers) 
        {
            if (layer.GetName() === name)
                return layer;
        }
        return null;
    }

    SetCurrentCanvas(layer)
    {
        if (layer && layer != this._current_layer) 
        {
            if (this._current_layer)
                this._current_layer.GetCanvas().style.outline = "";

            this._current_layer = layer;
            this._current_layer.GetCanvas().focus();
            this.RefreshLabels();

            if (this._parent_layers_dropdown) 
            {
                for (let item of this._parent_layers_dropdown._list_entries) 
                {
                    if (item.GetText() === this._current_layer.GetName()) 
                    {
                        this._parent_layers_dropdown.SetValue(item._index);
                        return;
                    }
                }
            }
        }
    }

    GetCurrentLayer()
    {
        if (this._current_layer)
            return this._current_layer;
        return null;
    }

    GetCurrentCanvasAttributes()
    {
        if (this._current_layer) 
        {
            return {
                unique_id : this._current_layer._unique_id,
                x         : this._current_layer.GetX(),
                y         : this._current_layer.GetY(),
                z         : this._current_layer.GetZ(),
                width     : this._current_layer.GetWidth(),
                height    : this._current_layer.GetHeight(),
                alpha	  : this._current_layer.GetAlpha()
            };
        }

        return {};
    }

    CurrentCanvasSetCanvasDimensions(x, y, width, height)
    {
        if (this._current_layer) 
        {
            this._current_layer.SetCanvasDimensions(x, y, width, height);
            if (this._parent)
                this._parent.OnCanvasUpdate();
        }
    }

    CurrentCanvasSetDimensions(x, y, width, height)
    {
        if (this._current_layer) 
        {
            this._current_layer.SetDimensions(x, y, width, height);
            if (this._parent)
                this._parent.OnCanvasUpdate();
        }
    }

    UpdateZIndices()
    {
        for (let z = 0; z < this._layers.length; z++)
            this._layers[z].SetZ(z);
        
        if (this._current_layer && this._parent_z_label)
            this._parent_z_label.SetValue(this._current_layer.GetZ());
        
        if (this._parent)
            this._parent.OnCanvasUpdate();
    }

    SetAlpha(alpha)
    {
        if (this._current_layer) 
        {
            this._current_layer.SetAlpha(alpha);
            if (this._parent)
                this._parent.OnCanvasUpdate();
        }
    }

    SetCurrentLayerMinMax(new_min_width, new_min_height, new_max_width, new_max_height)
    {
        if (this.GetCurrentLayer()) 
        {
            if ((typeof new_min_width === "number" && !isNaN(new_min_width)) || (typeof new_min_height === "number" && !isNaN(new_min_height)))
                this._currentlayer.SetMinWidthHeight(new_min_width, new_min_height);
            if ((typeof new_max_width === "number" && !isNaN(new_max_width)) || (typeof new_max_height === "number" && !isNaN(new_max_height)))
                this._currentlayer.SetMinWidthHeight(new_max_width, new_max_height);
        }
    }

    CanvasClick()
    {
        if (this._current_layer) 
        {
            this._track_canvas_edge = false;
            this._transforming = true;
            if (CANVAS_EDGE.LEFT || CANVAS_EDGE.RIGHT || CANVAS_EDGE.TOP || CANVAS_EDGE.BOTTOM)
                this._resizing = true;

            //let current_canvas = this._current_layer.GetCanvas();
            //this._scale_adjustment = current_canvas.getBoundingClientRect().width / current_canvas.offsetWidth;
        }
    }

    TouchHandler(event)
    {
        let touches = event.touches;
        let first = touches[0];
        let target = ((touches.length !== 0) ? first.target : this._div);
        let simulated_event = document.createEvent("MouseEvent");

        switch(event.type)
        {
            case "touchstart":
                if (touches.length > 1) 
                {
                    this._touch_zoom = true;
                    return;
                } 
                else 
                {
                    let simulated_touch_pos = document.createEvent("MouseEvent");
                    simulated_touch_pos.initMouseEvent("mousemove", true, true, window, 1, 
                                                first.screenX, first.screenY, 
                                                first.clientX, first.clientY, false, 
                                                false, false, false, 0, null);
                    first.target.dispatchEvent(simulated_touch_pos);

                    simulated_event.initMouseEvent("mousedown", true, true, window, 1, 
                                                first.screenX, first.screenY, 
                                                first.clientX, first.clientY, false, 
                                                false, false, false, 0, null);
                }
                break;
            case "touchmove":
                if (touches.length > 1) 
                {
                    simulated_event.initMouseEvent("wheel", true, true);

                    let dist = Math.hypot(first.screenX - touches[1].screenX, first.screenY - touches[1].screenY);
                    let dist_diff = this._last_pinch_dist - dist;

                    if (dist_diff < 0)
                        simulated_event.deltaY = -1;
                    else if (dist_diff > 0)
                        simulated_event.deltaY = 1;
                    else
                        simulated_event.deltaY = 0;

                    this._last_pinch_dist = dist;
                    target = this._div;
                } 
                else 
                {
                    simulated_event.initMouseEvent("mousemove", true, true, window, 1, 
                                                first.screenX, first.screenY, 
                                                first.clientX, first.clientY, false, 
                                                false, false, false, 0, null);
                }
                break;
            case "touchend":
            case "touchcancel":
                if (touches.length < 2)
                    this._touch_zoom = false;
                if (touches.length >= 1)
                    return;
                else
                    simulated_event.initMouseEvent("mouseup", true, true);
                target = window;
                break;
            default:
                return;
        }

        target.dispatchEvent(simulated_event);
        event.preventDefault();
    }

    OnMouseDown(event)
    {
        ModifyMouseCoordinates(event);

        this._mouse_down_details =
        {
            x				: event._client_x,
            y				: event._client_y,
            canvas_x		: this._current_layer.GetCanvasX(),
            canvas_y		: this._current_layer.GetCanvasY(),
            canvas_width	: this._current_layer.GetCanvasWidth(),
            canvas_height	: this._current_layer.GetCanvasHeight()
        };

        this._old_window_mouse_up = window.onmouseup;
        this._old_window_mouse_move = window.onmousemove;

        let self_obj = this;

        window.onmouseup = function(event) 
        {
            self_obj.OnMouseUp(event);
        };

        window.onmousemove = function(event) 
        {
            self_obj.OnMouseMove(event);
        };
    }

    OnMouseUp(event)
    {
        ModifyMouseCoordinates(event);

        this._track_canvas_edge = true;
        this._transforming = false;
        this._resizing = false;
        this._div.style.cursor = "default";
        
        window.onmouseup = this._old_window_mouse_up;
        window.onmousemove = this._old_window_mouse_move;
    }

    OnMouseMove(event)
    {
        ModifyMouseCoordinates(event);

        if (this._transforming && this._current_layer) 
        {
            let canvas_width = this._current_layer.GetCanvasWidth();
            let canvas_height = this._current_layer.GetCanvasHeight();
            let canvas_movement_x = (event._client_x - this._mouse_down_details.x) / this._scale;
            let canvas_movement_y = (event._client_y - this._mouse_down_details.y) / this._scale;
            
            if (this._resizing) 
            {
                let new_x = 0;
                let new_y = 0;
                let new_width = 0;
                let new_height = 0;

                if (CANVAS_EDGE.LEFT)
                    new_width += -canvas_movement_x;
                if (CANVAS_EDGE.RIGHT)
                    new_width += canvas_movement_x;
                if (CANVAS_EDGE.TOP)
                    new_height += -canvas_movement_y;
                if (CANVAS_EDGE.BOTTOM)
                    new_height += canvas_movement_y;

                if (this._current_layer._fixed_aspect_ratio) 
                {
                    let width_ratio = this._current_layer._aspect_ratio / (1 + this._current_layer._aspect_ratio);
                    let new_wh_sum = new_width + new_height;

                    new_width = Math.round(new_wh_sum * width_ratio);
                    new_height = Math.round((this._mouse_down_details.canvas_width + new_width) / this._current_layer._aspect_ratio) - this._mouse_down_details.canvas_height;
                }

                if (CANVAS_EDGE.LEFT)
                    new_x = -new_width;
                if (CANVAS_EDGE.TOP)
                    new_y = -new_height;

                let CheckNull = function(val, add) 
                { return (val === 0 ? null : (val + add))}
                this.CurrentCanvasSetCanvasDimensions(
                    CheckNull(new_x, this._mouse_down_details.canvas_x),
                    CheckNull(new_y, this._mouse_down_details.canvas_y),
                    CheckNull(new_width, this._mouse_down_details.canvas_width),
                    CheckNull(new_height, this._mouse_down_details.canvas_height));
            } 
            else 
            {
                this.CurrentCanvasSetCanvasDimensions(this._mouse_down_details.canvas_x + canvas_movement_x, this._mouse_down_details.canvas_y + canvas_movement_y, null, null);
            }
        }
    }

    OnKeyDown(event)
    {
        switch (event.keyCode) 
        {
            case 17: // control
                this._scroll_zoom = true;
                break;
            case 18: // alt
                this._scroll_z = true;
                break;
            case 187: // plus/equals
                if (event.shiftKey) // plus
                    this._scale += 0.1;
                    this.ApplyScale();
                break;
            case 189: // minus/underscore
                if (!event.shiftKey) // minus
                    this._scale -= 0.1;
                    this.ApplyScale();
                break;
            default:
                break;
        }
    }

    OnKeyUp(event)
    {
        switch (event.keyCode) 
        {
            case 17: // control
                this._scroll_zoom = false;
                break;
            case 18: // alt
                this._scroll_z = false;
                break;
            default:
                break;
        }
    }

    OnWheel(event)
    {
        event.preventDefault();
        if (this._scroll_zoom || this._touch_zoom) 
        {
            let scale_diff = (Math.sign(event.deltaY) / -10);
            if (this._touch_zoom)
                scale_diff *= 0.1;
            this._scale += scale_diff;
            this.ApplyScale();
        }

        if (this._scroll_z) 
        {
            this.SetCurrentZIndex(this._current_layer.GetZ() + Math.sign(event.deltaY) * -1);
        }
    }

    SetCursor()
    {
        if (CANVAS_EDGE.LEFT) 
        {
            if (CANVAS_EDGE.TOP)
                this._div.style.cursor = "nw-resize";
            else if (CANVAS_EDGE.BOTTOM)
                this._div.style.cursor = "sw-resize";
            else
                this._div.style.cursor = "w-resize";

            return;
        }
        if (CANVAS_EDGE.RIGHT) 
        {
            if (CANVAS_EDGE.TOP)
                this._div.style.cursor = "ne-resize";
            else if (CANVAS_EDGE.BOTTOM)
                this._div.style.cursor = "se-resize";
            else
                this._div.style.cursor = "e-resize";

            return;
        }
        if (CANVAS_EDGE.TOP) 
        {
            this._div.style.cursor = "n-resize";
            return;
        }
        if (CANVAS_EDGE.BOTTOM) 
        {
            this._div.style.cursor = "s-resize";
            return;
        }

        this._div.style.cursor = "move";
    }
}

export class OJPictureInPictureLayer
{    
    constructor(parent, canvas_params)
    {
        this._parent = parent;

        if (canvas_params != null)
        {
            this._unique_id = canvas_params.index;
            this._name = canvas_params.name;
            this._min_x = canvas_params.minimum_x;
            this._min_y = canvas_params.minimum_y;
            this._max_x = canvas_params.maximum_x;
            this._max_y = canvas_params.maximum_y;
            this._min_width = canvas_params.minimum_width;
            this._min_height = canvas_params.minimum_height;
            this._max_width = canvas_params.maximum_width;
            this._max_height = canvas_params.maximum_height;
            this._alpha = canvas_params.alpha;
            this._colour = canvas_params.colour;
            this._fixed_size = canvas_params.fixed_size;
            this._fixed_aspect_ratio = canvas_params.fixed_aspect_ratio;
            let x = canvas_params.x;
            let y = canvas_params.y;
            let width = canvas_params.width;
            let height = canvas_params.height;		
        }	
        else
        {
            this._unique_id = -1;
            this._name = "Layer";
            this._alpha = 102;
            this._colour = OJServerLink.Get()._altera_light_blue;
            this._fixed_size = false;
            this._fixed_aspect_ratio = true;
        }

        this._canvas = document.createElement("canvas");
        this._canvas.className = "picture_image_class";
        this._canvas.tabIndex = 0;
        this.SetAlpha(this._alpha);

        this._fixed_pos = false;

        this._aspect_ratio = 1;

        let self_obj = this;

        this._canvas.onmousedown = function(event)
        {
            self_obj.OnMouseDown(event);
        };

        this._canvas.onmousemove = function(event)
        {
            self_obj.OnMouseMove(event);
        };

        this._canvas.onmouseleave = function(event)
        {
            self_obj.OnMouseLeave(event);
        };

        this._canvas.onfocus = function(event)
        {
            self_obj.OnFocus(event);
        };

        this._canvas.addEventListener("touchstart", function(event) 
        { self_obj._parent.TouchHandler(event); });
        this._canvas.addEventListener("touchmove", function(event) 
        { self_obj._parent.TouchHandler(event); });
        this._canvas.addEventListener("touchend", function(event) 
        { self_obj._parent.TouchHandler(event); });
        this._canvas.addEventListener("touchcancel", function(event) 
        { self_obj._parent.TouchHandler(event); });

        if (canvas_params != null)
        {
            let temp_fixed_size = this._fixed_size;
            let temp_aspect_ratio = this._fixed_aspect_ratio;
            this._fixed_size = false;
            this._fixed_aspect_ratio = false;

            this.SetDefaultDimensions(width, height);
            this.SetDimensions(x, y, width, height);

            this._fixed_size = temp_fixed_size;
            this._fixed_aspect_ratio = temp_aspect_ratio;
        }
    };

    Destroy () 
    {
        UI.RemoveFromParentElement(this._canvas);
    }

    DrawContext ()
    {
        let context = this._canvas.getContext("2d");
        context.clearRect(0, 0, this._canvas.width, this._canvas.height);
        context.globalAlpha = this._alpha / 255;
        context.fillStyle = this._colour;
        context.fillRect(0, 0, this._canvas.width, this._canvas.height);
    }

    GetCanvas ()
    {
        if (this._canvas)
            return this._canvas;
        return null;
    }

    GetName ()
    {
        return this._name;
    }

    GetAlpha ()
    {
        return this._alpha;
    }

    GetX ()
    {
        return this._parent.CanvasToImage(this.GetCanvasX());
    }

    GetCanvasX ()
    {
        let x = parseInt(this._canvas.style.left);
        return isNaN(x) ? 0 : x;
    }

    GetY ()
    {
        return this._parent.CanvasToImage(this.GetCanvasY());
    }

    GetCanvasY ()
    {
        let y = parseInt(this._canvas.style.top);
        return (isNaN(y) ? 0 : y);
    }

    GetZ ()
    {
        return parseInt(this._canvas.style.zIndex);
    }

    GetWidth ()
    {
        return this._parent.CanvasToImage(this.GetCanvasWidth());
    }

    GetCanvasWidth ()
    {
        let width = parseInt(this._canvas.style.width);
        return (isNaN(width) ? 0 : width);
    }

    GetHeight ()
    {
        return this._parent.CanvasToImage(this.GetCanvasHeight());
    }

    GetCanvasHeight ()
    {
        let height = parseInt(this._canvas.style.height);
        return (isNaN(height) ? 0 : height);
    }

    SetAlpha (alpha)
    {
        if (alpha < 0)
            this._alpha = 0;
        else if (alpha > 255)
            this._alpha = 255;
        else
            this._alpha = alpha;

        this.DrawContext();
    }
    SetX (x)
    {
        if (!this._fixed_pos) 
        {
            let min_x = -(this.GetWidth());
            let max_x = parseInt(this._parent._div.style.width);

            if (x < this._min_x)
                x = this._min_x;
            if (x > this._max_x)
                x = this._max_x;

            this._canvas.style.left = this._parent.ImageToCanvas(x) + "px";

            if (this._canvas.style.left < min_x)
                this._canvas.style.left = min_x;
            if (this._canvas.style.left > max_x)
                this._canvas.style.left = max_x;
        }
    }

    SetY (y)
    {
        if (!this._fixed_pos) 
        {
            let min_y = -(this.GetHeight());
            let max_y = parseInt(this._parent._div.style.width);

            if (y < this._min_y)
                y = this._min_y;
            if (y > this._max_y)
                y = this._max_y;

            this._canvas.style.top = this._parent.ImageToCanvas(y) + "px";

            if (this._canvas.style.top < min_y)
                this._canvas.style.top = min_y;
            if (this._canvas.style.top > max_y)
                this._canvas.style.top = max_y;
        }
    }

    SetZ (z)
    {
        if (typeof z === "number" && !isNaN(z))
            this._canvas.style.zIndex = z;
    }
    SetWidth (width)
    {
        if (!this._fixed_size) 
        {

            if (width < this._min_width)
                width = this._min_width;
            if (width > this._max_width)
                width = this._max_width;

            this._canvas.style.width = this._parent.ImageToCanvas(width) + "px";

            if (this._canvas.style.width < this._min_width)
                this._canvas.style.width = this._min_width;
            if (this._canvas.style.width > this._max_width)
                this._canvas.style.width = this._max_width;
        }
    }

    SetHeight (height)
    {
        if (!this._fixed_size) 
        {

            if (height < this._min_height)
                height = this._min_height;
            if (height > this._max_height)
                height = this._max_height;

            this._canvas.style.height = this._parent.ImageToCanvas(height) + "px";

            if (this._canvas.style.height < this._min_height)
                this._canvas.style.height = this._min_height;
            if (this._canvas.style.height > this._max_height)
                this._canvas.style.height = this._max_height;
        }
    }

    SetMinWidthHeight (new_min_width, new_min_height)
    {
        if (typeof new_min_width === "number" && new_min_width > 0 && new_min_width < this._max_width) 
        {
            this._min_width = new_min_width;
            if (this.GetWidth() < new_min_width)
                this.SetWidth(new_min_width);
        }
        if (typeof new_min_height === "number" && new_min_height > 0 && new_min_width < this._max_height) 
        {
            this._min_height = new_min_height;
            if (this.GetHeight() < new_min_height)
                this.SetHeight(new_min_height);
        }
    }

    SetMaxWidthHeight (new_max_width, new_max_height)
    {
        if (typeof new_max_width === "number" && new_max_width > this._min_width) 
        {
            this._max_width = new_max_width;
            if (this.GetWidth() > new_max_width)
                this.SetWidth(new_max_width);
        }
        if (typeof new_max_height === "number" && new_max_height > this._min_height) 
        {
            this._max_height = new_max_height;
            if (this.GetHeight() > new_max_height)
                this.SetHeight(new_max_height);
        }
    }

    SetDefaultDimensions (width, height)
    {
        this._default_width = width;
        this._default_height = height;
    }

    SetCanvasDimensions (x, y, width, height)
    {
        let image_x = null;
        let image_y = null;
        let image_width = null;
        let image_height = null;
        if (typeof x === "number")
            image_x = this._parent.CanvasToImage(x);
        if (typeof y === "number")
            image_y = this._parent.CanvasToImage(y);
        if (typeof width === "number")
            image_width = this._parent.CanvasToImage(width);
        if (typeof height === "number")
            image_height = this._parent.CanvasToImage(height);
        this.SetDimensions(image_x, image_y, image_width, image_height);

    }

    SetDimensions (x, y, width, height)
    {
        if (!this._fixed_size) 
        {
            if (typeof width === "number")
                this.SetWidth(width);
            if (typeof height === "number")
                this.SetHeight(height);
        }

        if (!this._fixed_pos) 
        {
            if (typeof x === "number")
                this.SetX(x);
            if (typeof y === "number")
                this.SetY(y);
        }

        if (!this._fixed_aspect_ratio) 
        {
            let calculated_aspect_ratio = this.GetWidth() / this.GetHeight();
            this._aspect_ratio = Math.round(calculated_aspect_ratio * 100 + Number.EPSILON) / 100;
            this._canvas.style.aspectRatio = this._aspect_ratio;
        } 
        else 
        {
            if (typeof height === "number" && typeof width !== "number") 
                this.SetWidth(this.GetHeight() * this._aspect_ratio);

            let calc_height = Math.round(this.GetWidth() / this._aspect_ratio);
            if (this.GetHeight() !== calc_height) 
            {
                this.SetHeight(calc_height);
            }
        }

        if (this._parent)
            this._parent.RefreshLabels();
    }

    OnMouseDown (vent)
    {
        ModifyMouseCoordinates(event);

        this._parent.SetCurrentCanvas(this);
        this._parent.CanvasClick();
    }

    OnMouseLeave (event)
    {
        if (this._parent && this._parent._track_canvas_edge) 
        {
            CANVAS_EDGE.LEFT = false;
            CANVAS_EDGE.RIGHT = false;
            CANVAS_EDGE.TOP = false;
            CANVAS_EDGE.BOTTOM = false;

            if (!this._parent._transforming)
                this._parent.GetElement().style.cursor = "default";
        }
    }

    OnMouseMove (event)
    {
        ModifyMouseCoordinates(event);

        let edge_tolerance_max = 20;

        let width = this.GetCanvasWidth();
        let height = this.GetCanvasHeight();	
        let edge_tolerance_x = width / 4;
        let edge_tolerance_y = height / 4;
        
        if (edge_tolerance_x > edge_tolerance_max)
            edge_tolerance_x = edge_tolerance_max;
        if (edge_tolerance_y > edge_tolerance_max)
            edge_tolerance_y = edge_tolerance_max;

        if (this._parent._track_canvas_edge) 
        {
            if (!this._fixed_size) 
            {
                let pos_x = event._offset_x;
                let pos_y = event._offset_y;

                CANVAS_EDGE.RIGHT = (pos_x > (width - edge_tolerance_x));
                CANVAS_EDGE.BOTTOM = (pos_y > (height - edge_tolerance_y));

                CANVAS_EDGE.LEFT = (CANVAS_EDGE.RIGHT ? false : (pos_x < edge_tolerance_x));
                CANVAS_EDGE.TOP = (CANVAS_EDGE.BOTTOM ? false : (pos_y < edge_tolerance_y));
            }

            this._parent.SetCursor();
        }
    }

    OnFocus (event)
    {
        this._canvas.style.outline = "5px solid gold";
    }
}

/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJWindowElement, ANCHOR_TYPE } from "./OJL.js";
import { OJLib } from "./OJL.js";

export class OJImage extends OJWindowElement
{
    constructor(url, auto_size)
    {
        // Base class constructor
        super();
        this._class_name = "OJImage";

        // Control will resize to the size of the image if auto_size is set
        this._auto_size = auto_size;
        this._actual_image_width = 0;
        this._actual_image_height = 0;
        this._image_element = document.createElement("img");
        this._image_element._picture_button = this;
        this._image_element.className = "picture_button_class";
        this._image_element.ondragstart = function()
        {
            return false;
        };

        this._image_element.onload = function()
        {
            var width = this.width;
            var height = this.height;
            this._picture_button.UpdatePictureSize(width, height);
        };

        this._client_area.appendChild(this._image_element);
        this._image_element.src = url;
    }

    UpdatePictureSize(width, height)
    {
        this._actual_image_width = width;
        this._actual_image_height = height;

        if (this._auto_size)
        {
            this._image_element.style.width = width + "px";
            this._image_element.style.height = height + "px";

            this.SetRightAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: width });
            this.SetBottomAnchor({ _type: ANCHOR_TYPE.FIXED_SIZE, _fixed_size: height });

            // If parent has a PositionChildren function(eg OJGrid) then call it
            if (typeof (this._parent.PositionChildren) === "function")
                this._parent.PositionChildren();
        }
        else
        {
            this._image_element.style.width = this._width + "px";
            this._image_element.style.height = this._height + "px";
        }
    }

    SetUrl(url)
    {
        this._image_element.src = url;
    }

    Resize(x, y, width, height)
    {
        var size_changed = super.Resize(x, y, width, height);

        if (this._auto_size)
        {
            if (this._actual_image_width > 0)
            {
                var image_x = (width - this._actual_image_width) / 2;
                var image_y = (height - this._actual_image_height) / 2;

                OJLib.SetElementLocation(this._image_element, image_x, image_y);
            }
        }
        else
        {
            OJLib.SetElementPosition(this._image_element, 0, 0, width, height);
        }

        return size_changed;
    }
}

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

// For issuing commands to server and receiving same commands in return
export class OJCommandController
{
    constructor(command, callback_object, callback_function)
    {
        this._command = command;
        this._callback_object = callback_object;
        this._callback_function = callback_function;

        OJServerLink.Get().RegisterCommandController(this);
    }

    Destroy()
    {
        OJServerLink.Get().UnregisterCommandController(this);
    }

    Send(params)
    {
        OJServerLink.Get().ExecuteServerCommandWithParams(this._command, params);
    }

    Process(json_ui_update)
    {
        // The server will return a json item with the name of the callback function
        if (this._command in json_ui_update)
        {
            // Now callback our object with the parameters
            var params = json_ui_update[this._command];
            this._callback_object[this._callback_function](params);
            return true;
        }

        return false;
    }
}

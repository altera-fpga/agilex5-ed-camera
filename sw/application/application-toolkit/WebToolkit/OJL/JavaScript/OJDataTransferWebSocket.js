/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJLib } from "./OJL.js";
import { OJWebSocket } from "./OJL.js";

export class OJDataTransferWebSocket
{
    constructor(data_type)
    {
        this._data_type = data_type;
        this._ready = false;
        this._pending_send_data = null;
        this._data_transfer_web_socket = new OJWebSocket(OJLib._websocket_server_url, "datatransfer", this);
    }

    Destroy()
    {
        this._data_transfer_web_socket.Destroy();
    }

    SendData(data)
    {
        if (this._ready)
        {
            this._data_transfer_web_socket.Send(data);
        }
        else
        {
            this._pending_send_data = data;
        }
    }

    OnWebSocketReady(event)
    {
        this._ready = true;

        if (this._pending_send_data != null)
            this._data_transfer_web_socket.Send(this._pending_send_data);
    }

    WebSocketDestroyed(event)
    {
        this._ready = false;
    }

    OnOpenWebSocket(event)
    {
    }

    OnCloseWebSocket(event)
    {
        this._ready = false;
    }
    
    OnErrorWebSocket(event)
    {
    }

    OnWebSocketServiceUnavailable(reason)
    {

    }

    OnMessageWebSocket(web_socket, message)
    {
        // // Async message received from server intermediate layer.
        // // The message is XML as one string
        // if (message._is_binary)
        //     return; // Not handled

        // let text = message._text_data;
        // if (text == "")
        //     return;

        // if (this._show_in_commands)
        // {
        //     OJLib.Trace("Receive: " + text, false);
        // }

        // let text_length = text.length;
        // let json_object = JSON.parse(text);

        // let json_main_ui = json_object.MainUserInterface;
        // if (json_main_ui != null)
        // {
        //     this.LoadMainUiFromJson(json_main_ui);
        //     return;
        // }

        // if (!this._ui_loaded)
        //     return;

        // let json_ui_update = json_object.UiUpdate;
        // if (json_ui_update != null)
        // {
        //     this.UiUpdate(json_ui_update);
        //     return;
        // }
    }    

    
    // OnOpenWebSocket
    // OnClose
    // MessageHandler
    // OnErrorWebSocket
    // WebSocketDestroyed
    // OnCloseWebSocket
    // OnMessageWebSocket
    // OnWebSocketServiceUnavailable
    // OnWebSocketReady
}



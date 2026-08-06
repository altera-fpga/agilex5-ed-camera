/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

let DataType =
{
    INT8: 0,
    UINT8: 1,
    INT16: 2,
    UINT16: 3,
    INT32: 4,
    UINT32 : 5
};

// owner can implement these callbacks:
//		OnOpenWebSocket
//		OnErrorWebSocket
//		WebSocketDestroyed
//		OnCloseWebSocket
//		OnMessageWebSocket

export class OJWebSocket
{
    constructor(url, omnitek_ws_protocol, owner, opts)
    {
        this._owner = owner;
        this._web_socket = new WebSocket(url, omnitek_ws_protocol);
        this._web_socket._shutdown = false;
        this._web_socket._owner = this;
        this._web_socket.binaryType = "arraybuffer";

        this._sub_protocol = omnitek_ws_protocol;
        this._web_socket._omnitek_ws_protocol = omnitek_ws_protocol;
        this._have_protocol = false;
        this._legacy_protocol = false;
        this._is_binary = false;
        this._hex_lookup = null;
        this._socket_shutdown = false;
        this._binary_data_type = DataType.UINT8;
        this._is_open = false;

        if (opts != null)
        {
            if (opts.binary_data_type != null)
                this._binary_data_type = opts.binary_data_type;
        }

        this._web_socket.onopen = function(event)
        {
            this._owner._is_open = true;

            if ((this._owner != null) && (this._owner._owner != null))
            {
                if ("OnOpenWebSocket" in this._owner._owner)
                    this._owner._owner.OnOpenWebSocket(event);
            }
        };

        this._web_socket.onclose = function(event)
        {
            this._owner.OnClose(event);
        };

        this._web_socket.onmessage = function(event)
        {
            if (this._socket_shutdown)
                return;

            if (this._owner != null)
                this._owner.MessageHandler(event);
        };

        this._web_socket.onerror = function(event)
        {
            if (this._socket_shutdown)
                return;

            if ((this._owner != null) && (this._owner._owner != null))
            {
                if ("OnErrorWebSocket" in this._owner._owner)
                    this._owner._owner.OnErrorWebSocket(event);
            }
        };
    }

    Destroy()
    {
        this._socket_shutdown = true;
        this.Close();

        // Wait for OnClose callback from WS to continue destruction
    }

    DestroyComplete()
    {
        // Remove circular reference
        if (this._hex_lookup != null)
            this._hex_lookup.length = 0;

        this._web_socket.onopen = null;
        this._web_socket.onclose = null;
        this._web_socket.onmessage = null;
        this._web_socket.onerror = null;
        this._web_socket._owner = null;
        this._web_socket = null;

        if ("WebSocketDestroyed" in this._owner)
            this._owner.WebSocketDestroyed();

        this._owner = null;
    }

    OnClose(event)
    {
        if ("OnCloseWebSocket" in this._owner)
            this._owner.OnCloseWebSocket(event);

        this._is_open = false;
        this.DestroyComplete();
    }

    IsOpen()
    {
        return this._is_open;
    }

    MessageHandler(event)
    {
        if (this._socket_shutdown)
            return;

        if (this._have_protocol)
        {
            if ("OnMessageWebSocket" in this._owner)
            {
                let data = event.data;

                // Legacy binary always comes in as a string
                let is_string_data = (typeof data == "string" || data instanceof String);

                if (this._legacy_protocol)
                {
                    // Check legacy data type
                    let is_binary = data.substr(0, 4) == "<b/>";
                    let real_data = data.substr(4);

                    if (is_binary)
                    {
                        let binary_data = this.Decode16BitString(real_data);
                        this._owner.OnMessageWebSocket(this,
                                                        {
                                                            _binary_data: binary_data,
                                                            _is_binary: true,
                                                            _data_type: this._binary_data_type
                                                        });
                    }
                    else
                    {
                        this._owner.OnMessageWebSocket(this, { _text_data: real_data, _is_binary: false });
                    }
                }
                else
                {
                    // We can handle both binary and text
                    if (is_string_data)
                    {
                        this._owner.OnMessageWebSocket(this, { _text_data: data, _is_binary: false });
                    }
                    else
                    {
                        let binary_data = null;

                        if (this._binary_data_type == DataType.UINT8)
                            binary_data = new Uint8Array(data);
                        else if (this._binary_data_type == DataType.INT16)
                            binary_data = new Int16Array(data);
                        else if (this._binary_data_type == DataType.UINT16)
                            binary_data = new Uint16Array(data);
                        else if (this._binary_data_type == DataType.UINT32)
                            binary_data = new Uint32Array(data);

                        this._owner.OnMessageWebSocket(this,
                                                        {
                                                            _binary_data: binary_data,
                                                            _is_binary: true,
                                                            _data_type: this._binary_data_type
                                                        });
                    }
                }
            }
        }
        else
        {
            // First message is protocol notification

            // We can't use XML parser, since this class is used in
            // Web Workers, where these are not supported
            let text = event.data;
            let neg_pos = text.indexOf("WebSocketNegotiation");
            if (neg_pos < 0)
            {
                let not_avail_pos = text.indexOf("ServiceUnavailable");
                if (not_avail_pos > 0)
                {
                    let reason_pos = text.indexOf("reason") + 8;
                    let reason_str = text.substr(reason_pos);
                    let reason_end_pos = reason_str.indexOf("'");
                    reason_str = reason_str.substr(0, reason_end_pos);

                    if ("OnWebSocketServiceUnavailable" in this._owner)
                        this._owner.OnWebSocketServiceUnavailable(reason_str);
                }

                this.Close();
                return;
            }

            // Format is like this: <WebSocketNegotiation version='13' is_binary='true'/>
            let ver_pos = text.indexOf("version") + 9;
            let num_str = text.substr(ver_pos);
            let ver_end_pos = num_str.indexOf("'");
            num_str = num_str.substr(0, ver_end_pos);
            let version = parseInt(num_str);
            this._legacy_protocol = (version < 13);

            let bin_pos = text.indexOf("is_binary") + 11;
            let bin_str = text.substr(bin_pos);
            let bin_end_pos = bin_str.indexOf("'");
            bin_str = bin_str.substr(0, bin_end_pos);
            this._is_binary = (bin_str == "true");

            this._have_protocol = true;

            if ("OnWebSocketReady" in this._owner)
                this._owner.OnWebSocketReady();
        }
    }

    GetBufferedAmount()
    {
        return this._web_socket.bufferedAmount;
    }

    Send(message)
    {
        if (this._socket_shutdown)
            return;

        if (this._web_socket != null)
            this._web_socket.send(message);
        else
            console.log("!! Send Lost !!");
    }

    Close()
    {
        if (this._web_socket != null)
        {
            this._web_socket._socket_shutdown = true;
            this._web_socket.close();
        }
    }

    BuildHexLookupTable()
    {
        this._hex_lookup = [];
        let alphabet = "0123456789ABCDEF";
        for (let i = 0; i < 16; i++)
        {
            this._hex_lookup[alphabet[i]] = i;
        }
    }

    Decode16BitString(data)
    {
        if (this._hex_lookup == null)
            this.BuildHexLookupTable();

        let binary_data = null;

        if (this._binary_data_type == DataType.UINT8)
        {
            let len = data.length / 2;
            binary_data = new Uint8Array(len);
            for (let i = 0; i < len; i++)
            {
                let v = this._hex_lookup[data[i * 2]] * 0x10 +
                        this._hex_lookup[data[i * 2 + 1]];

                binary_data[i] = v;
            }
        }
        else if (this._binary_data_type == DataType.INT16)
        {
            let len = data.length / 4;
            binary_data = new Int16Array(len);
            for (let i = 0; i < len; i++)
            {
                let b0 = this._hex_lookup[data[i * 4 + 0]] * 0x10 +
                        this._hex_lookup[data[i * 4 + 1]];
                let b1 = this._hex_lookup[data[i * 4 + 2]] * 0x10 +
                        this._hex_lookup[data[i * 4 + 3]];

                binary_data[i] = b1 * 0x100 + b0;
            }
        }
        else if (this._binary_data_type == DataType.UINT16)
        {
            let len = data.length / 4;
            binary_data = new Uint16Array(len);
            for (let i = 0; i < len; i++)
            {
                let b0 = this._hex_lookup[data[i * 4 + 0]] * 0x10 +
                        this._hex_lookup[data[i * 4 + 1]];
                let b1 = this._hex_lookup[data[i * 4 + 2]] * 0x10 +
                        this._hex_lookup[data[i * 4 + 3]];

                binary_data[i] = b1 * 0x100 + b0;
            }
        }
        else if (this._binary_data_type == DataType.UINT32)
        {
            let len = data.length / 8;
            binary_data = new Uint32Array(len);
            for (let i = 0; i < len; i++)
            {
                let b0 = this._hex_lookup[data[i * 8 + 0]] * 0x10 +
                        this._hex_lookup[data[i * 8 + 1]];
                let b1 = this._hex_lookup[data[i * 8 + 2]] * 0x10 +
                        this._hex_lookup[data[i * 8 + 3]];
                let b2 = this._hex_lookup[data[i * 8 + 4]] * 0x10 +
                        this._hex_lookup[data[i * 8 + 5]];
                let b3 = this._hex_lookup[data[i * 8 + 6]] * 0x10 +
                        this._hex_lookup[data[i * 8 + 7]];

                binary_data[i] = b3 * 1000000 + b2 * 10000 + b1 * 0x100 + b0;
            }
        }

        return binary_data;
    }
}

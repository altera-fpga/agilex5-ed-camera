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
import { OJCommandController } from "./OJL.js";
import { OJLib } from "./OJL.js";

let file_download_in_progress = false;
let file_download_pending_resize = false;

export class OJFileDownloader
{
    constructor(params)
    {
        file_download_in_progress = true;
        this._file_name = params.file_name;
        this._download_id = params.download_id;
        this._last_item = params.last_item;
        this._receive_file_name = "";
        this._web_socket = new OJWebSocket(OJLib._websocket_server_url, "filedownload", this);
    }

    Destroy()
    {
        file_download_in_progress = false;

        if (this._web_socket != null)
        {
            this._web_socket.Destroy();
            this._web_socket = null;
        }

        if (file_download_pending_resize)
        {
            file_download_pending_resize = false;
            OnResizeDone();
        }
    }

    OnWebSocketReady(event)
    {
        this._transfer_id = CreateTransferId();

        let parameters = {};
        parameters.TransferId = this._transfer_id;
        parameters.ClientId = OJServerLink.Get()._client_id;

        this.SendCommand("StartDownload", parameters);
    }

    SendCommand(command, parameters)
    {
        let command_object = { Command: { _value: command, Params: parameters } };
        let json_string = JSON.stringify(command_object);
        this._web_socket.Send(json_string);
    }

    OnMessageWebSocket(web_socket, message)
    {
        let is_text = !message._is_binary;

        if (is_text)
        {
            let text = message._text_data;

            let json_object = JSON.parse(text);
            let action = json_object._action;

            let download_start = json_object.FileDownloadStart;
            if (download_start != null)
            {
                this._receive_file_name = "";
                let transfer_id = download_start._transfer_id;
                let index = download_start._index;
                this._receive_file_name = download_start._file_name;
                return;
            }

            let download_complete = json_object.DownloadComplete;
            if (download_complete != null)
            {
                OJServerLink.Get().FileDownloaderFinished(this);
            }

            let download_failed = json_object.DownloadFailed;
            if (download_failed != null)
            {
                OJServerLink.Get().FileDownloaderFinished(this);
            }
        }
        else
        {
            if (this._receive_file_name.length > 0)
            {
                let blob = new Blob([message._binary_data], { type: "application/octet-stream" });
                saveAs(blob, this._receive_file_name);

                // Request next
                let parameters = {};
                // parameters.TransferId = this._transfer_id;
                // parameters.ClientId = OJServerLink.Get()._client_id;

                this.SendCommand("SendNextFile", parameters);
            }
        }
    }
}

export class OJUploadRequest
{
    constructor()
    {
        this._params = [];
    }

    AddParameter(param_name, param_value)
    {
        this._params.push(new OJParamPair(param_name, param_value));
    }

    GetString()
    {
        let str = "?";
        for (let i = 0; i < this._params.length; i++)
        {
            str += this._params[i]._name + "=" + this._params[i]._value;
            if (i < (this._params.length - 1))
                str += "&";
        }

        return str;
    }
}

class OJParamPair
{
    constructor(name, value)
    {
        this._name = name;
        this._value = value;
    }
}

export class OJLocalFileUploader
{
    constructor(transfer_type, multiple, file_extension)
    {
        this._transfer_type = transfer_type;

        if (multiple != null)
            this._multiple = multiple;
        else
            this._multiple = false;

        this._file_extension = file_extension;
        this._fetch_local_files_command_handler = null;
    }

    Destroy()
    {
        this.RemoveCommandHandler();
    }

    // Request for the files from the server
    SelectFiles(transfer_type, multiple, extension)
    {
        this._transfer_type = transfer_type;
        this._multiple = multiple;
        this._file_extension = extension;

        this.RemoveCommandHandler();
        this._fetch_local_files_command_handler = new OJCommandController("FetchLocalFiles", this, "FetchLocalFilesCB");
        OJServerLink.Get().AddCommandHandler(this._fetch_local_files_command_handler);

        this._fetch_local_files_command_handler.Send(this._transfer_type + "," + this._file_extension);
    }

    FetchLocalFilesCB(params)
    {
    }

    RemoveCommandHandler()
    {
        if (this._fetch_local_files_command_handler != null)
        {
            OJServerLink.Get().RemoveCommandHandler(this._fetch_local_files_command_handler);
            this._fetch_local_files_command_handler.Destroy();
            this._fetch_local_files_command_handler = null;
        }
    }
}

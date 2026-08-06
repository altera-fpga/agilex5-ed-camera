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
import { OJLib, UI, ObjectCallback } from "./OJL.js";
import { OJWebSocket } from "./OJL.js";

let file_upload_in_progress = false;
let file_upload_pending_resize = false;

export class OJFileUploader
{
    constructor(websocket_server_url)
    {
        this._transfer_type = "";
        this._transfer_id = 0;
        this._selected_files = [];
        this._multiple = false;
        this._file_extension = "";
        this._web_socket = null;
        this._current_file_name = "<unknown>";
        this._data_buffer = null;
        this._remaining_buffer_length = 0;
        this._remaining_buffer_start = 0;
        this._send_next_slice_timeout = null;
        this._item_id = 0;
        this._current_file_index = 0;
        this._websocket_server_url = websocket_server_url;
    }

    Destroy()
    {
        this.FinishWebSocketTransfer();
        file_upload_in_progress = false;
        this._selected_files.length = 0;
        this._selected_files = null;

        if (OJLib._file_select_callback != null)
        {
            OJLib._file_select_callback.Destroy();
            OJLib._file_select_callback = null;
        }

        if (this._file_reader != null)
        {
            this._file_reader._file_uploader = this;
            this._file_reader = null;
        }

        if (this._web_socket != null)
        {
            this._web_socket.Destroy();
            this._web_socket = null;
        }

        if (file_upload_pending_resize)
        {
            file_upload_pending_resize = false;
            OnResizeDone();
        }

        this._opts = null;
    }

    SelectFiles(transfer_type, dialog_title, multiple, extension, opts)
    {
        this._transfer_type = transfer_type;
        this._dialog_title = dialog_title;
        this._multiple = multiple;
        this._file_extension = extension;
        this._opts = opts;

        if (this._web_socket != null)
        {
            if (this._web_socket.IsOpen())
            {
                // Transfer in progress
                OJLib.Trace("A transfer is already in progress");
                return;
            }

            this._web_socket.Destroy();
            this._web_socket = null;
        }

        // These are global params in OmniTek.js
        OJLib._file_select_callback = new ObjectCallback(this, "FilesSelected");

        // UI._invisible_input is global 'input' element in OmniTek.js
        UI._invisible_input.multiple = this._multiple;
        UI._invisible_input.value = "";
        UI._invisible_input.accept = this._file_extension;
        UI._invisible_input.click();
    }

    UploadFiles(transfer_type, files)
    {
        this._transfer_type = transfer_type;
        this.FilesSelected(files);
    }

    FilesSelected(files)
    {
        if (files.length == 0)
            return;

        this._selected_files.length = 0;
        for (let i = 0; i < files.length; i++)
            this._selected_files.push(files[i]);

        this._selected_files.sort(CompareFiles);

        if (OJLib._file_select_callback != null)
        {
            OJLib._file_select_callback.Destroy();
            OJLib._file_select_callback = null;
        }

        file_upload_in_progress = true;
        this._web_socket = new OJWebSocket(this._websocket_server_url, "filetransfer", this);
    }

    FindFileNameComponents(file_name)
    {
        // /data/Generator/Sequence_0000.yuv
        let slash = "/";

        if (file_name.substr(0, 1) != "/")
            slash = "\\" // For windows

        let components = { _valid: false };

        let extension_dot = file_name.lastIndexOf(".");
        if (extension_dot < 0)
            return components;

        components._extension = file_name.substr(extension_dot + 1);

        // Remove extension
        // /data/Generator/Sequence_0000
        file_name = file_name.substr(0, extension_dot);

        let folder_end_pos = file_name.lastIndexOf(slash);
        let file_title = file_name;
        let folder = "";

        if (folder_end_pos >= 0)
        {
            file_title = file_name.substr(folder_end_pos + 1);
            folder = file_name.substr(0, folder_end_pos);
        }

        components._folder = folder;

        // Now split file_title into prefix and index
        // Find number of digits in the index
        let i = 0;
        for (i = 0; i < 5; i++)
        {
            let ch = file_title.charAt(file_title.length - i - 1);
            if (!IsDigit(ch))
                break;
        }

        components._prefix = file_title.substr(0, file_title.length - i);
        components._index = file_title.substr(file_title.length - i);
        components._valid = true;
        return components;
    }

    ReadAndSendFile(file_index)
    {
        if (file_index < this._selected_files.length)
        {
            this._current_file_index = file_index;
            let file = this._selected_files[file_index];
            let size = file.size;

            if (this._file_reader != null)
            {
                this._file_reader._file_uploader = null;
                this._file_reader = null;
            }

            let parameters = {};
            parameters.TransferId = this._transfer_id;
            parameters.Filename = file.name;
            parameters.FileSize = size;
            parameters.NumSegments = 1;
            parameters.FileIndex = file_index;

            this.SendCommand("TransferFileStart", parameters);

            //OJLib.Trace("Create FileReader for " + file.name);
            this._file_reader = new FileReader();
            this._file_reader._file_uploader = this;
            this._file_reader.onload = FileLoaded;
            this._file_reader.onprogress = FileLoadProgress;
            this._current_file_name = file.name;
            this._file_reader.readAsArrayBuffer(file);
        }
        else
        {
            let parameters = { Error: "Bad file index" };
            this.SendCommand("FileUploadError", parameters);
        }
    }

    OnWebSocketReady(event)
    {
        this._transfer_id = CreateTransferId();

        let parameters = {};
        parameters.NumFiles = this._selected_files.length;
        parameters.TransferId = this._transfer_id;
        parameters.ClientId = OJServerLink.Get()._client_id;
        parameters.TransferType = this._transfer_type;

        // For a sequence, the first filename is used as the folder name
        parameters.FileName0 = "";

        for (let i = 0; i < this._selected_files.length; i++)
        {
            let file_name_id = "FileName" + i;
            parameters[file_name_id] = this._selected_files[i].name;
        }

        this.SendCommand("FileUploadStart", parameters);
    }

    OnCloseWebSocket(event)
    {
        this.FinishWebSocketTransfer();
    }

    SendCommand(command, parameters)
    {
        let command_object = { Command: { value: command, params: parameters } };
        let json_string = JSON.stringify(command_object);
        this._web_socket.Send(json_string);
    }

    OnMessageWebSocket(web_socket, message)
    {
        let text = message._text_data;

        let json_object = JSON.parse(text);
        let action = json_object._action;

        let send_me_file = json_object.SendMeFile;
        if (send_me_file != null)
        {
            let transfer_id = send_me_file.transfer_id;
            let file_index = send_me_file.file_index;

            // item_id is usually the pattern id used in the generator
            this._item_id = send_me_file.item_id;

            //OJLib.Trace("SendMeFile received, transfer_id = " + transfer_id + ", file_index = " + file_index);
            this.ReadAndSendFile(file_index);
            return;
        }

        let transfer_complete = json_object.TransferComplete;
        if (transfer_complete != null)
        {
            let transfer_id = transfer_complete.transfer_id;
            if (this._web_socket != null)
                this._web_socket.Close();
        }

        let cancel_upload = json_object.CancelUpload;
        let send_file_segment = json_object.SendFileSegment;
    }

    FileLoaded(event)
    {
        //OJLib.Trace("FileLoaded " + this._current_file_name);

        this._data_buffer = this._file_reader.result;
        this._remaining_buffer_length = this._data_buffer.byteLength;
        this._remaining_buffer_start = 0;

        if (this._send_next_slice_timeout != null)
            clearInterval(this._send_next_slice_timeout);

        let send_in_one_chunk = false;

        if (send_in_one_chunk)
        {
            this._web_socket.Send(this._data_buffer);
            //this.SendCommand("BinaryFileComplete");
            this.FinishWebSocketTransfer();
        }
        else
        {
            this._send_next_slice_timeout = setInterval(SendNextSlice, 100, this);
        }
    }

    FileLoadProgress(event)
    {
        if ((this._transfer_type == "GeneratorPatternImport") ||
            (this._transfer_type == "GeneratorSequenceImport"))
        {
            let percent = 0;

            if (event.lengthComputable)
            {
                percent = ((100 * event.loaded) / event.total) | 0;
                let json = {};
                json._unique_id = this._item_id;
                json._label = "Reading";
                if (this._selected_files.length > 1)
                {
                    let index_num = this._current_file_index + 1;
                    json._label += " " + index_num + "/" + this._selected_files.length;
                }
                json._progress = percent;

                GetGeneratorWindow().GeneratorPatternProgress(json);
            }
        }
    }

    SendNextSlice()
    {
        // Wait until previous data has been sent
        if (this._web_socket.GetBufferedAmount() > 0)
            return;

        if (this._remaining_buffer_length > 0)
        {
            // 16 meg at a time
            let send_length = Math.min(this._remaining_buffer_length, 0x1000000);
            let sub_array = this._data_buffer.slice(this._remaining_buffer_start, this._remaining_buffer_start + send_length);
            //OJLib.Trace("SendNextSlice start = " + this._remaining_buffer_start + ", length = " + send_length);

            //OJLib.Trace("Send " + count + " length = " + send_length);
            this._web_socket.Send(sub_array);

            this._remaining_buffer_start += send_length;
            this._remaining_buffer_length -= send_length;
        }
        else if (this._remaining_buffer_length <= 0)
        {
            //OJLib.Trace("SendNextSlice done");
            //this.SendCommand("BinaryFileComplete");
            this.FinishWebSocketTransfer();
        }
    }

    FinishWebSocketTransfer()
    {
        if (this._send_next_slice_timeout != null)
            clearInterval(this._send_next_slice_timeout);

        this._send_next_slice_timeout = null;
        this._data_buffer = null;
        this._remaining_buffer_length = 0;
        this._remaining_buffer_start = 0;
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

function SendNextSlice(file_uploader)
{
    file_uploader.SendNextSlice();
}

function IsDigit(character)
{
    return ((character >= "0") && (character <= "9"));
}

function CreateTransferId()
{
    return (100000 + Math.random() * 1000000) | 0;
}

function FileLoaded(event)
{
    this._file_uploader.FileLoaded(event);
}

function FileLoadProgress(event)
{
    this._file_uploader.FileLoadProgress(event);
}

function CompareFiles(file_a, file_b)
{
    if (file_a.name == file_b.name)
        return 0;
    else if (file_a.name > file_b.name)
        return 1;
    else
        return -1;
}


/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "AtUtils.h"
#include "FileTransferWebSocketCommandProcessor.h"
#include "ParamList.h"
#include "FileTransfer.h"

std::recursive_mutex FileTransferWebSocketCommandProcessor::_nFileTransferWebSocketsCS;
int FileTransferWebSocketCommandProcessor::_nFileTransferWebSockets = 0;

FileTransferWebSocketCommandProcessor::FileTransferWebSocketCommandProcessor(IWebSocketService* iweb_socket)
:	WebSocketCommandHandler(iweb_socket)
{
    std::lock_guard lock(_nFileTransferWebSocketsCS);
    _nFileTransferWebSockets++;
}

FileTransferWebSocketCommandProcessor::~FileTransferWebSocketCommandProcessor()
{
}

void FileTransferWebSocketCommandProcessor::Shutdown()
{
    WebSocketCommandHandler::Shutdown();

    if (_spActiveFileTransfer)
        _spActiveFileTransfer->Cancel();

    std::lock_guard lock(_nFileTransferWebSocketsCS);
    _nFileTransferWebSockets--;
}

bool FileTransferWebSocketCommandProcessor::ProcessCommand(const std::string& command_value, ONamedParamList& parameter_list)
{
    if (command_value == "FileUploadStart")
    {
        std::shared_ptr<WebSocketFileTransfer> spFileTransfer(new WebSocketFileTransfer(this, parameter_list));
        _activeTransfers.push_back(std::move(spFileTransfer));
    }
    else if (command_value == "FileUploadFinish")
    {
        int transferId = 0;
        AtUtils::FromString(parameter_list.GetParam("TransferId"), transferId);

        DeleteFileTransfer(transferId);
    }
    else if (command_value == "TransferFileStart")
    {
        int transferId = 0;
        AtUtils::FromString(parameter_list.GetParam("TransferId"), transferId);

        _spActiveFileTransfer = GetFileTransfer(transferId);

        if (_spActiveFileTransfer)
        {
            _spActiveFileTransfer->TransferFileStart(parameter_list);
        }
        else
        {
            std::cout << "> TransferFileStart can't find transferId = " << transferId << std::endl;
        }
    }
    else if (command_value == "TransferFileSegment")
    {
    }
    else if (command_value == "BinaryFileComplete")
    {
    }

    return true;
}

void FileTransferWebSocketCommandProcessor::Process(std::vector<std::shared_ptr<ByteArray>>& binary_data,
                                                    bool final_frame)
{
    size_t total_size = 0;

    for (auto& data_item : binary_data)
        total_size += data_item->size();

    if (_spActiveFileTransfer)
    {
        // Returns true if full transfer is complete
        // Use false for final frame

        if (_spActiveFileTransfer->ReceiveBinaryData(binary_data))
        {
            AtUtils::Remove<std::shared_ptr<WebSocketFileTransfer>>(_activeTransfers, _spActiveFileTransfer);
            _spActiveFileTransfer = nullptr;
        }
    }
}

void FileTransferWebSocketCommandProcessor::Ready()
{
}

std::shared_ptr<WebSocketFileTransfer> FileTransferWebSocketCommandProcessor::GetFileTransfer(int transferId)
{
    for (auto& spActiveTransfer : _activeTransfers)
    {
        if (spActiveTransfer->_transferId == transferId)
            return spActiveTransfer;
    }

    return nullptr;
}

void FileTransferWebSocketCommandProcessor::DeleteFileTransfer(int transferId)
{
    for (auto i = _activeTransfers.begin(); i != _activeTransfers.end(); ++i)
    {
        if ((*i)->_transferId == transferId)
        {
            _activeTransfers.erase(i);
            return;
        }
    }
}


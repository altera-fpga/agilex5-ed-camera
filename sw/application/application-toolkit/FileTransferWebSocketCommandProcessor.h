/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "WebSocketCommandProcessor.h"

class ONamedParamList;
class WebSocketFileTransfer;

class FileTransferWebSocketCommandProcessor : public WebSocketCommandHandler
{
public:
    FileTransferWebSocketCommandProcessor(IWebSocketService* pIWebSocketService);
    ~FileTransferWebSocketCommandProcessor() override;
    bool ProcessCommand(const std::string& commandValue, ONamedParamList& parameterList) override;
    void Process(std::vector<std::shared_ptr<ByteArray>>& binaryData, bool finalFrame) override;
    void Ready() override;

    std::shared_ptr<WebSocketFileTransfer> GetFileTransfer(int transfer_id);

protected:
    virtual void Shutdown();

private:
    void DeleteFileTransfer(int transferId);

    std::vector<std::shared_ptr<WebSocketFileTransfer>> _activeTransfers;
    std::shared_ptr<WebSocketFileTransfer> _spActiveFileTransfer;

    static std::recursive_mutex _nFileTransferWebSocketsCS;
    static int _nFileTransferWebSockets;
};


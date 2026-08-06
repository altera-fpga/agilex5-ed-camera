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
#include "CommonUiUpdate.h"

class DataTransferWebSocketCommandProcessor : public WebSocketCommandHandler
{
public:
	DataTransferWebSocketCommandProcessor(IWebSocketService* pIWebSocketService);
	~DataTransferWebSocketCommandProcessor() override;
	void Ready() override;

	// WebSocketCommandHandler interface
    bool ProcessCommand(const std::string& commandValue, ONamedParamList& parameterList) override;
    void Process(std::vector<std::shared_ptr<ByteArray>>& binaryData, bool finalFrame) override;

    static void SetReceiver(ReceiveDataCB receiveDataCB);

private:
    static ReceiveDataCB _receiveDataCB;
};

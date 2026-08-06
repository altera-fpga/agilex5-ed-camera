/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "DataTransferWebSocketCommandProcessor.h"

ReceiveDataCB DataTransferWebSocketCommandProcessor::_receiveDataCB;

DataTransferWebSocketCommandProcessor::DataTransferWebSocketCommandProcessor(IWebSocketService* pIWebSocketService)
:	WebSocketCommandHandler(pIWebSocketService)
{
}

DataTransferWebSocketCommandProcessor::~DataTransferWebSocketCommandProcessor()
{
}

void DataTransferWebSocketCommandProcessor::SetReceiver(ReceiveDataCB receiveDataCB)
{
    _receiveDataCB = std::move(receiveDataCB);
}

void DataTransferWebSocketCommandProcessor::Ready()
{
}

void DataTransferWebSocketCommandProcessor::Process(std::vector<std::shared_ptr<ByteArray>>& binaryData,
                                                    bool finalFrame)
{
    // Receive binary data from client.
    uint32_t clientId = 0;

    if (_receiveDataCB)
    {
        // The receiver only expects a single buffer,
        // so if there are multiple, pack them first
        if (binaryData.size() == 1)
        {
            _receiveDataCB(clientId, *binaryData[0]);
        }
        else if (binaryData.size() > 1)
        {
            std::vector<uint8_t> combinedBuffer;
            for (auto spByteArray : binaryData)
            {
                std::vector<uint8_t>& sourceBuffer = *spByteArray;
                combinedBuffer.insert(combinedBuffer.end(), sourceBuffer.begin(), sourceBuffer.end());
            }

            _receiveDataCB(clientId, combinedBuffer);
        }
    }
}

// Core processor thread
bool DataTransferWebSocketCommandProcessor::ProcessCommand(const std::string& commandValue,
                                                           ONamedParamList& parameterList)
{
    return true;

    // if (commandValue == "GetImage")
    // {
    //     std::lock_guard lock(_pendingBufferCS);
    //     if (_spPendingImage)
    //         SendBuffer(_spPendingImage);
    //     else
    //         _sendNext = true;
    //     return true;
    // }
    // else
    //     return false;
}

/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "IHttpServerHost.h"
#include "AtUtils.h"
#include "CommonTypes.h"

class ONamedParamList;
class DataSendRequestSegment;
class DataSendRequest;
class ICommonApplicationBase;

class WebSocketCommandHandler : public IWebSocketCallback,
                                public AtUtils::ICommandChecker
{
    friend class CPUpdateUiCmd;
    friend class UiProcessWebSocketMessage;
    friend class UiProcessBinaryWebSocketMessage;

public:
    WebSocketCommandHandler(IWebSocketService* pIWebSocket);

    static bool Create(IWebSocketService* pIWebSocket);
    void ReceiveWebSocketMessage(IWebSocketMessage* pMessage) override;

    virtual void Process(std::shared_ptr<AtUtils::IJson> spCommandDoc);
    virtual void Process(std::vector<std::shared_ptr<ByteArray>>& binaryData, bool finalFrame) {} // Not pure virtual
    virtual bool ProcessCommand(const std::string& commandValue,
                                ONamedParamList& parameterList) { return false; }

    // ICommandChecker implementations
    bool TestCommandForRemoval(AtUtils::Message* pCommand);

    // If you derive from WebSocketCommandHandler, you must call
    // DestructionStarted in your derived destructor.
    //virtual void CallDestructionStarted() = 0;

    void SendToClient(ONamedParamList& parameterList);
    void SendToClient(std::shared_ptr<ByteArray> spBinaryData);

protected:
    virtual ~WebSocketCommandHandler();
    void Lock(); // Called from CPUpdateUiCmd and UiProcessWebSocketMessage
    void Unlock(); // Called from CPUpdateUiCmd
    void Shutdown() override;
    std::recursive_mutex _destructorLock;
    bool _inDestructor;
};

class SilentWebSocketCommandProcessor : public WebSocketCommandHandler
{
public:
    SilentWebSocketCommandProcessor(IWebSocketService* pIWebSocket);
    void Process(std::shared_ptr<AtUtils::IJson> spCommandDoc) override {}
    void Ready() override {}
};

class BinaryWebSocketMessage : public IWebSocketMessage
{
public:
    BinaryWebSocketMessage(std::shared_ptr<DataSendRequest>& data_send_request);
    virtual ~BinaryWebSocketMessage();
    virtual std::vector<std::shared_ptr<std::vector<uint8_t>>> GetData(bool& isText, bool& finalFrame);

private:
    std::shared_ptr<DataSendRequest> _spDataSendRequest;
};

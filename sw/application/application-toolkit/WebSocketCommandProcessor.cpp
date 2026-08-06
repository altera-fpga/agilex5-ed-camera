/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "AtUtils.h"
#include "UiWebSocketCommandProcessor.h"
#include "ParamList.h"
#include "DataSegment.h"
#include "FileTransferWebSocketCommandProcessor.h"
#include "PictureWebSocketCommandProcessor.h"
#include "DataTransferWebSocketCommandProcessor.h"
#include "CommonCommands.h"
#include "CommonApplicationBase.h"
#include "WebServer.h"

WebSocketCommandHandler::WebSocketCommandHandler(IWebSocketService* pIWebSocket)
:	IWebSocketCallback(pIWebSocket)
,	_inDestructor(false)
{
}

WebSocketCommandHandler::~WebSocketCommandHandler()
{
}

void WebSocketCommandHandler::Shutdown()
{
    {
        std::lock_guard lock(_destructorLock);
        _inDestructor = true;
    }

    // We must make sure there are no outstanding UiProcessWebSocketMessage
    // or CMProcessWebSocketMessage messages
    // on the command processor (since they have a pointer to this)
    ICommonApplicationBase* pAppBase = CommonApplicationBase::Get();
    if (pAppBase)
    {
        auto pMessageQueue = pAppBase->GetMessageQueue();
        if (pMessageQueue)
            pMessageQueue->FlushMessages(this);
    }
}

void WebSocketCommandHandler::Lock()
{
    _pShutdownCritSec->lock();
}

void WebSocketCommandHandler::Unlock()
{
    _pShutdownCritSec->unlock();
}

void WebSocketCommandHandler::ReceiveWebSocketMessage(IWebSocketMessage* message)
{
    {
        std::lock_guard lock(_destructorLock);
        if (_inDestructor)
        {
            std::cout << "WebSocketCommandHandler::ReceiveWebSocketMessage received when ~WebSocketCommandHandler called\n";
            return;
        }
    }

    std::shared_ptr<IWebSocketCallback> spBase = shared_from_this();
    std::shared_ptr<WebSocketCommandHandler> spMe = std::dynamic_pointer_cast<WebSocketCommandHandler>(spBase);
    std::weak_ptr<WebSocketCommandHandler> wpMe(spMe);

    bool isText = true;
    bool finalFrame = true;
    std::vector<std::shared_ptr<ByteArray>> data = message->GetData(isText, finalFrame);
    if (data.empty())
        return;

    if (isText)
    {
        for (auto spData : data)
        {
            std::string text = (char*)spData->data();
            auto spCommandDoc = AtUtils::IJson::Create(text);

            if (!spCommandDoc)
                continue;

            auto spJsonObject = spCommandDoc->Parse();

            // If the document is empty skip it
            if (!spJsonObject)
                continue;

            // Your derived class implements Process. Do this on the main thread
            CommonApplicationBase::Get()->AddCommand(new UiProcessWebSocketMessage(wpMe, spCommandDoc));
        }
    }
    else
    {
        // Received binary block
        // Your derived class implements Process. Do this on the CoreProcessor thread

        // Receiving binary will result in the command saving the data to a file, which
        // we can't allow on the core processor thread since it will block the system.
        // Instead use the CoreMinion
        CommonApplicationBase::Get()->AddCommand(new UiProcessBinaryWebSocketMessage(wpMe, data, finalFrame));
    }
}

bool WebSocketCommandHandler::Create(IWebSocketService* pIWebSocket)
{
    std::string subProtocolString = pIWebSocket->GetSubProtocolString();
    std::shared_ptr<IWebSocketCallback> spCommandProcessor;
    AtUtils::MakeLower(subProtocolString);

    if (subProtocolString == "uicommandprocessor")
        spCommandProcessor = std::make_shared<UiWebSocketCommandProcessor>(pIWebSocket);
    else if (subProtocolString == "filetransfer")
        spCommandProcessor = std::make_shared<FileTransferWebSocketCommandProcessor>(pIWebSocket);
    else if (subProtocolString == "picture")
        spCommandProcessor = std::make_shared<PictureWebSocketCommandProcessor>(pIWebSocket);
    else if (subProtocolString == "datatransfer")
        spCommandProcessor = std::make_shared<DataTransferWebSocketCommandProcessor>(pIWebSocket);

    if (spCommandProcessor)
        pIWebSocket->SetCallback(spCommandProcessor);

    return (spCommandProcessor != nullptr);
}

void WebSocketCommandHandler::SendToClient(ONamedParamList& parameterList)
{
    JsonDOM jsonDOM(parameterList.GetParam("command").c_str());
    AtUtils::IJsonObjectPtr spCommandObject = jsonDOM.GetRoot();

    std::vector<NamedParamPair> paramPairs = parameterList.GetParamPairs();

    for (auto& paramPair : paramPairs)
    {
        if (paramPair._name != "command")
            spCommandObject->AddValue(paramPair._name.c_str(), paramPair._value);
    }

    std::shared_ptr<std::string> spXmlStr = jsonDOM.ToString();
    IWebSocketCallback::SendToClient(new TextWebSocketMessage(std::move(spXmlStr)));
}

void WebSocketCommandHandler::SendToClient(std::shared_ptr<ByteArray> binary_data)
{
    std::shared_ptr<DataSendRequest> spDataSendRequest(new DataSendRequest(std::move(binary_data)));
    IWebSocketCallback::SendToClient(new BinaryWebSocketMessage(spDataSendRequest));
}

// Commands from JavaScript client are in JSON
void WebSocketCommandHandler::Process(std::shared_ptr<AtUtils::IJson> spCommandJSON)
{
    // Command node should be the root node
    auto spRootObject = spCommandJSON->RootObject();

    auto spCommandObject = spRootObject->GetObject("Command");
    if (spCommandObject)
    {
        auto spCommandValue = spCommandObject->GetValue("value");

        if (spCommandValue)
        {
            std::string commandValueString = spCommandValue->GetValue<std::string>();

            auto spParams = spCommandObject->GetValue("params");
            auto spValueObject = spParams ? spParams->GetObject() : nullptr;

            if (spValueObject)
            {
                ONamedParamList namedParams(std::move(spValueObject), 0);
                namedParams.SetBaseCommand(commandValueString);

                ProcessCommand(commandValueString, namedParams);
            }
            else
            {
                ONamedParamList namedParams;
                namedParams.SetBaseCommand(commandValueString);

                ProcessCommand(commandValueString, namedParams);

            }
        }
    }
}

// ICommandChecker
bool WebSocketCommandHandler::TestCommandForRemoval(AtUtils::Message* command)
{
    uint32_t message_id = command->GetID();
    if (message_id == (uint32_t)CommonMessageIDs::UI_PROCESS_WEB_SOCKET_MESSAGE_CMD)
    {
        // Is it ours? There could be multiple WebSocketCommandHandler's
        UiProcessWebSocketMessage* pTextCommand = dynamic_cast<UiProcessWebSocketMessage*>(command);
        if (pTextCommand)
        {
            std::shared_ptr spCommandHandler = pTextCommand->_wpCommandHandler.lock();
            return (spCommandHandler.get() == this);
        }
    }
    else if (message_id == (uint32_t)CommonMessageIDs::UI_PROCESS_BINARY_WEB_SOCKET_MESSAGE_CMD)
    {
        // Is it ours? There could be multiple WebSocketCommandHandler's
        UiProcessBinaryWebSocketMessage* pBinaryCommand = dynamic_cast<UiProcessBinaryWebSocketMessage*>(command);
        if (pBinaryCommand)
        {
            std::shared_ptr spCommandHandler = pBinaryCommand->_wpCommandHandler.lock();
            return (spCommandHandler.get() == this);
        }
    }

    return false;
}

//////

SilentWebSocketCommandProcessor::SilentWebSocketCommandProcessor(IWebSocketService* pIWebSocket)
:	WebSocketCommandHandler(pIWebSocket)
{
}

//////
//static bool show_outgoing_traffic = false;

/////

BinaryWebSocketMessage::BinaryWebSocketMessage(std::shared_ptr<DataSendRequest>& spDataSendRequest)
:	_spDataSendRequest(spDataSendRequest)
{
}

BinaryWebSocketMessage::~BinaryWebSocketMessage()
{
}

std::vector<std::shared_ptr<ByteArray>> BinaryWebSocketMessage::GetData(bool& isText, bool& finalFrame)
{
    finalFrame = true;
    return _spDataSendRequest->GetData(isText);
}


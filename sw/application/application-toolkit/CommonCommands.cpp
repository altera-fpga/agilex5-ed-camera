/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "CommonCommands.h"
#include "CommonApplicationBase.h"
#include "WebSocketCommandProcessor.h"
#include "CommonUiLayer.h"

UiProcessWebSocketMessage::UiProcessWebSocketMessage(std::weak_ptr<WebSocketCommandHandler> wpCommandHandler,
                                                     std::shared_ptr<AtUtils::IJson>& spJsonDOM)
:	Command((uint32_t)CommonMessageIDs::UI_PROCESS_WEB_SOCKET_MESSAGE_CMD)
,	_wpCommandHandler(wpCommandHandler)
,	_spCommandDoc(spJsonDOM)
{
}

void UiProcessWebSocketMessage::PreExecute()
{
    // Called before Execute while the queue is locked,
    // So we are guaranteed access to _wpCommandHandler
    auto wpCommandHandler = _wpCommandHandler.lock();
    if (wpCommandHandler)
        wpCommandHandler->Lock();
}

void UiProcessWebSocketMessage::Execute()
{
    // Text message now in XML
    auto wpCommandHandler = _wpCommandHandler.lock();
    if (wpCommandHandler)
        wpCommandHandler->Process(_spCommandDoc);
}

void UiProcessWebSocketMessage::PostExecute()
{
    // Called after Execute while the queue is not locked,
    // But we have locked _wpCommandHandler in PreExecute,
    // So we are guaranteed access to _wpCommandHandler
    auto wpCommandHandler = _wpCommandHandler.lock();
    if (wpCommandHandler)
        wpCommandHandler->Unlock();
}

////////////////////////////////////////////////////////////
UiProcessBinaryWebSocketMessage::UiProcessBinaryWebSocketMessage(std::weak_ptr<WebSocketCommandHandler> wpCommandHandler,
                                                                 std::vector<std::shared_ptr<ByteArray>>& binaryData,
                                                                 bool finalFrame)
:	Command((uint32_t)CommonMessageIDs::UI_PROCESS_BINARY_WEB_SOCKET_MESSAGE_CMD)
,	_wpCommandHandler(wpCommandHandler)
,	_binaryData(binaryData)
,	_finalFrame(finalFrame)
{
}

void UiProcessBinaryWebSocketMessage::PreExecute()
{
    // Called before Execute while the queue is locked,
    // So we are guaranteed access to _wpCommandHandler
    auto wpCommandHandler = _wpCommandHandler.lock();
    if (wpCommandHandler)
        wpCommandHandler->Lock();
}

void UiProcessBinaryWebSocketMessage::Execute()
{
    // Binary message
    auto wpCommandHandler = _wpCommandHandler.lock();
    if (wpCommandHandler)
        wpCommandHandler->Process(_binaryData, _finalFrame);
}

void UiProcessBinaryWebSocketMessage::PostExecute()
{
    // Called after Execute while the queue is not locked,
    // But we have locked _wpCommandHandler in PreExecute,
    // So we are guaranteed access to _wpCommandHandler
    auto wpCommandHandler = _wpCommandHandler.lock();
    if (wpCommandHandler)
        wpCommandHandler->Unlock();
}





/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "AtUtils.h"
#include "CommonMessageIDs.h"
#include "CommonTypes.h"

class ICommonApplicationBase;

using Command = AtUtils::Message;

class WebSocketCommandHandler;

class UiProcessWebSocketMessage : Command
{
    friend class WebSocketCommandHandler;

public:
    UiProcessWebSocketMessage(std::weak_ptr<WebSocketCommandHandler> wpCommandHandler,
                              std::shared_ptr<AtUtils::IJson>& spJsonDOM);
    void PreExecute() override;
    void Execute() override;
    void PostExecute() override;

private:
    std::weak_ptr<WebSocketCommandHandler> _wpCommandHandler;
    std::shared_ptr<AtUtils::IJson> _spCommandDoc;
};

class UiProcessBinaryWebSocketMessage : public Command
{
    friend class WebSocketCommandHandler;

public:
    UiProcessBinaryWebSocketMessage(std::weak_ptr<WebSocketCommandHandler> wpCommandHandler,
                                    std::vector<std::shared_ptr<ByteArray>>& binaryData,
                                    bool finalFrame);
    void PreExecute() override;
    void Execute() override;
    void PostExecute() override;

private:
    std::weak_ptr<WebSocketCommandHandler>    _wpCommandHandler;
    std::vector<std::shared_ptr<ByteArray>> _binaryData;
    bool _finalFrame;
};


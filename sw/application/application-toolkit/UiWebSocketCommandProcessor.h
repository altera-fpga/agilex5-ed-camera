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
#include "AtUtils.h"
#include "IHttpServer.h"
#include "ICommand.h"
#include "ObserverPattern.h"
#include "CommonApplicationBase.h"

class UiUpdate;
class xml_document;
class CoreProcessor;
class ParamList;
using ParamListPtr = std::shared_ptr<ParamList>;

class UiWebSocketCommandProcessor : public WebSocketCommandHandler,
                                    public Observer<UiUpdate>
{
public:
    UiWebSocketCommandProcessor(IWebSocketService* pIWebSocket);
    ~UiWebSocketCommandProcessor();
    void DoUnregister() override {}

    void Process(std::shared_ptr<AtUtils::IJson> commandDoc) override;
    void Ready() override;

    // Observer updates
    bool Update(UiUpdate& data) override;

    uint32_t GetClientID() { return _clientID; }
    static uint32_t GetLastClientID();

private:
    void AddCommands();
    bool ProcessCommand(const std::string& commandValue, ParamListPtr& spParameterList);
    uint32_t _clientID;

    CommandMap				_clientCommandMap; // Commands specific to this client connection
    std::string				_hostname;
    AtUtils::Event			_commandsReady;
    static std::recursive_mutex _nextClientIdCs;
    static uint32_t         _nextClientID;
    static uint32_t         _lastClientID;

public:
    // Command callback functions
    void GetMainUserInterfaceCB(ParamListPtr& parameterList);
    void UiTraceCB(ParamListPtr& parameterList);
    void DialogResultCB(ParamListPtr& parameterList);
};

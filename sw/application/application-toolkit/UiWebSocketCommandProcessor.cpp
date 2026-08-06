/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "UiWebSocketCommandProcessor.h"
#include "ParamList.h"
#include "CommonUiUpdate.h"
#include "CommonUiLayer.h"

UiWebSocketCommandProcessor::UiWebSocketCommandProcessor(IWebSocketService* pIWebSocket)
:	WebSocketCommandHandler(pIWebSocket)
,	_clientID(0)
{
}

UiWebSocketCommandProcessor::~UiWebSocketCommandProcessor()
{
    Observer<UiUpdate>::Unregister();
}

std::recursive_mutex UiWebSocketCommandProcessor::_nextClientIdCs;
uint32_t UiWebSocketCommandProcessor::_nextClientID = 100;
uint32_t UiWebSocketCommandProcessor::_lastClientID = 0;

void UiWebSocketCommandProcessor::Ready()
{
    // Register for UI updates
    Observer<UiUpdate>::Register(CommonApplicationBase::Get()->GetUiNotifier().get());

    // Send a command to the client telling him his client id
    {
        std::lock_guard lock(_nextClientIdCs);
        _clientID = _nextClientID++;
        _lastClientID = _clientID;
    }

    AddCommands();
}

uint32_t UiWebSocketCommandProcessor::GetLastClientID()
{
    std::lock_guard lock(_nextClientIdCs);
    return _lastClientID;
}


// Add our personal commands
#define CommandCB OIServerCommandHelper<UiWebSocketCommandProcessor>

void UiWebSocketCommandProcessor::AddCommands()
{
    ICommandMap& map = _clientCommandMap;

    map.Add(new UiServerCommand("GetMainUserInterface", [this](ParamListPtr& parameterList) { GetMainUserInterfaceCB(parameterList); }));
    map.Add(new UiServerCommand("UiTrace", [this](ParamListPtr& parameterList) { UiTraceCB(parameterList); }));
    map.Add(new UiServerCommand("DialogResult", [this](ParamListPtr& parameterList) { DialogResultCB(parameterList); }));

    _commandsReady.Set();
}

// Commands from JavaScript client are in JSON
void UiWebSocketCommandProcessor::Process(std::shared_ptr<AtUtils::IJson> spCommandJSON)
{
    _commandsReady.Wait();

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

            if (spParams)
            {
                std::string paramsValueString = spParams->GetValue<std::string>();

                // Split the paramsValueString string into ',' separated strings
                auto paramList = std::make_shared<ParamList>(paramsValueString, _clientID);
                paramList->SetBaseCommand(commandValueString);
                paramList->SetJsonData(spCommandJSON, spCommandObject->GetObject("CommandData"));

                ProcessCommand(commandValueString, paramList);
            }
        }
        else
        {
            spCommandValue = spCommandObject->GetValue("value_with_params");

            if (spCommandValue)
            {
                std::string commandValueString = spCommandValue->GetValue<std::string>();

                // Split the params_value string into ',' separated strings
                auto paramList = std::make_shared<ParamList>(commandValueString, _clientID);
                commandValueString = paramList->ExtractBaseCommand();
                paramList->SetJsonData(spCommandJSON, spCommandObject->GetObject("CommandData"));
                ProcessCommand(commandValueString, paramList);
            }
        }
    }
}

bool UiWebSocketCommandProcessor::ProcessCommand(const std::string& commandValue,
                                                 ParamListPtr& spParameterList)
{
    UiServerCommand* pServerCommand = _clientCommandMap.LookupCommand(commandValue);

    if (pServerCommand)
    {
        // Found a command that is for this client connection
        pServerCommand->Execute(spParameterList);
        return true;
    }

    ICommonApplicationBase* pCoreApp = CommonApplicationBase::Get();
    ICommandMap* pServerCommandMap = pCoreApp->GetCommandMap();
    pServerCommand = pServerCommandMap->LookupCommand(commandValue);

    if (pServerCommand)
    {
        // Found a system wide command

        // Delegate to the main application message queue so all commands from
        // different clients get serialized
        ParamListPtr spParameterListCopy = spParameterList;
        auto executeOnAppQueueCB = [pServerCommand, spParam = std::move(spParameterListCopy)]() { pServerCommand->Execute(spParam); };
        pCoreApp->AddCommand(executeOnAppQueueCB);
        return true;
    }

    return false;
}

bool UiWebSocketCommandProcessor::Update(UiUpdate& data)
{
    // Some commands don't need to be propagated back to their source
    // client (eg window moves)
    if (!data.IsEmpty() && data.UpdateClient(_clientID))
    {
        std::shared_ptr<std::string> spTextString = data.GetJsonString();
        if (spTextString)
            _pIWebSocket->SendToClient(new TextWebSocketMessage(std::move(spTextString)));
    }

    return true;
}

//////////////////

void UiWebSocketCommandProcessor::GetMainUserInterfaceCB(ParamListPtr& spParameterList)
{
    std::shared_ptr<std::string> mainUiXml = CommonUiLayer::Get()->GetMainUserInterfaceXml(spParameterList->GetClientID());
    if (!mainUiXml->empty())
    {
        TextWebSocketMessage* pWebSocketMessage = new TextWebSocketMessage(std::move(mainUiXml));
        _pIWebSocket->SendToClient(pWebSocketMessage);
    }
}

void UiWebSocketCommandProcessor::UiTraceCB(ParamListPtr& spParameterList)
{
}

void UiWebSocketCommandProcessor::DialogResultCB(ParamListPtr& spParameterList)
{
    if (spParameterList->size() > 0)
    {
        uint32_t buttonIndex = 0;
        AtUtils::FromString<uint32_t>(spParameterList->at(0), buttonIndex);
        bool okButton = (buttonIndex == 0);

        ModalDialogUiUpdate::CallCloseCallback(spParameterList->GetClientID(), okButton);
    }
}

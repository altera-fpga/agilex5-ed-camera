/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>

class ParamList;
using ParamListPtr = std::shared_ptr<ParamList>;
class ICommandMap;

using UiServerCB = std::function<void(ParamListPtr& parameterList)>;

class UiServerCommand
{
public:
    UiServerCommand(const std::string& callbackName, UiServerCB callbackFunction)
    :   _callbackName(callbackName)
    ,   _callbackFunction(std::move(callbackFunction))
    {
    }

    const std::string& GetCallbackName() { return _callbackName; }

    void Execute(ParamListPtr parameter_list);

private:
    std::string _callbackName;
    UiServerCB  _callbackFunction;
};

// Map of string to command. Each UiWebSocketCommandProcessor will have one
// (one for each client) and the OServer has one for system wide commands

class ICommandMap
{
public:
    virtual ~ICommandMap() {}
    virtual bool Remove(const std::string& command) = 0;
    virtual bool Add(UiServerCommand* command) = 0;
    virtual UiServerCommand* LookupCommand(const std::string& command_value) = 0;
};

class CommandMap : public ICommandMap,
                   public std::unordered_map<std::string, UiServerCommand*>
{
public:
    CommandMap();
    ~CommandMap();

    bool Remove(const std::string& command) override;
    bool Add(UiServerCommand* command) override;
    UiServerCommand* LookupCommand(const std::string& command_value) override;

private:
    std::recursive_mutex _cs;
};

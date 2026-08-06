/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "AtUtils.h"
#include "ICommand.h"
#include "ParamList.h"

void UiServerCommand::Execute(ParamListPtr parameterList)
{
    _callbackFunction(parameterList);
}

///////////////////////////////////////////////////////////////////////////////

CommandMap::CommandMap()
{
}

CommandMap::~CommandMap()
{
    std::lock_guard lock(_cs);

    // Delete the actual commands
    for (auto& mapEntry : *this)
    {
        UiServerCommand* command = mapEntry.second;
        delete command;
    }
}

bool CommandMap::Remove(const std::string& command)
{
    std::lock_guard lock(_cs);
    bool removed = AtUtils::Remove<std::string, UiServerCommand*>(*this, command);
    return removed;
}

bool CommandMap::Add(UiServerCommand* command)
{
    std::lock_guard lock(_cs);
    const std::string& baseName = command->GetCallbackName();

    if (baseName.empty())
    {
        delete command;
        return false;
    }

    // You need to assign _base_command name in
    // the constructor of your derived ICommand class
    UiServerCommand* temp = nullptr;
    if (AtUtils::Lookup<std::string, UiServerCommand*>(*this, baseName, temp))
    {
        // You must have a unique baseName
        assert(0);
        delete command;
        return false;
    }

    operator[](baseName) = command;
    return true;
}

UiServerCommand* CommandMap::LookupCommand(const std::string& command_value)
{
    std::lock_guard lock(_cs);
    UiServerCommand* found_command = nullptr;
    AtUtils::Lookup<std::string, UiServerCommand*>(*this, command_value, found_command);
    return found_command;
}

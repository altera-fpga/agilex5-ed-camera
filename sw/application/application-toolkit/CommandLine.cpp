/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "CommandLine.h"
#include "AtUtils.h"

// Program -option=value
CommandLine::CommandLine(int argumentCount, char* argumentValues[])
{
    if (argumentCount > 0)
        _executableName = argumentValues[0];

    for (int i = 1; i < argumentCount; i++)
    {
        // Trace(TL_MSG, "<%s>\n", argumentValues[i]);
        std::string inputString(argumentValues[i]);
        if ((AtUtils::Left(inputString, 1) == "-") || (AtUtils::Left(inputString, 1) == "/"))
        {
            inputString = AtUtils::Mid(inputString, 1);
            size_t equals = inputString.find("=");
            std::string option;
            std::string value;

            if (equals == std::string::npos)
            {
                option = std::move(inputString);
            }
            else
            {
                option = AtUtils::Left(inputString, equals);
                value = AtUtils::Mid(inputString, equals + 1);
            }

            AtUtils::TrimRight(option);
            AtUtils::TrimLeft(value);
            AtUtils::MakeLower(option);
            _optionMap[option] = std::move(value);
        }
    }
}

std::string CommandLine::GetOptionValue(const char* optionName)
{
    std::string value;
    AtUtils::Lookup<std::string, std::string>(_optionMap, optionName, value);
    return value;
}

bool CommandLine::HaveOption(const char* optionName)
{
    std::string value;
    return AtUtils::Lookup<std::string, std::string>(_optionMap, optionName, value);
}

bool CommandLine::GetOption(const char* optionName, std::string& optionValue)
{
    std::string value;
    if (AtUtils::Lookup<std::string, std::string>(_optionMap, optionName, value))
    {
        optionValue = std::move(value);
        return true;
    }
    else
        return false;
}

size_t CommandLine::NumOptions()
{
    return _optionMap.size();
}

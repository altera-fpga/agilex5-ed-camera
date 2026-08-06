/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "CommandLineFlags.h"

const std::vector<const char*> CommandLineFlags::UsageHelp = {
    "usage", "help", "h"
};

bool CommandLineFlags::HaveOption(std::shared_ptr<CommandLine> cmd, const char* opt) 
{
    return cmd->HaveOption(opt);
}

bool CommandLineFlags::HaveOption(std::shared_ptr<CommandLine> cmd, const std::vector<const char*>& choices) 
{
    return HaveAnyOf(std::move(cmd), choices);
}

bool CommandLineFlags::HaveAnyOf(std::shared_ptr<CommandLine> cmd, const std::vector<const char*>& choices) 
{
    for (auto& opt : choices) 
    {
        if (CommandLineFlags::HaveOption(cmd, opt)) 
        {
            return true;
        }
    }
    return false;
}

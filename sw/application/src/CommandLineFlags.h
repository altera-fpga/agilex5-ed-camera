/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include "CommandLine.h"
#include <memory>
#include <vector>

struct CommandLineFlags 
{
    static constexpr char IgnoreFailedHdmiInit[] = "ignore-failed-hdmi-init";
    static constexpr char IgnoreFailedCoreInit[] = "ignore-failed-core-init";
    static constexpr char PrintCompileConfig[] = "print-compile-config";
    static constexpr char ShowI2CAccesses[] = "show-i2c-accesses";
    static constexpr char DebugUi[] = "debug-ui";
    static constexpr char PowerUser[] = "pu";
    static const std::vector<const char*> UsageHelp;

    static bool HaveOption(std::shared_ptr<CommandLine> cmd, const char* opt);
    static bool HaveOption(std::shared_ptr<CommandLine> cmd, const std::vector<const char*>& choices);
    static bool HaveAnyOf(std::shared_ptr<CommandLine> cmd, const std::vector<const char*>& choices);
};

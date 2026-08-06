/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "CommonApplicationBase.h"
#include "AiPipeline.h"

#include "AIRuntimeControls.h"

#include "Hapi.h"
#include "Logging.h"



class AiTabbedUiControls
{
    public:
        AiTabbedUiControls(std::function<std::shared_ptr<TopLevelUiTab>(const std::string&, BooleanControlCB, const std::string&)> appCreateUiTabCB,
                            const std::shared_ptr<AiPipeline>& spAiPipeline, bool debugMode, bool powerUser);
        ~AiTabbedUiControls() {};

        void CreateCoreDlaRuntimeTab();

    private:
        std::function<std::shared_ptr<TopLevelUiTab>(const std::string&, BooleanControlCB, const std::string&)> _appCreateUiTabCB;

        std::shared_ptr<TopLevelUiTab> _spAIRuntimeTab;
            std::shared_ptr<AIRuntimeControls> _spAIRuntime;

        std::shared_ptr<AiPipeline> _spAiPipeline;

        bool _debugMode = false;
        bool _powerUser = false;
        std::function<void(uint32_t)> _warpCaptureCB = nullptr;
};

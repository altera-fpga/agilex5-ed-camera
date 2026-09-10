/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "AiTabbedUiControls.h"

#include "VvpIspDemo.h"

AiTabbedUiControls::AiTabbedUiControls(std::function<std::shared_ptr<TopLevelUiTab>(const std::string&, BooleanControlCB, const std::string&)> appCreateUiTabCB,
                                        const std::shared_ptr<AiPipeline>& spAiPipeline, bool debugMode, bool powerUser)
    : _appCreateUiTabCB(std::move(appCreateUiTabCB)),
      _spAiPipeline(spAiPipeline),
      _debugMode(debugMode),
      _powerUser(powerUser)
{
}

void AiTabbedUiControls::CreateCoreDlaRuntimeTab()
{
    auto aiRuntimeTabControlsEnabledCB = [this](uint32_t clientID, bool enabled)
    {
        if (enabled)
        {
            // Do nothing
        }
        else
        {
            // Also do nothing, the freedom of choice is yours
        }
    };

    _spAIRuntimeTab = _appCreateUiTabCB("AI runtime", aiRuntimeTabControlsEnabledCB, "AI runtime controls.");
    _spAIRuntime = _spAIRuntimeTab->AddHapiControl<AIRuntimeControls>(_spAiPipeline->GetCoreDlaRuntime(), _spAiPipeline->GetAiResultsRenderer(), VvpIspDemo::Get(), _powerUser);
}

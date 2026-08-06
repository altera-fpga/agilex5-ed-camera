/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "SwitchControls.h"
#include "SwitchUtils.h"
#include "UiElements.h"

#include "AtUtils.h"

using namespace SwApi;

#if defined(__linux__)
#include <unistd.h>
#endif

SwitchControls::SwitchControls(const std::shared_ptr<SwApi::ISwitch>& spSwitch,
                                const std::string& name)
    : _spSwitch(spSwitch)
    , _name(name)
{
    if (_spSwitch) {
        // Initial setup here?
    } else {

    }
}

std::vector<std::shared_ptr<UiControlContainer>> SwitchControls::AddUiElements() {
    if (not _spSwitch) {
        return {};
    }

    auto spContainer = std::make_shared<UiControlContainer>(_name,
                                                            GetSettingsSectionName());

    //region Input selection drop-down
    auto inputCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) {
        auto chosenInput = (uint8_t)selected._userItemData;
        if (not _spSwitch->ConnectOnly(chosenInput, 0)) {
            std::cout << "Failed to connect.";
        }
    };

    uint32_t inputs = _spSwitch->GetNumberOfInputsAvailable();
    std::vector<UiEnumOption> inputOptions;
    for (uint32_t i = 0; i < inputs; i++) {
        inputOptions.emplace_back(AtUtils::ToString(i), i);
    }

    _spInputSelectControl = spContainer->AddEnumControl("Input select",
                                                        inputOptions,
                                                        inputCB,
                                                        "Input",
                                                        0);
    //endregion

    return {std::move(spContainer)};
}

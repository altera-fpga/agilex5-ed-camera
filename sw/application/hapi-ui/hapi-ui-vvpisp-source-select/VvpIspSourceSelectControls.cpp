/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "VvpIspSourceSelectControls.h"
#include "SwitchUtils.h"
#include "UiElements.h"

#include "AtUtils.h"


VvpIspSourceSelectControls::VvpIspSourceSelectControls(const std::string& name,
                                                        const std::vector<std::string>& inputOpts,
                                                        std::function<void(const uint32_t)> inputSelectCb):
    _name(name),
    _inputOpts{inputOpts},
    _inputSelectCb{std::move(inputSelectCb)},
    _spInputSelectControl{nullptr}
{
}


std::vector<std::shared_ptr<UiControlContainer>> VvpIspSourceSelectControls::AddUiElements()
{
    if(_inputOpts.empty())
        return {};

    std::vector<UiEnumOption> uiEnumOpts{};

    for(std::size_t idx = 0; idx < _inputOpts.size(); ++idx)
        uiEnumOpts.emplace_back(_inputOpts[idx], idx);

    auto spContainer = std::make_shared<UiControlContainer>(_name, GetSettingsSectionName());

    auto uiEnumCb = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t){
        if(_inputSelectCb){
            const auto idx = selected._userItemData;
            _inputSelectCb(idx);
        }
    };


    _spInputSelectControl = spContainer->AddEnumControl("Input Select",
                                                        uiEnumOpts,
                                                        uiEnumCb,
                                                        "Input",
                                                        1);

    return {std::move(spContainer)};
}


void VvpIspSourceSelectControls::ApplySelectedInput()
{
    const uint32_t idx =  _spInputSelectControl->GetSelectedIndex();
    _spInputSelectControl->UpdateValue(idx, true);
}

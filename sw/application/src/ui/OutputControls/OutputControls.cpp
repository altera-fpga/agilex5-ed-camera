/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "OutputControls.h"
#include "dptx_formats.h"


OutputControls::OutputControls(const std::string& name,
    const std::vector<std::string>& sourceOpts,
    std::function<void(const uint32_t)> sourceCb,    
    const std::vector<std::string>& overrideOpts,
    std::function<void(const uint32_t, const bool)> overrideCb,
    bool powerUser):
    _name(name),
    _sourceOpts{sourceOpts},
    _sourceCb{std::move(sourceCb)},
    _overrideOpts{overrideOpts},
    _overrideCb{std::move(overrideCb)},
    _spOverrideControl{nullptr},
    _bpc10Support{false},
    _powerUser{powerUser}
{
}


std::vector<std::shared_ptr<UiControlContainer>> OutputControls::AddUiElements() 
{
    auto spContainer = std::make_shared<UiControlContainer>(_name, GetSettingsSectionName());

    if(!_sourceOpts.empty())
    {
        std::vector<UiEnumOption> uiEnumOpts{};

        for(std::size_t idx = 0; idx < _sourceOpts.size(); ++idx)
            uiEnumOpts.emplace_back(_sourceOpts[idx], idx);

        auto uiEnumCb = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t){
            if(_sourceCb){
                const auto idx = selected._userItemData;
                _sourceCb(idx);
            }
        };

        _spSourceSelectControl = spContainer->AddEnumControl("Source",
            uiEnumOpts,
            uiEnumCb,
            "OutputSource",
            0);
    }

    _spCurrentFormatLabel = spContainer->AddLabelControl("Output format:", "N/A");

    if(_powerUser)
    {
        _spStatusLabel = spContainer->AddLabelControl("Status:", "0x0");

        if(!_overrideOpts.empty())
        {
            std::vector<UiEnumOption> uiEnumOpts{};

            for(std::size_t idx = 0; idx < _overrideOpts.size(); ++idx)
                uiEnumOpts.emplace_back(_overrideOpts[idx], idx);

            auto uiEnumCb = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t){
                if(_overrideCb){
                    const auto idx = selected._userItemData;
                    _overrideCb(idx, _bpc10Support);
                }
            };

            _spOverrideControl = spContainer->AddEnumControl("Format override",
                uiEnumOpts,
                uiEnumCb,
                "FormatOverride", 0);
        }
    }

    return {std::move(spContainer)};
}


void OutputControls::UpdateStatus(const uint32_t status, const std::string& format_str, const std::vector<uint32_t>& format_opts, const bool bpc10_support)
{
    // Current output format
    _spCurrentFormatLabel->UpdateValue(format_str);

    if(_powerUser)
    {
        // Current DP TX status
        _spStatusLabel->UpdateValue(std::format("{:#010x}", status));

        //const bool bpc10 = false;

        // Update output format override dropdown box
        std::vector<UiEnumOption> overrideOptions = {};

        for(std::size_t idx = 0; idx < format_opts.size(); ++idx)
        {
            const auto format = dptx_formats_get(format_opts[idx]);

            if(format)
                overrideOptions.emplace_back(format->str, format_opts[idx] + 1); // +1 to avoid 0 value which is "Not set"
        }

        // Get currently selected option
        const auto prev_index = _spOverrideControl->GetSelectedIndex();
        const auto prev_option = _spOverrideControl->GetOption(prev_index);
        const auto prev_user_data = prev_option._userItemData;

        // Remove old options
        while(_spOverrideControl->GetLength() > 1)
        {
            std::size_t index = _spOverrideControl->GetLength() - 1;
            const auto option = _spOverrideControl->GetOption(index);
            _spOverrideControl->RemoveEnumItem(index, option._label);
        }

        // Add new override options
        // If the new set supports previously selected override
        // then restore it otherwise default to "Not set"
        uint32_t next_user_data = 0;

        for(std::size_t idx = 0; idx < overrideOptions.size(); ++idx)
        {
            const auto& opt = overrideOptions[idx];

            _spOverrideControl->AddEnumItem(opt._label, opt._userItemData);

            if(prev_user_data == opt._userItemData)
            {
                next_user_data = prev_user_data;
            }
        }

        _spOverrideControl->UpdateValue(next_user_data);

        _bpc10Support = bpc10_support;
    }
}

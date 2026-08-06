/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include "HapiVvpMixer.h"
#include <string>

namespace SwApi
{

    enum class MixerLayerState
    {
        MixerLayerDisabled,
        MixerLayerConsumed,
        MixerLayerEnabled,
        MixerLayerEnabledSoftStart,
        MixerLayerConsumedEnabledSoftStart,
        MixerLayerInvalid
    };

    std::string ToString(const MixerLayerState& inputConfig);

    enum class MixerBlendMode
    {
        MixerBlendModeTransparent = kIntelVvpMixerBlendTransparent,
        MixerBlendModeOpaque = kIntelVvpMixerBlendOpaque,
        MixerBlendModeStaticAlpha = kIntelVvpMixerBlendStaticAlpha,
        MixerBlendModeInputAlpha = kIntelVvpMixerBlendInputAlpha
    };

    std::string ToString(const MixerBlendMode& inputBlendMode);

    enum class MixerBlendCaps
    {
        MixerBlendCapsNone = kIntelVvpMixerBlendConfigNone,
        MixerBlendCapsStaticAlphaOnly = kIntelVvpMixerBlendConfigStaticAlpha,
        MixerBlendCapsInputAlphaOnly = kIntelVvpMixerBlendConfigInputAlpha,
        MixerBlendCapsBoth = kIntelVvpMixerBlendConfigStaticOrInputAlpha
    };

    std::string ToString(const MixerBlendCaps& inputBlendCaps);

} // namespace SwApi

std::ostream& operator<<(std::ostream& o, const SwApi::MixerLayerState& inputConfig);
std::ostream& operator<<(std::ostream& o, const SwApi::MixerBlendMode& inputBlendMode);
std::ostream& operator<<(std::ostream& o, const SwApi::MixerBlendCaps& inputBlendCaps);

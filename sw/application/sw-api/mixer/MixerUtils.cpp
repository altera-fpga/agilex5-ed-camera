/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "MixerUtils.h"

#include <string>
#include <sstream>

std::string SwApi::ToString(const SwApi::MixerLayerState& inputConfig)
{
    switch (inputConfig)
    {
        case SwApi::MixerLayerState::MixerLayerDisabled:
        {
            return "MixerLayerDisabled";
            break;
        }
        case SwApi::MixerLayerState::MixerLayerConsumed:
        {
            return "MixerLayerConsumed";
            break;
        }
        case SwApi::MixerLayerState::MixerLayerEnabled:
        {
            return "MixerLayerEnabled";
            break;
        }
        case SwApi::MixerLayerState::MixerLayerEnabledSoftStart:
        {
            return "MixerLayerEnabledSoftStart";
            break;
        }
        case SwApi::MixerLayerState::MixerLayerConsumedEnabledSoftStart:
        {
            return "MixerLayerConsumedEnabledSoftStart";
            break;
        }
        case SwApi::MixerLayerState::MixerLayerInvalid:
        default:
        {
            return "MixerLayerInvalid";
            break;
        }
    }
}

std::string SwApi::ToString(const SwApi::MixerBlendMode& inputBlendMode)
{
    switch (inputBlendMode)
    {
        case SwApi::MixerBlendMode::MixerBlendModeOpaque:
        {
            return "MixerBlendModeOpaque";
            break;
        }
        case SwApi::MixerBlendMode::MixerBlendModeInputAlpha:
        {
            return "MixerBlendModeInputAlpha";
            break;
        }
        case SwApi::MixerBlendMode::MixerBlendModeStaticAlpha:
        {
            return "MixerBlendModeStaticAlpha";
            break;
        }
        case SwApi::MixerBlendMode::MixerBlendModeTransparent:
        default:
        {
            return "MixerBlendModeTransparent";
            break;
        }
    }
}

std::string ToString(const SwApi::MixerBlendCaps& inputBlendCaps)
{
    switch (inputBlendCaps)
    {
        case SwApi::MixerBlendCaps::MixerBlendCapsBoth:
        {
            return "MixerBlendCapsBoth";
            break;
        }
        case SwApi::MixerBlendCaps::MixerBlendCapsStaticAlphaOnly:
        {
            return "MixerBlendCapsStaticAlphaOnly";
            break;
        }
        case SwApi::MixerBlendCaps::MixerBlendCapsInputAlphaOnly:
        {
            return "MixerBlendCapsInputAlphaOnly";
            break;
        }
        case SwApi::MixerBlendCaps::MixerBlendCapsNone:
        default:
        {
            return "MixerBlendCapsNone";
            break;
        }
    }
}

std::ostream& operator<<(std::ostream& o, const SwApi::MixerLayerState& inputConfig)
{
    return o << SwApi::ToString(inputConfig);
}

std::ostream& operator<<(std::ostream& o, const SwApi::MixerBlendMode& inputBlendMode)
{
    return o << SwApi::ToString(inputBlendMode);
}

std::ostream& operator<<(std::ostream& o, const SwApi::MixerBlendCaps& inputBlendCaps)
{
    return o << SwApi::ToString(inputBlendCaps);
}
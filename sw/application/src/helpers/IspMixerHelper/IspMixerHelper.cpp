/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "IspMixerHelper.h"
#include <cassert>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include <random>


IspMixerHelper::IspMixerHelper(const std::shared_ptr<SwApi::IMixer>& spMixer, const bool hasOverlay, const bool hasLogo):
    SwUtils::Thread("IspLogoBouncing"),
    OVERLAY_LAYER(INVALID_LAYER),
    LOGO_LAYER(INVALID_LAYER),
    _mode{Mode::Invalid},
    _ispEnabled{false},
    _ispHidden{false},
    _overlayEnabled{false},
    _overlayHidden{false},
    _logoEnabled{false},
    _logoHidden{false},
    _logoWidth{LOGO_WIDTH},
    _logoHeight{LOGO_HEIGHT},
    _logoPosition{LogoPosition::TopRight},
    _spMixer{spMixer}
{
    assert(_spMixer != nullptr);

    uint32_t next_layer = ISP_LAYER+1;
    if(hasOverlay)
    {
        OVERLAY_LAYER = next_layer;
        next_layer++;
    }

    if(hasLogo)
    {
        LOGO_LAYER = next_layer;
        next_layer++;
    }

    _spMixer->SetOutputWidth(OUTPUT_WIDTH_DEFAULT);
    _spMixer->SetOutputHeight(OUTPUT_HEIGHT_DEFAULT);

    _spMixer->SetLayerResolution(ISP_LAYER, 1920, 1080);
    _spMixer->SetLayerOffsetPosition(ISP_LAYER, 0, 0);
    _spMixer->SetLayerBlendMode(ISP_LAYER, SwApi::MixerBlendMode::MixerBlendModeOpaque);
    UpdateMixerLayerVisibility(ISP_LAYER);

    // Overlay is on layer isp_layers+1 if overlay eenabled
    if(OVERLAY_LAYER != INVALID_LAYER)
    {
        _spMixer->SetLayerResolution(OVERLAY_LAYER, OUTPUT_WIDTH_DEFAULT, OUTPUT_HEIGHT_DEFAULT);
        _spMixer->SetLayerOffsetPosition(OVERLAY_LAYER, 0, 0);
        _spMixer->SetLayerBlendMode(OVERLAY_LAYER, SwApi::MixerBlendMode::MixerBlendModeInputAlpha);
        UpdateMixerLayerVisibility(OVERLAY_LAYER);
    }

    // Logo is on layer isp_layers+1 (layer isp_layers+2 if overlay eenabled)
    if(LOGO_LAYER != INVALID_LAYER)
    {
        _spMixer->SetLayerResolution(LOGO_LAYER, LOGO_WIDTH, LOGO_HEIGHT);
        _spMixer->SetLayerOffsetPosition(LOGO_LAYER, 0, 0);
        _spMixer->SetLayerStaticAlphaViaDouble(LOGO_LAYER, 10, 0.5);
        _spMixer->SetLayerBlendMode(LOGO_LAYER, SwApi::MixerBlendMode::MixerBlendModeInputAlpha);
        UpdateMixerLayerVisibility(LOGO_LAYER);
    }
}


IspMixerHelper::~IspMixerHelper()
{
    StopThread();
}


void IspMixerHelper::UpdateMixerLayerVisibility(const uint32_t layer_id)
{
    auto layerState = SwApi::MixerLayerState::MixerLayerConsumed;

    if(layer_id == ISP_LAYER)
    {
        if(_ispEnabled && (!_ispHidden))
            layerState = SwApi::MixerLayerState::MixerLayerEnabledSoftStart;
        else
            layerState = SwApi::MixerLayerState::MixerLayerConsumedEnabledSoftStart;

        _spMixer->SetLayerEnableState(ISP_LAYER, layerState);
    }
    else if (layer_id == OVERLAY_LAYER)
    {
        if(OVERLAY_LAYER != INVALID_LAYER)
        {
            if(_overlayEnabled && (!_overlayHidden))
                layerState = SwApi::MixerLayerState::MixerLayerEnabledSoftStart;

            _spMixer->SetLayerEnableState(OVERLAY_LAYER, layerState);
        }
    }
    else if (layer_id == LOGO_LAYER)
    {
        if(_logoEnabled && (!_logoHidden))
            layerState = SwApi::MixerLayerState::MixerLayerEnabledSoftStart;

        _spMixer->SetLayerEnableState(LOGO_LAYER, layerState);
    }
}


void IspMixerHelper::EnableIsp(bool v)
{
    _ispEnabled = v;
    UpdateMixerLayerVisibility(ISP_LAYER);
}


bool IspMixerHelper::IsIspEnabled() const 
{
    return _ispEnabled;
}


void IspMixerHelper::SetIspResolution(const uint32_t width, const uint32_t height)
{
    _spMixer->SetLayerResolution(ISP_LAYER, width, height);
}

void IspMixerHelper::HideIsp(bool v)
{
    _ispHidden = v;
    UpdateMixerLayerVisibility(ISP_LAYER);
}


void IspMixerHelper::EnableLogo(bool v)
{
    if(LOGO_LAYER != INVALID_LAYER)
    {
        _logoEnabled = v;
        UpdateMixerLayerVisibility(LOGO_LAYER);
    }
}


void IspMixerHelper::HideLogo(bool v)
{
    if(LOGO_LAYER != INVALID_LAYER)
    {
        _logoHidden = v;
        UpdateMixerLayerVisibility(LOGO_LAYER);
    }
}



bool IspMixerHelper::IsLogoEnabled() const 
{
    return _logoEnabled;
}


void IspMixerHelper::EnableOverlay(bool v)
{
    if(OVERLAY_LAYER != INVALID_LAYER)
    {
        _overlayEnabled = v;
        UpdateMixerLayerVisibility(OVERLAY_LAYER);
    }
}


void IspMixerHelper::HideOverlay(bool v)
{
    if(OVERLAY_LAYER != INVALID_LAYER)
    {
        _overlayHidden = v;
        UpdateMixerLayerVisibility(OVERLAY_LAYER);
    }
}


bool IspMixerHelper::IsOverlayEnabled() const 
{
    return _overlayEnabled;
}


void IspMixerHelper::SetLogoResolution(const uint32_t width, const uint32_t height)
{
    _logoWidth = width;
    _logoHeight = height;
    _spMixer->SetLayerResolution(LOGO_LAYER, width, height);
}


void IspMixerHelper::SetLogoPositionXY(const uint32_t x, const uint32_t y)
{
    if(LOGO_LAYER != INVALID_LAYER)
    {
        _spMixer->SetLayerOffsetPosition(LOGO_LAYER, x, y);
    }
}


void IspMixerHelper::SetLogoPosition(const LogoPosition& logoPosition)
{
    if(LOGO_LAYER != INVALID_LAYER)
    {
        _logoPosition = logoPosition;
        UpdateLogoLayer();
    }
}


void IspMixerHelper::LogoBouncingFunc()
{
    // Standard mersenne_twister_engine seeded with std::random_device()
    static std::mt19937 gen(std::random_device{}()); 
    std::uniform_real_distribution<float> dist(0.0f, -std::numbers::pi_v<float> / 2.0f);

    // Random angle 0..90 degrees
    static const float ANGLE = dist(gen);
    static constexpr float SPEED = 0.002f;
    static float dx = cosf(ANGLE);
    static float dy = sinf(ANGLE);

    _bounceX += dx * SPEED;
    _bounceY += dy * SPEED;

    if(_bounceX < 0.0f)
    {
        _bounceX = 0.0f;
        dx = -dx;
    }

    if(_bounceX > 1.0f)
    {
        _bounceX = 1.0f;
        dx = -dx;
    }

    if(_bounceY < 0.0f)
    {
        _bounceY = 0.0f;
        dy = -dy;
    }

    if(_bounceY > 1.0f)
    {
        _bounceY = 1.0f;
        dy = -dy;
    }

    const uint32_t x = static_cast<uint32_t>((GetOutputWidth() - GetLogoWidth()) * _bounceX);
    const uint32_t y = static_cast<uint32_t>((GetOutputHeight() - GetLogoHeight()) * _bounceY);

    SetLogoPositionXY(x, y);    
}


void IspMixerHelper::UpdateLogoLayer()
{
    if(LOGO_LAYER != INVALID_LAYER)
    {
        StopThread();
        _shutdownEvent.Reset();

        bool valid_state = (GetOutputWidth() > 0) &&  (GetOutputHeight() > 0);
        bool enableLogo = valid_state;

        if(valid_state)
        {
            switch(_logoPosition)
            {
                case LogoPosition::LogoDisabled:
                {
                    enableLogo = false;
                    break;
                }
                case LogoPosition::TopLeft:
                {
                    SetLogoPositionXY(0, 0);
                    break;
                }
                case LogoPosition::TopRight:
                {
                    SetLogoPositionXY(GetOutputWidth() - GetLogoWidth(), 0);
                    break;
                }
                case LogoPosition::BottomLeft:
                {
                    SetLogoPositionXY(0, GetOutputHeight() - GetLogoHeight());
                    break;
                }
                case LogoPosition::BottomRight:
                {
                    SetLogoPositionXY(GetOutputWidth() - GetLogoWidth(), GetOutputHeight() - GetLogoHeight());            
                    break;
                }
                case LogoPosition::Bouncing:
                {
                    StartThread();
                    break;
                }
                default:
                    break;
            }
        }

        EnableLogo(enableLogo);
    }
}


uint32_t IspMixerHelper::GetLogoWidth() const
{
    return _logoWidth;
}


uint32_t IspMixerHelper::GetLogoHeight() const
{
    return _logoHeight;
}


void IspMixerHelper::SetLogoOpacity(const float v)
{
    if(LOGO_LAYER != INVALID_LAYER)
    {
        _spMixer->SetLayerStaticAlphaViaDouble(LOGO_LAYER, 10, v); // FIXME - Hardcoded BPS value!
    }
}


void IspMixerHelper::SetOutputResolution(const uint32_t width, const uint32_t height)
{
    _spMixer->SetOutputWidth(width);
    _spMixer->SetOutputHeight(height);

    if(_isp_layers == 1)
    {
        // Pipeline is on layer 1
        _spMixer->SetLayerResolution(ISP_LAYER, width, height);
    }

    // Overlay is on layer 2 if overlay eenabled
    if(OVERLAY_LAYER != INVALID_LAYER)
    {
        _spMixer->SetLayerResolution(OVERLAY_LAYER, width, height);
    }

    // Logo offset might need to be updated
    // using new resolution
    if(LOGO_LAYER != INVALID_LAYER)
    {
        UpdateLogoLayer();
    }
}


uint32_t IspMixerHelper::GetOutputWidth() const
{
    return _spMixer->GetOutputWidth();
}


uint32_t IspMixerHelper::GetOutputHeight() const
{
    return _spMixer->GetOutputHeight();
}


void IspMixerHelper::SetMode(const Mode& mode)
{
    _mode = mode;

    switch(_mode)
    {
        case Mode::ISP:
        {
            if(IsIspEnabled())
                _spMixer->SetLayerEnableState(ISP_LAYER, SwApi::MixerLayerState::MixerLayerEnabledSoftStart);

            if(IsOverlayEnabled())
                if(OVERLAY_LAYER != INVALID_LAYER)
                    _spMixer->SetLayerEnableState(OVERLAY_LAYER, SwApi::MixerLayerState::MixerLayerEnabledSoftStart);

            if(IsLogoEnabled())
                if(LOGO_LAYER != INVALID_LAYER)
                    _spMixer->SetLayerEnableState(LOGO_LAYER, SwApi::MixerLayerState::MixerLayerEnabledSoftStart);
 
        }
        break;
        case Mode::ColorBars:
        {
            _spMixer->SetLayerEnableState(ISP_LAYER, SwApi::MixerLayerState::MixerLayerConsumed);
            if(OVERLAY_LAYER != INVALID_LAYER)
                _spMixer->SetLayerEnableState(OVERLAY_LAYER, SwApi::MixerLayerState::MixerLayerConsumed);
            if(LOGO_LAYER != INVALID_LAYER)
                _spMixer->SetLayerEnableState(LOGO_LAYER, SwApi::MixerLayerState::MixerLayerConsumed);
        }
        break;
        case Mode::Screensaver:
        {
            _spMixer->SetLayerEnableState(ISP_LAYER, SwApi::MixerLayerState::MixerLayerConsumed);
        }
        break;
        default:
        break;
    };
}


IspMixerHelper::Mode IspMixerHelper::GetMode()
{
    return _mode;
}

void IspMixerHelper::RunThread()
{
    prctl(PR_SET_NAME, _threadName.c_str(),0,0,0);

    while (!_shutdownEvent.IsSignalled())
    {
        LogoBouncingFunc();
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}

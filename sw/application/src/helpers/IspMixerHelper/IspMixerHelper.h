/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include "IMixer.h"
#include <memory>
#include "SwUtilsThread.h"


class IspMixerHelper : public SwUtils::Thread
{
public:
    IspMixerHelper(const std::shared_ptr<SwApi::IMixer>& spMixer, const bool hasOverlay=false, const bool hasLogo=false);
    ~IspMixerHelper();
    IspMixerHelper(const IspMixerHelper& other) = delete;
    IspMixerHelper& operator=(const IspMixerHelper& other) = delete;
    IspMixerHelper(IspMixerHelper&& other) = delete;
    IspMixerHelper& operator=(IspMixerHelper&& other) = delete;

    enum LogoPosition
    {
        LogoDisabled = 0,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Bouncing
    };    

    enum class Mode
    {
        ISP,
        ColorBars,
        Screensaver,
        Invalid
    };

    void SetMode(const Mode& mode);
    Mode GetMode();

    void EnableIsp(bool v);
    bool IsIspEnabled() const;
    void SetIspResolution(const uint32_t width, const uint32_t height);

    void HideIsp(bool v);

    void EnableLogo(bool v);
    bool IsLogoEnabled() const;

    void HideLogo(bool v);

    void EnableOverlay(bool v);
    bool IsOverlayEnabled() const;

    void HideOverlay(bool v);

    void SetLogoResolution(const uint32_t width, const uint32_t height);
    void SetLogoPositionXY(const uint32_t x, const uint32_t y);
    void SetLogoPosition(const LogoPosition& logoPosition);

    uint32_t GetLogoWidth() const;
    uint32_t GetLogoHeight() const;
    void SetLogoOpacity(const float v);

    void SetOutputResolution(const uint32_t width, const uint32_t height);
    uint32_t GetOutputWidth() const;
    uint32_t GetOutputHeight() const;

    std::shared_ptr<SwApi::IMixer> GetIMixer() { return _spMixer; }

    virtual void RunThread() override;

private:
    void UpdateLogoLayer();
    void LogoBouncingFunc();

    static constexpr uint32_t OUTPUT_WIDTH_DEFAULT = 3840;
    static constexpr uint32_t OUTPUT_HEIGHT_DEFAULT = 2160;

    static constexpr uint32_t LOGO_WIDTH = 256;
    static constexpr uint32_t LOGO_HEIGHT = 256;

    static constexpr uint32_t INVALID_LAYER = 0xFFFFFFFFU;
    static constexpr uint32_t ISP_LAYER = 1;
    uint32_t OVERLAY_LAYER;
    uint32_t LOGO_LAYER;

    void UpdateMixerLayerVisibility(const uint32_t layer_id);

    Mode _mode;
    
    // Layers enable status as requested by user or app
    // Note: this can differ from the actual layer state in the mixer
    // when for example both logo and isp layers are hidden and the
    // Color bars test output is being displayed
    bool _ispEnabled;
    bool _ispHidden;
    uint32_t _isp_layers;

    bool _overlayEnabled;
    bool _overlayHidden;

    bool _logoEnabled;
    bool _logoHidden;
    uint32_t _logoWidth;
    uint32_t _logoHeight;

    LogoPosition _logoPosition;
    float _bounceX = 0.0;
    float _bounceY = 0.0;

    std::shared_ptr<SwApi::IMixer> _spMixer;
};

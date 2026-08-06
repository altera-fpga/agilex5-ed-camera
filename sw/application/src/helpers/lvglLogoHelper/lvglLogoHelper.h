/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include <memory>
#include "SwUtilsThread.h"
#include <lvgl.h>
#include "IDrmHelper.h"
#include "IUIConnection.h"
#include "ILogoControl.h"

class lvglLogoHelper : public SwUtils::Thread
{
public:
    lvglLogoHelper(const std::shared_ptr<ILogoControl>& logoControl, IUIConnection* uiConnection);
    ~lvglLogoHelper();
    lvglLogoHelper(const lvglLogoHelper& other) = delete;
    lvglLogoHelper& operator=(const lvglLogoHelper& other) = delete;
    lvglLogoHelper(lvglLogoHelper&& other) = delete;
    lvglLogoHelper& operator=(lvglLogoHelper&& other) = delete;

    enum LogoPosition
    {
        LogoDisabled = 0,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Bouncing
    };    

    void SetLogoPosition(const LogoPosition& logoPosition);
    void SetLogoOpacity(const float v);

    virtual void RunThread() override;

private:
    void Draw();
    uint32_t GetOutputWidth() const;
    uint32_t GetOutputHeight() const;

    void SetLogoPositionXY(const uint32_t x, const uint32_t y);

    void UpdateLogoLayer();

    std::shared_ptr<ILogoControl> _logoControl;
    IUIConnection* _uiConnection;
    std::shared_ptr<IDrmHelper> _drmHelper;
    bool _logo_enabled;
    uint32_t _logo_width;
    uint32_t _logo_height;
    uint32_t _logo_x;
    uint32_t _logo_y;
    float _logo_alpha;
    lv_obj_t * _logo_img;
    lv_obj_t * _ip_sting;

    LogoPosition _logoPosition;
    float _bounceX;
    float _bounceY;

    bool _bounceXDir;
    bool _bounceYDir;
    float _bounceXItr;
    float _bounceYItr;

    bool _ip_address_enabled;
    bool _last_ip_address_enabled;
    bool _initialized;
    bool _dirty;
};
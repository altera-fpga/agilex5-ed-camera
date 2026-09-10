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
#include <optional>
#include "SwUtilsThread.h"
#include <lvgl.h>
#include "IOverlayHelper.h"
#include "IUIConnection.h"
#include "ILogoControl.h"

class lvglLogoHelper : public SwUtils::Thread
{
public:
    lvglLogoHelper(const std::weak_ptr<ILogoControl>& logoControl, IUIConnection* uiConnection);
    ~lvglLogoHelper();
    lvglLogoHelper(const lvglLogoHelper& other) = delete;
    lvglLogoHelper& operator=(const lvglLogoHelper& other) = delete;
    lvglLogoHelper(lvglLogoHelper&& other) = delete;
    lvglLogoHelper& operator=(lvglLogoHelper&& other) = delete;

    void SetLogoOpacity(const float v);

    void Start();

    virtual void RunThread() override;

private:
    void Draw();
    uint32_t GetOutputWidth() const;
    uint32_t GetOutputHeight() const;

    void UpdateLogoLayer();

    std::weak_ptr<ILogoControl> _wspLogoControl;
    IUIConnection* _uiConnection;
    std::shared_ptr<IOverlayHelper> _overlayHelper;
    uint32_t _logo_width;
    uint32_t _logo_height;
    float _logo_alpha;
    lv_obj_t * _logo_img;
    lv_obj_t * _ip_sting;
    std::optional<bool> _ip_address_enabled;
    bool _initialized;
    bool _dirty;
};
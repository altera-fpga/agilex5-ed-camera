/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "lvglLogoHelper.h"
#include <cassert>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include <lvgl.h>

lvglLogoHelper::lvglLogoHelper(const std::weak_ptr<ILogoControl>& logoControl, IUIConnection* uiConnection):
    SwUtils::Thread("lvglLogoHelper"),
    _wspLogoControl(logoControl),
    _uiConnection(uiConnection),
    _overlayHelper(IOverlayHelper::GetIOverlayHelper()),
    _logo_width(0),
    _logo_height(0),
    _logo_alpha(0.75),
    _logo_img(nullptr),
    _ip_sting(nullptr),
    _ip_address_enabled{std::nullopt},
    _initialized(false),
    _dirty(true)
{
}

lvglLogoHelper::~lvglLogoHelper()
{
    StopThread();

    std::lock_guard<std::recursive_mutex> lock(IOverlayHelper::GetLVGLMutex());
    
    if(_logo_img)
    {
        lv_obj_del(_logo_img);
        _logo_img = nullptr;
    }
    if(_ip_sting)
    {
        lv_obj_del(_ip_sting);
        _ip_sting = nullptr;
    }
}

void lvglLogoHelper::Start()
{
    StartThread();
}

void lvglLogoHelper::Draw()
{
    bool ip_address_enabled_now = true;

    // Hide IP address if UI has connected
    if(_uiConnection && _uiConnection->UiConnected())
        ip_address_enabled_now = false;

    bool ip_address_enabled_changed = false;
    
    // If no previous value known or it has actually changed
    if(!_ip_address_enabled.has_value() || (*_ip_address_enabled != ip_address_enabled_now))
    {
        ip_address_enabled_changed = true;
        if(_logo_img)
        {
            lv_obj_del(_logo_img);
            _logo_img = nullptr;
        }
        if(_ip_sting)
        {
            lv_obj_del(_ip_sting);
            _ip_sting = nullptr;
        }
        _initialized = false;
    }

    if(!_initialized)
    {
        _initialized = true;
        if(ip_address_enabled_now)
        {
            _logo_width = 300;
            _logo_height = 180;
        }
        else
        {
            _logo_width = 144;
            _logo_height = 144;
        }
        _ip_address_enabled = ip_address_enabled_now;

        auto spLogoControl = _wspLogoControl.lock();
        if(spLogoControl)
        {
            spLogoControl->SetOverlayResolution(_logo_width, _logo_height);
        }
        if(!_logo_img)
        {
            _logo_img = lv_image_create(_overlayHelper->GetOverlayScreenActive());

            if(_logo_img)
            {
                lv_image_set_src(_logo_img, "A:images/altera-144.bmp");
                lv_obj_align(_logo_img, LV_ALIGN_TOP_LEFT, 0, 0);
                lv_obj_set_style_opa(_logo_img, (lv_opa_t)(255*_logo_alpha), 0);
            }
        }
        if(!_ip_sting && ip_address_enabled_now)
        {
            std::string ip_url = _uiConnection->GetUiUrl(false);
            _ip_sting = lv_label_create(_overlayHelper->GetOverlayScreenActive());

            if(_ip_sting)
            {
                lv_label_set_text(_ip_sting, ip_url.c_str());
                lv_obj_align(_ip_sting, LV_ALIGN_BOTTOM_LEFT, 0, 0);

                static lv_style_t style;
                lv_style_init(&style);
                lv_style_set_bg_opa(&style, LV_OPA_COVER);
                lv_style_set_bg_color(&style, lv_color_make(64, 64, 64));
                lv_style_set_text_color(&style, lv_color_make(255, 255, 255));
                if(ip_url.length() > 30)
                {
                    lv_style_set_text_font(&style, &lv_font_montserrat_18);
                }
                else
                {
                    lv_style_set_text_font(&style, &lv_font_montserrat_26);
                }
                lv_obj_add_style(_ip_sting, &style, 0);                
            }
        }
        _dirty = true;
    }

    if(_dirty)
    {
        if(_logo_img)
        {
            lv_obj_set_style_opa(_logo_img, (lv_opa_t)(255*_logo_alpha), 0);
        }
        _overlayHelper->FlushOverlay();
        _dirty = false;
    }
}


void lvglLogoHelper::UpdateLogoLayer()
{
    std::lock_guard<std::recursive_mutex> lock(IOverlayHelper::GetLVGLMutex());
    Draw();
}


void lvglLogoHelper::SetLogoOpacity(const float v)
{
    std::lock_guard<std::recursive_mutex> lock(IOverlayHelper::GetLVGLMutex());
    _logo_alpha = v;
    _dirty = true;
}


uint32_t lvglLogoHelper::GetOutputWidth() const
{
    uint32_t overlay_width = lv_display_get_horizontal_resolution(nullptr);
    return overlay_width;
}


uint32_t lvglLogoHelper::GetOutputHeight() const
{
    uint32_t overlay_height = lv_display_get_vertical_resolution(nullptr);
    return overlay_height;
}


void lvglLogoHelper::RunThread()
{
    prctl(PR_SET_NAME, _threadName.c_str(),0,0,0);
    while (!_shutdownEvent.IsSignalled())
    {
        UpdateLogoLayer();
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
}

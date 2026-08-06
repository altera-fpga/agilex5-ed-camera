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

lvglLogoHelper::lvglLogoHelper(const std::shared_ptr<ILogoControl>& logoControl, IUIConnection* uiConnection):
    SwUtils::Thread("lvglLogoHelper"),
    _logoControl(logoControl),
    _uiConnection(uiConnection),
    _drmHelper(IDrmHelper::GetIDrmHelper()),
    _logo_enabled(true),
    _logo_width(0),
    _logo_height(0),
    _logo_x(0),
    _logo_y(0),
    _logo_alpha(0.75),
    _logo_img(nullptr),
    _ip_sting(nullptr),
    _logoPosition{LogoPosition::Bouncing},
    _bounceX(0.0),
    _bounceY(0.0),
    _bounceXDir(false),
    _bounceYDir(false),
    _bounceXItr(0.0018),
    _bounceYItr(0.0032),
    _ip_address_enabled(true),
    _last_ip_address_enabled(false),
    _initialized(false),
    _dirty(true)
{
    StartThread();
}

lvglLogoHelper::~lvglLogoHelper()
{
    StopThread();

    std::lock_guard<std::recursive_mutex> lock(IDrmHelper::GetLVGLMutex());
    
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


void lvglLogoHelper::Draw()
{
    if(!_initialized)
    {
        _initialized = true;
        _logo_width = 300;
        _logo_height = 180;
        _logoControl->SetOverlayResolution(_logo_width, _logo_height);
        if(!_logo_img)
        {
            _logo_img = lv_image_create(_drmHelper->GetOverlayScreenActive());

            if(_logo_img)
            {
                lv_image_set_src(_logo_img, "A:images/altera-144.bmp");
                lv_obj_align(_logo_img, LV_ALIGN_TOP_LEFT, 0, 0);
                lv_obj_set_style_opa(_logo_img, (lv_opa_t)(255*_logo_alpha), 0);
            }
        }
        if(!_ip_sting && (_uiConnection != nullptr))
        {
            std::string ip_url = _uiConnection->GetUiUrl(false);
            _ip_sting = lv_label_create(_drmHelper->GetOverlayScreenActive());

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
    if(_last_ip_address_enabled != _ip_address_enabled)
    {
        _last_ip_address_enabled = _ip_address_enabled;
        if(_ip_sting)
        {
            if(_ip_address_enabled)
            {
                lv_obj_clear_flag(_ip_sting, LV_OBJ_FLAG_HIDDEN);
            }
            else
            {
                lv_obj_add_flag(_ip_sting, LV_OBJ_FLAG_HIDDEN);
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
        _drmHelper->FlushOverlay();
        _dirty = false;
    }
}


void lvglLogoHelper::SetLogoPositionXY(const uint32_t x, const uint32_t y)
{
    _logo_x = x;
    _logo_y = y;
}


void lvglLogoHelper::SetLogoPosition(const LogoPosition& logoPosition)
{
    _logoPosition = logoPosition;
    UpdateLogoLayer();
}


void lvglLogoHelper::UpdateLogoLayer()
{
    bool logo_enable = true;
    std::lock_guard<std::recursive_mutex> lock(IDrmHelper::GetLVGLMutex());

    uint32_t logo_width = _logo_width;
    uint32_t logo_height = _logo_height;
    
    if(_uiConnection)
    {
        _ip_address_enabled = !_uiConnection->UiConnected();
    }
    if(!_ip_address_enabled)
    {
        if(_logo_img)
        {
            logo_width = lv_obj_get_width(_logo_img);
            logo_height = lv_obj_get_height(_logo_img);
        }
    }

    switch(_logoPosition)
    {
        case LogoPosition::LogoDisabled:
        {
            logo_enable = false;
            break;
        }
        case LogoPosition::TopLeft:
        {
            SetLogoPositionXY(0, 0);
            break;
        }
        case LogoPosition::TopRight:
        {
            SetLogoPositionXY(GetOutputWidth() - _logo_width, 0);
            break;
        }
        case LogoPosition::BottomLeft:
        {
            SetLogoPositionXY(0, GetOutputHeight() - _logo_height);
            break;
        }
        case LogoPosition::BottomRight:
        {
            SetLogoPositionXY(GetOutputWidth() - _logo_width, GetOutputHeight() - _logo_height);            
            break;
        }
        case LogoPosition::Bouncing:
        {
            if (_bounceX <= (0.0 + _bounceXItr))
            {
                _bounceXDir = true;
            }
            if (_bounceY <= (0.0 + _bounceYItr))
            {
                _bounceYDir = true;
            }

            if (_bounceX >= (1.0 - _bounceXItr))
            {
                _bounceXDir = false;
            }
            if (_bounceY >= (1.0 - _bounceYItr))
            {
                _bounceYDir = false;
            }

            _bounceX += (_bounceXDir) ? (_bounceXItr) : (-_bounceXItr);
            _bounceY += (_bounceYDir) ? (_bounceYItr) : (-_bounceYItr);

            const uint32_t x = (GetOutputWidth() - _logo_width) * _bounceX;
            const uint32_t y = (GetOutputHeight() - _logo_height) * _bounceY;

            SetLogoPositionXY(x, y);
        }
        default:
            break;
    }

    _logo_enabled = logo_enable;

    Draw();
}


void lvglLogoHelper::SetLogoOpacity(const float v)
{
    std::lock_guard<std::recursive_mutex> lock(IDrmHelper::GetLVGLMutex());
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

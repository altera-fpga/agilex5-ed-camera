/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "IOverlayHelper.h"
#include <src/misc/lv_timer_private.h>
#include <src/core/lv_refr_private.h>
#include <src/draw/lv_draw_buf_private.h>
#include "drmHelper.h"

std::shared_ptr<IOverlayHelper> IOverlayHelper::_spOverlayHelper = nullptr;
std::recursive_mutex IOverlayHelper::_lvglMutex;

void IOverlayHelper::SetIOverlayHelperInst(std::shared_ptr<IOverlayHelper> spOverlayHelper)
{
    _spOverlayHelper = spOverlayHelper;
}   

std::shared_ptr<IOverlayHelper> IOverlayHelper::GetIOverlayHelper()
{
    return _spOverlayHelper;
}

std::recursive_mutex& IOverlayHelper::GetLVGLMutex()
{
    return _lvglMutex;
}

IOverlayHelper::IOverlayHelper()
{
    lv_init();
}

IOverlayHelper::~IOverlayHelper()
{
    if(_primary_display)
    {
        lv_display_delete(_primary_display);
    }
    if(_overlay_display)
    {
        lv_display_delete(_overlay_display);
    }
    lv_deinit();
}


void IOverlayHelper::SetPrimaryResolution(uint32_t width, uint32_t height, lv_color_format_t format)
{
    bool rc = true;

    _primary_width = width;
    _primary_height = height;
    if(_primary_width == 0 || _primary_height == 0)
    {
        _primary_width = 16;
        _primary_height = 16;
    }
    _primary_lvgl_format = format;

    if(_primary_display)
    {
        lv_display_delete(_primary_display);
        _primary_display = nullptr;
    }

    rc = SetPrimaryResolutionLow(_primary_width, _primary_height, _primary_lvgl_format);
    if(!rc)
    {
        std::cerr << "Failed to set primary resolution" << std::endl;
    }
    else
    {
        _primary_stride = GetPrimaryStrideLow();
        uint32_t primary_fb_size = _primary_height * _primary_stride;

        if(_primary_lvgl_format != LV_COLOR_FORMAT_UNKNOWN)
        {
            _primary_display = lv_display_create(_primary_width, _primary_height);
            if(_primary_display)
            {
                lv_display_set_user_data(_primary_display, this);
                lv_display_set_color_format(_primary_display, _primary_lvgl_format);
                lv_display_set_flush_cb(_primary_display, IOverlayHelper::flush_cb_static);
                lv_display_delete_refr_timer(_primary_display);
                lv_obj_set_style_bg_opa(lv_display_get_screen_active(_primary_display), LV_OPA_TRANSP, 0);
                lv_obj_set_style_bg_color(lv_display_get_screen_active(_primary_display), lv_color_make(0, 0, 0), 0);
                lv_obj_set_scrollbar_mode(lv_display_get_screen_active(_primary_display), LV_SCROLLBAR_MODE_OFF);
                
                uint8_t* buf0_ptr = GetPrimaryBuffer(0);
                if(buf0_ptr)
                {
                    lv_draw_buf_init(&_primary_buf[0], _primary_width, _primary_height, _primary_lvgl_format, _primary_stride, buf0_ptr, primary_fb_size);

                    uint8_t* buf1_ptr = GetPrimaryBuffer(1);
                    if(buf1_ptr)
                    {
                        lv_draw_buf_init(&_primary_buf[1], _primary_width, _primary_height, _primary_lvgl_format, _primary_stride, buf1_ptr, primary_fb_size);
                        lv_display_set_draw_buffers(_primary_display, &_primary_buf[0], &_primary_buf[1]);                                    
                    }
                    else
                    {
                        lv_display_set_draw_buffers(_primary_display, &_primary_buf[0], nullptr);
                    }
                }
                lv_display_set_render_mode(_primary_display, LV_DISPLAY_RENDER_MODE_DIRECT);
                lv_display_set_default(_primary_display);
            }
        }

        std::cout << "Primary " << _primary_width << "x" << _primary_height << " stride " << _primary_stride << std::endl << std::flush;
    }
}

void IOverlayHelper::SetOverlayResolution(uint32_t width, uint32_t height)
{
    bool rc = true;

    _overlay_width = width;
    _overlay_height = height;
    _overlay_lvgl_format = LV_COLOR_FORMAT_ARGB8888;

    if(_overlay_display)
    {
        lv_display_delete(_overlay_display);
        _overlay_display = nullptr;
    }

    rc = SetOverlayResolutionLow(width, height);
    if(!rc)
    {
        std::cerr << "Failed to set overlay resolution" << std::endl;
    }
    else
    {
        _overlay_stride = GetOverlayStrideLow();
        uint32_t overlay_fb_size = _overlay_height * _overlay_stride;

        _overlay_display = lv_display_create(_overlay_width, _overlay_height);
        if(_overlay_display)
        {
            lv_display_set_user_data(_overlay_display, this);
            lv_display_set_color_format(_overlay_display, _overlay_lvgl_format);
            lv_display_set_flush_cb(_overlay_display, IOverlayHelper::flush_cb_static);
            lv_display_delete_refr_timer(_overlay_display);
            lv_obj_set_style_bg_opa(lv_display_get_screen_active(_overlay_display), LV_OPA_TRANSP, 0);
            lv_obj_set_style_bg_color(lv_display_get_screen_active(_overlay_display), lv_color_make(0, 0, 0), 0);
            lv_obj_set_scrollbar_mode(lv_display_get_screen_active(_overlay_display), LV_SCROLLBAR_MODE_OFF);

            uint8_t* buf_ptr = GetOverlayBuffer();
            if(buf_ptr)
            {
                lv_draw_buf_init(&_overlay_buf, _overlay_width, _overlay_height, _overlay_lvgl_format, _overlay_stride, buf_ptr, overlay_fb_size);
                lv_display_set_draw_buffers(_overlay_display, &_overlay_buf, nullptr);
            }
            lv_display_set_render_mode(_overlay_display, LV_DISPLAY_RENDER_MODE_DIRECT);
        }
        std::cout << "Overlay " << _overlay_width << "x" << _overlay_height << " stride " << _overlay_stride << std::endl << std::flush;
    }
}   

bool IOverlayHelper::FlushPrimary()
{
	bool rc = true;

    std::lock_guard<std::recursive_mutex> lock(IOverlayHelper::GetLVGLMutex());

    // This will swap the buffers
    lv_timer_t disp_timer;
    disp_timer.user_data = _primary_display;
    lv_display_refr_timer(&disp_timer);

    rc = FlushPrimaryLow();

    return rc;
}

bool IOverlayHelper::FlushOverlay()
{
	bool rc = true;

    std::lock_guard<std::recursive_mutex> lock(IOverlayHelper::GetLVGLMutex());

    lv_timer_t disp_timer;
    disp_timer.user_data = _overlay_display;
    lv_display_refr_timer(&disp_timer);

    rc = FlushOverlayLow();

    return rc;
}

void IOverlayHelper::flush_cb_static(lv_display_t * display, const lv_area_t * area, uint8_t * px_map)
{
    IOverlayHelper* self = reinterpret_cast<IOverlayHelper*>(lv_display_get_user_data(display));
    if(self)
    {
        self->flush_cb(display, area, px_map);
    }
}

void IOverlayHelper::flush_cb(lv_display_t * display, const lv_area_t * area, uint8_t * px_map)
{
    /* IMPORTANT!!!
    * Inform LVGL that flushing is complete so buffer can be modified again. */
    lv_display_flush_ready(display);
}

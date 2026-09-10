/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include <lvgl.h>
#include <mutex>
#include <memory>

class IOverlayHelper
{
public:
    static void SetIOverlayHelperInst(std::shared_ptr<IOverlayHelper> spOverlayHelper);
    static std::shared_ptr<IOverlayHelper> GetIOverlayHelper();
    static std::recursive_mutex& GetLVGLMutex();
    
    IOverlayHelper();
    virtual ~IOverlayHelper();
    virtual bool Open(const char *const card, uint32_t width, uint32_t height, lv_color_format_t format) = 0;

    void SetPrimaryResolution(uint32_t width, uint32_t height, lv_color_format_t format);
    lv_display_t * GetPrimaryDisplay() { return _primary_display;}
    lv_obj_t * GetPrimaryScreenActive() { return lv_display_get_screen_active(_primary_display);}
    bool FlushPrimary();

    void SetOverlayResolution(uint32_t width, uint32_t height);
    lv_display_t * GetOverlayDisplay() { return _overlay_display;}
    lv_obj_t * GetOverlayScreenActive() { return lv_display_get_screen_active(_overlay_display);}
    bool FlushOverlay();

    static void flush_cb_static(lv_display_t * display, const lv_area_t * area, uint8_t * px_map);

protected:
    uint32_t GetPrimaryHeight() { return _primary_height;}
    uint32_t GetPrimaryWidth() { return _primary_width;}
    virtual bool SetPrimaryResolutionLow(uint32_t width, uint32_t height, lv_color_format_t format) = 0;
    virtual uint8_t* GetPrimaryBuffer(int32_t index) = 0;
    virtual uint32_t GetPrimaryStrideLow() = 0;
    virtual bool FlushPrimaryLow() = 0;

    uint32_t GetOverlayHeight() { return _overlay_height;}
    uint32_t GetOverlayWidth() { return _overlay_width;}
    virtual bool SetOverlayResolutionLow(uint32_t width, uint32_t height) = 0;
    virtual uint8_t* GetOverlayBuffer() = 0;
    virtual uint32_t GetOverlayStrideLow() = 0;
    virtual bool FlushOverlayLow() = 0;
    void flush_cb(lv_display_t * display, const lv_area_t * area, uint8_t * px_map);
private:
    static std::shared_ptr<IOverlayHelper> _spOverlayHelper;
    static std::recursive_mutex _lvglMutex;

    uint32_t _primary_height = 16;
    uint32_t _primary_width = 16;
    uint32_t _primary_stride = 0;
    lv_color_format_t _primary_lvgl_format = LV_COLOR_FORMAT_ARGB2222;
    lv_draw_buf_t _primary_buf[2];

    uint32_t _overlay_width = 0;
    uint32_t _overlay_height = 0;
    uint32_t _overlay_stride = 0;
    lv_color_format_t _overlay_lvgl_format = LV_COLOR_FORMAT_UNKNOWN;
    lv_draw_buf_t _overlay_buf;

    lv_display_t * _primary_display = nullptr;
    lv_display_t * _overlay_display = nullptr;
};
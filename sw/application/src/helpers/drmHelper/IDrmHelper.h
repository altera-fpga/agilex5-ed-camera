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

class IDrmHelper
{
public:
    static std::shared_ptr<IDrmHelper> GetIDrmHelper();
    static std::recursive_mutex& GetLVGLMutex();
    
    IDrmHelper() {}
    virtual ~IDrmHelper() {}
    virtual bool Open(const char *const card, uint32_t width, uint32_t height, lv_color_format_t format) = 0;

    virtual uint32_t GetPrimaryHeight() = 0;
    virtual uint32_t GetPrimaryWidth() = 0;
    virtual uint32_t GetPrimaryStride() = 0;
    virtual uint8_t* GetPrimaryBuffer() = 0;
    virtual lv_display_t * GetPrimaryDisplay() = 0;
    virtual lv_obj_t * GetPrimaryScreenActive() = 0;
    virtual bool FlushPrimary() = 0;

    virtual void SetOverlayResolution(uint32_t width, uint32_t height) = 0;
    virtual uint32_t GetOverlayHeight() = 0;
    virtual uint32_t GetOverlayWidth() = 0;
    virtual uint32_t GetOverlayStride() = 0;
    virtual uint8_t* GetOverlayBuffer() = 0;
    virtual lv_display_t * GetOverlayDisplay() = 0;
    virtual lv_obj_t * GetOverlayScreenActive() = 0;
    virtual bool FlushOverlay() = 0;
private:
    static std::shared_ptr<IDrmHelper> _spDrmHelper;
    static std::recursive_mutex _lvglMutex;
};
/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include <xf86drm.h>
#include <xf86drmMode.h>
#include "HapiVvpVfr.h"
#include "HapiAddrSpanExpander.h"
#include <mutex>
#include "SwUtilsThread.h"
#include <lvgl.h>
#include "IOverlayHelper.h" 
#include "SwUtils.h"
#include "SwUtilsMemTransfer.h"
#include "SwUtilsMemBuffer.h"

class DmaOverlayHelper : public IOverlayHelper, public SwUtils::Thread
{
public:
    static void Create(std::shared_ptr<Hapi::IHapi>  spHapi, uint16_t outputBaseUID);

    DmaOverlayHelper(std::shared_ptr<Hapi::IHapi>  spHapi, uint16_t outputBaseUID);
    ~DmaOverlayHelper();
    virtual bool Open(const char* const card, uint32_t width, uint32_t height, lv_color_format_t format) final override;

    virtual bool SetPrimaryResolutionLow(uint32_t width, uint32_t height, lv_color_format_t format) final override;
    virtual uint32_t GetPrimaryStrideLow() final override { return _primary_stride;}
    virtual uint8_t* GetPrimaryBuffer(int32_t index) final override;
    virtual bool FlushPrimaryLow() final override;
    
    virtual bool SetOverlayResolutionLow(uint32_t width, uint32_t height) final override;
    virtual uint32_t GetOverlayStrideLow() final override { return _overlay_stride;}
    virtual uint8_t* GetOverlayBuffer() final override;
    virtual bool FlushOverlayLow() final override;
    
    virtual void RunThread() override;

private:
    static constexpr uint32_t NUM_BUFFERS = 2;
    
    uint32_t _primary_stride = 0;
    uint32_t _primary_fb_size = 0;
    uint8_t* _primary_buf_local = nullptr;
    uint32_t _overlay_stride = 0;
    uint32_t _overlay_fb_size = 0;
    uint8_t *_overlay_buf_local = nullptr;

    Hapi::AddrSpanExpanderPtr _primary_ase;
    Hapi::VvpVfrPtr _primary_vfr;
    Hapi::AddrSpanExpanderPtr _overlay_ase;
    Hapi::VvpVfrPtr _overlay_vfr;

    std::shared_ptr<SwApi::IMemTransfer> _dataTransfer;

};


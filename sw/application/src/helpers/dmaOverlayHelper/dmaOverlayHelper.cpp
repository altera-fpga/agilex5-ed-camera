/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "dmaOverlayHelper.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/prctl.h>
#include <sys/prctl.h>

static const uintptr_t FPGA_EMIF_MSGDMA_BASE = 0x200000000;
static const uintptr_t FPGA_EMIF_PRIMARY_BASE = 0x40000000;
static const uintptr_t FPGA_EMIF_OVERLAY_BASE = 0x40800000;

static const uint32_t PITCH_ROUNDING_BYTES = 4096;
static uint32_t roundup_pitch(uint32_t v)
{
    return (v + (PITCH_ROUNDING_BYTES - 1)) & ~(PITCH_ROUNDING_BYTES - 1);
};    

void DmaOverlayHelper::Create(std::shared_ptr<Hapi::IHapi>  spHapi, uint16_t outputBaseUID)
{
    std::shared_ptr<DmaOverlayHelper> spOverlayHelper = std::make_shared<DmaOverlayHelper>(spHapi, outputBaseUID);
    IOverlayHelper::SetIOverlayHelperInst(spOverlayHelper);
}

DmaOverlayHelper::DmaOverlayHelper(std::shared_ptr<Hapi::IHapi>  spHapi, uint16_t outputBaseUID)
    : SwUtils::Thread("DmaOverlayHelper")
    , _primary_stride(0)
    , _primary_fb_size(0)
    , _overlay_stride(0)
    , _overlay_fb_size(0)
{
    _dataTransfer = SwApi::MemTransferMsgdma::Create("/dev/msgdma_userio0");
    if(!_dataTransfer){
        std::cerr <<"Failed to create MSGDMA data transfer";
    }

    const uint32_t OVERLAY_VFR_UID = outputBaseUID;
    _overlay_vfr = spHapi->CreateByUniqueID<Hapi::VvpVfr>(OVERLAY_VFR_UID);
    if (!_overlay_vfr)
    {
        std::cerr << "Overlay VFR not present\n" << std::flush;
    }
    else
    {
        intel_vvp_vfr_set_bufset_base_addr(_overlay_vfr->GetInstance(), 0, FPGA_EMIF_OVERLAY_BASE & 0x7FFFFFFFULL);
        intel_vvp_vfr_set_bufset_inter_buffer_offset(_overlay_vfr->GetInstance(), 0, 0);
        intel_vvp_vfr_set_bufset_inter_line_offset(_overlay_vfr->GetInstance(), 0, 16);
        intel_vvp_vfr_set_bufset_field_count(_overlay_vfr->GetInstance(), 0, 1);

        intel_vvp_vfr_set_bufset_bps(_overlay_vfr->GetInstance(), 0, 8);
        intel_vvp_vfr_set_bufset_colorspace(_overlay_vfr->GetInstance(), 0, 0);
        intel_vvp_vfr_set_bufset_cositing(_overlay_vfr->GetInstance(), 0, 0);
        intel_vvp_vfr_set_bufset_interlace(_overlay_vfr->GetInstance(), 0, 0);
        intel_vvp_vfr_set_bufset_subsampling(_overlay_vfr->GetInstance(), 0, 0x3);

        intel_vvp_vfr_set_bufset_width(_overlay_vfr->GetInstance(), 0, 16);
        intel_vvp_vfr_set_bufset_height(_overlay_vfr->GetInstance(), 0, 16);
        intel_vvp_core_set_img_info_width(_overlay_vfr->GetInstance(), 16);
        intel_vvp_core_set_img_info_height(_overlay_vfr->GetInstance(), 16);

        intel_vvp_core_set_img_info_interlace(_overlay_vfr->GetInstance(), 0);
        intel_vvp_core_set_img_info_subsampling(_overlay_vfr->GetInstance(), 0x3);
        intel_vvp_core_set_img_info_cositing(_overlay_vfr->GetInstance(), 0);
        intel_vvp_core_set_img_info_colorspace(_overlay_vfr->GetInstance(), 0);

        intel_vvp_vfr_set_buffer_mode(_overlay_vfr->GetInstance(), kIntelVvpVfrSingleSet);

        intel_vvp_vfr_set_run_mode(_overlay_vfr->GetInstance(), kIntelVvpVfrStop);

        intel_vvp_vfr_commit_writes(_overlay_vfr->GetInstance());

        intel_vvp_vfr_set_starting_buffer_set(_overlay_vfr->GetInstance(), 0);
        intel_vvp_vfr_set_buffer_mode(_overlay_vfr->GetInstance(), kIntelVvpVfrSingleSet);
        intel_vvp_vfr_set_run_mode(_overlay_vfr->GetInstance(), kIntelVvpVfrFreeRunning);
        intel_vvp_vfr_commit_writes(_overlay_vfr->GetInstance());
    }

    const uint32_t PRIMARY_VFR_UID = outputBaseUID + 1;
    _primary_vfr = spHapi->CreateByUniqueID<Hapi::VvpVfr>(PRIMARY_VFR_UID);
    if (!_primary_vfr)
    {
        std::cerr << "Primary VFR not present\n" << std::flush;
    }
    else
    {
        intel_vvp_vfr_set_bufset_base_addr(_primary_vfr->GetInstance(), 0, FPGA_EMIF_PRIMARY_BASE & 0x7FFFFFFFULL);
        intel_vvp_vfr_set_bufset_inter_buffer_offset(_primary_vfr->GetInstance(), 0, 0);
        intel_vvp_vfr_set_bufset_inter_line_offset(_primary_vfr->GetInstance(), 0, 16);
        intel_vvp_vfr_set_bufset_field_count(_primary_vfr->GetInstance(), 0, 1);

        intel_vvp_vfr_set_bufset_bps(_primary_vfr->GetInstance(), 0, 8);
        intel_vvp_vfr_set_bufset_colorspace(_primary_vfr->GetInstance(), 0, 0);
        intel_vvp_vfr_set_bufset_cositing(_primary_vfr->GetInstance(), 0, 0);
        intel_vvp_vfr_set_bufset_interlace(_primary_vfr->GetInstance(), 0, 0);
        intel_vvp_vfr_set_bufset_subsampling(_primary_vfr->GetInstance(), 0, 0x3);

        intel_vvp_vfr_set_bufset_width(_primary_vfr->GetInstance(), 0, 16);
        intel_vvp_vfr_set_bufset_height(_primary_vfr->GetInstance(), 0, 16);
        intel_vvp_core_set_img_info_width(_primary_vfr->GetInstance(), 16);
        intel_vvp_core_set_img_info_height(_primary_vfr->GetInstance(), 16);

        intel_vvp_core_set_img_info_interlace(_primary_vfr->GetInstance(), 0);
        intel_vvp_core_set_img_info_subsampling(_primary_vfr->GetInstance(), 0x3);
        intel_vvp_core_set_img_info_cositing(_primary_vfr->GetInstance(), 0);
        intel_vvp_core_set_img_info_colorspace(_primary_vfr->GetInstance(), 0);

        intel_vvp_vfr_set_buffer_mode(_primary_vfr->GetInstance(), kIntelVvpVfrSingleSet);

        intel_vvp_vfr_set_run_mode(_primary_vfr->GetInstance(), kIntelVvpVfrStop);

        intel_vvp_vfr_commit_writes(_primary_vfr->GetInstance());

        intel_vvp_vfr_set_starting_buffer_set(_primary_vfr->GetInstance(), 0);
        intel_vvp_vfr_set_buffer_mode(_primary_vfr->GetInstance(), kIntelVvpVfrSingleSet);
        intel_vvp_vfr_set_run_mode(_primary_vfr->GetInstance(), kIntelVvpVfrFreeRunning);
        intel_vvp_vfr_commit_writes(_primary_vfr->GetInstance());
    }
}

DmaOverlayHelper::~DmaOverlayHelper()
{
    StopThread();
}

bool DmaOverlayHelper::Open(const char* const card, uint32_t width, uint32_t height, lv_color_format_t format)
{
    bool rc = true;
    (void)card;

    if (rc)
    {
        SetPrimaryResolution(width, height, format);
        StartThread();
    }
    return rc;
}

bool DmaOverlayHelper::SetPrimaryResolutionLow(uint32_t width, uint32_t height, lv_color_format_t format)
{
    bool rc = true;

    if(_primary_buf_local)
    {
        free(_primary_buf_local);
        _primary_buf_local = nullptr;
    }

    uint32_t pitch = GetPrimaryWidth() * 4;
    uint32_t pixels_per_symbol = 1;

    switch(format)
    {
    case LV_COLOR_FORMAT_ARGB8888:
        pitch = width * 4;
        pixels_per_symbol = 1;
        break;
#if 0
    case LV_COLOR_FORMAT_ARGB4444:
        pitch = width * 2;
        pixels_per_symbol = 2;
        break;
#endif
    case LV_COLOR_FORMAT_ARGB2222:
        pitch = width;
        pixels_per_symbol = 4;
        break;
    case LV_COLOR_FORMAT_XRGB8888:
        pitch = width * 4;
        pixels_per_symbol = 1;
        break;
    default:
        pitch = width * 4;
        pixels_per_symbol = 1;
    }

    _primary_stride = roundup_pitch(pitch);
    if(_primary_vfr)
    {
        intel_vvp_vfr_set_bufset_width(_primary_vfr->GetInstance(), 0, GetPrimaryWidth()/* / pixels_per_symbol*/);
        intel_vvp_vfr_set_bufset_height(_primary_vfr->GetInstance(), 0, GetPrimaryHeight());
        intel_vvp_core_set_img_info_width(_primary_vfr->GetInstance(), GetPrimaryWidth()/* / pixels_per_symbol*/);
        intel_vvp_core_set_img_info_height(_primary_vfr->GetInstance(), GetPrimaryHeight());
        intel_vvp_vfr_set_bufset_inter_line_offset(_primary_vfr->GetInstance(), 0, _primary_stride);
        intel_vvp_vfr_commit_writes(_primary_vfr->GetInstance());
    }

    _primary_fb_size = GetPrimaryHeight() * _primary_stride;
    _primary_buf_local = (uint8_t*)aligned_alloc(4096, _primary_fb_size);
    memset(_primary_buf_local, 0, _primary_fb_size);

    return rc;
}

uint8_t* DmaOverlayHelper::GetPrimaryBuffer(int32_t index)
{
    if (index < 0 || index >= 1) {
        return nullptr;
    }
    return _primary_buf_local;
}

bool DmaOverlayHelper::SetOverlayResolutionLow(uint32_t width, uint32_t height)
{
    bool rc = true;

    if(_overlay_buf_local)
    {
        free(_overlay_buf_local);
        _overlay_buf_local = nullptr;
    }

    uint32_t pitch = GetOverlayWidth() * 4;

    _overlay_stride = roundup_pitch(pitch);
    if(_overlay_vfr)
    {
        intel_vvp_vfr_set_bufset_width(_overlay_vfr->GetInstance(), 0, GetOverlayWidth());
        intel_vvp_vfr_set_bufset_height(_overlay_vfr->GetInstance(), 0, GetOverlayHeight());
        intel_vvp_core_set_img_info_width(_overlay_vfr->GetInstance(), GetOverlayWidth());
        intel_vvp_core_set_img_info_height(_overlay_vfr->GetInstance(), GetOverlayHeight());
        intel_vvp_vfr_set_bufset_inter_line_offset(_overlay_vfr->GetInstance(), 0, _overlay_stride);
        intel_vvp_vfr_commit_writes(_overlay_vfr->GetInstance());
    }

    _overlay_fb_size = GetOverlayHeight() * _overlay_stride;
    _overlay_buf_local = (uint8_t*)aligned_alloc(4096, _overlay_fb_size);
    memset(_overlay_buf_local, 0, _overlay_fb_size);

    return rc;
}

uint8_t* DmaOverlayHelper::GetOverlayBuffer()
{
    return _overlay_buf_local;
}

bool DmaOverlayHelper::FlushPrimaryLow()
{
	bool rc = true;

    msync(_primary_buf_local, _primary_fb_size, MS_SYNC);

    if(_dataTransfer)
    {
        _dataTransfer->TransferToTarget(FPGA_EMIF_MSGDMA_BASE + FPGA_EMIF_PRIMARY_BASE, _primary_buf_local, _primary_fb_size);
    }

    return rc;
}

bool DmaOverlayHelper::FlushOverlayLow()
{
	bool rc = true;

    msync(_overlay_buf_local, _overlay_fb_size, MS_SYNC);

    if(_dataTransfer)
    {
        _dataTransfer->TransferToTarget(FPGA_EMIF_MSGDMA_BASE + FPGA_EMIF_OVERLAY_BASE, _overlay_buf_local, _overlay_fb_size);
    }

    return rc;
}

void DmaOverlayHelper::RunThread()
{
    prctl(PR_SET_NAME, _threadName.c_str(),0,0,0);

    int timeout = 100;

    while (!_shutdownEvent.IsSignalled())
    {
        ::std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    }
}

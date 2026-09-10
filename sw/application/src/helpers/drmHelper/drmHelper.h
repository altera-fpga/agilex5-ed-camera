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
#include <drm_fourcc.h>
#include "HapiVvpVfr.h"
#include "HapiAddrSpanExpander.h"
#include <mutex>
#include "SwUtilsThread.h"
#include <lvgl.h>
#include "IOverlayHelper.h" 

class DrmHelper : public IOverlayHelper, public SwUtils::Thread
{
public:
    static void Create();

    DrmHelper();
    ~DrmHelper();
    DrmHelper(const DrmHelper& other) = delete;
    DrmHelper& operator=(const DrmHelper& other) = delete;
    DrmHelper(DrmHelper&& other) = delete;
    DrmHelper& operator=(DrmHelper&& other) = delete;
    virtual bool Open(const char *const card, uint32_t width, uint32_t height, lv_color_format_t format) final override;

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
    
    class drm_object {
    public:
        bool valid;
        drmModeObjectProperties *props;
        drmModePropertyRes **props_info;
        uint32_t id;
    };

    struct modeset_buf {
        uint32_t width;
        uint32_t height;
        uint32_t stride;
        uint32_t size;
        uint32_t handle;
        uint8_t *map;
        uint32_t fb;
    };

    class modeset_dev {
    public:
        modeset_dev();

        drm_object connector;
        drm_object crtc;
        drm_object primary_plane;
        drm_object overlay_plane;
        bool overlay_dirty;

        drmModeModeInfo mode;
        uint32_t mode_blob_id;
        uint32_t crtc_index;

        bool pflip_pending;
        bool cleanup;

        bool pending_modeset;

        modeset_buf primary_bufs[NUM_BUFFERS];
        int32_t write_fb;
        int32_t read_fb;

        modeset_buf overlay_buf;

        std::chrono::steady_clock::time_point timeLastFlush;

        std::recursive_mutex lock_fb;
    };

    std::shared_ptr<modeset_dev> _modeset;

    bool FlushModeset();

    bool modeset_open(const char * const node);
    int64_t get_property_value(drmModeObjectPropertiesPtr props, const char *name);
    void modeset_get_object_properties(struct drm_object *obj, uint32_t type);
    bool set_drm_object_property(drmModeAtomicReq *req, struct drm_object *obj, const char *name, uint64_t value);
    bool modeset_find_crtc(drmModeRes *res, drmModeConnector *conn);
    bool modeset_find_primary_plane();
    bool modeset_find_overlay_plane();
    void modeset_drm_object_fini(struct drm_object *obj);
    bool modeset_setup_objects();
    void modeset_destroy_objects();
    bool modeset_create_fb(uint32_t drm_format, struct modeset_buf *buf);
    void modeset_destroy_fb(struct modeset_buf* buf);
    bool modeset_setup_primary_framebuffers(drmModeConnector *conn);
    bool modeset_setup_overlay_framebuffers(drmModeConnector *conn);
    void modeset_output_destroy();
    bool modeset_output_create(drmModeRes *res, drmModeConnector *conn);
    bool modeset_prepare();
    bool modeset_atomic_prepare_commit(drmModeAtomicReq *req);
    void modeset_cleanup();

    static void modeset_page_flip_event(int fd, unsigned int frame,
                    unsigned int sec, unsigned int usec,
                    unsigned int crtc_id, void *data);

private:
    int _fd;
    uint32_t _primary_drm_format;
    uint32_t _primary_stride;
    uint32_t _primary_fb_size;
    uint32_t _overlay_drm_format;
    uint32_t _overlay_stride;
    uint32_t _overlay_fb_size;

    lv_display_t * _primary_display;
    lv_display_t * _overlay_display;

    std::shared_ptr<SwUtils::OS> _os;
};


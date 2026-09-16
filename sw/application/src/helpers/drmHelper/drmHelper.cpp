/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "drmHelper.h"
#include <poll.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include <src/misc/lv_timer_private.h>
#include <src/core/lv_refr_private.h>

void DrmHelper::Create()
{
    std::shared_ptr<DrmHelper> spOverlayHelper = std::make_shared<DrmHelper>();
    IOverlayHelper::SetIOverlayHelperInst(spOverlayHelper);
}

DrmHelper::modeset_dev::modeset_dev()
    : mode_blob_id(0)
    , crtc_index(0)
    , pflip_pending(false)
    , cleanup(false)
    , pending_modeset(true)
    , write_fb(0)
    , read_fb(1)
    , timeLastFlush(std::chrono::steady_clock::now())
{
    memset(&connector, 0, sizeof(connector));
    memset(&crtc, 0, sizeof(crtc));
    memset(&primary_plane, 0, sizeof(primary_plane));
    memset(&overlay_plane, 0, sizeof(overlay_plane));
    memset(&mode, 0, sizeof(mode));
    memset(&primary_bufs, 0, sizeof(primary_bufs));
    memset(&overlay_buf, 0, sizeof(overlay_buf));
}

DrmHelper::DrmHelper()
    : SwUtils::Thread("DrmHelper")
    , _fd(-1)
    , _primary_stride(0)
    , _primary_fb_size(0)
    , _overlay_stride(0)
    , _overlay_fb_size(0)
{

    _os = SwUtils::OS::Get();
    lv_init();
}

DrmHelper::~DrmHelper()
{
    StopThread();
}

bool DrmHelper::Open(const char* const card, uint32_t width, uint32_t height, lv_color_format_t format)
{
    bool rc = false;

    /* open the DRM device */
	rc = modeset_open(card);

    if (rc)
    {
        /* prepare all connectors and CRTCs */
        rc = modeset_prepare();
    }

    if (rc)
    {
        SetPrimaryResolution(width, height, format);
        StartThread();
    }
    return rc;
}

bool DrmHelper::SetPrimaryResolutionLow(uint32_t width, uint32_t height, lv_color_format_t format)
{
    bool rc = true;
    switch(format)
    {
        case LV_COLOR_FORMAT_ARGB8888:
            _primary_drm_format = DRM_FORMAT_ARGB8888;
            break;
#if 0
        case LV_COLOR_FORMAT_ARGB4444:
            _primary_drm_format = DRM_FORMAT_ARGB4444;
            break;
#endif
        case LV_COLOR_FORMAT_ARGB2222:
            _primary_drm_format = DRM_FORMAT_R8;
            break;
        case LV_COLOR_FORMAT_XRGB8888:
            _primary_drm_format = DRM_FORMAT_XRGB8888;
            break;
        case LV_COLOR_FORMAT_UNKNOWN:
            _primary_drm_format = DRM_FORMAT_XRGB8888;
            break;
        default:
            std::cerr << "Unsupported color format " << format << std::endl;
            return false;
    }

    drmModeConnector *conn;
    conn = drmModeGetConnector(_fd, _modeset->connector.id);

    if (rc && !modeset_find_primary_plane()) {
        std::cerr << "no valid plane for crtc " << _modeset->crtc.id << std::endl;
    }

    if(rc && (_modeset->primary_plane.valid))
    {
        modeset_get_object_properties(&_modeset->primary_plane, DRM_MODE_OBJECT_PLANE);
        if (!_modeset->primary_plane.props)
        {
            rc = false;
        }
    }

    if (rc && !modeset_setup_primary_framebuffers(conn)) {
        std::cerr << "cannot create primary framebuffers for connector " << _modeset->connector.id << std::endl;
        modeset_destroy_objects();
        rc = false;
    }

    if(rc)
    {
        _primary_stride = _modeset->primary_bufs[0].stride;
        _primary_fb_size = GetPrimaryHeight() * _primary_stride;
        _modeset->pending_modeset = true;
    }

    return rc;
}

uint8_t* DrmHelper::GetPrimaryBuffer(int32_t index)
{
    if (index < 0 || index >= 2) {
        return nullptr;
    }
    return _modeset->primary_bufs[index].map;
}

bool DrmHelper::SetOverlayResolutionLow(uint32_t width, uint32_t height)
{
    bool rc = true;
    _overlay_drm_format = DRM_FORMAT_ARGB8888;

    drmModeConnector *conn;
    conn = drmModeGetConnector(_fd, _modeset->connector.id);

    if (rc && !modeset_find_overlay_plane()) {
        std::cerr << "no valid plane for crtc " << _modeset->crtc.id << std::endl;
    }

    if (rc && !modeset_setup_overlay_framebuffers(conn)) {
        std::cerr << "cannot create overlay framebuffers for connector " << _modeset->connector.id << std::endl;
        modeset_destroy_objects();
        rc = false;
    }

    if(rc && (_modeset->overlay_plane.valid))
    {
        modeset_get_object_properties(&_modeset->overlay_plane, DRM_MODE_OBJECT_PLANE);
        if (!_modeset->overlay_plane.props)
        {
            rc = false;
        }
    }

    if(rc)
    {
        _overlay_stride = _modeset->overlay_buf.stride;
        _overlay_fb_size = GetOverlayHeight() * _overlay_stride;
        _modeset->pending_modeset = true;
    }

    return rc;
}

uint8_t* DrmHelper::GetOverlayBuffer()
{
    return _modeset->overlay_buf.map;
}

bool DrmHelper::FlushPrimaryLow()
{
    bool rc = true;

    if(_modeset->write_fb == 0)
    {
        _modeset->read_fb = 0;
        _modeset->write_fb = 1;
    }
    else
    {
        _modeset->read_fb = 1;
        _modeset->write_fb = 0;
    }

    rc = FlushModeset();

    return rc;
}

bool DrmHelper::FlushOverlayLow()
{
    bool rc = true;

    std::lock_guard<std::recursive_mutex> lock(_modeset->lock_fb);
    _modeset->overlay_dirty = true;
    rc = FlushModeset();

    return rc;
}

bool DrmHelper::FlushModeset()
{
    bool rc = true;

    std::lock_guard<std::recursive_mutex> lock(IOverlayHelper::GetLVGLMutex());

    /* perform actual modesetting on each found connector+CRTC */
    if(_modeset)
    {
        std::lock_guard<std::recursive_mutex> lock(_modeset->lock_fb);

        if(_modeset->cleanup)
        {
            std::cerr << "FlushModeset: cleanup in progress, skipping flush" << std::endl;
            return false;
        }
        if(_modeset->pflip_pending)
        {
            //std::cerr << "FlushModeset: page flip pending, skipping flush" << std::endl;
            return false;
        }

        int ret, flags;
        drmModeAtomicReq *req;

        /* prepare modeset on all outputs */
        req = drmModeAtomicAlloc();
        if(!req)
        {
            std::cerr << "failed to allocate atomic request (" << errno << "): " << strerror(errno) << std::endl;
            rc = false;
        }
        if(rc)
        {
            rc = modeset_atomic_prepare_commit(req);
            if (!rc)
            {
                std::cerr << "prepare atomic commit failed (" << errno << "): " << strerror(errno) << std::endl;
                drmModeAtomicFree(req);
            }
        }

        if(rc)
        {
            if(_modeset->pending_modeset)
            {
                /* perform test-only atomic commit */
                flags = DRM_MODE_ATOMIC_TEST_ONLY | DRM_MODE_ATOMIC_ALLOW_MODESET;
                ret = drmModeAtomicCommit(_fd, req, flags, _modeset.get());
                if (ret < 0) {
                    std::cerr << "test-only atomic commit failed (" << errno << "): " << strerror(errno) << std::endl;
                    drmModeAtomicFree(req);
                    rc = false;
                }
            }
        }
        if(rc)
        {
            flags = DRM_MODE_PAGE_FLIP_EVENT;// | DRM_MODE_ATOMIC_NONBLOCK;
            if(_modeset->pending_modeset)
            {
                flags |= DRM_MODE_ATOMIC_ALLOW_MODESET;
                _modeset->pending_modeset = false;
            }
            else
            {
                flags |= DRM_MODE_ATOMIC_NONBLOCK;
            }
            ret = drmModeAtomicCommit(_fd, req, flags, _modeset.get());
            if (ret)
            {
                _modeset->timeLastFlush = std::chrono::steady_clock::now();
                drmModeAtomicFree(req);
                std::cerr << "modeset atomic commit failed (" << errno << "): " << strerror(errno) << std::endl;
                rc = false;
            }
            else
            {
                _modeset->pflip_pending = true;
                _modeset->timeLastFlush = std::chrono::steady_clock::now();
                drmModeAtomicFree(req);
            }
        }
    }

    return rc;
}

bool DrmHelper::modeset_open(const char *const node)
{
    int ret = 0;
    bool rc = true;
    
    uint64_t has_dumb;

    _fd = open(node, O_RDWR | O_CLOEXEC);
    if (_fd < 0) {
        rc = false;
        std::cerr << "cannot open '" << node << "': " << strerror(errno) << std::endl;
    }

    if(rc)
    {
        ret = drmSetClientCap(_fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
        if (ret)
        {
            std::cerr << "failed to set universal planes cap, " << ret << std::endl;
            rc = false;
        }
    }

    if(rc)
    {
        ret = drmSetClientCap(_fd, DRM_CLIENT_CAP_ATOMIC, 1);
        if (ret)
        {
            std::cerr << "failed to set atomic cap, " << ret << std::endl;
            rc = false;
        }
    }

    if(rc)
    {
        if (drmGetCap(_fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || !has_dumb)
        {
            std::cerr << "drm device '" << node << "' does not support dumb buffers" << std::endl;
            close(_fd);
            rc = false;
        }
    }

    return rc;
}

int64_t DrmHelper::get_property_value(drmModeObjectPropertiesPtr props, const char *name)
{
    drmModePropertyPtr prop;
    uint64_t value;
    bool found;

    found = false;
    if(props)
    {
        for (uint32_t j = 0; j < props->count_props && !found; j++)
        {
            prop = drmModeGetProperty(_fd, props->props[j]);
            if(prop)
            {
                if (!strcmp(prop->name, name))
                {
                    value = props->prop_values[j];
                    found = true;
                }
                drmModeFreeProperty(prop);
            }
        }
    }

    if (!found)
        return -1;
    return value;
}


bool DrmHelper::modeset_output_create(drmModeRes *res, drmModeConnector *conn)
{
    bool rc = true;

    /* creates an output structure */
    _modeset = std::make_shared<modeset_dev>();
    _modeset->connector.id = conn->connector_id;

    /* check if a monitor is connected */
    if (conn->connection != DRM_MODE_CONNECTED) {
        std::cerr << "ignoring unused connector " << conn->connector_id << std::endl;
        rc = false;
    }

    if(rc)
    {
        /* check if there is at least one valid mode */
        if (conn->count_modes == 0) {
            std::cerr << "no valid mode for connector " << conn->connector_id << std::endl;
            rc = false;
        }
    }

    if(rc)
    {
        /* copy the mode information into our output structure */
        memcpy(&_modeset->mode, &conn->modes[2], sizeof(_modeset->mode));
        /* create the blob property using out->mode and save its id in the output*/
        if (drmModeCreatePropertyBlob(_fd, &_modeset->mode, sizeof(_modeset->mode),
                                    &_modeset->mode_blob_id) != 0) {
            std::cerr << "couldn't create a blob property" << std::endl;
            rc = false;
        }
        else
        {
            std::cerr << "mode for connector " << conn->connector_id << " is " << _modeset->mode.hdisplay << "x" << _modeset->mode.vdisplay << std::endl;

            if (rc && !modeset_find_crtc(res, conn)) {
                std::cerr << "no valid crtc for connector " << conn->connector_id << std::endl;
                rc = false;
            }

            if (rc && !modeset_setup_objects()) {
                std::cerr << "cannot get plane properties" << std::endl;
                rc = false;
            }

            if(!rc)
            {
                uint32_t mode_blob_id = _modeset->mode_blob_id;
                _modeset = nullptr;
                drmModeDestroyPropertyBlob(_fd, mode_blob_id);
            }
        }
    }

    return rc;
}

bool DrmHelper::modeset_prepare()
{
    drmModeRes *res;
    drmModeConnector *conn;
    bool rc = true;

    /* retrieve resources */
    res = drmModeGetResources(_fd);
    if (!res) {
        std::cerr << "cannot retrieve DRM resources (" << errno << "): " << strerror(errno) << std::endl;
        rc = false;
    }

    if(rc)
    {
        /* we only expect one connectors */
        if(res->count_connectors >= 1)
        {
            uint32_t connector = res->connectors[0];
            /* get information for the connector */
            conn = drmModeGetConnector(_fd, connector);
            if (!conn) {
                std::cerr << "cannot retrieve DRM connector " << connector << " (" << errno << "): " << strerror(errno) << std::endl;
                rc = false;
            }

            if(rc)
            {
                rc = modeset_output_create(res, conn);
                drmModeFreeConnector(conn);
            }
        }
        /* free resources again */
        drmModeFreeResources(res);
    }
    return rc;
}

bool DrmHelper::modeset_find_crtc(drmModeRes *res, drmModeConnector *conn)
{
    drmModeEncoder *enc;
    uint32_t crtc;
    bool rc = false;

    /* first try the currently conected encoder+crtc */
    if (conn->encoder_id)
        enc = drmModeGetEncoder(_fd, conn->encoder_id);
    else
        enc = NULL;

    if (enc) {
        if (enc->crtc_id) {
            crtc = enc->crtc_id;
            if(_modeset)
            {
                if (_modeset->crtc.id == crtc)
                {
                    crtc = 0;
                }

                if (crtc > 0) {
                    _modeset->crtc.id = crtc;
                    /* find the CRTC's index */
                    for (int i = 0; i < res->count_crtcs; ++i) {
                        if (res->crtcs[i] == crtc) {
                            _modeset->crtc_index = i;
                            break;
                        }
                    }
                    rc = true;
                }
            }
        }

        drmModeFreeEncoder(enc);
    }

    if(!rc)
    {
        /* If the connector is not currently bound to an encoder or if the
        * encoder+crtc is already used by another connector (actually unlikely
        * but lets be safe), iterate all other available encoders to find a
        * matching CRTC. */
        for (int i = 0; i < conn->count_encoders; ++i) {
            enc = drmModeGetEncoder(_fd, conn->encoders[i]);
            if (!enc) {
                std::cerr << "cannot retrieve encoder " << i << ":" << conn->encoders[i] << " (" << errno << "): " << strerror(errno) << std::endl;
                continue;
            }

            /* iterate all global CRTCs */
            for (int j = 0; j < res->count_crtcs; ++j) {
                /* check whether this CRTC works with the encoder */
                if (!(enc->possible_crtcs & (1 << j)))
                    continue;

                /* check that no other device already uses this CRTC */
                crtc = res->crtcs[j];
                if(_modeset)
                {
                    if (_modeset->crtc.id == crtc)
                    {
                        crtc = 0;
                    }

                    /* we have found a CRTC, so save it and return */
                    if (crtc > 0) {
                        _modeset->crtc.id = crtc;
                        _modeset->crtc_index = j;
                        rc = true;
                        break;
                    }
                }
            }

            drmModeFreeEncoder(enc);
            if(rc)
            {
                break;
            }
        }
    }

    if(!rc)
    {
        std::cerr << "cannot find suitable CRTC for connector " << conn->connector_id << std::endl;
    }

    return rc;
}

bool DrmHelper::modeset_find_primary_plane()
{
    drmModePlaneResPtr plane_res;
    bool rc = true;

    plane_res = drmModeGetPlaneResources(_fd);
    if (!plane_res) {
        std::cerr << "drmModeGetPlaneResources failed: " << strerror(errno) << std::endl;
        rc = false;
    }

    if(rc)
    {
        rc = false;
        for (uint32_t i = 0; (i < plane_res->count_planes) && !rc; i++)
        {
            int plane_id = plane_res->planes[i];

            drmModePlanePtr plane = drmModeGetPlane(_fd, plane_id);
            if (!plane)
            {
                std::cerr << "drmModeGetPlane(" << plane_id << ") failed: " << strerror(errno) << std::endl;
                continue;
            }

            bool found_format = false;
            for(uint32_t f = 0; f < plane->count_formats; f++)
            {
                if(plane->formats[f] == _primary_drm_format)
                {
                    found_format = true;
                    break;
                }
            }
            if(!found_format)
            {
                drmModeFreePlane(plane);
                continue;
            }


            if (plane->possible_crtcs & (1 << _modeset->crtc_index))
            {
                drmModeObjectPropertiesPtr props =
                    drmModeObjectGetProperties(_fd, plane_id, DRM_MODE_OBJECT_PLANE);

                if (get_property_value(props, "type") == DRM_PLANE_TYPE_PRIMARY)
                {
                    _modeset->primary_plane.valid = true;
                    _modeset->primary_plane.id = plane_id;
                    rc = true;
                }

                drmModeFreeObjectProperties(props);
            }

            drmModeFreePlane(plane);
        }

        drmModeFreePlaneResources(plane_res);

        if (rc)
        {
            std::cout << "found primary plane, id: " << _modeset->primary_plane.id << std::endl;
        }
        else
        {
            std::cout << "couldn't find a primary plane" << std::endl;
        }
    }
    return rc;
}

bool DrmHelper::modeset_find_overlay_plane()
{
    drmModePlaneResPtr plane_res;
    bool rc = true;

    plane_res = drmModeGetPlaneResources(_fd);
    if (!plane_res) {
        std::cerr << "drmModeGetPlaneResources failed: " << strerror(errno) << std::endl;
        rc = false;
    }

    if(rc)
    {
        rc = false;
        for (uint32_t i = 0; (i < plane_res->count_planes) && !rc; i++)
        {
            int plane_id = plane_res->planes[i];

            drmModePlanePtr plane = drmModeGetPlane(_fd, plane_id);
            if (!plane)
            {
                std::cerr << "drmModeGetPlane(" << plane_id << ") failed: " << strerror(errno) << std::endl;
                continue;
            }

            bool found_format = false;
            for(uint32_t f = 0; f < plane->count_formats; f++)
            {
                if(plane->formats[f] == _overlay_drm_format)
                {
                    found_format = true;
                    break;
                }
            }
            if(!found_format)
            {
                drmModeFreePlane(plane);
                continue;
            }


            if (plane->possible_crtcs & (1 << _modeset->crtc_index))
            {
                drmModeObjectPropertiesPtr props =
                    drmModeObjectGetProperties(_fd, plane_id, DRM_MODE_OBJECT_PLANE);

                if (get_property_value(props, "type") == DRM_PLANE_TYPE_OVERLAY)
                {
                    _modeset->overlay_plane.valid = true;
                    _modeset->overlay_plane.id = plane_id;
                    rc = true;
                }

                drmModeFreeObjectProperties(props);
            }

            drmModeFreePlane(plane);
        }

        drmModeFreePlaneResources(plane_res);

        if (rc)
        {
            std::cout << "found overlay plane, id: " << _modeset->overlay_plane.id << std::endl;
        }
        else
        {
            std::cout << "couldn't find a overlay plane" << std::endl;
        }
    }
    return rc;
}

void DrmHelper::modeset_drm_object_fini(struct drm_object *obj)
{
    for (uint32_t i = 0; i < obj->props->count_props; i++)
    {
        drmModeFreeProperty(obj->props_info[i]);
    }
    free(obj->props_info);
    obj->props_info = nullptr;
    drmModeFreeObjectProperties(obj->props);
    obj->props = nullptr;
}

bool DrmHelper::modeset_setup_objects()
{
    bool rc = true;
    struct drm_object *connector = &_modeset->connector;
    struct drm_object *crtc = &_modeset->crtc;
    struct drm_object *primary_plane = &_modeset->primary_plane;
    struct drm_object *overlay_plane = &_modeset->overlay_plane;

    modeset_get_object_properties(connector, DRM_MODE_OBJECT_CONNECTOR);
    if (!connector->props)
    {
        rc = false;
    }

    if(rc)
    {
        modeset_get_object_properties(crtc, DRM_MODE_OBJECT_CRTC);
        if (!crtc->props)
        {
            modeset_drm_object_fini(connector);
            rc = false;
        }
    }

    if(rc && (primary_plane->id != 0))
    {
        modeset_get_object_properties(primary_plane, DRM_MODE_OBJECT_PLANE);
        if (!primary_plane->props)
        {
            modeset_drm_object_fini(crtc);
            modeset_drm_object_fini(connector);
            rc = false;
        }
    }

    if(rc && (overlay_plane->id != 0))
    {
        modeset_get_object_properties(overlay_plane, DRM_MODE_OBJECT_PLANE);
        if (!overlay_plane->props)
        {
            modeset_drm_object_fini(primary_plane);
            modeset_drm_object_fini(crtc);
            modeset_drm_object_fini(connector);
            rc = false;
        }
    }

    return rc;
}

void DrmHelper::modeset_destroy_objects()
{
    modeset_drm_object_fini(&_modeset->connector);
    modeset_drm_object_fini(&_modeset->crtc);
    modeset_drm_object_fini(&_modeset->primary_plane);
    modeset_drm_object_fini(&_modeset->overlay_plane);
}

bool DrmHelper::modeset_create_fb(uint32_t drm_format, struct modeset_buf *buf)
{
    struct drm_mode_create_dumb creq;
    struct drm_mode_destroy_dumb dreq;
    struct drm_mode_map_dumb mreq;
    bool rc = true;
    int ret;
    uint32_t handles[4] = {0, 0, 0, 0};
    uint32_t pitches[4] = {0, 0, 0, 0};
    uint32_t offsets[4] = {0, 0, 0, 0};

    /* create dumb buffer */
    memset(&creq, 0, sizeof(creq));
    creq.width = buf->width;
    creq.height = buf->height;
    switch(drm_format)
    {
    case DRM_FORMAT_ARGB8888:
        creq.bpp = 32;
        break;
#if 0
    case DRM_FORMAT_ARGB4444:
        creq.bpp = 16;
        break;
#endif
    case DRM_FORMAT_R8:
        creq.bpp = 8;
        break;
    case DRM_FORMAT_XRGB8888:   
        creq.bpp = 32;
        break;
    default:
        creq.bpp = 32;
        break;
    }

    ret = drmIoctl(_fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
    if (ret < 0) {
        std::cerr << "cannot create dumb buffer (" << errno << "): " << strerror(errno) << std::endl;
        rc = false;
    }

    if(rc)
    {
        buf->stride = creq.pitch;
        buf->size = creq.size;
        buf->handle = creq.handle;

        handles[0] = buf->handle;
        pitches[0] = buf->stride;

        ret = drmModeAddFB2(_fd, buf->width, buf->height, 
                                drm_format, 
                                handles, pitches, offsets, 
                                &buf->fb, 0);

        if (ret) {
            std::cerr << "cannot create framebuffer (" << errno << "): " << strerror(errno) << std::endl;
            rc = false;
        }

        if(rc)
        {

            /* prepare buffer for memory mapping */
            memset(&mreq, 0, sizeof(mreq));
            mreq.handle = buf->handle;
            ret = drmIoctl(_fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
            if (ret) {
                std::cerr << "cannot map dumb buffer (" << errno << "): " << strerror(errno) << std::endl;
                rc = false;
            }
        }

        if(rc)
        {
            /* perform actual memory mapping */
            buf->map = (uint8_t*)mmap(0, buf->size, PROT_READ | PROT_WRITE, MAP_SHARED,
                        _fd, mreq.offset);
            if (buf->map == MAP_FAILED) {
                std::cerr << "cannot mmap dumb buffer (" << errno << "): " << strerror(errno) << std::endl;
                rc = false;
            }

            if(rc)
            {
                /* clear the framebuffer to 0 */
                memset(buf->map, 0x33, buf->size);
            }
        }
    }

    if(!rc)
    {
        if(buf->fb)
        {
            drmModeRmFB(_fd, buf->fb);
        }
        memset(&dreq, 0, sizeof(dreq));
        dreq.handle = buf->handle;
        drmIoctl(_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
    }
    return rc;
}

void DrmHelper::modeset_destroy_fb(struct modeset_buf *buf)
{
    struct drm_mode_destroy_dumb dreq;

    munmap(buf->map, buf->size);

    /* delete framebuffer */
    drmModeRmFB(_fd, buf->fb);

    /* delete dumb buffer */
    memset(&dreq, 0, sizeof(dreq));
    dreq.handle = buf->handle;
    drmIoctl(_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
}

bool DrmHelper::modeset_setup_primary_framebuffers(drmModeConnector *conn)
{
    bool rc = true;

    for (uint32_t i = 0; i < NUM_BUFFERS; i++)
    {
        _modeset->primary_bufs[i].width = _modeset->mode.hdisplay;
        _modeset->primary_bufs[i].height = _modeset->mode.vdisplay;

        rc = modeset_create_fb( _primary_drm_format, &_modeset->primary_bufs[i]);
        if (!rc) {
            for (uint32_t j = 0; j < i; j++)
            {
                modeset_destroy_fb(&_modeset->primary_bufs[j]);
            }
            break;
        }
    }

    if(rc)
    {
        std::lock_guard<std::recursive_mutex> lock(_modeset->lock_fb);

        _modeset->write_fb = 0;
    }

    return rc;
}

bool DrmHelper::modeset_setup_overlay_framebuffers(drmModeConnector *conn)
{
    bool rc = true;

    _modeset->overlay_buf.width = GetOverlayWidth();
    _modeset->overlay_buf.height = GetOverlayHeight();

    if(_modeset->overlay_buf.fb)
    {
        drmModeRmFB(_fd, _modeset->overlay_buf.fb);
    }

    rc = modeset_create_fb( _overlay_drm_format, &_modeset->overlay_buf);

    return rc;
}

void DrmHelper::modeset_get_object_properties(struct drm_object *obj, uint32_t type)
{
    const char *type_str;
    unsigned int i;

    obj->props = drmModeObjectGetProperties(_fd, obj->id, type);
    if (!obj->props) {
        switch(type) {
            case DRM_MODE_OBJECT_CONNECTOR:
                type_str = "connector";
                break;
            case DRM_MODE_OBJECT_PLANE:
                type_str = "plane";
                break;
            case DRM_MODE_OBJECT_CRTC:
                type_str = "CRTC";
                break;
            default:
                type_str = "unknown type";
                break;
        }
        std::cerr << "cannot get " << type_str << " " << obj->id << " properties: " << strerror(errno) << std::endl;
        return;
    }

    obj->props_info = (drmModePropertyRes **)calloc(obj->props->count_props, sizeof(*obj->props_info));
    for (i = 0; i < obj->props->count_props; i++)
    {
        obj->props_info[i] = drmModeGetProperty(_fd, obj->props->props[i]);
    }
}

bool DrmHelper::set_drm_object_property(drmModeAtomicReq *req, drm_object *obj,
                   const char *name, uint64_t value)
{
    uint32_t prop_id = 0;
    bool rc = false;

    for (uint32_t i = 0; i < obj->props->count_props; i++) {
        if ((obj->props_info[i]) && !strcmp(obj->props_info[i]->name, name)) {
            prop_id = obj->props_info[i]->prop_id;
            rc = true;
            break;
        }
    }

    if (!rc)
    {
        std::cerr << "no object property: " << name << std::endl;
    }

    if(rc)
    {
        int ret = drmModeAtomicAddProperty(req, obj->id, prop_id, value);
        if(ret < 0)
        {
            rc = false;
        }
    }
    return rc;
}

bool DrmHelper::modeset_atomic_prepare_commit(drmModeAtomicReq *req)
{
    bool rc = true;
    drm_object *primary_plane = &_modeset->primary_plane;
    modeset_buf *primary_buf = &_modeset->primary_bufs[_modeset->read_fb];
    drm_object *overlay_plane = &_modeset->overlay_plane;
    modeset_buf *overlay_buf = &_modeset->overlay_buf;

    const bool disable_planes = _modeset->cleanup;

    const uint64_t primary_fb_id = disable_planes ? 0 : primary_buf->fb;
    const uint64_t primary_crtc_id = disable_planes ? 0 : _modeset->crtc.id;
    const uint64_t primary_src_w = disable_planes ? 0 : (static_cast<uint64_t>(primary_buf->width) << 16);
    const uint64_t primary_src_h = disable_planes ? 0 : (static_cast<uint64_t>(primary_buf->height) << 16);
    const uint64_t primary_crtc_w = disable_planes ? 0 : primary_buf->width;
    const uint64_t primary_crtc_h = disable_planes ? 0 : primary_buf->height;

    const uint64_t overlay_fb_id = disable_planes ? 0 : overlay_buf->fb;
    const uint64_t overlay_crtc_id = disable_planes ? 0 : _modeset->crtc.id;
    const uint64_t overlay_src_w = disable_planes ? 0 : (static_cast<uint64_t>(overlay_buf->width) << 16);
    const uint64_t overlay_src_h = disable_planes ? 0 : (static_cast<uint64_t>(overlay_buf->height) << 16);
    const uint64_t overlay_crtc_w = disable_planes ? 0 : overlay_buf->width;
    const uint64_t overlay_crtc_h = disable_planes ? 0 : overlay_buf->height;

    const uint64_t connector_crtc_id = disable_planes ? 0 : _modeset->crtc.id;
    const uint64_t crtc_mode_id = disable_planes ? 0 : _modeset->mode_blob_id;
    const uint64_t crtc_active = disable_planes ? 0 : 1;

    if (!set_drm_object_property(req, &_modeset->connector, "CRTC_ID", connector_crtc_id))
    {
        rc = false;
    }

    if (!set_drm_object_property(req, &_modeset->crtc, "MODE_ID", crtc_mode_id))
    {
        rc = false;
    }

    if (!set_drm_object_property(req, &_modeset->crtc, "ACTIVE", crtc_active))
    {
        rc = false;
    }

    if (!set_drm_object_property(req, primary_plane, "FB_ID", primary_fb_id))
    {
        rc = false;
    }
    if (!set_drm_object_property(req, primary_plane, "CRTC_ID", primary_crtc_id))
    {
        rc = false;
    }
    if (!set_drm_object_property(req, primary_plane, "SRC_X", 0))
    {
        rc = false;
    }
    if (!set_drm_object_property(req, primary_plane, "SRC_Y", 0))
    {
        rc = false;
    }
    if (!set_drm_object_property(req, primary_plane, "SRC_W", primary_src_w))
    {
        rc = false;
    }
    if (!set_drm_object_property(req, primary_plane, "SRC_H", primary_src_h))
    {
        rc = false;
    }
    if (!set_drm_object_property(req, primary_plane, "CRTC_X", 0))
    {
        rc = false;
    }
    if (!set_drm_object_property(req, primary_plane, "CRTC_Y", 0))
    {
        rc = false;
    }
    if (!set_drm_object_property(req, primary_plane, "CRTC_W", primary_crtc_w))
    {
        rc = false;
    }
    if (!set_drm_object_property(req, primary_plane, "CRTC_H", primary_crtc_h))
    {
        rc = false;
    }

    {
        uint32_t damage_blob_id = 0;
        struct drm_mode_rect rects[1];
        rects[0].x1 = 0;
        rects[0].y1 = 0;
        rects[0].x2 = primary_buf->width;
        rects[0].y2 = primary_buf->height;
        uint32_t num_rects = 1;

        drmModeCreatePropertyBlob(_fd, &rects, sizeof(struct drm_mode_rect) * num_rects, &damage_blob_id);

        if (!set_drm_object_property(req, primary_plane, "FB_DAMAGE_CLIPS", damage_blob_id))
        {
            rc = false;
        }
    }

    if(_modeset->overlay_plane.valid)
    {
        if (!set_drm_object_property(req, overlay_plane, "FB_ID", overlay_fb_id))
        {
            rc = false;
        }
        if (!set_drm_object_property(req, overlay_plane, "CRTC_ID", overlay_crtc_id))
        {
            rc = false;
        }
        if (!set_drm_object_property(req, overlay_plane, "SRC_X", 0))
        {
            rc = false;
        }
        if (!set_drm_object_property(req, overlay_plane, "SRC_Y", 0))
        {
            rc = false;
        }
        if (!set_drm_object_property(req, overlay_plane, "SRC_W", overlay_src_w))
        {
            rc = false;
        }
        if (!set_drm_object_property(req, overlay_plane, "SRC_H", overlay_src_h))
        {
            rc = false;
        }
        if (!set_drm_object_property(req, overlay_plane, "CRTC_X", 0))
        {
            rc = false;
        }
        if (!set_drm_object_property(req, overlay_plane, "CRTC_Y", 0))
        {
            rc = false;
        }
        if (!set_drm_object_property(req, overlay_plane, "CRTC_W", overlay_crtc_w))
        {
            rc = false;
        }
        if (!set_drm_object_property(req, overlay_plane, "CRTC_H", overlay_crtc_h))
        {
            rc = false;
        }

        {
            uint32_t damage_blob_id = 0;
            struct drm_mode_rect rects[1];
            uint32_t num_rects = 1;
            if(_modeset->overlay_dirty)
            {
                _modeset->overlay_dirty = false;
                rects[0].x1 = 0;
                rects[0].y1 = 0;
                rects[0].x2 = overlay_buf->width;
                rects[0].y2 = overlay_buf->height;
            }
            else
            {
                // special case for when the overlay is not dirty, we need to set a valid rectangle to avoid drmModeAtomicCommit() failing with EINVAL
                rects[0].x1 = 0;
                rects[0].y1 = 0;
                rects[0].x2 = 2;
                rects[0].y2 = 2;
            }
            drmModeCreatePropertyBlob(_fd, &rects, sizeof(struct drm_mode_rect) * num_rects, &damage_blob_id);
            if (!set_drm_object_property(req, overlay_plane, "FB_DAMAGE_CLIPS", damage_blob_id))
            {
                rc = false;
            }
        }
    }

    return rc;
}

void DrmHelper::modeset_page_flip_event(int fd, unsigned int frame,
                    unsigned int sec, unsigned int usec,
                    unsigned int crtc_id, void *data)
{
    modeset_dev* dev = (modeset_dev *)data;
    if(dev)
    {
        std::lock_guard<std::recursive_mutex> lock(dev->lock_fb);
        //dev->write_fb = (dev->write_fb == 1) ? 0 : 1;
        dev->pflip_pending = false;
    }
}

void  DrmHelper::modeset_output_destroy()
{
    /* destroy connector, crtc and plane objects */
    modeset_destroy_objects();

    /* destroy front/back framebuffers */
    for (uint32_t i = 0; i < NUM_BUFFERS; i++)
    {
        modeset_destroy_fb(&_modeset->primary_bufs[i]);
    }
    modeset_destroy_fb(&_modeset->overlay_buf);

    /* destroy mode blob property */
    drmModeDestroyPropertyBlob(_fd, _modeset->mode_blob_id);
}

void DrmHelper::modeset_cleanup()
{
    drmEventContext ev;
    int ret;

    /* init variables */
    memset(&ev, 0, sizeof(ev));
    ev.version = 3;
    ev.page_flip_handler2 = modeset_page_flip_event;

    /* if a page-flip is pending, wait for it to complete */
    _modeset->cleanup = true;
    std::cerr << "wait for pending page-flip to complete..." << std::endl;
    while (_modeset->pflip_pending) {
        ret = drmHandleEvent(_fd, &ev);
        if (ret)
            break;
    }

    int flags = DRM_MODE_ATOMIC_ALLOW_MODESET;
    bool rc = true;
    drmModeAtomicReq *req;

    /* prepare modeset on all outputs */
    req = drmModeAtomicAlloc();
    if(!req)
    {
        std::cerr << "failed to allocate atomic request (" << errno << "): " << strerror(errno) << std::endl;
        rc = false;
    }
    if(rc)
    {
        rc = modeset_atomic_prepare_commit(req);
        if (!rc)
        {
            std::cerr << "prepare atomic commit failed (" << errno << "): " << strerror(errno) << std::endl;
            drmModeAtomicFree(req);
        }
    }

    if(rc)
    {
        ret = drmModeAtomicCommit(_fd, req, flags, _modeset.get());
        if (ret)
        {
            drmModeAtomicFree(req);
            std::cerr << "modeset atomic commit failed (" << errno << "): " << strerror(errno) << std::endl;
            rc = false;
        }
        else
        {
            drmModeAtomicFree(req);
        }
    }

    /* destroy current output */
    modeset_output_destroy();
    _modeset->cleanup = false;
}


void DrmHelper::RunThread()
{
    prctl(PR_SET_NAME, _threadName.c_str(),0,0,0);

    int ret;
    struct pollfd  pfds;
     int timeout;
    drmEventContext ev;

    timeout = 100;

    memset(&ev, 0, sizeof(ev));
    ev.version = 3;
    ev.page_flip_handler2 = modeset_page_flip_event;

    pfds.fd = _fd;
    pfds.events = POLLIN;
    pfds.revents = 0;

    while (!_shutdownEvent.IsSignalled())
    {
        ret = poll(&pfds, 1, timeout);
        if (ret < 0) {
            std::cerr << "poll() failed with " << errno << ": " << strerror(errno) << std::endl;
            break;
        } else if (pfds.revents & POLLIN) {
            drmHandleEvent(_fd, &ev);
            pfds.revents = 0;
        }
    }

    modeset_cleanup();
}

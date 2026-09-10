/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __VFR_DRM_DRIVER_H__
#define __VFR_DRM_DRIVER_H__

#include <drm/drm_fourcc.h>
#include <linux/fb.h>
#include <linux/types.h>
#include <drm/drm_device.h>
#include <drm/drm_connector.h>
#include <drm/drm_crtc.h>
#include <drm/drm_encoder.h>

#ifdef USE_DMA
#include "vfr_drm_dma.h"
#endif

#include "vfr_drm_plane.h"

/* format array, use it to initialize a "struct vfr_drm_format" array */
#define VFR_DRM_FORMATS \
{ \
	{ "r5g6b5", 16, {11, 5}, {5, 6}, {0, 5}, {0, 0}, DRM_FORMAT_RGB565 }, \
	{ "r5g5b5a1", 16, {11, 5}, {6, 5}, {1, 5}, {0, 1}, DRM_FORMAT_RGBA5551 }, \
	{ "x1r5g5b5", 16, {10, 5}, {5, 5}, {0, 5}, {0, 0}, DRM_FORMAT_XRGB1555 }, \
	{ "a1r5g5b5", 16, {10, 5}, {5, 5}, {0, 5}, {15, 1}, DRM_FORMAT_ARGB1555 }, \
	{ "r8g8b8", 24, {16, 8}, {8, 8}, {0, 8}, {0, 0}, DRM_FORMAT_RGB888 }, \
	{ "x8r8g8b8", 32, {16, 8}, {8, 8}, {0, 8}, {0, 0}, DRM_FORMAT_XRGB8888 }, \
	{ "a8r8g8b8", 32, {16, 8}, {8, 8}, {0, 8}, {24, 8}, DRM_FORMAT_ARGB8888 }, \
	{ "x8b8g8r8", 32, {0, 8}, {8, 8}, {16, 8}, {0, 0}, DRM_FORMAT_XBGR8888 }, \
	{ "a8b8g8r8", 32, {0, 8}, {8, 8}, {16, 8}, {24, 8}, DRM_FORMAT_ABGR8888 }, \
	{ "x2r10g10b10", 32, {20, 10}, {10, 10}, {0, 10}, {0, 0}, DRM_FORMAT_XRGB2101010 }, \
	{ "a2r10g10b10", 32, {20, 10}, {10, 10}, {0, 10}, {30, 2}, DRM_FORMAT_ARGB2101010 }, \
	{ "a2r2g2b2", 8, {4, 2}, {2, 2}, {0, 2}, {6, 2}, DRM_FORMAT_R8 }, \
	{ "a4r4g4b4", 16, {8, 4}, {4, 4}, {0, 4}, {12, 4}, DRM_FORMAT_ARGB4444 }, \
}

/*
 * Data-Format for vfr-drm
 * @name: unique 0-terminated name that can be used to identify the mode
 * @red,green,blue: Offsets and sizes of the single RGB parts
 * @transp: Offset and size of the alpha bits. length=0 means no alpha
 * @fourcc: 32bit DRM four-CC code (see drm_fourcc.h)
 */
struct vfr_drm_format {
	const char *name;
	u32 bits_per_pixel;
	struct fb_bitfield red;
	struct fb_bitfield green;
	struct fb_bitfield blue;
	struct fb_bitfield transp;
	u32 fourcc;
};

struct vfr_drm_device {
    struct drm_device dev;

    /* vfr_drm settings */
    struct drm_display_mode default_mode;
    const struct drm_format_info *drm_format_info[8];

    /* memory management */
    struct iosys_map vfr_buffer_base;

    struct vfr_drm_plane primary;
    struct vfr_drm_plane overlay;

    /* modesetting */
    struct drm_crtc crtc;
    struct drm_encoder encoder;
    struct drm_connector connector;
    
    spinlock_t lock;
    bool irq_enable;
    struct hrtimer vblank_hrtimer;
    ktime_t vblank_period_ns;
    
#ifdef USE_DMA
    struct vfr_drm_dma_ctx dma_ctx;
#endif
};

struct vfr_drm_device *vfr_drm_device_of_dev(struct drm_device *dev);

#endif /* __VFR_DRM_DRIVER_H__ */

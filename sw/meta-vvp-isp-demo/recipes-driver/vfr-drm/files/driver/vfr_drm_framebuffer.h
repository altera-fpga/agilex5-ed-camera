/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __VFR_DRM_FRAMEBUFFER_H__
#define __VFR_DRM_FRAMEBUFFER_H__

#include <linux/fb.h>
#include <linux/types.h>
#include <drm/drm_framebuffer.h>


struct vfr_drm_framebuffer
{
    struct drm_framebuffer fb;
};

struct vfr_drm_framebuffer *vfr_drm_framebuffer_of_fb(struct drm_framebuffer *fb);

struct drm_framebuffer *
vfr_drm_fb_create(struct drm_device *dev, struct drm_file *file,
			     const struct drm_format_info *info,
			     const struct drm_mode_fb_cmd2 *mode_cmd);
                 
#endif /* __VFR_DRM_FRAMEBUFFER_H__ */

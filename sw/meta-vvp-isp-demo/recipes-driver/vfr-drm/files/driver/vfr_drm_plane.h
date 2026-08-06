/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __VFR_DRM_PLANE_H__
#define __VFR_DRM_PLANE_H__

#include <linux/types.h>
#include <linux/iosys-map.h>
#include "intel_vvp_vfr.h"
#include "intel_addr_span_expander.h"

struct vfr_drm_plane {
    struct drm_plane plane;
    struct vfr_drm_device *sdev;
    uint32_t format;
    unsigned int width;
    unsigned int height;
    unsigned int pitch;
    enum drm_plane_type plane_type;
    unsigned int vfr_irq_index;
    unsigned int vfr_mem_index;
    unsigned int ase_mem_index;
    phys_addr_t fb_offset;
    struct iosys_map base;
    size_t nformats;
    uint32_t formats[8];
    int hw_irq;
    intel_addr_span_expander_instance ase_instance;
    intel_vvp_vfr_instance vfr_instance;
};

struct vfr_drm_plane *vfr_drm_plane_of_plane(struct drm_plane *plane);

int vfr_drm_plane_add(struct vfr_drm_device *sdev, struct platform_device *pdev, struct vfr_drm_plane *vfr_drm_p);

void vfr_drm_plane_remove(struct vfr_drm_plane *vfr_drm_p);

#endif /* __VFR_DRM_PLANE_H__ */

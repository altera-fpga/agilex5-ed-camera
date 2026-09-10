/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __VFR_DRM_DMA_H__
#define __VFR_DRM_DMA_H__

#include <linux/types.h>

struct vfr_drm_device;
struct vfr_drm_framebuffer;

struct vfr_drm_dma_ctx
{
    struct dma_chan *dma;
    phys_addr_t dma_emif_start_addr;
    phys_addr_t dma_emif_end_addr;
};

int vfr_drm_dma_init(struct vfr_drm_device *sdev, struct platform_device *pdev);
void vfr_drm_dma_deinit(struct vfr_drm_device *sdev);

ssize_t vfr_drm_dma_write(struct vfr_drm_device *sdev, struct vfr_drm_framebuffer *vfr_drm_fb, dma_addr_t addr, const loff_t *offp);

#endif /* __VFR_DRM_DMA_H__ */

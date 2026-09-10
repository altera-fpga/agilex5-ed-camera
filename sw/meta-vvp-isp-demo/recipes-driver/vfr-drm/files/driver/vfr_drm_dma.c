// SPDX-License-Identifier: GPL-2.0-only

#include <linux/aperture.h>
#include <linux/clk.h>
#include <linux/of_clk.h>
#include <linux/minmax.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_data/simplefb.h>
#include <linux/platform_device.h>
#include <linux/pm_domain.h>
#include <linux/regulator/consumer.h>
#include <linux/jiffies.h>
#include <linux/hrtimer.h>

#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>

#include <drm/drm_print.h>
#include <drm/drm_atomic.h>
#include <drm/drm_device.h>
#include <drm/drm_drv.h>
#include <drm/drm_gem.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_gem_framebuffer_helper.h>

#include "vfr_drm_driver.h"
#include "vfr_drm_dma.h"
#include "vfr_drm_plane.h"
#include "vfr_drm_framebuffer.h"


#ifdef USE_DMA

/*
 * DMA Helpers for vfr_drm
 */

static int set_dma_emif_addr(struct vfr_drm_device *sdev, struct dma_chan *dma,
              const loff_t *offp, const size_t count);
              
              
int vfr_drm_dma_init(struct vfr_drm_device *sdev, struct platform_device *pdev)
{
    struct drm_device *dev = &sdev->dev;
    struct device_node *of_node = pdev->dev.of_node;
    struct vfr_drm_dma_ctx *dma_ctx = &sdev->dma_ctx;
    const char *chan_str = { 0 };
    phys_addr_t of_val;

    
    /* We need to get the DT entry for the EMIF connected memory region */
#ifdef CONFIG_PHYS_ADDR_T_64BIT
    if (of_property_read_u64(of_node, "dma_mem_start", &of_val)) {
#else
    if (of_property_read_u32(of_node, "dma_mem_start", &of_val)) {
#endif
        drm_err(dev, "dma_mem_start not specified.\n");
        return -ENODEV;
    }
    dma_ctx->dma_emif_start_addr = of_val;

#ifdef CONFIG_PHYS_ADDR_T_64BIT
    if (of_property_read_u64(of_node, "dma_mem_end", &of_val)) {
#else
    if (of_property_read_u32(of_node, "dma_mem_end", &of_val)) {
#endif
        drm_err(dev, "dma_mem_end not specified.\n");
        return -ENODEV;
    }
    dma_ctx->dma_emif_end_addr = of_val;

    if (of_property_read_string(of_node, "dma-names",
                    &chan_str)) {
        drm_err(dev, "dma_chan name not found in devicetree.\n");
        return -ENODEV;
    }
    dma_ctx->dma = dma_request_chan(&pdev->dev, chan_str);
    if (IS_ERR(dma_ctx->dma)) {
        drm_err(dev, "Failed to request dma %px\n", dma_ctx->dma);
        return PTR_ERR(dma_ctx->dma);
    }
    
    return 0;
}


void vfr_drm_dma_deinit(struct vfr_drm_device *sdev)
{
    if (sdev->dma_ctx.dma) {
        dma_release_channel(sdev->dma_ctx.dma);
        sdev->dma_ctx.dma = NULL;
    }
}


static int set_dma_emif_addr(struct vfr_drm_device *sdev, struct dma_chan *dma,
              const loff_t *offp, const size_t count)
{
    struct drm_device *dev = &sdev->dev;
    struct vfr_drm_dma_ctx *dma_ctx = &sdev->dma_ctx;

    struct dma_slave_config dma_config = { 0 };
    phys_addr_t emif_addr = dma_ctx->dma_emif_start_addr + *offp;
    uint32_t dma_align = ((1 << dma->device->copy_align) - 1);

    /* Check that we are setting up correct emif address */
    if (emif_addr < dma_ctx->dma_emif_start_addr) {
        drm_warn(dev, "%s:%u: set_dma_emif_addr\n", __func__, __LINE__);
        return -EINVAL;
    }
    if ((emif_addr + count) >= dma_ctx->dma_emif_end_addr) {
        drm_warn(dev, "%s:%u: set_dma_emif_addr\n", __func__, __LINE__);
        return -EINVAL;
    }

    /* Check that the address and size as on the correct dma align */
    if (emif_addr & dma_align) {
        drm_warn(dev, "%s:%u: set_dma_emif_addr\n", __func__, __LINE__);
        return -EINVAL;
    }
    if (count & dma_align) {
        drm_warn(dev, "%s:%u: set_dma_emif_addr\n", __func__, __LINE__);
        return -EINVAL;
    }

    dma_config.src_addr = emif_addr;
    dma_config.dst_addr = emif_addr;
    return dmaengine_slave_config(dma, &dma_config);
}


ssize_t vfr_drm_dma_write(struct vfr_drm_device *sdev, struct vfr_drm_framebuffer *vfr_drm_fb, dma_addr_t addr, const loff_t *offp)
{
    struct drm_device *dev = &sdev->dev;
    struct vfr_drm_dma_ctx *dma_ctx = &sdev->dma_ctx;
    struct drm_gem_object *obj = drm_gem_fb_get_obj(&vfr_drm_fb->fb, 0);

    int result;
    dma_cookie_t cookie;
    enum dma_status status;
    struct scatterlist sg;
    struct dma_async_tx_descriptor *descriptor = NULL;

    drm_dbg(dev, "%s:%u: Writing %zu bytes at position %lld\n",
         __func__, __LINE__, obj->size, *offp);

    sg_init_table(&sg, 1);
    sg_dma_address(&sg) = addr;
    sg_dma_len(&sg) = obj->size;

    /* Setup the EMIF region prior to the transfer */
    result = set_dma_emif_addr(sdev, dma_ctx->dma, offp, obj->size);
    if (result < 0) {
        goto skip_dma;
    }

    descriptor =
        dmaengine_prep_slave_sg(dma_ctx->dma, &sg,
                            1, DMA_MEM_TO_DEV,
                            DMA_PREP_INTERRUPT);
    if (!descriptor) {
        drm_warn(dev, "Failed to prepare DMA descriptor\n");
        result = -EINVAL;
        goto skip_dma;
    }

    cookie = dmaengine_submit(descriptor);
    if (dma_submit_error(cookie)) {
        drm_warn(dev, "dma_submit_error failed\n");
        result = -EINVAL;
        goto skip_dma;
    }

    status = dma_sync_wait(dma_ctx->dma, cookie);
    dmaengine_terminate_sync(dma_ctx->dma);
    if (status != DMA_COMPLETE) {
        drm_warn(dev, "DMA Failed\n");
        result = -EINVAL;
        goto skip_dma;
    }
    else
    {
        drm_dbg(dev, "DMA passed\n");
        result = obj->size;
    }
    
skip_dma:
    return result;
}

#endif
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

#include <drm/clients/drm_client_setup.h>
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_state_helper.h>
#include <drm/drm_connector.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_damage_helper.h>
#include <drm/drm_device.h>
#include <drm/drm_drv.h>
#include <drm/drm_fbdev_shmem.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_gem_atomic_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_gem_shmem_helper.h>
#include <drm/drm_managed.h>
#include <drm/drm_modeset_helper_vtables.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_vblank.h>
#include <drm/drm_panic.h>

#include "vfr_drm_driver.h"
#include "vfr_drm_plane.h"
#include "vfr_drm_framebuffer.h"
#include "intel_vvp_vfr.h"
#include "intel_addr_span_expander.h"

#define DRIVER_NAME    "vfr-drm"
#define DRIVER_DESC    "DRM driver for altera VFR DRM platform devices"
#define DRIVER_MAJOR    1
#define DRIVER_MINOR    0


/*
 * Helpers for vfr_drm
 */

static const uint32_t PITCH_ROUNDING_BYTES = 1024;
static uint32_t roundup_pitch(uint32_t v)
{
    return (v + (PITCH_ROUNDING_BYTES - 1)) & ~(PITCH_ROUNDING_BYTES - 1);
};    

static const struct drm_format_info *
vfr_drm_get_validated_format(struct drm_device *dev, const char *format_name)
{
    static const struct vfr_drm_format formats[] = VFR_DRM_FORMATS;
    const struct vfr_drm_format *fmt = formats;
    const struct vfr_drm_format *end = fmt + ARRAY_SIZE(formats);
    const struct drm_format_info *info;

    if (!format_name) {
        drm_err(dev, "vfr_drm: missing framebuffer format\n");
        return ERR_PTR(-EINVAL);
    }

    while (fmt < end) {
        if (!strcmp(format_name, fmt->name)) {
            info = drm_format_info(fmt->fourcc);
            if (!info)
                return ERR_PTR(-EINVAL);
            return info;
        }
        ++fmt;
    }

    drm_err(dev, "vfr_drm: unknown framebuffer format %s\n",
        format_name);

    return ERR_PTR(-EINVAL);
}

struct vfr_drm_device *vfr_drm_device_of_dev(struct drm_device *dev)
{
    return container_of(dev, struct vfr_drm_device, dev);
}

static struct resource *
vfr_drm_get_memory_of(struct drm_device *dev, struct device_node *of_node)
{
    struct resource r, *res;
    int err;

    err = of_reserved_mem_region_to_resource(of_node, 0, &r);
    if (err)
        return NULL;

    res = devm_kmemdup(dev->dev, &r, sizeof(r), GFP_KERNEL);
    if (!res)
        return ERR_PTR(-ENOMEM);

    if (of_property_present(of_node, "reg"))
        drm_warn(dev, "preferring \"memory-region\" over \"reg\" property\n");

    return res;
}

static const struct drm_display_mode vfr_drm_supported_modes[] = {
    {DRM_MODE_INIT(60, 1920, 1080, 0, 0)},
    {DRM_MODE_INIT(60, 1280, 720, 0, 0)},
    {DRM_MODE_INIT(60, 960, 540, 0, 0)}
};
static const uint32_t NUM_MODES = sizeof(vfr_drm_supported_modes)/sizeof(struct drm_display_mode);

/*
 * Modesetting
 */

#if 1
static enum hrtimer_restart vfr_drm_vblank_simulate(struct hrtimer *timer)
{
    struct vfr_drm_device *sdev = container_of(timer, struct vfr_drm_device, vblank_hrtimer);
    struct drm_device *dev = &sdev->dev;
    struct drm_crtc *crtc = &sdev->crtc;
    u64 ret_overrun;
    bool ret;

    ret_overrun = hrtimer_forward_now(&sdev->vblank_hrtimer, sdev->vblank_period_ns);
    if (ret_overrun != 1)
        drm_warn(dev, "%s: vblank timer overrun\n", __func__);

    spin_lock( &(sdev->lock) );
    ret = drm_crtc_handle_vblank(crtc);
    if (!ret)
        drm_err(dev, "vfr_drm failure on handling vblank");

    spin_unlock( &( sdev->lock ) );

    return HRTIMER_RESTART;
}

static int vfr_drm_enable_vblank(struct drm_crtc *crtc)
{
    struct vfr_drm_device *sdev = vfr_drm_device_of_dev(crtc->dev);
    struct drm_device *dev = &sdev->dev;

    sdev->irq_enable = true;
    drm_info(dev, "vfr_drm: vfr_drm_enable_vblank\n");

    hrtimer_setup(&sdev->vblank_hrtimer, &vfr_drm_vblank_simulate, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    //sdev->vblank_period_ns = ktime_set(0, 16666667);
    sdev->vblank_period_ns = ktime_set(0, 50000000);
    drm_info(dev, "vfr_drm : frame period ns = %lld\n", ktime_to_ns(sdev->vblank_period_ns));
    hrtimer_start(&sdev->vblank_hrtimer, sdev->vblank_period_ns, HRTIMER_MODE_REL);

    if(sdev->primary.vfr_instance.core_instance.base != NULL)
    {
        // Enable VFR Core interrupts
        INTEL_VVP_VFR_REG_IOWR((&sdev->primary.vfr_instance), VFR_REG_IRQ_CONTROL, 1);
    }
    
    return 0;
}

static void vfr_drm_disable_vblank(struct drm_crtc *crtc)
{
    struct vfr_drm_device *sdev = container_of(crtc, struct vfr_drm_device, crtc);
    struct drm_device *dev = &sdev->dev;

    sdev->irq_enable = false;
    drm_info(dev, "vfr_drm: vfr_drm_disable_vblank\n");
    hrtimer_cancel(&sdev->vblank_hrtimer);

    if(sdev->primary.vfr_instance.core_instance.base != NULL)
    {
        // Enable VFR Core interrupts
        INTEL_VVP_VFR_REG_IOWR((&sdev->primary.vfr_instance), VFR_REG_IRQ_CONTROL, 0);
    }
}

#if 0
static bool vfr_drm_get_vblank_timestamp_from_timer(struct drm_crtc *crtc,
    int *max_error,
    ktime_t *vblank_time,
    bool in_vblank_irq)
{
    struct vfr_drm_device *sdev = container_of(crtc, struct vfr_drm_device, crtc);
    struct drm_vblank_crtc *vblank = drm_crtc_vblank_crtc(crtc);

    if (!READ_ONCE(vblank->enabled)) {
        *vblank_time = ktime_get();
        return true;
    }

    *vblank_time = READ_ONCE(sdev->vblank_hrtimer.node.expires);

    if (WARN_ON(*vblank_time == vblank->time))
        return true;

    /*
     * To prevent races we roll the hrtimer forward before we do any
     * interrupt processing - this is how real hw works (the interrupt is
     * only generated after all the vblank registers are updated) and what
     * the vblank core expects. Therefore we need to always correct the
     * timestampe by one frame.
     */
    *vblank_time -= sdev->vblank_period_ns;

    return true;
}
#endif
#endif

static void vfr_drm_atomic_enable(struct drm_crtc *crtc, struct drm_atomic_state *state)
{
    drm_crtc_vblank_on(crtc);
}

static void vfr_drm_atomic_disable(struct drm_crtc *crtc, struct drm_atomic_state *state)
{
    drm_crtc_vblank_off(crtc);
}


static void vfr_drm_atomic_begin(struct drm_crtc *crtc,
                   struct drm_atomic_state *state)
{
    struct vfr_drm_device *sdev = container_of(crtc, struct vfr_drm_device, crtc);

    /* This lock is held across the atomic commit to block vblank timer */
    spin_lock_irq(&sdev->lock);
}

static void vfr_drm_atomic_flush(struct drm_crtc *crtc,
                   struct drm_atomic_state *state)
{
    struct vfr_drm_device *sdev = container_of(crtc, struct vfr_drm_device, crtc);

    if (crtc->state->event) {
        spin_lock(&crtc->dev->event_lock);

        if (drm_crtc_vblank_get(crtc) != 0)
            drm_crtc_send_vblank_event(crtc, crtc->state->event);
        else
            drm_crtc_arm_vblank_event(crtc, crtc->state->event);

        spin_unlock(&crtc->dev->event_lock);

        crtc->state->event = NULL;
    }

    spin_unlock_irq(&sdev->lock);
}

static enum drm_mode_status vfr_drm_crtc_helper_mode_valid(struct drm_crtc *crtc,
                                 const struct drm_display_mode *mode)
{
    struct vfr_drm_device *sdev = vfr_drm_device_of_dev(crtc->dev);
    struct drm_device *dev = &sdev->dev;
    enum drm_mode_status status = MODE_BAD;
    
    for(uint32_t m = 0; m < NUM_MODES; m++)
    {
        status = drm_crtc_helper_mode_valid_fixed(crtc, mode, &vfr_drm_supported_modes[m]);
        drm_info(dev, "vfr_drm : vfr_drm_crtc_helper_mode_valid %u status %u\n", m, status);
        if(status == MODE_OK)
        {
            break;
        }
    }
    drm_info(dev, "vfr_drm : vfr_drm_crtc_helper_mode_valid status %u\n", status);
    return status;
}

/*
 * The CRTC is always enabled. Screen updates are performed by
 * the primary plane's atomic_update function. Disabling clears
 * the screen in the primary plane's atomic_disable function.
 */
static const struct drm_crtc_helper_funcs vfr_drm_crtc_helper_funcs = {
    .mode_valid = vfr_drm_crtc_helper_mode_valid,
    .atomic_check = drm_crtc_helper_atomic_check,
    .atomic_begin = vfr_drm_atomic_begin,
    .atomic_flush = vfr_drm_atomic_flush,
    .atomic_enable = vfr_drm_atomic_enable,
    .atomic_disable = vfr_drm_atomic_disable,
};

static const struct drm_crtc_funcs vfr_drm_crtc_funcs = {
    .reset = drm_atomic_helper_crtc_reset,
    .destroy = drm_crtc_cleanup,
    .set_config = drm_atomic_helper_set_config,
    .page_flip = drm_atomic_helper_page_flip,
    .atomic_duplicate_state = drm_atomic_helper_crtc_duplicate_state,
    .atomic_destroy_state = drm_atomic_helper_crtc_destroy_state,
#if 1
    .enable_vblank = vfr_drm_enable_vblank,
    .disable_vblank = vfr_drm_disable_vblank,
#if 0
    .get_vblank_timestamp = vfr_drm_get_vblank_timestamp_from_timer,
#endif
#endif
};

static const struct drm_encoder_funcs vfr_drm_encoder_funcs = {
    .destroy = drm_encoder_cleanup,
};

static int vfr_drm_connector_helper_get_modes(struct drm_connector *connector)
{
    struct drm_device *dev = connector->dev;
    for(uint32_t m = 0; m < NUM_MODES; m++)
    {
        struct drm_display_mode *mode;

        mode = drm_mode_duplicate(dev, &vfr_drm_supported_modes[m]);
        if (!mode) {
            drm_err(dev, "Failed to duplicate mode \n");
            return 0;
        }

        if (mode->name[0] == '\0')
            drm_mode_set_name(mode);

        if(m == 0)
        {
            mode->type |= DRM_MODE_TYPE_PREFERRED;
        }
        drm_mode_probed_add(connector, mode);
    }
    
    return NUM_MODES;
}

static const struct drm_connector_helper_funcs vfr_drm_connector_helper_funcs = {
    .get_modes = vfr_drm_connector_helper_get_modes,
};

static const struct drm_connector_funcs vfr_drm_connector_funcs = {
    .reset = drm_atomic_helper_connector_reset,
    .fill_modes = drm_helper_probe_single_connector_modes,
    .destroy = drm_connector_cleanup,
    .atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
    .atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
};

static const struct drm_mode_config_funcs vfr_drm_mode_config_funcs = {
    .fb_create = vfr_drm_fb_create,
    .atomic_check = drm_atomic_helper_check,
    .atomic_commit = drm_atomic_helper_commit,
};

/*
 * Init / Cleanup
 */

static struct drm_display_mode vfr_drm_mode(unsigned int width,
                          unsigned int height,
                          unsigned int width_mm,
                          unsigned int height_mm)
{
    const struct drm_display_mode mode = {
        DRM_MODE_INIT(60, width, height, width_mm, height_mm)
    };

    return mode;
}

static bool vfr_drm_is_listed_fourcc(const uint32_t *fourccs, size_t nfourccs, uint32_t fourcc)
{
    const uint32_t *fourccs_end = fourccs + nfourccs;

    while (fourccs < fourccs_end) {
        if (*fourccs == fourcc)
            return true;
        ++fourccs;
    }
    return false;
}

static size_t vfr_drm_build_fourcc_list(struct drm_device *dev,
                   const u32 *native_fourccs, size_t native_nfourccs,
                   u32 *fourccs_out, size_t nfourccs_out)
{
    /*
     * XRGB8888 is the default fallback format for most of userspace
     * and it's currently the only format that should be emulated for
     * the primary plane. Only if there's ever another default fallback,
     * it should be added here.
     */
    static const u32 extra_fourccs[] = {
        DRM_FORMAT_XRGB8888,
    };
    static const size_t extra_nfourccs = ARRAY_SIZE(extra_fourccs);

    u32 *fourccs = fourccs_out;
    const u32 *fourccs_end = fourccs_out + nfourccs_out;
    size_t i;

    /*
     * The device's native formats go first.
     */

    for (i = 0; i < native_nfourccs; ++i) {
        u32 fourcc = native_fourccs[i];

        if (vfr_drm_is_listed_fourcc(fourccs_out, fourccs - fourccs_out, fourcc)) {
            continue; /* skip duplicate entries */
        } else if (fourccs == fourccs_end) {
            drm_warn(dev, "Ignoring native format %p4cc\n", &fourcc);
            continue; /* end of available output buffer */
        }

        drm_dbg_kms(dev, "adding native format %p4cc\n", &fourcc);

        *fourccs = fourcc;
        ++fourccs;
    }

    /*
     * The extra formats, emulated by the driver, go second.
     */

    for (i = 0; (i < extra_nfourccs) && (fourccs < fourccs_end); ++i) {
        u32 fourcc = extra_fourccs[i];

        if (vfr_drm_is_listed_fourcc(fourccs_out, fourccs - fourccs_out, fourcc)) {
            continue; /* skip duplicate and native entries */
        } else if (fourccs == fourccs_end) {
            drm_warn(dev, "Ignoring emulated format %p4cc\n", &fourcc);
            continue; /* end of available output buffer */
        }

        drm_dbg_kms(dev, "adding emulated format %p4cc\n", &fourcc);

        *fourccs = fourcc;
        ++fourccs;
    }

    return fourccs - fourccs_out;
}

static struct vfr_drm_device *vfr_drm_device_create(struct drm_driver *drv,
                            struct platform_device *pdev)
{
    struct device_node *of_node = pdev->dev.of_node;
    struct vfr_drm_device *sdev;
    struct drm_device *dev;
    int width, height;
    struct resource *res, *mem = NULL;
    struct drm_crtc *crtc;
    struct drm_encoder *encoder;
    struct drm_connector *connector;
    int ret;

    sdev = devm_drm_dev_alloc(&pdev->dev, drv, struct vfr_drm_device, dev);
    if (IS_ERR(sdev))
        return ERR_CAST(sdev);
    dev = &sdev->dev;
    platform_set_drvdata(pdev, sdev);

    if (of_node) {
        mem = vfr_drm_get_memory_of(dev, of_node);
        if (IS_ERR(mem))
            return ERR_CAST(mem);
    } else {
        drm_err(dev, "no vfr_drm configuration found\n");
        return ERR_PTR(-ENODEV);
    }

    sdev->default_mode = vfr_drm_mode(1920, 1080, 0, 0);
    sdev->drm_format_info[0] = vfr_drm_get_validated_format(dev, "a8r8g8b8");
    sdev->drm_format_info[1] = vfr_drm_get_validated_format(dev, "a4r4g4b4");
    sdev->drm_format_info[2] = vfr_drm_get_validated_format(dev, "a2r2g2b2");
    sdev->drm_format_info[3] = NULL;

    sdev->primary.format = 0;
    sdev->primary.width = 0;
    sdev->primary.height = 0;
    sdev->primary.pitch = roundup_pitch(1920*4);
    sdev->primary.plane_type = DRM_PLANE_TYPE_PRIMARY;
    sdev->primary.vfr_irq_index = 1;
    sdev->primary.ase_mem_index = 3;
    sdev->primary.vfr_mem_index = 4;


    sdev->overlay.format = 0;
    sdev->overlay.width = 0;
    sdev->overlay.height = 0;
    sdev->overlay.pitch = roundup_pitch(144*4);
    sdev->overlay.plane_type = DRM_PLANE_TYPE_OVERLAY;
    sdev->overlay.vfr_irq_index = 0;
    sdev->overlay.ase_mem_index = 1;
    sdev->overlay.vfr_mem_index = 2;

    drm_dbg(dev, "display mode={" DRM_MODE_FMT "}\n", DRM_MODE_ARG(&sdev->default_mode));
    drm_dbg(dev, "framebuffer format=%p4cc, size=1920x1080\n",
        &sdev->drm_format_info[sdev->primary.format]->format);


    /*
     * Memory management
     */

    if (mem) {
        void *vfr_buffer_base;

        ret = devm_aperture_acquire_for_platform_device(pdev, mem->start,
                                resource_size(mem));
        if (ret) {
            drm_err(dev, "could not acquire memory range %pr: %d\n", mem, ret);
            return ERR_PTR(ret);
        }

        drm_dbg(dev, "using system memory framebuffer at %pr\n", mem);

        vfr_buffer_base = devm_memremap(dev->dev, mem->start, resource_size(mem), MEMREMAP_WC);
        if (IS_ERR(vfr_buffer_base))
            return vfr_buffer_base;
        
        iosys_map_set_vaddr(&sdev->vfr_buffer_base, vfr_buffer_base);
    } else {
        void __iomem *vfr_buffer_base;

        res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        if (!res)
            return ERR_PTR(-EINVAL);

        ret = devm_aperture_acquire_for_platform_device(pdev, res->start,
                                resource_size(res));
        if (ret) {
            drm_err(dev, "could not acquire memory range %pr: %d\n", res, ret);
            return ERR_PTR(ret);
        }

        drm_dbg(dev, "using I/O memory framebuffer at %pr\n", res);

        mem = devm_request_mem_region(&pdev->dev, res->start, resource_size(res),
                          drv->name);
        if (!mem) {
            /*
             * We cannot make this fatal. Sometimes this comes from magic
             * spaces our resource handlers simply don't know about. Use
             * the I/O-memory resource as-is and try to map that instead.
             */
            drm_warn(dev, "could not acquire memory region %pr\n", res);
            mem = res;
        }

        vfr_buffer_base = devm_ioremap_wc(&pdev->dev, mem->start, resource_size(mem));
        if (!vfr_buffer_base)
            return ERR_PTR(-ENOMEM);

        iosys_map_set_vaddr_iomem(&sdev->vfr_buffer_base, vfr_buffer_base);
    }

    sdev->primary.fb_offset = mem->start;
    sdev->overlay.fb_offset = mem->start + 0x800000;
    sdev->primary.base = IOSYS_MAP_INIT_OFFSET(&sdev->vfr_buffer_base, 0);
    sdev->overlay.base = IOSYS_MAP_INIT_OFFSET(&sdev->vfr_buffer_base, 0x800000);

    /*
     * Modesetting
     */

    ret = drmm_mode_config_init(dev);
    if (ret)
        return ERR_PTR(ret);

    dev->mode_config.min_width = 0;
    dev->mode_config.max_width = vfr_drm_supported_modes[0].hdisplay;
    dev->mode_config.min_height = 0;
    dev->mode_config.max_height = vfr_drm_supported_modes[0].vdisplay;
    dev->mode_config.preferred_depth = sdev->drm_format_info[0]->depth;
    dev->mode_config.funcs = &vfr_drm_mode_config_funcs;

    uint32_t native_formats[ARRAY_SIZE(sdev->drm_format_info)];
    uint32_t nnative_formats = 0;
    for(uint32_t f = 0; f < ARRAY_SIZE(sdev->drm_format_info); f++)
    {
        if(sdev->drm_format_info[f] != NULL)
        {
            native_formats[f] = sdev->drm_format_info[f]->format;
            nnative_formats++;
        }
        else
        {
            break;
        }
    }

    /* Primary plane */

    sdev->primary.nformats = vfr_drm_build_fourcc_list(dev, native_formats, nnative_formats,
                        sdev->primary.formats, ARRAY_SIZE(sdev->primary.formats));

    ret = vfr_drm_plane_add(sdev, pdev, &sdev->primary);
    if (ret)
        return ERR_PTR(ret);

    /* Overlay plane */

    sdev->overlay.nformats = 1;
    sdev->overlay.formats[0] = DRM_FORMAT_ARGB8888;
    
    ret = vfr_drm_plane_add(sdev, pdev, &sdev->overlay);
    if (ret)
        return ERR_PTR(ret);
    
    /* CRTC */

    crtc = &sdev->crtc;
    ret = drm_crtc_init_with_planes(dev, crtc, &sdev->primary.plane, NULL,
                    &vfr_drm_crtc_funcs, NULL);
    if (ret)
        return ERR_PTR(ret);
    drm_crtc_helper_add(crtc, &vfr_drm_crtc_helper_funcs);

    /* Encoder */

    encoder = &sdev->encoder;
    ret = drm_encoder_init(dev, encoder, &vfr_drm_encoder_funcs,
                   DRM_MODE_ENCODER_NONE, NULL);
    if (ret)
        return ERR_PTR(ret);
    encoder->possible_crtcs = drm_crtc_mask(crtc);
    sdev->overlay.plane.possible_crtcs = drm_crtc_mask(crtc);

    /* Connector */

    connector = &sdev->connector;
    ret = drm_connector_init(dev, connector, &vfr_drm_connector_funcs,
                 DRM_MODE_CONNECTOR_Unknown);
    if (ret)
        return ERR_PTR(ret);
    drm_connector_helper_add(connector, &vfr_drm_connector_helper_funcs);
    drm_connector_set_panel_orientation_with_quirk(connector,
                               DRM_MODE_PANEL_ORIENTATION_UNKNOWN,
                               width, height);

    ret = drm_connector_attach_encoder(connector, encoder);
    if (ret)
        return ERR_PTR(ret);

    drm_mode_config_reset(dev);

    return sdev;
}

/*
 * DRM driver
 */

DEFINE_DRM_GEM_FOPS(vfr_drm_fops);

static struct drm_driver vfr_drm_driver = {
    DRM_GEM_SHMEM_DRIVER_OPS,
    DRM_FBDEV_SHMEM_DRIVER_OPS,
    .name            = DRIVER_NAME,
    .desc            = DRIVER_DESC,
    .major            = DRIVER_MAJOR,
    .minor            = DRIVER_MINOR,
    .driver_features    = DRIVER_ATOMIC | DRIVER_GEM | DRIVER_MODESET,
    .fops            = &vfr_drm_fops,
};

/*
 * Platform driver
 */

static int vfr_drm_probe(struct platform_device *pdev)
{
    struct vfr_drm_device *sdev;
    struct drm_device *dev;
    int ret;

    sdev = vfr_drm_device_create(&vfr_drm_driver, pdev);
    if (IS_ERR(sdev))
        return PTR_ERR(sdev);
    dev = &sdev->dev;

    drm_info(dev, "vfr_drm: probe\n");

    spin_lock_init(&(sdev->lock));

    drm_vblank_init(dev, 1);

    ret = drm_dev_register(dev, 0);
    if (ret)
        return ret;

    drm_client_setup(dev, sdev->drm_format_info[sdev->primary.format]);

    return 0;
}

static void vfr_drm_remove(struct platform_device *pdev)
{
    struct vfr_drm_device *sdev = platform_get_drvdata(pdev);
    struct drm_device *dev = &sdev->dev;
    unsigned long flags;
    
    spin_lock_irqsave( &(sdev->lock), flags );

    vfr_drm_plane_remove(&sdev->overlay);
    vfr_drm_plane_remove(&sdev->primary);   

    spin_unlock_irqrestore( &( sdev->lock ), flags );


    drm_dev_unplug(dev);
}

static const struct of_device_id vfr_drm_of_match_table[] = {
    { .compatible = "altera,vfr-drm", },
    { },
};
MODULE_DEVICE_TABLE(of, vfr_drm_of_match_table);

static const struct platform_device_id vfr_drm_platform_ids[] = {
    { .name = "vfr-drm" },
    { }
};
MODULE_DEVICE_TABLE(platform, vfr_drm_platform_ids);

static struct platform_driver vfr_drm_platform_driver = {
    .driver = {
        .name = "vfr-drm", /* connect to sysfb */
        .of_match_table = of_match_ptr(vfr_drm_of_match_table),
    },
    .probe = vfr_drm_probe,
    .remove = vfr_drm_remove,
    .id_table = vfr_drm_platform_ids,
};


module_platform_driver(vfr_drm_platform_driver);

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL v2");

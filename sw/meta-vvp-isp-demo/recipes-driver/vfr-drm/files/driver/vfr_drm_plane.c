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
#include <drm/drm_fb_dma_helper.h>
#include <drm/drm_managed.h>
#include <drm/drm_modeset_helper_vtables.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_vblank.h>
#include <drm/drm_panic.h>

#include "vfr_drm_driver.h"
#ifdef USE_DMA
#include "vfr_drm_dma.h"
#endif
#include "vfr_drm_plane.h"
#include "vfr_drm_framebuffer.h"
#include "intel_vvp_vfr.h"
#include "intel_addr_span_expander.h"

static const uint64_t vfr_drm_plane_format_modifiers[] = {
    DRM_FORMAT_MOD_LINEAR,
    DRM_FORMAT_MOD_INVALID
};

static const uint32_t PITCH_ROUNDING_BYTES = 1024;
static uint32_t roundup_pitch(uint32_t v)
{
    return (v + (PITCH_ROUNDING_BYTES - 1)) & ~(PITCH_ROUNDING_BYTES - 1);
};    

struct vfr_drm_plane *vfr_drm_plane_of_plane(struct drm_plane *plane)
{
    return container_of(plane, struct vfr_drm_plane, plane);
}

static int vfr_drm_plane_helper_atomic_check(struct drm_plane *plane,
                               struct drm_atomic_state *state)
{
    struct vfr_drm_plane *vfr_drm_p = vfr_drm_plane_of_plane(plane);

    struct drm_plane_state *new_plane_state = drm_atomic_get_new_plane_state(state, plane);
    struct drm_shadow_plane_state *new_shadow_plane_state =
        to_drm_shadow_plane_state(new_plane_state);
    struct drm_framebuffer *new_fb = new_plane_state->fb;
    struct drm_crtc *new_crtc = new_plane_state->crtc;
    struct drm_crtc_state *new_crtc_state = NULL;
    struct drm_device *dev = plane->dev;
    struct vfr_drm_device *sdev = vfr_drm_device_of_dev(dev);
    int ret;

    if (new_crtc)
        new_crtc_state = drm_atomic_get_new_crtc_state(state, new_crtc);

    ret = drm_atomic_helper_check_plane_state(new_plane_state, new_crtc_state,
                          DRM_PLANE_NO_SCALING,
                          DRM_PLANE_NO_SCALING,
                          true, false);
    if (ret)
        return ret;
    else if (!new_plane_state->visible)
        return 0;

    
    drm_dbg(dev, "fb format=%p4cc %p ovl format=%p4cc %p\n",
        &new_fb->format->format,
        new_fb->format,
        &sdev->drm_format_info[vfr_drm_p->format]->format,
        sdev->drm_format_info[vfr_drm_p->format]);
    if (new_fb->format != sdev->drm_format_info[vfr_drm_p->format]) {
        void *buf;

        /* format conversion necessary; reserve buffer */
        buf = drm_format_conv_state_reserve(&new_shadow_plane_state->fmtcnv_state,
                            vfr_drm_p->pitch, GFP_KERNEL);
        if (!buf)
            return -ENOMEM;
    }

    return 0;
}

static void vfr_drm_plane_helper_atomic_update(struct drm_plane *plane,
                             struct drm_atomic_state *state)
{
    struct vfr_drm_plane *vfr_drm_p = vfr_drm_plane_of_plane(plane);

    struct drm_plane_state *plane_state = drm_atomic_get_new_plane_state(state, plane);
    struct drm_plane_state *old_plane_state = drm_atomic_get_old_plane_state(state, plane);
#ifndef USE_DMA
    struct drm_shadow_plane_state *shadow_plane_state = to_drm_shadow_plane_state(plane_state);
#endif
    struct drm_framebuffer *fb = plane_state->fb;
    struct vfr_drm_framebuffer *vfr_drm_fb = vfr_drm_framebuffer_of_fb(fb);
#ifdef USE_DMA
    dma_addr_t addr;
#endif
    struct drm_framebuffer *old_fb = old_plane_state->fb;
    struct drm_device *dev = plane->dev;
    struct vfr_drm_device *sdev = vfr_drm_device_of_dev(dev);
#ifndef USE_DMA
    struct drm_atomic_helper_damage_iter iter;
    struct drm_rect damage;
    int ret, idx;
#endif
    unsigned long flip_flags;
    bool pending_flip;
    unsigned int write_fb_index;
    
    (void)vfr_drm_fb; // currently unused

    if (fb == NULL)
        return;

#ifdef USE_DMA
    addr = drm_fb_dma_get_gem_addr(&vfr_drm_fb->fb, plane_state, 0);
#endif

#ifndef USE_DMA
    ret = drm_gem_fb_begin_cpu_access(fb, DMA_FROM_DEVICE);
    if (ret)
        return;

    if (!drm_dev_enter(dev, &idx))
        goto out_drm_gem_fb_end_cpu_access;
#endif

    if(fb && (fb != old_fb))
    {       
        uint32_t width = fb->width;
        uint32_t height = fb->height;
        uint32_t pitch = width * 4;
        uint32_t pixels_per_symbol = 1;
        if(fb->format)
        {
            uint32_t format = fb->format->format;
            switch(format)
            {
            case DRM_FORMAT_ARGB8888:
                pitch = width * 4;
                pixels_per_symbol = 1;
                break;
            case DRM_FORMAT_ARGB4444:
                pitch = width * 2;
                pixels_per_symbol = 2;
                break;
            case DRM_FORMAT_R8:
                pitch = width;
                pixels_per_symbol = 4;
                break;
            case DRM_FORMAT_XRGB8888:
                pitch = width * 4;
                pixels_per_symbol = 1;
                break;
            default:
                pitch = width * 4;
                pixels_per_symbol = 1;
            }
            for(uint32_t index = 0; index < ARRAY_SIZE(sdev->drm_format_info); index++)
            {
                if(sdev->drm_format_info[index] == NULL)
                {
                    break;
                }
                if(sdev->drm_format_info[index]->format == format)
                {
                    vfr_drm_p->format = index;
                    break;
                }
            }
        }
        pitch = roundup_pitch(pitch);
        if((vfr_drm_p->width != width) || (vfr_drm_p->height != height) || (vfr_drm_p->pitch != pitch))
        {
            drm_info(dev, "vfr_drm: vfr_drm_primary_plane_helper_atomic_update new fb config %ux%u pitch=%u\n", width, height, pitch);
            vfr_drm_p->width = width;
            vfr_drm_p->height = height;
            vfr_drm_p->pitch = pitch;
            if(vfr_drm_p->vfr_instance.core_instance.base != NULL)
            {
                intel_vvp_vfr_set_num_buffer_sets(&vfr_drm_p->vfr_instance, vfr_drm_p->num_frame_buffers);
                for(uint32_t index = 0; index < vfr_drm_p->num_frame_buffers; index++)
                {
                    intel_vvp_vfr_set_bufset_width(&vfr_drm_p->vfr_instance, index, width /* /pixels_per_symbol*/);
                    intel_vvp_vfr_set_bufset_height(&vfr_drm_p->vfr_instance, index, height);
                    intel_vvp_vfr_set_bufset_inter_line_offset(&vfr_drm_p->vfr_instance, index, pitch);
                }
                intel_vvp_core_set_img_info_width(&vfr_drm_p->vfr_instance, width /* /pixels_per_symbol*/);
                intel_vvp_core_set_img_info_height(&vfr_drm_p->vfr_instance, height);
                intel_vvp_vfr_commit_writes(&vfr_drm_p->vfr_instance);
            }
        }
    }

    spin_lock_irqsave(&vfr_drm_p->flip_lock, flip_flags);
    pending_flip = vfr_drm_p->pending_flip;
    write_fb_index = vfr_drm_p->write_fb_index;
    spin_unlock_irqrestore(&vfr_drm_p->flip_lock, flip_flags);

#ifndef USE_DMA
    if(pending_flip)
    {
        drm_dbg(dev, "vfr_drm: vfr_drm_plane_helper_atomic_update pending flip %u \n", vfr_drm_p->num_frame_buffers);
    }
    else
    {
        drm_atomic_helper_damage_iter_init(&iter, old_plane_state, plane_state);
        drm_atomic_for_each_plane_damage(&iter, &damage) {
            struct drm_rect dst_clip = plane_state->dst;
            struct iosys_map dst = vfr_drm_p->base[write_fb_index];

            if (!drm_rect_intersect(&dst_clip, &damage))
                continue;

            iosys_map_incr(&dst, drm_fb_clip_offset(vfr_drm_p->pitch, sdev->drm_format_info[vfr_drm_p->format], &dst_clip));
            drm_fb_blit(&dst, &vfr_drm_p->pitch, sdev->drm_format_info[vfr_drm_p->format]->format, shadow_plane_state->data,
                    fb, &damage, &shadow_plane_state->fmtcnv_state);
        }
    }

    drm_dev_exit(idx);
out_drm_gem_fb_end_cpu_access:
    drm_gem_fb_end_cpu_access(fb, DMA_FROM_DEVICE);
#else
    if(!pending_flip)
    {
        if(plane_state->fb_damage_clips)
        {
            unsigned int damage = drm_plane_get_damage_clips_count(plane_state);
            if(damage > 1)
            {
                vfr_drm_dma_write(sdev, vfr_drm_fb, addr , &vfr_drm_p->dma_emif_offset[write_fb_index]);
            }
            else if(damage == 1)
            {
                struct drm_mode_rect *clip = drm_plane_get_damage_clips(plane_state);
                if(clip)
                {
                    // special case for no damage, the clip is set to 0,0,2,2
                    if(clip->x1 == 0 && clip->y1 == 0 && clip->x2 == 2 && clip->y2 == 2)
                    {
                        //drm_dbg(dev, "no damage\n");
                    }
                    else
                    {
                        vfr_drm_dma_write(sdev, vfr_drm_fb, addr , &vfr_drm_p->dma_emif_offset[write_fb_index]);
                    }
                }
            }
        }
        else
        {
            //drm_dbg(dev, "no damage info\n");
            vfr_drm_dma_write(sdev, vfr_drm_fb, addr , &vfr_drm_p->dma_emif_offset[write_fb_index]);
        }
        if(vfr_drm_p->num_frame_buffers > 1)
        {
            spin_lock_irqsave(&vfr_drm_p->flip_lock, flip_flags);
            vfr_drm_p->pending_flip = true;
            spin_unlock_irqrestore(&vfr_drm_p->flip_lock, flip_flags);
            intel_vvp_vfr_set_starting_buffer_set(&vfr_drm_p->vfr_instance, write_fb_index);
            intel_vvp_vfr_commit_writes(&vfr_drm_p->vfr_instance);
        }
    }
#endif
}

static const struct drm_plane_helper_funcs vfr_drm_plane_helper_funcs = {
#ifndef USE_DMA
    DRM_GEM_SHADOW_PLANE_HELPER_FUNCS,
#endif
    .atomic_check = vfr_drm_plane_helper_atomic_check,
    .atomic_update = vfr_drm_plane_helper_atomic_update,
};

static const struct drm_plane_funcs vfr_drm_plane_funcs = {
    .update_plane = drm_atomic_helper_update_plane,
    .disable_plane = drm_atomic_helper_disable_plane,
    .destroy = drm_plane_cleanup,
#ifndef USE_DMA
    DRM_GEM_SHADOW_PLANE_FUNCS,
#else
    .reset = drm_atomic_helper_plane_reset,
    .atomic_duplicate_state = drm_atomic_helper_plane_duplicate_state,
    .atomic_destroy_state = drm_atomic_helper_plane_destroy_state,
#endif
};

static irqreturn_t vfr_drm_irq( int irq, void * dev_id )
{
    struct vfr_drm_plane *vfr_drm_p = (struct vfr_drm_plane *)dev_id;
    struct vfr_drm_device *sdev = vfr_drm_p->sdev;
    irqreturn_t irq_return = IRQ_NONE;

    // Disable VFR Core interrupts
    INTEL_VVP_VFR_REG_IOWR((&vfr_drm_p->vfr_instance), VFR_REG_IRQ_CONTROL, 0);

    // Retrieve interrupt status
    uint32_t irq_status = INTEL_VVP_VFR_REG_IORD((&vfr_drm_p->vfr_instance), VFR_REG_IRQ_STATUS);
    
    // Clear the interrupt
    INTEL_VVP_VFR_REG_IOWR((&vfr_drm_p->vfr_instance), VFR_REG_IRQ_STATUS, irq_status);

    spin_lock(&vfr_drm_p->flip_lock);
    if(vfr_drm_p->pending_flip)
    {
        unsigned int old_write_fb_index = vfr_drm_p->write_fb_index;

        vfr_drm_p->pending_flip = false;
        vfr_drm_p->write_fb_index = vfr_drm_p->read_fb_index;
        vfr_drm_p->read_fb_index = old_write_fb_index;
        spin_unlock(&vfr_drm_p->flip_lock);
    }
    else
    {
        spin_unlock(&vfr_drm_p->flip_lock);
    }

#if 1
    if(irq_status & 1)
    {
        // Real interrupt received: re-arm the watchdog so the hrtimer only
        // fires a dummy vblank if no real one arrives within the period.
        hrtimer_start(&sdev->vblank_hrtimer, sdev->vblank_period_ns, HRTIMER_MODE_REL);
        drm_crtc_handle_vblank( &(sdev->crtc) );
        irq_return = IRQ_HANDLED;
    }
#endif

    // Enable VFR Core interrupts
    INTEL_VVP_VFR_REG_IOWR((&vfr_drm_p->vfr_instance), VFR_REG_IRQ_CONTROL, 1);

    return irq_return;
}

int vfr_drm_plane_add(struct vfr_drm_device *sdev, struct platform_device *pdev, struct vfr_drm_plane *vfr_drm_p)
{
    struct drm_device *dev = &sdev->dev;
    int ret;
    
    vfr_drm_p->sdev = sdev;
    spin_lock_init(&vfr_drm_p->flip_lock);
    
    ret = drm_universal_plane_init(dev, &vfr_drm_p->plane, 0, &vfr_drm_plane_funcs,
                       vfr_drm_p->formats, vfr_drm_p->nformats,
                       vfr_drm_plane_format_modifiers,
                       vfr_drm_p->plane_type, NULL);
    if (ret)
        return ret;
    drm_plane_helper_add(&vfr_drm_p->plane, &vfr_drm_plane_helper_funcs);
    drm_plane_enable_fb_damage_clips(&vfr_drm_p->plane); 

    vfr_drm_p->hw_irq = platform_get_irq(pdev, vfr_drm_p->vfr_irq_index);
    if(vfr_drm_p->hw_irq > 0)
    {
        drm_info(dev, "vfr_drm: VFR irq found %i\n", vfr_drm_p->hw_irq);
        int status = devm_request_irq(&pdev->dev, vfr_drm_p->hw_irq, vfr_drm_irq, IRQF_SHARED, "VFR DRM", vfr_drm_p );
        if( status )
        {
            drm_err(dev, "vfr_drm: Couldn't request IRQ line - error %d!\n",status);
        }
        else
        {
            drm_info(dev, "vfr_drm: Hooked upto IRQ line %u\n", vfr_drm_p->hw_irq );

            struct resource *ase_mem = NULL;
            void __iomem *ase_base;

            ase_mem = platform_get_resource(pdev, IORESOURCE_MEM, vfr_drm_p->ase_mem_index);
            if (!ase_mem)
            {
                drm_err(dev, "vfr_drm: Failed to retrieve resource mem %u!\n", vfr_drm_p->ase_mem_index);
            }
            else
            {
                if(ase_mem->start != 0)
                {
                    drm_info(dev, "using I/O ASE at %pr\n", ase_mem);

                    ase_base = devm_ioremap_resource(&pdev->dev, ase_mem);
                    if (!ase_base)
                    {
                        drm_err(dev, "vfr_drm: Failed to map ase registers!\n");
                    }
                    else
                    {
                        drm_info(dev, "using ASE mapped at %p\n", ase_base);
                        intel_addr_span_expander_init(&vfr_drm_p->ase_instance, (intel_vvp_core_base)ase_base);
                        intel_addr_span_expander_set_window_address(&vfr_drm_p->ase_instance, 0, vfr_drm_p->fb_offset[0] & 0xFFFFFFFF80000000ULL);
                    }
                }
            }
            
            struct resource *fvr_mem = NULL;
            void __iomem *vfr_base;

            fvr_mem = platform_get_resource(pdev, IORESOURCE_MEM, vfr_drm_p->vfr_mem_index);
            if (!fvr_mem)
            {
                drm_err(dev, "vfr_drm: Failed to retrieve resource mem %u!\n", vfr_drm_p->vfr_mem_index);
            }
            else
            {
                drm_info(dev, "using I/O VFR at %pr\n", fvr_mem);

                vfr_base = devm_ioremap_resource(&pdev->dev, fvr_mem);
                if (!vfr_base)
                {
                    drm_err(dev, "vfr_drm: Failed to map vfr registers!\n");
                }
                else
                {
                    drm_info(dev, "using VFR mapped at %p\n", vfr_base);
                    //intel_addr_span_expander_init(&vfr_drm_p->ase_instance, (intel_vvp_core_base)ase_base);
                    intel_vvp_vfr_init(&vfr_drm_p->vfr_instance, (intel_vvp_core_base)vfr_base);

                    intel_vvp_vfr_set_num_buffer_sets(&vfr_drm_p->vfr_instance, vfr_drm_p->num_frame_buffers);
                    for(uint32_t index = 0; index < vfr_drm_p->num_frame_buffers; index++)
                    {
#ifdef USE_DMA
                        intel_vvp_vfr_set_bufset_base_addr(&vfr_drm_p->vfr_instance, index, vfr_drm_p->dma_emif_offset[index] & 0x7FFFFFFFULL);
#else
                        intel_vvp_vfr_set_bufset_base_addr(&vfr_drm_p->vfr_instance, index, vfr_drm_p->fb_offset[index] & 0x7FFFFFFFULL);
#endif
                        intel_vvp_vfr_set_bufset_inter_buffer_offset(&vfr_drm_p->vfr_instance, index, 0);
                        intel_vvp_vfr_set_bufset_inter_line_offset(&vfr_drm_p->vfr_instance, index, vfr_drm_p->pitch);
                        intel_vvp_vfr_set_bufset_field_count(&vfr_drm_p->vfr_instance, index, 1);

                        intel_vvp_vfr_set_bufset_bps(&vfr_drm_p->vfr_instance, index, 8);
                        intel_vvp_vfr_set_bufset_colorspace(&vfr_drm_p->vfr_instance, index, 0);
                        intel_vvp_vfr_set_bufset_cositing(&vfr_drm_p->vfr_instance, index, 0);
                        intel_vvp_vfr_set_bufset_interlace(&vfr_drm_p->vfr_instance, index, 0);
                        intel_vvp_vfr_set_bufset_subsampling(&vfr_drm_p->vfr_instance, index, 0x3);

                        intel_vvp_vfr_set_bufset_width(&vfr_drm_p->vfr_instance, index, vfr_drm_p->width);
                        intel_vvp_vfr_set_bufset_height(&vfr_drm_p->vfr_instance, index, vfr_drm_p->height);

                        intel_vvp_vfr_set_bufset_num_buffers(&vfr_drm_p->vfr_instance, index, 1);
                    }
                    intel_vvp_core_set_img_info_width(&vfr_drm_p->vfr_instance, vfr_drm_p->width);
                    intel_vvp_core_set_img_info_height(&vfr_drm_p->vfr_instance, vfr_drm_p->height);

                    intel_vvp_core_set_img_info_interlace(&vfr_drm_p->vfr_instance, 0);
                    intel_vvp_core_set_img_info_subsampling(&vfr_drm_p->vfr_instance, 0x3);
                    intel_vvp_core_set_img_info_cositing(&vfr_drm_p->vfr_instance, 0);
                    intel_vvp_core_set_img_info_colorspace(&vfr_drm_p->vfr_instance, 0);

                    intel_vvp_vfr_set_buffer_mode(&vfr_drm_p->vfr_instance, kIntelVvpVfrSingleSet);

                    intel_vvp_vfr_set_run_mode(&vfr_drm_p->vfr_instance, kIntelVvpVfrStop);

                    intel_vvp_vfr_commit_writes(&vfr_drm_p->vfr_instance);

                    intel_vvp_vfr_set_starting_buffer_set(&vfr_drm_p->vfr_instance, 0);
                    intel_vvp_vfr_set_buffer_mode(&vfr_drm_p->vfr_instance, kIntelVvpVfrSingleSet);
                    intel_vvp_vfr_set_run_mode(&vfr_drm_p->vfr_instance, kIntelVvpVfrFreeRunning);
                    intel_vvp_vfr_commit_writes(&vfr_drm_p->vfr_instance);
                }
            }
        }
    }
 
    return 0;
}

void vfr_drm_plane_remove(struct vfr_drm_plane *vfr_drm_p)
{
    if(vfr_drm_p->hw_irq > 0)
    {
        free_irq( vfr_drm_p->hw_irq, vfr_drm_p );
    }
}

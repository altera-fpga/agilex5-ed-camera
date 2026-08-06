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

struct vfr_drm_framebuffer *vfr_drm_framebuffer_of_fb(struct drm_framebuffer *fb)
{
    return container_of(fb, struct vfr_drm_framebuffer, fb);
}

static void vfr_drm_fb_destroy(struct drm_framebuffer *fb)
{
    struct drm_device *dev = fb->dev;
    struct vfr_drm_framebuffer *vfr_drm_fb = vfr_drm_framebuffer_of_fb(fb);
    
    drm_gem_fb_destroy(fb);
    kfree(vfr_drm_fb);
}

static const struct drm_framebuffer_funcs vfr_drm_fb_funcs = {
	.destroy	= vfr_drm_fb_destroy,
	.create_handle	= drm_gem_fb_create_handle,
	.dirty		= drm_atomic_helper_dirtyfb,
};

struct drm_framebuffer *
vfr_drm_fb_create(struct drm_device *dev, struct drm_file *file,
			     const struct drm_format_info *info,
			     const struct drm_mode_fb_cmd2 *mode_cmd)
{
	struct vfr_drm_framebuffer *vfr_drm_fb;

	vfr_drm_fb = kzalloc(sizeof(*vfr_drm_fb), GFP_KERNEL);
	if (!vfr_drm_fb)
		return ERR_PTR(-ENOMEM);
    drm_gem_fb_init_with_funcs(dev, &vfr_drm_fb->fb, file, info, mode_cmd, &vfr_drm_fb_funcs);
    return &vfr_drm_fb->fb;
}



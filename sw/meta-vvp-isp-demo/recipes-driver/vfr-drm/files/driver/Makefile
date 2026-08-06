# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/
obj-m := vfr_drm.o
vfr_drm-y += vfr_drm_driver.o
vfr_drm-y += vfr_drm_plane.o
vfr_drm-y += vfr_drm_framebuffer.o
vfr_drm-y += hal/intel_vvp_vfr.o
vfr_drm-y += hal/intel_vvp_core.o
vfr_drm-y += hal/intel_addr_span_expander.o

subdir-ccflags-y += -Ihal

SRC := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) clean
	
modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

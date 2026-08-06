# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"


SRC_URI:append = "\
  file://u-boot-misc-config.cfg \
"

SRC_URI:append:agilex5_axe5_eagle = " file://0003-2G-arrow.patch "
SRC_URI:append:agilex5_macnica_sulfur = " file://0003-2G-macnica.patch "

# Remove the patch below if necessary after upgrading u-boot to the most recent version from meta-intel-fpga
SRC_URI:append:agilex5_axe5_eagle = " file://v1-0001-HSD-15015933655-ddr-altera-agilex5-Hack-dual-port-DO-NOT-MERGE.patch "

# DE25S board currently using the region at 0x82000000 as the frame writer buffer
# Because of that we move the default kernel load address
SRC_URI:append:agilex5_terasic_de25s = "\
  file://0001_de25s_move_kernel_load_addr.patch \
  file://de25s_move_sys_load_addr.cfg \
"

## DE25-Nano currently uses shared HPS memory setup too
SRC_URI:append:agilex5_terasic_de25_nano = "\
  file://0001_de25s_move_kernel_load_addr.patch \
  file://de25s_move_sys_load_addr.cfg \
"


IMAGE_BOOT_ARGS:append = " fbcon=map:1"

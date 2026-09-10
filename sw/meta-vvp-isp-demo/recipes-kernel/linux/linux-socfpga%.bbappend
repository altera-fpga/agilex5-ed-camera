# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

FILESEXTRAPATHS:append := ":${THISDIR}/files"

SRC_URI:append:agilex5_mk_a5e065bb32aes1 = ' \
    file://i2c.cfg \
    file://msgdma.cfg \
    file://vfr_drm.cfg \
    file://0005-altera-msgdma.patch \
    file://0007-altera-msgdma-terminate-all.patch \
    '

SRC_URI:append:agilex5_mk_a5e065bb32aea = ' \
    file://i2c.cfg \
    file://msgdma.cfg \
    file://vfr_drm.cfg \
    file://0005-altera-msgdma.patch \
    file://0007-altera-msgdma-terminate-all.patch \
    '

SRC_URI:append:agilex5_dk_a5e065bb32aes = ' \
    file://89d1397540cd290dbdb9efa663700437e9f8b5dd_revert.patch \
    file://i2c.cfg \
    '

SRC_URI:append:agilex5_axe5_eagle = ' \
    file://0001-remove-arrow-refdes-devs.patch \
    file://i2c.cfg \
    file://arrow.cfg '

SRC_URI:append:agilex5_macnica_sulfur = ' \
    file://i2c.cfg '

SRC_URI:append:agilex5_terasic_de25s = ' \
    file://stratix_svc.cfg \
    file://i2c.cfg '    

SRC_URI:append:agilex5_terasic_de25_nano = ' \
    file://stratix_svc.cfg \
    file://i2c.cfg '

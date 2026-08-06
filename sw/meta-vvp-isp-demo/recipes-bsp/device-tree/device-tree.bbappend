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

SRC_URI:append:agilex5_terasic_de25s = ' file://agilex5_terasic_de25s_vvp-isp-vfw-buffer.dtsi'
SRC_URI:append:agilex5_terasic_de25_nano = ' file://agilex5_terasic_de25s_vvp-isp-vfw-buffer.dtsi'

SRC_URI:append:agilex5_mk_a5e065bb32aes1 = ' file://agilex5_overlay_fb.dtsi'
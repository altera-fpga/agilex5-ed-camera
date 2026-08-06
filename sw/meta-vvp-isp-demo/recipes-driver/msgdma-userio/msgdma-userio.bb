# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

DESCRIPTION = "Yocto recipe for userio module"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/BSD-3-Clause;md5=550794465ba0ec5312d6919e203a55f9"
inherit module

# Set source paths
SRC_URI = "file://Makefile;subdir=${S} \
           file://msgdma_userio_chr.c;subdir=${S} \
           file://msgdma_userio_chr.h;subdir=${S} \
           file://msgdma_userio.h;subdir=${S} \
           file://msgdma_userio_platform.c;subdir=${S} \
           "

S = "${WORKDIR}/src"

RPROVIDES_${PN}:append = "module-msgdma-userio"

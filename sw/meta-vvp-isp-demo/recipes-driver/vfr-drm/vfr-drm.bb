# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

DESCRIPTION = "Yocto recipe for vfr_drm module"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"
inherit module systemd

# Set source paths
SRC_URI = "file://.;subdir=${S}"
S = "${UNPACKDIR}/vfr-drm-${PV}"

RPROVIDES_${PN}:append = "module-vfr-drm"

MODULES_MODULE_SYMVERS_LOCATION = "driver"

IMAGE_BOOT_ARGS:append = " fbcon=map:1"

EXTRA_OEMAKE:append = "${@bb.utils.contains('APP_FEATURES', 'ISP_AI', ' EXTRA_CFLAGS=-DUSE_DMA', '', d)}"

do_compile:prepend() {
    cd ${S}/driver
}

do_install:prepend() {
    install -d ${D}/usr/bin
    install -m 0755 ${S}/vfr_drm_ocs2dto.sh ${D}/usr/bin/vfr_drm_ocs2dto.sh

    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -d ${D}${systemd_system_unitdir}
        install -m 644 ${S}/altera-vfr_drm-ocs.service ${D}${systemd_system_unitdir}/altera-vfr_drm-ocs.service
    fi

    cd ${S}/driver
}

FILES:${PN} += " \
    ${sysconfdir} \
    /usr/bin \
    /etc/local.d \
        ${@bb.utils.contains('DISTRO_FEATURES','systemd','${systemd_system_unitdir}/altera-vfr_drm-ocs.service','',d)} \
"

SYSTEMD_SERVICE:${PN} = "${@bb.utils.contains('DISTRO_FEATURES','systemd','altera-vfr_drm-ocs.service','',d)}"
SYSTEMD_AUTO_ENABLE = "enable"

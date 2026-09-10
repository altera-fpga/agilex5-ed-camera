# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

DESCRIPTION = "Altera OCS over UIO framework"
LICENSE = "CLOSED"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI += " \
             file://ocs2dto.sh \
             file://altera-ocs.service.in \
            "
S = "${UNPACKDIR}"
inherit systemd

OECMAKE_SOURCEPATH = "${S}/application"
BUILDDIR = "${WORKDIR}/build"
OECMAKE_TARGET_INSTALL = "install_deploy"

OCS_CAPABILITY_BASE ?= "0x40000000"

do_install:append() {
    install -d ${D}/usr/bin
    install -m 0755 ${S}/ocs2dto.sh ${D}/usr/bin/ocs2dto.sh

    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -d ${D}${systemd_system_unitdir}
        #install -m 644 ${WORKDIR}/altera-ocs.service ${D}${systemd_system_unitdir}/altera-ocs.service
        install -m 644  ${S}/altera-ocs.service.in \
            ${D}${systemd_system_unitdir}/altera-ocs.service
        sed -i "s:@OCS_CAPABILITY_BASE@:${OCS_CAPABILITY_BASE}:g" \
            ${D}${systemd_system_unitdir}/altera-ocs.service        
    fi
}

FILES:${PN} += " \
    ${sysconfdir} \
    /usr/bin \
    /etc/local.d \
        ${@bb.utils.contains('DISTRO_FEATURES','systemd','${systemd_system_unitdir}/altera-ocs.service','',d)} \
"

SYSTEMD_SERVICE:${PN} = "${@bb.utils.contains('DISTRO_FEATURES','systemd','altera-ocs.service','',d)}"
SYSTEMD_AUTO_ENABLE = "enable"

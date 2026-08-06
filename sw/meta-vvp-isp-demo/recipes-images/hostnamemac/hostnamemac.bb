# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

SUMMARY = "Append MAC address to hostname"
SECTION = "hostnamemac"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit systemd
inherit features_check

S = "${WORKDIR}"
UNPACKDIR = "${S}"

SRC_URI = "file://hostnamemac.sh \
           file://hostnamemac.service \
	   "

SYSTEMD_SERVICE:${PN} = "hostnamemac.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_install() {
	install -d ${D}${systemd_system_unitdir}
	install -m 0644  ${S}/hostnamemac.service ${D}${systemd_system_unitdir}

	install -d ${D}${bindir}
	install -m 0755 ${S}/hostnamemac.sh ${D}${bindir}
}

FILES:${PN} += "${systemd_system_unitdir}/hostnamemac.service ${bindir}/hostnamemac.sh"
REQUIRED_DISTRO_FEATURES="systemd"

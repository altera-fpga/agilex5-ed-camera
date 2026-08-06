# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

SUMMARY = "Set MAC address from config file in NV storage"
SECTION = "setmacaddress"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit systemd
inherit features_check

S = "${WORKDIR}"
UNPACKDIR = "${S}"

SRC_URI = "file://setmacaddress.sh \
           file://setmacaddress.rules \
           file://setmacaddress.service \
           "

SYSTEMD_SERVICE:${PN} = "setmacaddress.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_install() {
	install -d ${D}${sysconfdir}/udev/rules.d
	install -m 0644  ${S}/setmacaddress.rules ${D}${sysconfdir}/udev/rules.d

	install -d ${D}${systemd_system_unitdir}
	install -m 0644  ${S}/setmacaddress.service ${D}${systemd_system_unitdir}

	install -d ${D}${bindir}
	install -m 0755 ${S}/setmacaddress.sh ${D}${bindir}

	sed -i -e s:/sbin:${base_sbindir}:g -e s:/etc:${sysconfdir}:g ${D}${bindir}/setmacaddress.sh
}

FILES:${PN} += "${systemd_system_unitdir}/setmacaddress.service ${bindir}/setmacaddress.sh"
REQUIRED_DISTRO_FEATURES="systemd"

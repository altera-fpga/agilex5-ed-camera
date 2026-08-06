SUMMARY = "Expand root partition on first boot"
DESCRIPTION = "Service to expand root partition on first boot to fill remaining disk space"
SECTION = "devel"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://expand-root-partition;beginline=2;endline=19;md5=4e4fe90fdb9d3df550dac049197f0cdc"

inherit systemd allarch

SRC_URI = "\
    file://expand-root-partition \
    file://expand-root-partition.service"

S = "${WORKDIR}"

do_install () {
    install -d ${D}${systemd_unitdir}/system
    install -m 0644 ${S}/expand-root-partition.service ${D}${systemd_unitdir}/system

    install -d ${D}${sbindir}
    install -m 0755 ${S}/expand-root-partition ${D}${sbindir}
}

SYSTEMD_SERVICE:${PN} = "expand-root-partition.service"

RDEPENDS:${PN} = "\
    e2fsprogs-resize2fs \
    parted \
    util-linux \
    udev"


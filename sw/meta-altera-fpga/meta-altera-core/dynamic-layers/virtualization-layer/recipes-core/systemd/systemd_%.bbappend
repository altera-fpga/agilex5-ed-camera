FILESEXTRAPATHS:append := ":${THISDIR}/files"

SRC_URI:append = " file://80-docker.conf"

do_install:append() {
    install -d ${D}${sysconfdir}/sysctl.d/
    install -m 0644 ${UNPACKDIR}/80-docker.conf ${D}${sysconfdir}/sysctl.d/
}

FILESEXTRAPATHS:append := ":${THISDIR}/files"

SRC_URI:append = " file://daemon.json"

do_install:append() {
    install -d ${D}${sysconfdir}/docker
    install -m 0644 ${UNPACKDIR}/daemon.json ${D}${sysconfdir}/docker/daemon.json
}

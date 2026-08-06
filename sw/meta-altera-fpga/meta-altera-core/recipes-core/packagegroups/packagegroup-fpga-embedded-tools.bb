DESCRIPTION = "FPGA Embedded Tools"

inherit packagegroup

PACKAGES = "\
    ${PN} \
"

RDEPENDS:${PN} = "\
    devmem2 \
"

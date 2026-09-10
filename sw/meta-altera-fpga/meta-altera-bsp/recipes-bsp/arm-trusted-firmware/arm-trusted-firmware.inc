DESCRIPTION = "ARM Trusted Firmware"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM ?= "file://docs/license.rst;md5=189505435dbcdcc8caa63c46fe93fa89"

python () {
    if not d.getVar('ATF_PLAT'):
        raise bb.parse.SkipRecipe("ATF_PLAT variable is not set!")
}

PROVIDES = "arm-trusted-firmware"

DEPENDS = "u-boot-tools"

inherit deploy

PACKAGE_ARCH = "${MACHINE_ARCH}"

ATF_BRANCH ?= "socfpga_v2.3"
ATF_REPO ?= "github.com/altera-fpga/arm-trusted-firmware.git"

SRC_URI = "git://${ATF_REPO};protocol=https;branch=${ATF_BRANCH}"

CFLAGS = "-Wno-error=array-bounds"

# Let the Makefile handle setting up the CFLAGS and LDFLAGS as it is a standalone application
EXTRA_OEMAKE = 'CROSS_COMPILE="${TARGET_PREFIX}" PLAT="${ATF_PLAT}" CFLAGS="${CFLAGS}" DEPRECATED=1'
LDFLAGS[unexport] = "1"
AS[unexport] = "1"
LD[unexport] = "1"
PARALLEL_MAKE = ""

ALLOW_EMPTY:${PN} = "1"

do_configure[noexec] = "1"

do_compile() {
	oe_runmake -C ${S} bl31
}

do_deploy() {
	install -d ${DEPLOYDIR}
	install -m 0644 ${S}/build/${ATF_PLAT}/release/bl31.bin ${DEPLOYDIR}/bl31.bin-${MACHINE}-${PV}-${PR}
	ln -sf bl31.bin-${MACHINE}-${PV}-${PR} ${DEPLOYDIR}/bl31.bin
}

addtask deploy after do_compile

HOMEPAGE = "http://www.denx.de/wiki/U-Boot/WebHome"
SECTION = "bootloaders"
LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM ?= "file://Licenses/README;md5=30503fd321432fc713238f582193b78e"

python () {
    if not d.getVar('UBOOT_CONFIG'):
        raise bb.parse.SkipRecipe("UBOOT_CONFIG variable is not set!")
}

DEPENDS = "bash \
           bc-native \
           bison-native \
           coreutils-native \
           dtc-native \
           flex-native \
           python3-setuptools-native \
           swig-native \
           u-boot-default-script \
           u-boot-tools \
           virtual/kernel"

DEPENDS:append = "${@ ' arm-trusted-firmware' if d.getVar('ATF_PLAT') else ''}"

do_configure[depends] += "${@ 'arm-trusted-firmware:do_deploy' if d.getVar('ATF_PLAT') else ''}"

UBOOT_REPO ?= "github.com/altera-fpga/u-boot-socfpga.git"
UBOOT_VERSION_PREFIX ?= "socfpga_"
UBOOT_BRANCH ?= "${UBOOT_VERSION_PREFIX}${UBOOT_VERSION}"

SRCREV ?= "${AUTOREV}"
PV:append = "+git${SRCPV}"

S = "${WORKDIR}/git"
B = "${WORKDIR}/build"

RPROVIDES:${PN} = "u-boot"

SRC_URI = "git://${UBOOT_REPO};protocol=https;branch=${UBOOT_BRANCH}"

do_compile:prepend() {
    # Copy ATF binary if required
	if [ -n "${ATF_PLAT}" ]; then
		for config in ${UBOOT_MACHINE}; do
			cp ${DEPLOY_DIR_IMAGE}/bl31.bin ${B}/${config}/bl31.bin
			cp ${DEPLOY_DIR_IMAGE}/bl31.bin ${S}/bl31.bin
		done
	fi
}

do_install:append() {
	for config in ${UBOOT_MACHINE}; do
		for type in ${UBOOT_CONFIG}; do
			cp ${B}/${config}/u-boot.itb ${B}/${config}/u-boot-${type}.itb
			install -D -m 644 ${B}/${config}/u-boot-${type}.itb ${D}/boot/u-boot-${type}-${PV}-${PR}.itb
			ln -sf u-boot-${type}-${PV}-${PR}.itb ${D}/boot/u-boot.itb-${type}
			ln -sf u-boot-${type}-${PV}-${PR}.itb ${D}/boot/u-boot.itb
			rm -rf  ${D}/boot/?*.img*
		done
	done
}

require ${COREBASE}/meta/recipes-bsp/u-boot/u-boot.inc

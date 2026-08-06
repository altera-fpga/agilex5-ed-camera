SUMMARY = "U-boot boot scripts for Altera SoCFPGA devices"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "u-boot-mkimage-native \
           dtc-native"

INHIBIT_DEFAULT_DEPS = "1"

inherit deploy nopackages
PACKAGE_ARCH = "${MACHINE_ARCH}"

SRC_URI = "file://uboot_script.its"

S = "${WORKDIR}"

do_install[noexec] = "1"

# Setup boot script environment
DOLLAR = "$"

do_configure() {
    rm -f ${S}/uboot.txt

    if [ "${IMAGE_TYPE}" = "mmc" ]; then
        if [ "${@bb.utils.contains('SOCFPGA_FEATURES', 'bitstream', 'true', 'false', d)}" = "true" ]; then
            echo 'load mmc 0:1 ${DOLLAR}{loadaddr} top.core.rbf && dcache flush; fpga load 0 ${DOLLAR}{loadaddr} ${DOLLAR}{filesize};' >> ${S}/uboot.txt
        fi

        echo 'mmc rescan;' >> ${S}/uboot.txt
        echo 'load mmc 0:1 ${DOLLAR}{kernel_addr_r} Image;' >> ${S}/uboot.txt
        echo 'load mmc 0:1 ${DOLLAR}{fdt_addr_r} ${DTS_NAME}.dtb;' >> ${S}/uboot.txt
        echo 'setenv bootargs "${IMAGE_BOOT_ARGS} root=${DOLLAR}{mmcroot} rw rootwait";' >> ${S}/uboot.txt

        if [ "${@bb.utils.contains('SOCFPGA_FEATURES', 'bitstream', 'true', 'false', d)}" = "true" ]; then
            echo 'bridge enable;' >> ${S}/uboot.txt
        fi

        echo 'booti ${DOLLAR}{kernel_addr_r} - ${DOLLAR}{fdt_addr_r};' >> ${S}/uboot.txt
        echo 'exit;' >> ${S}/uboot.txt
    fi
}

do_compile() {
    mkimage -f "${S}/uboot_script.its" ${B}/boot.scr.uimg
}

do_deploy() {
    install -m 0644 ${B}/boot.scr.uimg ${DEPLOYDIR}/boot.scr.uimg
}

addtask do_deploy after do_compile before do_build

PROVIDES += "u-boot-default-script"

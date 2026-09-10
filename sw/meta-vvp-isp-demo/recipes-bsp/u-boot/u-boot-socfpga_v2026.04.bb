require recipes-bsp/u-boot/u-boot-socfpga-common.inc

LIC_FILES_CHKSUM = "file://Licenses/README;md5=2ca5f2c35c8cc335f0a19756634782f1"

UBOOT_VERSION = "v2026.04"
UBOOT_SRCREV ?= "e09d6fcc95f33b8a60377103f6e37e635660a3ee"

SRCREV = "${UBOOT_SRCREV}"

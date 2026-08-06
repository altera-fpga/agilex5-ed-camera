DESCRIPTION = "Linux Kernel for Intel SoCFPGA"
LICENSE = "GPL-2.0-only"

FILESEXTRAPATHS:prepend := "${THISDIR}/config:"

KERNEL_REPO ?= "github.com/altera-fpga/linux-socfpga.git"

# Build branch name
LINUX_VERSION_PREFIX ?= "socfpga-"
LINUX_VERSION_SUFFIX ?= ""

SRCREV ?= "${AUTOREV}"
SRCREV_machine ?= "${SRCREV}"
PV = "${LINUX_VERSION}"
PV:append = "+git${SRCPV}"

KBRANCH ?= "${LINUX_VERSION_PREFIX}${LINUX_VERSION}${LINUX_VERSION_SUFFIX}"

SRC_URI = "git://${KERNEL_REPO};branch=${KBRANCH};protocol=https"
SRC_URI:append = "${@bb.utils.contains('SOCFPGA_FEATURES', 'jffs2', ' file://jffs2.scc', '', d)}"
SRC_URI:append = "${@bb.utils.contains('SOCFPGA_FEATURES', 'gpio_sys', ' file://gpio_sys.scc', '', d)}"
SRC_URI:append = "${@bb.utils.contains('SOCFPGA_FEATURES', 'usb_adapter', ' file://usb_adapter.scc', '', d)}"
SRC_URI:append = "${@bb.utils.contains('SOCFPGA_FEATURES', 'uio', ' file://uio.scc', '', d)}"
SRC_URI:append = "${@bb.utils.contains('SOCFPGA_FEATURES', 'agilex5', ' file://agilex5.scc', '', d)}"
SRC_URI:append = "${@bb.utils.contains('SOCFPGA_FEATURES', 'rt', ' file://rt.scc', '', d)}"

require recipes-kernel/linux/linux-yocto.inc

KCONFIG_MODE ?= "--alldefconfig"

INSANE_SKIP:${PN}-src = "buildpaths"


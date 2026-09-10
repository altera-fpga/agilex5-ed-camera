LINUX_VERSION = "6.18.2"
LINUX_VERSION_SUFFIX = "-lts"
RT_LINUX_VERSION = "6.18.13"
RT_PATCH_VERSION = "4"

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

SRCREV = "4a5e70221d2f9b38072fe1bae720b79754f2102b"

include linux-socfpga.inc

SRC_URI:append = "${@bb.utils.contains('SOCFPGA_FEATURES', 'rt', ' https://cdn.kernel.org/pub/linux/kernel/projects/rt/6.18/older/patch-${RT_LINUX_VERSION}-rt${RT_PATCH_VERSION}.patch.gz;name=rt-patch;apply=yes', '', d)}"
SRC_URI[rt-patch.sha256sum] = "3fa9894501df230db90c8dab3a05702d5246b05adca50276121662c8eb7fa9f1"

ERROR_QA:remove = "patch-status"

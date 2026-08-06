KERNEL_IMAGETYPE ?= "Image"
KERNEL_CLASSES:append = " kernel-fitimage"
SOCFPGA_FEATURES ?= "jffs2 gpio_sys usb_adapter"

KBUILD_DEFCONFIG ?= "defconfig"

SPL_BINARY ?= "spl/u-boot-spl-dtb.hex"

UBOOT_ENTRYPOINT ?= "0x4000000"
UBOOT_LOADADDRESS ?= "0x4000000"
UBOOT_ELF ?= "u-boot"
UBOOT_BINARY ?= "u-boot.itb"
UBOOT_SUFFIX ?= "itb"

FIT_KERNEL_COMP_ALG = "none"

EXTRA_IMAGECMD:jffs2 ?= "-n -q -e 0x10000"

SERIAL_CONSOLES ?= "115200;ttyS0"

IMAGE_FSTYPES ?= "cpio ext4 wic.gz wic.bmap"
WKS_FILE ?= "sdimage-agilex.wks.in"

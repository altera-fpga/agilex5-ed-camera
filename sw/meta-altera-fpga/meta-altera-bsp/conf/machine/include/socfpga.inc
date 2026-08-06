SOC_FAMILY = "socfpga"

KERNEL_PROVIDER ?= "linux-socfpga"
PREFERRED_PROVIDER_virtual/kernel ?= "${KERNEL_PROVIDER}"

PREFERRED_PROVIDER_virtual/bootloader ?= "u-boot-socfpga"
PREFERRED_PROVIDER_u-boot ?= "u-boot-socfpga"

PREFERRED_PROVIDER_u-boot-default-script ?= "socfpga-u-boot-scr"

MACHINE_EXTRA_RRECOMMENDS = " kernel-modules"

IMAGE_TYPE ??= "mmc"

do_image_wic[depends] += "mtools-native:do_populate_sysroot dosfstools-native:do_populate_sysroot virtual/bootloader:do_deploy virtual/kernel:do_deploy"


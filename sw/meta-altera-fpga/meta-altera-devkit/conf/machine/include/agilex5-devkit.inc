DTS_NAME ?= "socfpga_agilex5_socdk"

UBOOT_CONFIG ??= "agilex5-socdk-atf"
UBOOT_CONFIG[agilex5-socdk-atf] = "socfpga_agilex5_defconfig"

require conf/machine/include/agilex5.inc

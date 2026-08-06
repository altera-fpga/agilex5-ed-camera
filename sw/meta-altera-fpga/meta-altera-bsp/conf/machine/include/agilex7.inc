require conf/machine/include/arm/arch-armv8a.inc
require conf/machine/include/socfpga.inc
require conf/machine/include/agilex.inc

ATF_PLAT ?= "agilex"
DTS_NAME ??= "socfpga_agilex_socdk"

KERNEL_DEVICETREE ?= "intel/${DTS_NAME}.dtb"

# BOOT
require conf/machine/include/socfpga-boot.inc

# UBIFS
UBIFS_LEB_SIZE = "126976"
UBIFS_LEB_CNT = "7800"
UBIFS_PEB_SIZE = "128"
PAGE_SIZE = "2048"

require conf/machine/include/ubifs.inc

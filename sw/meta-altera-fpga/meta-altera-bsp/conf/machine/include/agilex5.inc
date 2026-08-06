require conf/machine/include/arm/arch-armv8-2a.inc
require conf/machine/include/socfpga.inc
require conf/machine/include/agilex.inc

ATF_PLAT ?= "agilex5"
DTS_NAME ??= "socfpga_agilex5_socdk"

SOCFPGA_FEATURES:append = " agilex5"

# BOOT
require conf/machine/include/socfpga-boot.inc
IMAGE_BOOT_ARGS:append = " console=ttyS0,115200 init=/sbin/init"

# UBIFS
UBIFS_LEB_SIZE = "1032192"
UBIFS_LEB_CNT = "8500"
UBIFS_PEB_SIZE = "1024"
PAGE_SIZE = "8192"

require conf/machine/include/ubifs.inc

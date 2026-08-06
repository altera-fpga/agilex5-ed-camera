MACHINE_ESSENTIAL_EXTRA_RDEPENDS += "\
    device-tree \
    ${@ 'fpga-bitstream' if "bitstream" in d.getVar('SOCFPGA_FEATURES') else ''}"

IMAGE_BOOT_ARGS ?= "earlycon panic=-1"

# Add variables for wic creation of sdcard image
IMAGE_BOOT_FILES ?= " \
			${UBOOT_BINARY} \
			Image \
			boot.scr.uimg \
			devicetree/${DTS_NAME}.dtb;${DTS_NAME}.dtb \
			${@ 'top.core.rbf' if "bitstream" in d.getVar('SOCFPGA_FEATURES') else ''}"

SUMMARY = "Intel SoCFPGA Bitstream"
DESCRIPTION = "Custom FPGA bitstream for SOC Development Kit"
SECTION = "bsp"

inherit deploy

LICENSE = "Proprietary"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Proprietary;md5=0557f9d92cf58f2ccdd50f62f8ac0b28"

python () {
    if "bitstream" not in d.getVar('SOCFPGA_FEATURES'):
        raise bb.parse.SkipRecipe("FPGA bitstream recipe will be skipped!")
}

FPGA_BST_SRC_URI ??= ""
FPGA_BST_SHA256SUM ??= ""

SRC_URI:append = " ${FPGA_BST_SRC_URI}"
SRC_URI[sha256sum] ??= "${FPGA_BST_SHA256SUM}"

PROVIDES = "virtual/bitstream"

PACKAGES = "${PN}"

PACKAGE_ARCH = "${MACHINE_ARCH}"

python find_rbf () {
    import os

    workdir = d.getVar('WORKDIR')
    rbf_files = [f for f in os.listdir(workdir) if os.path.isfile(os.path.join(workdir, f)) and f.endswith(".rbf")]

    if len(rbf_files) == 0:
        bb.fatal("FPGA core bitstream file not found!")
        return

    # Get first RBF
    d.setVar('FPGA_BST_PATH', str(rbf_files[0]))
}
do_install[prefuncs] += "find_rbf"
do_deploy[prefuncs] += "find_rbf"

do_install () {
    install -D -m 0644 ${WORKDIR}/${FPGA_BST_PATH} ${D}/boot/top.core.rbf
}

do_deploy () {
    install -D -m 0644 ${WORKDIR}/${FPGA_BST_PATH} ${DEPLOYDIR}/top.core.rbf
}

FILES:${PN} = "\
    boot/* \
"

addtask install after do_configure before do_deploy
addtask deploy after do_install

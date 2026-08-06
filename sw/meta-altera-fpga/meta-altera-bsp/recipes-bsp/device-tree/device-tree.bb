SUMMARY = "Intel SoCFPGA Device Tree"
DESCRIPTION = "Custom device tree entries for Intel SoCFPGA platform"
SECTION = "bsp"

inherit devicetree

COMPATIBLE_MACHINE = "${MACHINE}"

DT_FILES_PATH = "${WORKDIR}/build-source"
DT_SRC_URI ??= ""
DT_SHA256SUM ??= ""

SRC_URI:append = " ${DT_SRC_URI}"
SRC_URI[sha256sum] ??= "${DT_SHA256SUM}"

do_configure[depends] += "virtual/kernel:do_configure"

python do_dts_update () {
    import os
    import shutil

    workdir = d.getVar('DT_FILES_PATH')
    source = d.getVar('WORKDIR')
    dts_name = d.getVar('DTS_NAME')
    dts_path = os.path.join(workdir, dts_name + ".dts")
    dtsi_files = [f for f in os.listdir(source) if os.path.isfile(os.path.join(source, f)) and f.endswith(".dtsi")]

    if not os.path.isfile(dts_path):
        bb.fatal("Device tree file with name '" + dts_name + "' not found!")

    if len(dtsi_files) == 0:
        return

    # Parse dts file to get last include instance
    f = open(dts_path, "r")
    lines = f.readlines()
    inc_index = 0

    for i, l in enumerate(lines):
        if l.startswith("#include"):
            inc_index = i

    if inc_index == 0:
        return
    
    for i, p in enumerate(dtsi_files, start=1):
        shutil.copyfile(os.path.join(source, p), os.path.join(workdir, p))
        lines.insert(inc_index + i, '#include "' + p + '"' + os.linesep)

    f = open(dts_path, "w")
    f.writelines(lines)
    f.close()
}
addtask dts_update after do_configure before do_compile

do_configure:append() {   
    rm -rf ${DT_FILES_PATH}
    mkdir -p ${DT_FILES_PATH}

    if [ -f ${WORKDIR}/${DTS_NAME}.dts ]; then
        cp ${WORKDIR}/${DTS_NAME}.dts ${DT_FILES_PATH}/
    elif [ -f ${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/intel/${DTS_NAME}.dts ]; then
        cp ${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/intel/${DTS_NAME}.dts ${DT_FILES_PATH}/
    fi
}

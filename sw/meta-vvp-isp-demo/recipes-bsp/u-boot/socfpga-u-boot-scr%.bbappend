do_configure:append() {
    if [ "${IMAGE_TYPE}" = "mmc" ]; then
        if [ "${@bb.utils.contains('SOCFPGA_FEATURES', 'bitstream', 'true', 'false', d)}" = "false" ]; then
            sed -i 's/^\(booti.*\)$/bridge enable;\n\1/;' ${S}/uboot.txt
        fi
    fi
}

#!/bin/sh
# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the
# License.
# *******************************************************************************

script_dir=`dirname "$0"`

let "vfr_capid = $((0x24a))"
let "ase_capid = $((0x212))"

let "overlay_ase_uid = $((0x0))"
let "overlay_vfr_uid = $((0x32))"
let "primary_ase_uid = $((0x1))"
let "primary_vfr_uid = $((0x33))"

let "overlay_vfr_ba_hi = 0"
let "overlay_vfr_ba_lo = 0"
let "overlay_vfr_block_size = 0"
let "primary_vfr_ba_hi = 0"
let "primary_vfr_ba_lo = 0"
let "primary_vfr_block_size = 0"
let "overlay_ase_ba_hi = 0"
let "overlay_ase_ba_lo = 0"
let "overlay_ase_block_size = 0"
let "primary_ase_ba_hi = 0"
let "primary_ase_ba_lo = 0"
let "primary_ase_block_size = 0"

usage()
{
    echo "Usage: $0 [-h] [-d] [-c <CAPABILITY_BASE>]"
    echo "  -h         Show this help message"
    echo "  -c         <CAPABILITY_BASE> in hex"
}

CAPABILITY_BASE=0x40000000

while getopts "c:dhn" optname; do
    case "$optname" in
        c)
            CAPABILITY_BASE=$OPTARG
            ;;
        h)
            usage
            exit
            ;;

    esac
done

shift $((OPTIND-1))



###################################################################
# Functions
get_next_offset()
{
    local list_base=$1
    local this_base=$2

    let "offset_reg=${this_base} + 4"

    next_offset=`devmem ${offset_reg}`
    let "offset=`printf "%d" ${next_offset}` + ${list_base}"

    echo `printf "%d" ${offset}`
}

decode_capability_header()
{
    local this_base=$1

    let "reg=`devmem ${this_base}`"

    let "reg_addr=${this_base} + 4"
    let "reg_low=`devmem ${reg_addr}`"
    let "reg_addr=${this_base} + 8"
    let "reg_high=`devmem ${reg_addr}`"

}

decode_interrupt_capability()
{
    local this_base=$1
    decode_capability_header ${this_base}
}

decode_register_capability()
{
    local this_base=$1
    decode_capability_header ${this_base}
}

decode_v2_offset_capability()
{
    local "this_base=$1"

    let "reg_addr=${this_base}+12"
    let "reg=`devmem ${reg_addr}`"
    let "list_base=$2"
    let "capid=$(($(($reg>>16))&$((0xFFFF))))"

  # associate / unique ids
    let "reg_addr=${reg_addr}+4"
    let "reg=`devmem ${reg_addr}`"
    let "uid=$(($(($reg)) & $((0xFFFF))))"
    let "aid=$(($(($reg >> 16)) & $((0xFFFF))))"

    let "reg_addr=${reg_addr}+4"
    let "reg=`devmem ${reg_addr}`"
    let "irq_control_reg=$(($(($reg)) & $((0xFFFF))))"
    let "irq_enable_reg=$(($(($reg >> 16)) & $((0xFFFF))))"

    let "reg_addr=${reg_addr}+4"
    let "reg=`devmem ${reg_addr}`"
    let "num_blocks=$(($(($reg >> 24)) & $((0xFF))))"
    let "block_reg_count=$(($(($reg)) & $((0xFFFFFF))))"
    let "block_size=$(($(($block_reg_count)) * 4))"

    let "reg_addr=${reg_addr}+4"
    let "reg=`devmem ${reg_addr}`"
    let "block_offset_lo=${reg}"

    let "reg_addr=${reg_addr}+4"
    let "reg=`devmem ${reg_addr}`"
    let "block_offset_hi=${reg}"

    let "block_offset=$(($(($((${block_offset_hi})) << 32)) | $((${block_offset_lo}))))"
    let "block_addr = ${block_offset}+${list_base}"
    let "block_addr_hi = $((${block_addr} >> 32))"
    let "block_addr_lo = $((${block_addr} & 4294967295))"

    if [ "$capid" == "$vfr_capid" ]; then
        if [ "$uid" == "$overlay_vfr_uid" ]; then
            let "overlay_vfr_ba_hi = $block_addr_hi"
            let "overlay_vfr_ba_lo = $block_addr_lo"
            let "overlay_vfr_block_size = $block_size"
        elif [ "$uid" == "$primary_vfr_uid" ]; then
            let "primary_vfr_ba_hi = $block_addr_hi"
            let "primary_vfr_ba_lo = $block_addr_lo"
            let "primary_vfr_block_size = $block_size"
        fi
    elif [ "$capid" == "$ase_capid" ]; then
        if [ "$uid" == "$overlay_ase_uid" ]; then
            let "overlay_ase_ba_hi = $block_addr_hi"
            let "overlay_ase_ba_lo = $block_addr_lo"
            let "overlay_ase_block_size = $block_size"
        elif [ "$uid" == "$primary_ase_uid" ]; then
            let "primary_ase_ba_hi = $block_addr_hi"
            let "primary_ase_ba_lo = $block_addr_lo"
            let "primary_ase_block_size = $block_size"
        fi
    fi
}

decode_offset_capability()
{
    local this_base=$1
    local list_base=$2

    decode_capability_header ${this_base}

    let "reg=`devmem ${this_base}`"
    let "version=$(($(($reg>>8))&255))"

    # Decode Version 2 Header
    decode_v2_offset_capability ${this_base} ${list_base}

}

decode_capability()
{
  local this_base=$1
  local list_base=$2

  reg=`devmem ${this_base}`
  let "cap_type=${reg} & 0xFF"
  case `printf "0x%02x" ${cap_type}` in
   "0xff")
      decode_interrupt_capability ${this_base}
      ;;
    "0xfe")
      decode_register_capability ${this_base}
      ;;
    "0xfd")
      decode_offset_capability ${this_base} ${list_base} $3
       ;;
    *)
      echo "Unknown `printf "0x%02x" ${cap_type}`"
    ;;
  esac
}

###################################################################
# Main Script
echo "Generating device tree overlay for vfr-drm from OCS rom at ${CAPABILITY_BASE}"
let "list_base=`printf "%d" ${CAPABILITY_BASE}`"

next_base=${list_base}

while true; do
  decode_capability ${next_base} ${list_base}
  next_base=`get_next_offset ${list_base} ${next_base}`
  if [ "${next_base}" == "${list_base}" ]; then
    break;
  fi
 done

cat >vfr_drm_ocs.dtso <<EOF
/dts-v1/;
/plugin/;

/{
    vfr_drm@ {
        target-path = "/";
        __overlay__ {
            #address-cells = <2>;
            #size-cells = <2>;

            framebuffer0: framebuffer@0 {
                #address-cells = <2>;
                #size-cells = <2>;
                ranges;
                compatible = "altera,vfr-drm";
                reg = <0x0 0x88000000 0x0 0x02000000>,
EOF

if [ "$primary_vfr_ba_lo" != "0" ]; then
    printf "                    <0x%08X 0x%08X 0x0 0x10>,\n" $overlay_ase_ba_hi $overlay_ase_ba_lo >> vfr_drm_ocs.dtso
    printf "                    <0x%08X 0x%08X 0x0 0x%08X>,\n" $overlay_vfr_ba_hi $overlay_vfr_ba_lo $overlay_vfr_block_size >> vfr_drm_ocs.dtso
    printf "                    <0x%08X 0x%08X 0x0 0x10>,\n" $primary_ase_ba_hi $primary_ase_ba_lo >> vfr_drm_ocs.dtso
    printf "                    <0x%08X 0x%08X 0x0 0x%08X>;\n" $primary_vfr_ba_hi $primary_vfr_ba_lo $primary_vfr_block_size >> vfr_drm_ocs.dtso
    printf "                reg-names = \"framebuffer\", \"ase-overlay\", \"vfr-overlay\", \"ase-primary\", \"vfr-primary\";\n" >> vfr_drm_ocs.dtso
else
    printf "                    <0x%08X 0x%08X 0x0 0x10>,\n" $overlay_ase_ba_hi $overlay_ase_ba_lo >> vfr_drm_ocs.dtso
    printf "                    <0x%08X 0x%08X 0x0 0x%08X>;\n" $overlay_vfr_ba_hi $overlay_vfr_ba_lo $overlay_vfr_block_size >> vfr_drm_ocs.dtso
    printf "                reg-names = \"framebuffer\", \"ase-overlay\", \"vfr-overlay\";\n" >> vfr_drm_ocs.dtso
fi

cat >>vfr_drm_ocs.dtso <<EOF
                memory-region = <&overlay_fb_reserved>;
                interrupt-parent = <&intc>;
                interrupts = <0 19 4>, <0 20 4>;
                status = "okay";
                dma_mem_start = <0x00000010 0x00000000>;
                dma_mem_end = <0x00000013 0xFFFFFFFF>;
                dmas = <&msgdma1 0>;
                dma-names = "drm";
            };
        };
    };
};
EOF

dtc vfr_drm_ocs.dtso -o vfr_drm_ocs.dtbo
rmdir /sys/kernel/config/device-tree/overlays/vfr_drm_ocs/ >/dev/null 2>&1; echo
mkdir -p /sys/kernel/config/device-tree/overlays/vfr_drm_ocs
cat vfr_drm_ocs.dtbo > /sys/kernel/config/device-tree/overlays/vfr_drm_ocs/dtbo


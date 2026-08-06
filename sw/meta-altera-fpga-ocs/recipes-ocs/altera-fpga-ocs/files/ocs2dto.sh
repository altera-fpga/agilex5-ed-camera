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
    
    printf "            ocs_%04x_%04x_%04x: ocs_uio@%08X {\n" $capid $uid $aid $block_addr >> ocs.dtso
    printf "                linux,uio-name = \"ocs_%04x_%04x_%04x\";\n" $capid $uid $aid >> ocs.dtso
    printf "                compatible = \"generic-uio\";\n" >> ocs.dtso
    printf "                reg = <0x%08X 0x%08X 0 0x%08X>;\n" $block_addr_hi $block_addr_lo $block_size >> ocs.dtso
    printf "            };\n" >> ocs.dtso
    printf "\n" >> ocs.dtso

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
echo "Generating device tree overlay from OCS rom at ${CAPABILITY_BASE}"
let "list_base=`printf "%d" ${CAPABILITY_BASE}`"

cat >ocs.dtso <<EOF
/dts-v1/;
/plugin/;

/{
    osc@ {
        target-path = "/";
        __overlay__ {
            #address-cells = <2>;
            #size-cells = <2>;
EOF

next_base=${list_base}

while true; do
  decode_capability ${next_base} ${list_base}
  next_base=`get_next_offset ${list_base} ${next_base}`
  if [ "${next_base}" == "${list_base}" ]; then
    break;
  fi
 done

cat >>ocs.dtso <<EOF
        };
    };
};
EOF

dtc ocs.dtso -o ocs.dtbo
rmdir /sys/kernel/config/device-tree/overlays/ocs/ >/dev/null 2>&1; echo
mkdir -p /sys/kernel/config/device-tree/overlays/ocs
cat ocs.dtbo > /sys/kernel/config/device-tree/overlays/ocs/dtbo

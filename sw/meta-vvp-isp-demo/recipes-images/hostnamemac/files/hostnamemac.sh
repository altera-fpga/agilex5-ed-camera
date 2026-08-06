#!/bin/sh
# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

# if hostname set on USB storage set to this
# otherwise append MAC address to hostname

USB_STORAGE="sda1"
NV_STORAGE="mmcblk0p1"
HOSTNAME_FILE="hostname"
MAC_ADDRESS_FILE="ethernetmac.$1"

# Get MAC address from USB if available (mounting partition if necessary)
USB_MOUNT=$(grep ${USB_STORAGE} /proc/mounts)
if [[ -z "${USB_MOUNT}" ]]; then
    mkdir -p /mnt/usb
    if mount /dev/${USB_STORAGE} /mnt/usb; then
        HOSTNAME=$(cat "/mnt/usb/${HOSTNAME_FILE}" 2>/dev/null)
        umount /mnt/usb
    fi
else
    MOUNT_PATH=$(echo ${USB_MOUNT} | awk '{print $2}')
    HOSTNAME=$(cat "${MOUNT_PATH}/${HOSTNAME_FILE}" 2>/dev/null)
fi

if [[ -z "${HOSTNAME}" ]]; then
    if [ -e /etc/ethernetmac.eth0 ] ; then
        HOSTNAME_STEM="$(hostname | sed -e 's/\-.*//')"
        HOSTNAME_MAC="${HOSTNAME_STEM}-$(cat /etc/ethernetmac.eth0 | sed -e "s/\://g" -e "y/ABCDEF/abcdef/")"
        hostname ${HOSTNAME_MAC}
    fi
else
    hostname ${HOSTNAME}
fi

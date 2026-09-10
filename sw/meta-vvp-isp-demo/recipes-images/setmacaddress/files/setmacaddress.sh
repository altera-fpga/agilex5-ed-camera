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

# Attempt to set MAC address for given interface.  Search in turn
# 1) USB storage
# 2) NV storage 
# 3) /etc/ethernetmac.<interface>
# 4) ifconfig

IFACE=eth0              #TODO is there a way to get during boot, vs. hardcoding?
USB_STORAGE="sda1"
NV_STORAGE="mmcblk0p1"
MAC_REGEX='[0-9A-F]\{2\}\(:[0-9A-F]\{2\}\)\{5\}'
MAC_ADDRESS_FILE="ethernetmac.$1"
MAC_ETHERNETMAC=$(grep -o -i ${MAC_REGEX} "/etc/${MAC_ADDRESS_FILE}")

# Get MAC address from USB if available (mounting partition if necessary)
USB_MOUNT=$(grep ${USB_STORAGE} /proc/mounts)
if [[ -z "${USB_MOUNT}" ]]; then
    mkdir -p /mnt/usb
    if mount /dev/${USB_STORAGE} /mnt/usb; then
        MACADDRESS=$(grep -i -o ${MAC_REGEX} "/mnt/usb/${MAC_ADDRESS_FILE}" 2>/dev/null)
        umount /mnt/usb
    fi
else
    MOUNT_PATH=$(echo ${USB_MOUNT} | awk '{print $2}')
    MACADDRESS=$(grep -i -o ${MAC_REGEX} "${MOUNT_PATH}/${MAC_ADDRESS_FILE}" 2>/dev/null)
fi

# Get MAC address from SD card if available (mounting partition if necessary)
if [[ -z "${MACADDRESS}" ]]; then
    SD_MOUNT=$(grep ${SD_STORAGE} /proc/mounts)
    if [[ -z "${SD_MOUNT}" ]]; then
        mount /dev/${NV_STORAGE} /mnt
        MACADDRESS=$(grep -i -o ${MAC_REGEX} "/mnt/${MAC_ADDRESS_FILE}" 2>/dev/null)
        umount /mnt
    else
        MOUNT_PATH=$(echo ${SD_MOUNT} | awk '{print $2}')
        MACADDRESS=$(grep -i -o ${MAC_REGEX} "${MOUNT_PATH}/${MAC_ADDRESS_FILE}" 2>/dev/null)
    fi
fi

# Get MAC address from /etc/${MAC_ADDRESS_FILE}, then ifconfig
if [[ -z "${MACADDRESS}" ]]; then
    if [ -n "${MAC_ETHERNETMAC}" ] ; then
	MACADDRESS="${MAC_ETHERNETMAC}"
    else
	MACADDRESS=$(ifconfig $IFACE | grep -o -i ${MAC_REGEX})
    fi
fi

# update mac address file if different
if [[ "$MACADDRESS" != "$MAC_ETHERNETMAC" ]]; then
    echo ${MACADDRESS} >/etc/${MAC_ADDRESS_FILE}
fi

if [ -e /sbin/ifconfig ]; then
    /sbin/ifconfig $IFACE down && /sbin/ifconfig $IFACE hw ether ${MACADDRESS} && /sbin/ifconfig $IFACE up
elif [ -e /bin/ifconfig ]; then
    /bin/ifconfig $IFACE down && /bin/ifconfig $IFACE hw ether ${MACADDRESS} && /bin/ifconfig $IFACE up
fi

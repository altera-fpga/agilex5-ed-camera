# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

DESCRIPTION = "Builds sd card image"
LICENSE="MIT"

IMAGE_NAME_SUFFIX=""

inherit core-image

COMPATIBLE_MACHINE = "^(agilex5_.*)$"

IMAGE_FEATURES += "empty-root-password allow-empty-password"

VIRTUAL-RUNTIME_dev_manager = "udev"
VIRTUAL-RUNTIME_init_manager = "systemd"
VIRTUAL-RUNTIME_initscripts= " "
VIRTUAL-RUNTIME_login_manager = "busybox shadow"

export IMAGE_BASENAME = "vvp-isp-demo-image"

IMAGE_PREPROCESS_COMMAND += "do_systemd_network ; "
IMAGE_PREPROCESS_COMMAND += "do_modprobe_uio ; "

do_systemd_network () {
	install -d ${IMAGE_ROOTFS}${sysconfdir}/systemd/network
	cat << EOF > ${IMAGE_ROOTFS}${sysconfdir}/systemd/network/10-en.network
[Match]
Name=en*

[Network]
DHCP=yes
EOF

	cat << EOF > ${IMAGE_ROOTFS}${sysconfdir}/systemd/network/11-eth.network
[Match]
Name=eth*

[Network]
DHCP=yes
EOF
}

do_modprobe_uio () {
    cat << EOF > ${IMAGE_ROOTFS}${sysconfdir}/modules-load.d/uio_pdrv_genirq.conf
uio_pdrv_genirq
EOF
}

# Switch off Predictable Network Interface Names scheme to revert to eth0 etc
ROOTFS_POSTPROCESS_COMMAND:append = " mask_udev ; "

# Disable assignment of fixed ifname by masking udev's .link for default policy
mask_udev(){
	ln -sf /dev/null ${IMAGE_ROOTFS}/lib/systemd/network/99-default.link
}

RDEPENDS:${PN} = " \
    device-tree \
"

IMAGE_INSTALL:append = " \
    ${VIRTUAL-RUNTIME_dev_manager} \
    ${VIRTUAL-RUNTIME_init_manager} \
    ${VIRTUAL-RUNTIME_initscripts} \
    ${VIRTUAL-RUNTIME_login_manager} \
    packagegroup-core-ssh-openssh \
	mtd-utils-ubifs \
	i2c-tools \
	update-alternatives-opkg \
	gdbserver \
    avahi-daemon \
    avahi-autoipd \
    fuse3 \
    hostnamemac \
    setmacaddress \
    expand-root-partition \
    vvp-isp-demo-app \
    kernel-modules \
    kernel-module-uio \
    kernel-module-uio-pdrv-genirq \
    msgdma-userio \
    libdrm \
    libdrm-tests \
    vfr-drm \
    dosfstools\
    protobuf-dev \
    protobuf-compiler \
"

#	strace
#   perf

TOOLCHAIN_HOST_TASK:append = " nativesdk-protobuf-dev nativesdk-protobuf-compiler"
TOOLCHAIN_TARGET_TASK:append = " libdrm-dev protobuf-dev protobuf-compiler"

# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

DESCRIPTION = "Builds the VVP ISP demo application"
LICENSE = "CLOSED"

S = "${WORKDIR}"
UNPACKDIR = "${S}"

FILESEXTRAPATHS:prepend := "${THISDIR}/../../../:"
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI += " \
             file://application \
             file://start.sh \
             file://21-vvp-isp \
             file://uio_pdrv_genirq.conf \
             file://vvp-isp.service \
             file://start_icamera_proxy.sh \
             file://icamera-proxy.service \
             file://yolo_cnn/;subdir=${S}/application/src-ai/cnn_networks \
             git://github.com/lvgl/lvgl.git;branch=release/v9.2;protocol=https;name=lvgl;destsuffix=lvgl \
            "

SRC_URI += "${@bb.utils.contains('APP_FEATURES', 'ISP_AI', ' \
             https://storage.openvinotoolkit.org/repositories/openvino/packages/2025.4/linux/openvino_toolkit_ubuntu24_2025.4.0.20398.8fdad55727d_x86_64.tgz;name=openvino_x86_64 \
             https://storage.openvinotoolkit.org/repositories/openvino/packages/2025.4/linux/openvino_toolkit_ubuntu22_2025.4.0.20398.8fdad55727d_arm64.tgz;name=openvino_arm64 \
             https://github.com/zeux/pugixml/releases/download/v1.15/pugixml-1.15.tar.gz;name=pugixml \
             https://downloads.intel.com/akdlm/software/fpga_ai_suite/2026.1.1/altera-fpga-ai-suite-ubuntu-2026.1.1_amd64.deb;name=altera-fpga-ai-suite;subdir=altera-fpga-ai-suite \
            ', '', d)}"

SRC_URI[lvgl.sha256sum] = "494f08dbe53b63e1a9891f8c3f535762eacb0b3318086951daf747c4f493bfea"
SRC_URI[openvino_x86_64.sha256sum] = "c57bc759a04bf316d66dc42d644433cf3bd590ad640933630a0616efb380a630"
SRC_URI[openvino_arm64.sha256sum] = "d51ed6eb38933ad5e6424debe734568b758d0fd9b9df1b21098cfb3f20d20cd3"
SRC_URI[pugixml.sha256sum] = "655ade57fa703fb421c2eb9a0113b5064bddb145d415dd1f88c79353d90d511a"
SRC_URI[altera-fpga-ai-suite.sha256sum] = "f7353dd10f0ce83f9730c7271677456d91c65964056ebf4112cefb1143024b59"

SRCREV_lvgl = "59a6b61c9580b65089010c5273f2fcdd6c4d2aae"

inherit cmake systemd pkgconfig

OECMAKE_SOURCEPATH = "${S}/application"

BUILDDIR = "${WORKDIR}/build"

INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_SYSROOT_STRIP = "1"

INSANE_SKIP:${PN} = "dev-so"

# donot map framebuffer console to out drm fp device (fp0)
IMAGE_BOOT_ARGS:append = " fbcon=map:1"

do_configure:prepend() {
    cd ${S}/lvgl
    ${S}/application/src/helpers/drmHelper/patches/lvglPatchCommand.sh ${S}/lvgl

    if [ "${@bb.utils.contains('APP_FEATURES', 'ISP_AI', 'true', 'false', d)}" = "true" ]; then
        cd ${S}/altera-fpga-ai-suite
        patch -p 1 < ${S}/application/src-ai/cmake/CoreDLA.patch
        patch -p 1 < ${S}/application/src-ai/cmake/CoreDLAPerformance.patch
        patch -p 1 < ${S}/application/src-ai/cmake/CoreDLABypassOLT.patch
    fi

    cd ${S}/build
    export NATIVE_SYSROOT=${STAGING_DIR_NATIVE}
}

python do_proxyenv () {
    origenv = d.getVar("BB_ORIGENV", False)
    with open(d.getVar("S")+"/proxyenv.sh", "w") as f:
        for env_var in ('http_proxy', 'https_proxy', 'no_proxy', 'HTTP_PROXY', 'HTTPS_PROXY', 'NO_PROXY'):
            env_val = origenv.getVar(env_var, False)
            if not env_val is None:
                f.write(f"export {env_var}={env_val}\n")
}
do_proxyenv[vardepsexclude]="BB_ORIGENV"
addtask proxyenv after do_configure before do_compile

do_compile:prepend() {
    . ${S}/proxyenv.sh
}

EXTRA_OECMAKE = "-DCMAKE_BUILD_TYPE=Release \
                 -DProtobuf_HOST_PROTOC_EXECUTABLE=${RECIPE_SYSROOT_NATIVE}/usr/bin/protoc \
                 -DCMAKE_INSTALL_PREFIX=/home/root \
                 -DMACHINE:STRING=${MACHINE} \
                 -DFETCHCONTENT_SOURCE_DIR_LVGL='${S}/lvgl' \
                 \"
EXTRA_OECMAKE:append = "${@bb.utils.contains('APP_FEATURES', 'ISP_AI', ' -DISP_AI_BUILD=ON', '', d)}"
EXTRA_OECMAKE:append = "${@bb.utils.contains('APP_FEATURES', 'ISP_STITCH', ' -DISP_STITCH_BUILD=ON', '', d)}"

EXTRA_OECMAKE:append = "${@bb.utils.contains('APP_FEATURES', 'ISP_AI', ' \
                -DFETCHCONTENT_SOURCE_DIR_MYOPENVINOHOST="${S}/openvino_toolkit_ubuntu24_2025.4.0.20398.8fdad55727d_x86_64" \
                -DFETCHCONTENT_SOURCE_DIR_MYOPENVINO="${S}/openvino_toolkit_ubuntu22_2025.4.0.20398.8fdad55727d_arm64" \
                -DFETCHCONTENT_SOURCE_DIR_PUGIXML="${S}/pugixml-1.15" \
                -DFETCHCONTENT_SOURCE_DIR_MYCOREDLA="${S}/altera-fpga-ai-suite" \
                ', '', d)}"

OECMAKE_TARGET_INSTALL = "install_deploy"
do_install:append() {
    install -d ${D}${sysconfdir}/ld.so.conf.d/
    echo /home/root/lib > ${D}${sysconfdir}/ld.so.conf.d/VvpIdpDemo.conf
    install -d ${D}${sysconfdir}/local.d
    install -m 0644 ${S}/21-vvp-isp ${D}${sysconfdir}/local.d

    install -m 0755 ${WORKDIR}/start.sh ${D}/home/root

    install -d ${D}${sysconfdir}/modprobe.d
    install -m 0755 ${WORKDIR}/uio_pdrv_genirq.conf ${D}${sysconfdir}/modprobe.d/

    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -d ${D}${systemd_system_unitdir}
        install -m 644 ${WORKDIR}/vvp-isp.service ${D}${systemd_system_unitdir}/vvp-isp.service
    fi

    install -d ${D}/home/root/ICameraProxyServer
    install -m 0755 ${WORKDIR}/start_icamera_proxy.sh ${D}/home/root/ICameraProxyServer

    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -d ${D}${systemd_system_unitdir}
        install -m 644 ${WORKDIR}/icamera-proxy.service ${D}${systemd_system_unitdir}/icamera-proxy.service
    fi
}

DEPENDS = " fuse3 python3 libdrm xxd-native protobuf protobuf-native "
RDEPENDS_${PN} = " fuse3 protobuf "

FILES:${PN} += " \
    ${sysconfdir} \
    /home/root \
    /etc/local.d \
        ${@bb.utils.contains('DISTRO_FEATURES','systemd','${systemd_system_unitdir}/vvp-isp.service ${systemd_system_unitdir}/icamera-proxy.service','',d)} \
"

SYSTEMD_SERVICE:${PN} = "${@bb.utils.contains('DISTRO_FEATURES','systemd','vvp-isp.service icamera-proxy.service','',d)}"
SYSTEMD_AUTO_ENABLE = "enable"

# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

include(FetchContent)

set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
MyOpenVINOHost
OVERRIDE_FIND_PACKAGE 
URL     https://storage.openvinotoolkit.org/repositories/openvino/packages/2025.4/linux/openvino_toolkit_ubuntu24_2025.4.0.20398.8fdad55727d_x86_64.tgz
URL_HASH SHA256=c57bc759a04bf316d66dc42d644433cf3bd590ad640933630a0616efb380a630
)
FetchContent_MakeAvailable(MyOpenVINOHost)
set(OPENVINO_SETUPVARS ${myopenvinohost_SOURCE_DIR}/setupvars.sh)
set(OPENVINO_OVC ${myopenvinohost_SOURCE_DIR}/python/openvino/tools/ovc/ovc.py)

set(FETCHCONTENT_QUIET TRUE)

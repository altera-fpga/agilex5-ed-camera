#!/bin/bash
# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

SCRIPT=$(basename "$0") 
SCRIPTS_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
export SOURCE_DIR=$1
export DOWNLOADED_FILE=$2
echo "Clean"
rm -rf ${SOURCE_DIR}/opt ${SOURCE_DIR}/CMakeLists.txt
echo "Unpacking archive (can be slow!)"
ar x ${DOWNLOADED_FILE}
echo "Extracting data (can be slow!)"
cmake -E tar -xJf "data.tar.xz"
echo "Cleanup"
rm "data.tar.xz"
rm "control.tar.xz"
rm "debian-binary"
echo "Done"
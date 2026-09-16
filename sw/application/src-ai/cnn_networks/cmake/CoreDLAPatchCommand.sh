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
export DOWNLOADED_URL=$2
export DOWNLOADED_FILE=$3
cd ${SOURCE_DIR}
echo "Clean"
rm -rf opt CMakeLists.txt ${DOWNLOADED_FILE} data.tar.xz control.tar.xz debian-binary
echo "Download"
curl --header 'Accept-Encoding: text/html' --header 'Accept-Language: en-US' --header 'User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/153.0.0.0 Safari/537.36' --header 'Connection: keep-alive' ${DOWNLOADED_URL} --output ${DOWNLOADED_FILE}
echo "Unpacking archive (can be slow!)"
ar x ${DOWNLOADED_FILE}
echo "Extracting data (can be slow!)"
cmake -E tar -xJf "data.tar.xz"
echo "Cleanup"
rm "data.tar.xz"
rm "control.tar.xz"
rm "debian-binary"
echo "Done"
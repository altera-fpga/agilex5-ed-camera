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

patch -f -p 1 -i ${SCRIPTS_DIR}/0001_lvgl_rgba2222.patch
rm -f lv_conf.h
cp -f ${SOURCE_DIR}/lv_conf_template.h lv_conf.h
patch -f -i ${SCRIPTS_DIR}/0002_lvgl_conf.patch

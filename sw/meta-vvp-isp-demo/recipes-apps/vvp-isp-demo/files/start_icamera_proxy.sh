#! /bin/sh
# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

cd /home/root/ICameraProxyServer
(./ICameraProxyServer | tee /dev/console /tmp/icamera-proxy.log > /dev/null) &

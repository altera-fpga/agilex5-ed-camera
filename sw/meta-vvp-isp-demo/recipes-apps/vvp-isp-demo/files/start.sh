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

cd %ROOT_HOME%

# wait 5 secondsffor network to acquie ip address
i=5
while [ "$i" -gt 0 ]; do
    printf "\rStarting in: %d" "$i" | tee /dev/console
    sleep 1
    i=$((i - 1))
done

(./VvpIspDemo | tee /dev/console /tmp/vvp-isp-demo.log > /dev/null) &

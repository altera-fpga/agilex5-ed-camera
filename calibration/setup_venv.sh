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

python3 -m venv python_env
source python_env/bin/activate
python3 -m pip install --upgrade pip
python3 -m pip install -r requirements.txt

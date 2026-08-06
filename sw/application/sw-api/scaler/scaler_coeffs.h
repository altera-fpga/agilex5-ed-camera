/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "HapiVvpScaler.h"

void scaler_create_and_load_horizontal_coefficents(intel_vvp_scaler_instance* scaler, int in_width, int out_width, int bank);
void scaler_create_and_load_vertical_coefficents(intel_vvp_scaler_instance* scaler, int in_width, int out_width, int bank);
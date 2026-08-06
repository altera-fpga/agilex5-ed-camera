/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include <iosfwd>
#include <string>

/// This struct holds all the compile time, built-in decisions for the app.
struct VvpIspCompileTimeConfiguration {
    int STARTUP_DELAY_SECONDS = 0;
    int SHOW_HDMI_INPUT_CONTAINER = 1;
    int SHOW_HDMI_OUTPUT_CONTAINER = 1;
};

// Utility functions for logging the configuration.

std::string ToString(VvpIspCompileTimeConfiguration conf);
std::ostream& operator<<(std::ostream& os, VvpIspCompileTimeConfiguration conf);

/// Global instance of the compile time config flags.
static constexpr VvpIspCompileTimeConfiguration _VvpIspConfiguration;

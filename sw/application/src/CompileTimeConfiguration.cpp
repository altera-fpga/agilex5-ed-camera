/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "CompileTimeConfiguration.h"

#include <sstream>

std::string ToString(VvpIspCompileTimeConfiguration conf)  {
    std::stringstream ss;
    ss << conf;
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, VvpIspCompileTimeConfiguration conf) {
    return os << "VvpIspCompileTimeConfiguration("

        << "STARTUP_DELAY_SECONDS: " << conf.STARTUP_DELAY_SECONDS << ", "

        << "SHOW_HDMI_INPUT_CONTAINER: " << conf.SHOW_HDMI_INPUT_CONTAINER << ", "
        << "SHOW_HDMI_OUTPUT_CONTAINER: " << conf.SHOW_HDMI_OUTPUT_CONTAINER

        << ")";
}

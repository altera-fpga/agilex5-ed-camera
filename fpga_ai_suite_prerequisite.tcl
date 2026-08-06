# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

package require cmdline

proc download_openvino {{force 0}} {
    set savedDir [pwd]
    if { $force } {
        puts "removing openvino"
        file delete -force openvino.2025.4
    }
    if {! [file isdirectory openvino.2025.4]} {
        puts "creating openvino"
        file mkdir openvino.2025.4
        cd openvino.2025.4
        if {[catch {exec curl --silent "https://storage.openvinotoolkit.org/repositories/openvino/packages/2025.4/linux/openvino_toolkit_ubuntu24_2025.4.0.20398.8fdad55727d_x86_64.tgz" --output "openvino.2025.4.tgz"} msg ]} {
            puts $msg
            puts "ERROR: Failed to download OpenVino"
            exit -1
        }
        if {[catch {exec tar --strip-components=1 -xzf "openvino.2025.4.tgz"} msg ]} {
            puts $msg
            puts "ERROR: Failed to extract OpenVino"
            exit -1
        }
        file delete "openvino.2025.4.tgz"
    } else {
        puts "found openvino"
    }
    cd $savedDir
}

proc download_coredla {{force 0}} {
    set savedDir [pwd]
    if { $force } {
        puts "removing coredla"
        file delete -force fpga_ai_suite_2026.1.1
    }
    if {! [file isdirectory fpga_ai_suite_2026.1.1]} {
        puts "creating coredla"
        file mkdir fpga_ai_suite_2026.1.1
        cd fpga_ai_suite_2026.1.1
        if {[catch {exec curl --silent "https://downloads.intel.com/akdlm/software/fpga_ai_suite/2026.1.1/altera-fpga-ai-suite-ubuntu-2026.1.1_amd64.deb" --output "altera-fpga-ai-suite-ubuntu-2026.1.1_2026.1.1_amd64.deb"} msg ]} {
            puts $msg
            puts "ERROR: Failed to download FPGA AI Suite"
            exit -1
        }
        if {[catch {exec ar x "altera-fpga-ai-suite-ubuntu-2026.1.1_2026.1.1_amd64.deb"} msg ]} {
            puts $msg
            puts "ERROR: Failed to extract FPGA AI Suite RPM"
            exit -1
        }
        if {[catch {exec tar -xJf "data.tar.xz"} msg ]} {
            puts $msg
            puts "ERROR: Failed to extract FPGA AI Suite data"
            exit -1
        }
        file delete "altera-fpga-ai-suite-ubuntu-2026.1.1_2026.1.1_amd64.deb"
        file delete "data.tar.xz"
        file delete "control.tar.xz"
        file delete "debian-binary"
    } else {
        puts "found coredla"
    }
    cd $savedDir
}

proc setup_python_venv {{force 0}} {
    set savedDir [pwd]
    if { $force } {
        puts "removing python_venv"
        file delete -force python_venv
    }
    if {! [file isdirectory python_venv]} {
        puts "creating python_venv"
        if {[catch {exec python3 -m venv python_venv } msg ]} {
            puts $msg
            puts "ERROR: Failed to create python_venv"
            exit -1
        }
        puts "updating pip"
        [catch {exec bash -c ". ./python_venv/bin/activate && python3 -m pip install --upgrade pip"} msg ]
        [catch {exec bash -c ". ./python_venv/bin/activate && python3 -m pip install packaging"} msg ]
        puts "installing protobuf"
        if {[catch {exec bash -c ". ./python_venv/bin/activate && python3 -m pip install -r python_deps/requirements.txt"} msg ]} {
            puts $msg
            puts "ERROR: Installing protobuf in python_venv"
            exit -1
        }
        
    } else {
        puts "found python_venv"
    }
    cd $savedDir
}

proc improved_getoptions {arglistVar optlist usage} {
    upvar 1 $arglistVar argv
    # Warning: Internal cmdline function
    set opts [::cmdline::GetOptionDefaults $optlist result]
    while {[set err [::cmdline::getopt argv $opts opt arg]]} {
        if {$err < 0} {
            return -code error -errorcode {CMDLINE ERROR} $arg
        }
        set result($opt) $arg
    }
    if {[info exists result(?)] || [info exists result(help)]} {
        return -code error -errorcode {CMDLINE USAGE} \
            [::cmdline::usage $optlist $usage]
    }
    return [array get result]
}

proc main {} {
    variable ::argv0 $::quartus(args)
    # =====================================================================
    # Definition of arguments for the script

    # "log"       - Switch to enable forced recreation of assets

    set options {
        { "force"           "0"     "Force assets to be recreated" }
    }

    set usage "quartus_sh -t fpga_ai_suite_prerequisite.tcl (-force) (-help)"

    try {
        array set opts_hash [improved_getoptions ::argv $options $usage]
        puts "All options valid"
    } trap {CMDLINE USAGE} {msg} {
        # This trap is executed when the -help argument is used by the user
        puts stderr "\n"
        puts stderr $msg
        puts stderr "\nNAME"
        puts stderr "     fpga_ai_suite_prerequisite.tcl -- Script to create OpenVino, and FPGA AI Suite assets"
        puts stderr "\nDESCRIPTION"
        puts stderr "     fpga_ai_suite_prerequisite.tcl script creates FPGA AI Suite assets needed to build design."
        puts stderr "\nARGUMENTS"
        puts stderr "     -force    Recreate OpenVino, and FPGA AI Suite assets.\n"
        exit 0
    } trap {CMDLINE ERROR} {msg} {
        puts stderr "Error: $msg"
        exit 1
    }

    set v_force 0
    if { $opts_hash(force) } { set v_force 1 }

    download_openvino $v_force
    download_coredla $v_force
    setup_python_venv $v_force

    set pwd [pwd]
    set setenvfile [open "setenv.sh" w]    
    puts $setenvfile ". openvino.2025.4/setupvars.sh"
    puts $setenvfile ". fpga_ai_suite_2026.1.1/opt/altera/fpga_ai_suite_2026.1.1/dla/setupvars.sh"
    puts $setenvfile ". python_venv/bin/activate"
    puts $setenvfile "export LD_PRELOAD=\${COREDLA_ROOT}/lib/libstdc++.so.6"
    close $setenvfile
    
}

main

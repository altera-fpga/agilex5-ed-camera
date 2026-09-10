# YOLO v8 Nano detection and pose graph compilation

## Overview
This folder contains resources to help compile YOLO v8 Nano detection and pose
models for the FPGA AI Suite IP.

## Prerequisites
It is important to consider licensing and copyright when using the YOLO
networks, please review the [Ultralytics license details](https://www.ultralytics.com/license).
If acceptable, proceed to download one or both of the supported models:
<br/>

[yolov8n.pt Yolo v8 Nano Detection](https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8n.pt)<br/>
[yolov8n-pose.pt Yolo v8 Nano Detection](https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8n-pose.pt)<br/>

The files must be placed into this `yolo_cnn` directory.

To compile the networks you will require OpenVINO™ 2024.6.0 and Altera FPGA AI
Suite 2025.1. To make the process easier, this directory includes a cmake
script that will configure and run the compilation on an x86 Ubuntu linux
machine or a docker container. Note that the script requires cmake, python3,
python3-pip, python3-venv, and libpython3-dev packages to be installed.

## Compiling the networks
To generate the compiled networks, ensure the prerequisites are satisfied. If
behind a proxy, ensure HTTP_PROXY and HTTPS_PROXY environment variables are set
correctly.

### **<span style="color:#FFBF00">The downloaded yolo models must be placed in the same directory as this README.md!</span>**

To download OpenVINO™ and FPGA AI Suite, and to compile the networks enter the
following commands (note it can take time to download and extract OpenVINO™ and
the FPGA AI Suite):
```
mkdir -p compile
cd compile
cmake -G Ninja ..
ninja
```

The compile/output directory should now have the compiled assets needed to run
the Camera Solution System Example Design.
```
compile/output/
├── generated_arch.arch
│   ├── yolov8n-pose_dla_m2m_compiled_640_384.bin
│   └── yolov8n_dla_m2m_compiled_640_384.bin
├── yolov8n-pose_categories.txt
└── yolov8n_categories.txt

1 directory, 8 files
```

## Copy the Compiled AI Models to the microSD Card

The compiled models must be copied onto the microSD card for the Application
Software to use at runtime:

* `scp` and `ssh` the compiled models directly to the Development Kit (using its
  `<ip address>`):
  * Power up the Modular Development Kit (if not already powered) and set up the
    serial terminal emulator (minicom, TeraTerm, PuTTY, etc.):
    * Select the correct `COMx` port. (The Modular Development Kit presents 4
      serial COM ports over a single connection and the Linux system uses the 3rd
      port in order). Set the port configuration as follows:
      * 115200 baud rate, 8 Data bits, 1 Stop bit, CRC and Hardware flow control
        disabled.
    * The Linux OS will boot.
    * Take note of the Modular Development Kit IP address.
      * The IP address can also be found using the terminal by logging in as `root`
      (no password required) and querying the Ethernet controller:

      ```bash
      root
      ifconfig
      ```

      * `eth0` provides the IPv4 or IPv6 address to connect to.
 
  * Using a Linux terminal (or Windows equivalent like PowerShell) on your Host,
    copy the files from the output directory to the Development Kit:

    ```bash
    cd compile/output
    scp -r * root@<ip address>:
    ```
  * Ensure sdcard has stored the files
    ```bash
    ssh root@<ip address>
    sync
    ls *_categories.txt *.arch
    ```
    Outputs:
    ```bash
    yolov8n-pose_categories.txt  yolov8n_categories.txt

    generated_arch.arch:
    yolov8n-pose_dla_m2m_compiled_640_384.bin  yolov8n_dla_m2m_compiled_640_384.bin
    ```
  * The Development Kit can be powered down, and restarted to load the models.

## Understanding the compiler flow
[A breakdown of the steps required to compile the models are detailed here](./CompileStepsWalkthrough.md)

## Generating your own architecture file
The FPGA AI Suite IP uses an architecture file (`.arch`) to tune its
performance and size. An optimized architecture file
(`arch/gernerated_arch.arch`) is included with the Camera Solution System
Example Design.

[Steps to generate a custom optimized architecture file are detailed here](./ArchitectureOptimisation.md).

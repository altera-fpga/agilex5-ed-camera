# 4Kp30 Multi-Sensor Camera with AI Inference Solution System Example Design for Agilex™ 5 Devices - Repository

## Overview

The repository contains the necessary files and collateral to create and build
the Camera with AI Inference Solution System Example Design.

The products of this repository are generated using Software flow, Hardware
flow, and YOLO v8 Nano detection and pose graph compilation are:

| Product | Type | Description |
|----|----|----|
| `.sof` | SRAM Object File | FPGA bitstream to be loadeded over JTAG |
| `.rbf` | Raw Binary File | FPGA bitstream file to be loaded from the microSD Card |
| `.jic` | JTAG Indirect Configuration File | QSPI Flash programming file for booting from microSD Card |
| `.wiz.gz` | Image File | microSD Card Image file containing the Linux embedded system and Camera Software Application |
| `.txt` | Categories File | list of categories equating to each channel of the YOLO v8 graph result |
| `.bin` | Compiled Graph File | compiled YOLO v8 graph |

<br>

### License Requirements

The Camera Solution System Example Design supports the OpenCore Plus (OCP)
evaluation license. The resulting `.sof` can be tested on Hardware using both
the time limited and JTAG tethered features of the license.

Alternatively, full licenses for the VVP IP Suite, VVP Tone Mapping Operator
(TMO) IP, and 3D LUT IP are required to produce a `.jic` and `.rbf` for a
Hardware, SD Card turnkey solution.

The Altera® FPGA AI Suite license operates independently to the rest of the IP.
A full license gives you unlimited inferences whereas the evaluation license
for this System Example Design gives you 100k free inferences before it times
out (just under 1 hour at 30 FPS). Once expired, the AI Overlay will display a
permanent "license limit reached" banner.

The following table summarizes the solution and operation based on the license
combinations:

| Combination | Video Full | OCP Tethered | OCP Untethered | AI Full | AI Eval | SOF | JIC/RBF | Operation |
|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|----|
| 1 |&check;|&cross;|&cross;|&check;|&cross;|&check;|&check;| Unlimited |
| 2 |&check;|&cross;|&cross;|&cross;|&check;|&check;|&check;| 1 Hour Inference Limited |
| 3 |&cross;|&check;|&cross;|&check;|&cross;|&check;|&cross;| Tether Limited |
| 4 |&cross;|&check;|&cross;|&cross;|&check;|&check;|&cross;| 1 Hour Inference Limited |
| 5 |&cross;|&cross;|&check;|&check;|&cross;|&check;|&cross;| 1 Hour Untethered Limited |
| 6 |&cross;|&cross;|&check;|&cross;|&check;|&check;|&cross;| 1 Hour Untethered and Inference Limited |

</center>
<br/>


Note that for all cases, free licenses for MIPI D-Phy IP, MIPI CSI-2 IP, and
Nios® V Processor must be downloaded and installed.

[Refer to the main documentation for additional licensing information](../docs/camera/camera_4k_ai/camera_4k_ai.md)

<br>


## Software Flow

The [**Software flow**](./../sw/README.md#4kp30-multi-sensor-camera-with-ai-inference-solution-system-example-design-for-agilex-5-devices)
generates the appropriate microSD Card Image based on your license and solution
requirements.

<br>

## YOLO v8 Nano detection and pose graph compilation
The [**YOLO v8 Nano detection and pose graph compilation**](./../yolo_cnn/README.md#yolo-v8-nano-detection-and-pose-graph-compilation)
generates the compiled graphs to be copied to microSD card.

<br>

## Hardware Flow

The Hardware flow uses the Modular Design Toolkit (MDT) to create and build the
Quartus® project for the 4Kp30 Multi-Sensor Camera with AI Inference Solution
System Example Design. Note, a pregenerated Quartus® project (using the MDT
flow) is also provided for reference and exploration.

<br>

### Software Requirements

The MDT requires the following Linux versions of software tools:

* Quartus® Prime Pro 26.1.
* Nios® V Open-Source Tools 26.1 (installed with Quartus® Prime).

<br>

### Creating and compiling the 4Kp30 Multi-Sensor Camera with AI Inference Solution System Example Design

#### Create the design using the Modular Design Toolkit (MDT)

Follow the next steps to create the Quartus® and Platform Designer Project for
the 4Kp30 Multi-Sensor Camera with AI Inference Solution System Example Design:

* Create your workspace and clone the repository using `--recurse-submodules`:

```bash
cd <workspace>
git clone -b rel/26.1 --recurse-submodules https://github.com/altera-fpga/agilex5-ed-camera.git agilex5-ed-camera
```

* Define a `./<project>` location of your choice, creating directory structure
  where necessary.
* Navigate to the `agilex5-ed-camera` directory containing the cloned
  repository and create your project, selecting the XML variant based on your
  license and solution requirements:
  * (Note that the `LICENSED` variable within the `.xml` must match your FPGA
    AI Suite license (full or evaluation) otherwise you will get a default FPGA
    AI Suite 10k inference limited design (just over 5 minutes at 30 FPS), even
    if you have a full license).

```bash
# SOF MDT Flow (assumes license combination 4 or 6)
# (For license combination 3 or 5, modify the LICENSED variable in the .xml to 1. You must have a full license otherwise you will get a 10k inference limited design)
cd agilex5-ed-camera
quartus_sh -t ./modular-design-toolkit/scripts/create/create_shell.tcl -proj_path <project> -proj_name agilex5_modkit_vvpisp -xml_path ./AGX_5E_Altera_Modular_Dk_ISP_designs/AGX_5E_Modular_Devkit_ISP_AI_WARP_FF_RD.xml
```

```bash
# RBF MDT Flow (assumes license combination 1)
# (For license combination 2, modify the LICENSED variable in the .xml to 0, otherwise you will get a 10k inference limited design)
cd agilex5-ed-camera
quartus_sh -t ./modular-design-toolkit/scripts/create/create_shell.tcl -proj_path <project> -proj_name agilex5_modkit_vvpisp -xml_path ./AGX_5E_Altera_Modular_Dk_ISP_designs/AGX_5E_Modular_Devkit_ISP_AI_WARP_RD.xml
```

This will create your Quartus® Prime and Platform Designer Project in
 `./<project>`. The folder structure is consistent with the MDT methodology.

#### Using the pregenerated MDT Quartus® project

Alternatively, you can download the pregenerated MDT Quartus® and Platform
Designer Project for the 4Kp30 Multi-Sensor Camera with AI Inference Solution
System Example Design:

* Define a `./<project>` location of your choice, creating directory structure
  where necessary.
* Download [AGX_5E_Modular_Devkit_ISP_AI_WARP_RD.zip](https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1/AGX_5E_Modular_Devkit_ISP_AI_WARP_RD.zip)
  and copy it to your `./<project>` location.
* Navigate to the `./<project>` location, extract the project, and load Quartus® GUI:

```bash
unzip AGX_5E_Modular_Devkit_ISP_AI_WARP_RD.zip

    AGX_5E_Modular_Devkit_ISP_AI_WARP_RD/
    ├── Makefile                              ← Project Make file
    ├── README.md                             ← Quartus® Example Design Manager info file
    ├── agilex5_modkit_vvpisp.qpf             ← Quartus® Project File
    ├── agilex5_modkit_vvpisp.qsf             ← Quartus® Settings File
    ├── da_drc.dawf                           ← Quartus® Design Rule Check Waiver File
    ├── quartus.ini                           ← Quartus® ini file
    ├── sdc/                                  ← Synopsis Design Constraints files (Quartus® timing closure)
    ├── src/
    │   ├── custom_ip/                        ← Non-Quartus® IP
    │   ├── ip/                               ← Quartus® IP
    │   ├── rtl/                              ← Quartus® IP
    │   |   ├── agilex5_modkit_vvpisp.v       ← project top level rtl file
    │   ├── sw/
    │   |   ├── niosv_subsystem/              ← DisplayPort Nios® V `.hex` software file
    │   |   ├── niosv_subsystem_ai/           ← AI Stream Controller Nios® V `.hex` software file
    │   |   ├── u-boot/u-boot-spl-dtb.hex     ← U-boot `.hex` software patch file

cd AGX_5E_Modular_Devkit_ISP_AI_WARP_RD
quartus agilex5_modkit_vvpisp.qpf
```

  Note that the folder structure is no longer consistent with the MDT
  methodology but Quartus® Example Designs available through Quartus®.

* If you wish to explore the Platform Designer project, first run the IP
  Generation step (ensure you have the suitable licenses):
  * Click on the `IP Generation` option in the `Compilation Flow` window.
  * Once complete, select the `Files` Tab in the `Project Navigator` window,
    and locate the top level Platform Designer file
    `agilex5_modkit_vvpisp_qsys.qsys`. Double click to open it in Platform
    Designer. You can then navigate the designs IP subsystems.

#### Building the design using the Modular Design Toolkit (MDT)

Follow the next steps to build the 4Kp30 Multi-Sensor Camera with AI Inference
Solution System Example Design:

* Navigate to the `./<project>/scripts` directory and build your project,
  selecting the post processing step option for your chosen MDT flow:

```bash
cd ./<project>/scripts 
# SOF MDT Flow
quartus_sh -t build_shell.tcl -update_ocs -full_compile -ff_post_agx5e
```

```bash
cd ./<project>/scripts 
# RBF MDT Flow (not supported with OCP evaluation license)
quartus_sh -t build_shell.tcl -update_ocs -full_compile -hps_post_agx5e
```

The MDT build options (all of which are needed for a working build):

* `-update_ocs` is used to generate the automatic Offset Capability Structure
  (OCS) ROM, which will be built into the project during compilation.
* `-full_compile` performs not just the full Quartus compilation, but also
  compiles any Nios® V software into `.hex` ROM files built into the project
  during compilation.
* `-ff_post_agx5e` option post processes the FPGA First `.sof` with a first
  stage bootloader from a U-Boot secondary program loader binary file
  `u-boot-spl-dtb_ff.hex`.
* `-hps_post_agx5e` option post processes the HPS first `.sof` with a U-Boot
  secondary program loader binary file `u-boot-spl-dtb.hex`.

<br>

The FPGA programming file/s are located in the
`./<project>/quartus/output_files` directory and will differ depending on the
MDT flow used:

* For SOF MDT Flow [Refer to the main documentation for more details](../docs/camera/camera_4k_ai/flow2-sof-mdt.md):
  * `fsbl_agilex5_modkit_vvpisp_time_limited.sof`
* For RBF MDT Flow [Refer to the main documentation for more details](../docs/camera/camera_4k_ai/flow3-rbf-mdt.md):
  * `agilex5_modkit_vvpisp.hps_first.hps.jic`
  * `agilex5_modkit_vvpisp.hps_first.core.rbf`

#### Building the pregenerated MDT Quartus® project

You can build the pregenerated MDT Quartus® and Platform Designer Project for
the 4Kp30 Multi-Sensor Camera with AI Inference Solution System Example Design
allowing you to view extra information such as resource utilization and timing
reports:

* Using Quartus® GUI:

```bash
cd ./<project>/AGX_5E_Modular_Devkit_ISP_AI_WARP_RD
quartus agilex5_modkit_vvpisp.qpf
```

* Click on the `Compile Design` option in the `Compilation Flow` window.
  Once complete, you can view all the reports.
* Alternatively, you can use the MakeFile from the command line:

```bash
cd ./<project>/AGX_5E_Modular_Devkit_ISP_AI_WARP_RD
make hw_compile
```

To use the compiled design with the existing Application Software for the
Modular Development Kit, you will need to patch the `.sof` with a bootloader to
start the HPS. The make file command to use depends on your license:

* For full license, the patch will generate a `.jic` for the QSPI flash and an
  `.rbf` for the microSD Card. Note that the `.jic` typically only needs to be
  done once. Also note, that any design modification can cause the stock
  Application Software to fail to boot/operate correctly.
  The ouptut is equivalent to the RBF MDT Flow
  [Refer to the main documentation for more details](../docs/camera/camera_4k_ai/flow3-rbf-mdt.md).

```bash
cd ./<project>/AGX_5E_Modular_Devkit_ISP_AI_WARP_RD
make rbf_jic
```

The FPGA programming files are located in the
`./<project>/AGX_5E_Modular_Devkit_ISP_AI_WARP_RD/output_files`
directory:

  * `agilex5_modkit_vvpisp.hps_first.hps.jic`
  * `agilex5_modkit_vvpisp.hps_first.core.rbf`

* For a time limited Open Core Plus license, the `.sof` is patched to create a
  new `fsbl.sof` that can be put onto the board using JTAG. The ouptut is
  equivalent to the SOF MDT Flow
  [Refer to the main documentation for more details](../docs/camera/camera_4k_ai/flow2-sof-mdt.md).

```bash
cd ./<project>/AGX_5E_Modular_Devkit_ISP_AI_WARP_RD
make uboot_sof
```

The FPGA programming file is located in the
`./<project>/AGX_5E_Modular_Devkit_ISP_AI_WARP_RD/output_files` directory:

  * `fsbl_agilex5_modkit_vvpisp_time_limited.sof`

<br>

## Running the 4Kp30 Multi-Sensor Camera with AI Inference Solution System Example Design

[Refer to the main documentation](../docs/camera/camera_4k_ai/camera_4k_ai.md)

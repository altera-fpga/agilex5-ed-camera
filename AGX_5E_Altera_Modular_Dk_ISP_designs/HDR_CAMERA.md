# 4Kp60 Multi-Sensor HDR Camera Solution System Example Design for Agilex™ 5 Devices - Repository

## Overview

The repository contains the necessary files and collateral to create and build
the 4Kp60 Multi-Sensor HDR Camera Solution System Example Design.

The products of this repository are generated using both a Software and
Hardware flow, and are:

| Product | Type | Description |
|----|----|----|
| `.sof` | SRAM Object File | FPGA bitstream to be loadeded over JTAG |
| `.rbf` | Raw Binary File | FPGA bitstream file to be loaded from the microSD Card |
| `.jic` | JTAG Indirect Configuration File | QSPI Flash programming file for booting from microSD Card |
| `.wiz.gz` | Image File | microSD Card Image file containing the Linux embedded system and Camera Software Application |

<br>

### License Requirements

The Camera Solution System Example Design supports the OpenCore Plus (OCP)
evaluation license. The resulting `.sof` can be tested on Hardware using both
the time limited and JTAG tethered features of the license.

Alternatively, full licenses for the VVP IP Suite, VVP Tone Mapping Operator
(TMO) IP, VVP Warp IP, and 3D LUT IP are required to produce a `.jic` and
`.rbf` for a Hardware, SD Card turnkey solution.

Note that for all cases, free licenses for MIPI D-Phy IP, MIPI CSI-2 IP, and
Nios® V Processor must be downloaded and installed.

[Refer to the main documentation for additional licensing information](../docs/camera/camera_4k/camera_4k.md)

<br>


## Software Flow

The [**Software flow**](./../sw/README.md#4kp60-multi-sensor-hdr-camera-solution-system-example-design-for-agilex-5-devices)
generates the appropriate microSD Card Image based on your license and solution
requirements.

<br>

## Hardware Flow

The Hardware flow uses the Modular Design Toolkit (MDT) to create and build the
Quartus® project for the 4Kp60 Multi-Sensor HDR Camera Solution System Example
Design. Note, a pregenerated Quartus® project (using the MDT flow) is also
provided for standard compilation flow.

<br>

### Software Requirements

The MDT requires the following Linux versions of software tools:

* Quartus® Prime Pro 26.1.
* Nios® V Open-Source Tools 26.1 (installed with Quartus® Prime).

<br>

### Creating and compiling the 4Kp60 Multi-Sensor HDR Camera Solution System Example Design

#### Create the design using the Modular Design Toolkit (MDT)

Follow the next steps to create the Quartus® and Platform Designer Project for
the 4Kp60 Multi-Sensor HDR Camera Solution System Example Design:

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

```bash
# SOF MDT Flow
cd agilex5-ed-camera
quartus_sh -t ./modular-design-toolkit/scripts/create/create_shell.tcl -proj_path <project> -proj_name agilex5_modkit_vvpisp -xml_path ./AGX_5E_Altera_Modular_Dk_ISP_designs/AGX_5E_Modular_Devkit_ISP_FF_RD.xml
```

```bash
# RBF MDT Flow (not supported with OCP evaluation license)
cd agilex5-ed-camera
quartus_sh -t ./modular-design-toolkit/scripts/create/create_shell.tcl -proj_path <project> -proj_name agilex5_modkit_vvpisp -xml_path ./AGX_5E_Altera_Modular_Dk_ISP_designs/AGX_5E_Modular_Devkit_ISP_RD.xml
```

This will create your Quartus® Prime and Platform Designer Project in
 `./<project>`. The folder structure is consistent with the MDT methodology.

#### Using the pregenerated MDT Quartus® project

Alternatively, you can download the pregenerated MDT Quartus® and Platform
Designer Project for the 4Kp60 Multi-Sensor HDR Camera Solution System Example
Design:

* Define a `./<project>` location of your choice, creating directory structure
  where necessary.
* Download [AGX_5E_Modular_Devkit_ISP_RD.zip](https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1/AGX_5E_Modular_Devkit_ISP_RD.zip)
  and copy it to your `./<project>` location.
* Navigate to the `./<project>` location, extract the project, and load Quartus® GUI:

```bash
unzip AGX_5E_Modular_Devkit_ISP_RD.zip

    AGX_5E_Modular_Devkit_ISP_RD/
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
    │   |   ├── u-boot/u-boot-spl-dtb.hex     ← U-boot `.hex` software patch file

cd AGX_5E_Modular_Devkit_ISP_RD
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

Follow the next steps to build the 4Kp60 Multi-Sensor HDR Camera Solution
System Example Design:

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

* For SOF MDT Flow [Refer to the main documentation for more details](../docs/camera/camera_4k/flow2-sof-mdt.md):
  * `fsbl_agilex5_modkit_vvpisp_time_limited.sof`
* For RBF MDT Flow [Refer to the main documentation for more details](../docs/camera/camera_4k/flow3-rbf-mdt.md):
  * `agilex5_modkit_vvpisp.hps_first.hps.jic`
  * `agilex5_modkit_vvpisp.hps_first.core.rbf`

#### Building the pregenerated MDT Quartus® project

You can build the pregenerated MDT Quartus® and Platform Designer Project for
the 4Kp60 Multi-Sensor HDR Camera Solution System Example Design allowing you
to view extra information such as resource utilization and timing reports:

* Using Quartus® GUI:

```bash
cd ./<project>/AGX_5E_Modular_Devkit_ISP_RD
quartus agilex5_modkit_vvpisp.qpf
```

  * Click on the `Compile Design` option in the `Compilation Flow` window. Once
    complete, you can view all the reports.

* Alternatively, you can use the MakeFile from the command line:

```bash
cd ./<project>/AGX_5E_Modular_Devkit_ISP_RD
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
  [Refer to the main documentation for more details](../docs/camera/camera_4k/flow3-rbf-mdt.md).

```bash
cd ./<project>/AGX_5E_Modular_Devkit_ISP_RD
make rbf_jic
```

The FPGA programming files are located in the
`./<project>/AGX_5E_Modular_Devkit_ISP_RD/output_files` directory:

  * `agilex5_modkit_vvpisp.hps_first.hps.jic`
  * `agilex5_modkit_vvpisp.hps_first.core.rbf`

* For a time limited Open Core Plus license, the `.sof` is patched to create a
  new `fsbl.sof` that can be put onto the board using JTAG. The ouptut is
  equivalent to the SOF MDT Flow
  [Refer to the main documentation for more details](../docs/camera/camera_4k/flow2-sof-mdt.md).

```bash
cd ./<project>/AGX_5E_Modular_Devkit_ISP_RD
make uboot_sof
```

The FPGA programming file is located in the
`./<project>/AGX_5E_Modular_Devkit_ISP_RD/output_files` directory:

  * `fsbl_agilex5_modkit_vvpisp_time_limited.sof`

<br>

## Running the 4Kp60 Multi-Sensor HDR Camera Solution System Example Design

[Refer to the main documentation](../docs/camera/camera_4k/camera_4k.md)

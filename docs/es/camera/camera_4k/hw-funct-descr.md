

# 4Kp60 Multi-Sensor HDR Camera Solution System Example Design for Agilex™ 5 Devices - Hardware Functional Description

The hardware design for the 4Kp60 Multi-Sensor HDR Camera Solution System
Example Design uses the Modular Design Toolkit (MDT). The MDT is a method of
creating and building Platform Designer (PD) based Quartus® projects from a
single `.xml` file.
<br/>

> **Important Notes** <br/>
> **-** The following text can contain embedded links that aim to assist with
>       navigation, pointers to useful references and resources, or to add
        clarity. <br/>
> **-** There may be slight variations between screenshots and diagrams used
        here and the actual Camera Solution System Example Design.

<br/>

The main advantages of using MDT are:

* Enforces a hierarchical design approach (single level deep).
* Encourages design reuse through a library of off-the-shelf subsystems.
* Enables simple porting of designs to different development boards and FPGA
  devices.
* Provides consistent folder structure and helper scripts.
* Uses TCL scripting for the PD Quartus® project.
<br/>
<br/>


## MDT Overview

The MDT flow consists of 2 separate main steps; a create step and a build step.

The create step:

* Parses the design `.xml` file.
* Creates a Quartus® project.
* Creates a PD system for the project.
* Copies all the project files and adds all the MDT generated files to the Quartus® project.
<br/>
<br/>

The build step:

* Generates the Offset Capability Structure (OCS) ROM (detailed later).
* Compiles all the Nios® V Software into `.hex` files.
* Runs the Quartus® compilation flow.
* Post processes `.sof` files.
<br/>
<br/>


The following top level block diagram shows the main components and subsystems
for the Camera Solution System Example Design hardware.
<br/>

<br/>
<center markdown="1">

![top_block_diagram.](../camera_4k/images/HW/top_block_diagram.png)

**Top Level Hardware Block Diagram**
</center>
<br/>

* The Board and Clock subsystems contain IP related to the Modular Development
  Kit Carrier and SOM Board resources, such as buttons, switches, LEDs,
  reference clocks, and resets. They also forward the resources to the other
  subsystems.
* The HPS subsystem is an instance of the Agilex™ 5 HPS (Hard Processor System)
  which runs all the Linux software for the Camera Solution System Example
  Design. The subsystem includes an EMIF (External Memory Interface) for the
  HPS DDR4 SDRAM on the Modular Development Kit SOM Board and bridges out to
  the FPGA fabric for integration with other subsystems.
  In addition, it also instances the Modular Scatter-Gather Direct Memory
  Access IP (mSGDMA) for memory to memory copy offload.
* The OCS subsystem (readable by the Software Application) is a ROM describing
  the IP (and its capabilities) within the Camera Solution System Example
  Design. Capabilities include the IPs address offset within the system memory
  map allowing the software to auto discover the IP during boot. The main
  advantage of using the OCS is that Hardware (FPGA RTL) and Software can be
  co-developed in parallel without the need to continuously update the system
  memory map.

* MIPI_In, ISP_In, ISP, and VID_Out subsystems are the IP related to camera
  image ingress and Image Signal Processing (ISP). The MIPI_In subsystem
  includes the D-PHY for interfacing the Framos MIPI connectors on the Modular
  Development Kit Carrier Board to the FPGA.



* The EMIF subsystems are used for buffering image data and coefficients and
  include EMIFs for the FPGA DDR4 SDRAMs on the Modular Development Kit SOM
  Board.


* The DP_Tx and DP Nios® V subsystems are related to the DisplayPort (DP)
  output. The Nios® V is used to control the DP IP and along with some glue
  logic provides multi-rate support. The DP_Tx subsystem includes the DP Tx IP.
* The top level includes the FPGA pins and other logic such as the DP Tx Phy to
  drive the DP Tx connector on the Modular Development Kit Carrier Board.
<br/>
<br/>

The MDT flow describes the top level hardware block diagram using an `.xml`
source file. The `.xml` directly relates to the MDT generated PD Quartus®
project. Using the color-coding shown in the Top Level Hardware Block Diagram,
the following diagram illustrates an example of how some of the blocks would
relate to the MDT `.xml` source file and the MDT generated PD Quartus® project:
<br/>

<br/>
<center markdown="1">

![mdt_xml](../camera_4k_resources/images/HW/mdt_xml.png)

**An Example of a MDT PD Quartus® Project from the MDT `.xml` source file**
</center>
<br/>

The `.xml` also defines:

* The name of the overall project.
* The target development board.
* The target FPGA device.
* The QPDS version to use.
* Global PD parameters (for example Pixels In Parallel, Bits Per Symbol, etc.).
* Non-PD subsystems (like the Top level).
<br/>
<br/>


## Quartus® Project

The MDT PD Quartus® project and its subsystems for the Camera Solution System

Example Design (as instantiated from the [AGX_5E_Modular_Devkit_ISP_RD.xml]



file) are described in greater detail below.

<br/>
<center markdown="1">

![pd_top](../camera_4k/images/HW/pd_top.png)

**Camera Solution System Example Design Quartus® Project**
</center>
<br/>

MDT PD subsystems:

* [Board Subsystem](#board-subsystem)
* [Clock Subsystem](#clock-subsystem)
* [HPS Subsystem](#hps-subsystem)
* [OCS Subsystem](#ocs-subsystem)
* [MIPI_In Subsystem](#mipi_in-subsystem)
* [ISP_In Subsystem](#isp_in-subsystem)


* [ISP Subsystem](#isp-subsystem)


* [EMIF Subsystems](#emif-subsystems)
* [VID_Out Subsystem](#vid_out-subsystem)
* [DP Nios® V Subsystem](#dp-nios-v-subsystem)
* [DP_Tx Subsystem](#dp_tx-subsystem)
<br/>
<br/>


### **Board Subsystem**

The Board subsystem contains IP related to the Modular Development Kit
resources such as buttons, switches, and LEDs. The Board subsystem is part of
the MDT common subsystems and the Camera Solution System Example Design does
not necessarily use all the IP.
<br/>

<br/>
<center markdown="1">

![pd_board](../camera_4k/images/HW/pd_board.png)

**Board Subsystem**
</center>
<br/>

The Board subsystem also includes `.qsf` and `.sdc` files relating to the IO
assignments and timing constraints, as well as non-QPDS IP such as a reset
module needed for correct functionality.
<br/>
<br/>


### **Clock Subsystem**

The Clock subsystem contains IP related to the Modular Development Kit Carrier
and SOM Board reference clocks and resets, reset pulse extenders, PLLs for
system clock generation, and system reset synchronizers. The Clock subsystem is
part of the MDT common subsystems.
<br/>

<br/>
<center markdown="1">

![pd_clocks](../camera_4k/images/HW/pd_clocks.png)


**Clock Subsystem**
</center>
<br/>

The clocks and corresponding resets are distributed to the other subsystems and
are detailed in the following table (note that not all of the clocks are
necessarily used in this variant of the Camera Solution System Example Design):
<br/>
<br/>

<center markdown="1">

**Clocks and Resets**


| Clock/Reset | Frequency | Description |
|:----:|:----:| ---- |
| Ref | 100MHz | Board Input Reference (and DP Nios® V CPU interface) Clock |
| 0 | 297MHz | Video Clock |
| 1 | 148.5MHz | Half-rate Video and TMO Nios® V CPU interface Clock|
| 2 | 200MHz | IP agent (HPS CPU interface) Clock |
| 3 | 16MHz | DP Management (DP CPU interface) Clock |
| 4 | 50MHz | EMIF Calibration Clock |



<br/>
</center>

The Clock subsystem also includes non-QPDS IP, such as a reset extender,
needed for correct functionality.
<br/>
<br/>


### **HPS Subsystem**

The HPS subsystem (Hard Processor System) is mainly an instance of the “Hard
Processor System Agilex™ (or other) FPGA IP” and is generally configured
consistently with the GSRD:
[Agilex™ 5 E-Series Modular Development Board GSRD User Guide (26.1)]. However,
some modifications have been made, for example to increase the number of I2C
Masters. Likewise, some IP is not required for this variant of the Camera
Solution System Example Design. The HPS boots a custom version of Linux based
on Yocto to drive the Camera Solution System Example Design.
<br/>

<br/>
<center markdown="1">

![pd_hps](../camera_4k/images/HW/pd_hps.png)

**HPS Subsystem**
</center>
<br/>

<br/>
<center markdown="1">

![pd_hps_block_diagram](../camera_4k/images/HW/pd_hps_block_diagram.png)

**HPS Subsystem Block Diagram**
</center>
<br/>

Internally the basic HPS subsystem is composed of the HPS, EMIF for external
8GB HPS DDR4 SDRAM (available on the Modular Development Kit SOM Board), and
the full HPS to FPGA interface bridge. The HPS to FPGA bridge allows the
Software App to read and write IP registers, and IP memory tables to control
the Camera Solution System Example Design. In addition, the HPS also contains
the mSGDMA IP (Modular Scatter-Gather Direct Memory Access). This IP has access
to the FPGA DDR4 SDRAMs and can is used to offload memory copy functions
between the HPS and FPGA DDR4 SDRAMs. Finally, an Address Span Extender allows
an external VVP Video Frame Reader IP to access the HPS F2SDRAM port to read
directly from the HPS DDR4 SDRAM for the Overlay function. The HPS subsystem to
FPGA memory map is detailed in the following table:
<br/>
<br/>

<center markdown="1">

**HPS Subsystem to FPGA Memory Map**


| Address Start | Address End | Subsystem | Description |
|:----:|:----:| ---- | ---- |
| 0x4000_0000 | 0x4000_3FFF | OCS | Offset Capability Structure subsystem IP |
| 0x4020_0000 | 0x4027_FFFF | ISP | ISP subsystem IP |
| 0x4030_0000 | 0x4030_0FFF | ISP_In | ISP Input subsystem IP |
| 0x4040_0000 | 0x4040_3FFF | MIPI_In | MIPI Input subsystem IP |
| 0x4050_0000 | 0x4050_0FFF | Vid_Out | Video Output subsystem IP |
| 0x4080_0000 | 0x4080_001F | HPS mSGDMA | mSGDMA CSR |
| 0x4080_0020 | 0x4080_003F | HPS mSGDMA | mSGDMA Descriptor |
| 0x4080_0A00 | 0x4080_0A07 | Vid_Out | Overlay DMA - HPS DDR SDRAM Window|




<br/>
</center>

The mSGDMA memory map is detailed in the following table:

<br/>
<br/>

<center markdown="1">

**mSGDMA Memory Map**

| Address Start | Address End | Subsystem | Description |
|:----:|:----:| ---- | ---- |
| 0x0000_0000_0000 | 0x0000_7FFF_FFFF | Reserved | Reserved |
| 0x0000_8000_0000 | 0x0000_0009_FFFF | HPS DDR4 SDRAM | Lower 2GB of the HPS Physical RAM |
| 0x0000_A000_0000 | 0x0008_7FFF_FFFF | Reserved | Reserved |
| 0x0008_8000_0000 | 0x0009_FFFF_FFFF | HPS DDR4 SDRAM | Upper 6GB of the HPS Physical RAM |
| 0x000A_0000_0000 | 0x000F_FFFF_FFFF | Reserved | Reserved |
| 0x0010_0000_0000 | 0x0011_FFFF_FFFF | FPGA DDR4 SDRAM 1 | 8GB FPGA DDR4 SDRAM 1 |
| 0x0012_0000_0000 | 0x0013_FFFF_FFFF | FPGA DDR4 SDRAM 2 | 8GB FPGA DDR4 SDRAM 2 |
| 0x0014_0000_0000 | 0x001F_FFFF_FFFF | - | Unused |

<br/>
</center>

The HPS subsystem includes a `.qsf` file relating to the HPS IO assignments.
<br/>
<br/>


### **OCS Subsystem**

The OCS subsystem (Offset Capability Structure) provides a method to allow
the HPS software to self-discover all the IP within the project that it can
interact with.
<br/>

<br/>
<center markdown="1">

![pd_ocs](../camera_4k/images/HW/pd_ocs.png)

**OCS Subsystem**
</center>
<br/>

All VVP IP contain an OCS entry in the form of:

* Type - A unique identifier for the IP.
* Version - The IP version.
* ID Component - Instance number of the IP.
* Base - Base address of the IP register map.
* Size - Size of the IP Register map.
<br/>
<br/>

OCS entries are stored within the OCS IP inside a ROM. The IP itself can
contain any number of ROMs which are linked together within the IP with the
first ROM always being at a base address offset of 0x0. The Software App is
programmed to always assume that the OCS IP is the first IP on the HPS to FPGA
(hps2fpga) bridge (at a base address offset of 0x0) facilitating the auto
discovery process.
<br/>
<br/>

ROMs can be automatic or manually populated. The MDT flow uses a TCL script
during the build step to search for all the IP within the PD project,
extracting the OCS entry information, and building the automatic ROM. Manually
populated ROMs are used for IP that do not have an OCS entry. Typically, these
are non-VVP IP such as the Address Span Extender for example. An OCS
Modification subsystem is used to specify the manual ROM which MDT builds
during the create step. Note this does not create an additional PD subsystem as
it simply modifies the OCS subsystem. The Camera Solution System Example Design
contains an automatic and a manual ROM. Upon boot, the Software App reads the
entries from the ROMs to determine where the IP is; the driver version to load;
and how it should be used (using its instance number).
<br/>
<br/>


### **MIPI_In Subsystem**


The MIPI_In subsystem is used to ingress 12-bit RAW format 4Kp60 camera sensor
data from 2 Framos MIPI inputs.


<br/>

<br/>
<center markdown="1">

![pd_mipi_in](../camera_4k/images/HW/pd_mipi_in.png)

**MIPI_In Subsystem**

<br/>
<br/>

![pd_mipi_in_block_diagram](../camera_4k/images/HW/pd_mipi_in_block_diagram.png)

**MIPI_In Subsystem Block Diagram**
</center>
<br/>

The MIPI_In subsystem consists of a MIPI D-Phy IP configured to support 2
input links - one each for the 2 Framos MIPI sensor inputs. Each link is
configured for x4 MIPI lanes @1768 Mbps per lane providing enough bandwidth for
4Kp60 processing with 12-bit RAW format data.
<br/>
<br/>


The sensor also supports a Clear HDR feature. In this mode, the sensor
simultaneously captures two images at 25 FPS, one with a low gain level set to
the bright region and the other a high gain level set to the dark region. Since
the images are captured simultaneously, there are no motion chromatic
aberrations or other artifacts. The sensor passes the two images interleaved on
a line per basis over the same single MIPI link.
<br/>
<br/>

The MIPI D-Phy outputs data using a pair of 16-bit PHY Protocol Interfaces
(PPI) - one per sensor. Each PPI connects natively to a MIPI CSI-2 IP which
decodes the RAW12 format MIPI packets and outputs VVP AXI4-S format packets.
Each MIPI CSI-2 IP outputs either primary or a primary and secondary (for Clear
HDR mode) VVP AXI4-S Full streaming interfaces using 4 PIP (Pixels in Parallel)
at 297MHz. This rate was chosen to allow for any bursty data coming from the
sensor and because no real AXI4-S back pressure can be applied (the sensor
cannot be stalled).
<br/>
<br/>

The primary MIPI CSI-2 output interfaces each pass through a VVP Monitor IP to
determine if a sensor is on and passing the expected image data. The secondary
interfaces do not require a Monitor IP. VVP Protocol Converter IPs are then
used on all interfaces to change the protocol from VVP AXI4-S Full to VVP
AXI4-S Lite.
<br/>
<br/>

Each pair of MIPI CSI-2 IP output interfaces are then passed into
a non-QPDS Exposure Fusion IP which is used to combine the two low and high
gain images to produce a single 16-bit HDR image. If Clear HDR mode is in
bypass mode, the Exposure Fusion IP simply maps the 12-bit primary interface to
a 16-bit output interface (most significant 12-bits aligned with 4 least
significant zero bits).
<br/>
<br/>




A VVP PIP Converter IP hangs off each interface to buffer and convert the image
data from 4 PIP down to 2 PIP at 297MHz (enough bandwidth for 4Kp60
processing).

The buffer in the PIP Converter means that the MIPI CSI-2 IP Rx buffer is kept
to a minimum to minimize resource usage.
<br/>
<br/>

The MIPI_In subsystem also includes Input and Output PIO IPs which are used
to provide control and status for the Software App. Control and status can be
connected to external FPGA I/O, like control for the additional master, slave,
and sync sensor signals on the Framos connectors on the Modular Development Kit
Carrier Board. Equally though, control and status can exist within the FPGA,
like status for FPGA build specific capabilities such as a build timestamp,
frame rate and multi-sensor support, development board target, etc. These are
all Camera Solution System Example Design dependent and therefore may be used
in part or not at all.
<br/>
<br/>

The HPS subsystem to MIPI_In subsystem memory map is detailed in the
following table:
<br/>
<br/>

<center markdown="1">

**HPS Subsystem to MIPI_In Subsystem Memory Map**


| Address Start | Address End | Module | Description |
|:----:|:----:| ---- | ---- |
| 0x4040_0000 | 0x4040_0FFF | MIPI DPHY IP | MIPI D-Phy |
| 0x4040_1000 | 0x4040_100F | PIO IP | Input PIO (inst 10) |
| 0x4040_1010 | 0x4040_101F | PIO IP | Input PIO (inst 11) |
| 0x4040_1200 | 0x4040_12FF | Exposure Fusion IP | Exposure Fusion (inst 10) |
| 0x4040_1400 | 0x4040_14FF | Exposure Fusion IP | Exposure Fusion (inst 11) |
| 0x4040_1600 | 0x4040_17FF | VVP PIP Converter IP | Pixels In Parallel Converter (inst 10) |
| 0x4040_1800 | 0x4040_19FF | VVP PIP Converter IP | Pixels In Parallel Converter (inst 11) |
| 0x4040_1A00 | 0x4040_1BFF | VVP Monitor IP | Snoop (inst 10) |
| 0x4040_1C00 | 0x4040_1DFF | VVP Monitor IP | Snoop (inst 11) |
| 0x4040_2000 | 0x4040_2FFF | MIPI CSI-2 IP | MIPI CSI-2 - via Clock Crossing Bridge (inst 10) |
| 0x4040_3000 | 0x4040_3FFF | MIPI CSI-2 IP | MIPI CSI-2 - via Clock Crossing Bridge (inst 11) |



<br/>
</center>

The MIPI_In subsystem includes `.qsf` and `.sdc` files relating to the IO
assignments and timing constraints required for the design.
<br/>
<br/>


### **ISP_In Subsystem**

The ISP_In subsystem is used to provide the input into the ISP subsystem.
<br/>

<br/>
<center markdown="1">

![pd_isp_in](../camera_4k/images/HW/pd_isp_in.png)

**ISP_In Subsystem**

<br/>
<br/>

![pd_isp_in_block_diagram](../camera_4k/images/HW/pd_isp_in_block_diagram.png)

**ISP_In Subsystem Block Diagram**
</center>
<br/>

The ISP_In subsystem consists of a VVP Test Pattern Generator (TPG) IP, a
VVP Throttle IP, a non-QPDS Remosaic (RMS) IP, and a VVP Switch IP to switch
between the input sources. Switching, when activated in the Software App,
always occurs at the end of an active video line. Therefore, software must
guard against switching to camera sources that are not active to avoid lock-up.
The VVP Monitor IP within the MIPI_In subsystem should be used.
<br/>
<br/>


The first input into the Switch comes from the VVP TPG IP path. The TPG is
configured to support color bars and solid color output patterns which can be
selected by the Software App. Since the TPG only outputs the active picture (no
video timing taken into account) a Throttle is used to limit the TPG to 60 FPS.
<br/>
<br/>



The TPG only outputs an RGB (Red, Green, Blue) image. And since the ISP
operates on a Color Filter Array (CFA) image (sometimes known as a Color Filter
Mosaic or Bayer image), a conversion is required. The RMS IP performs this
conversion, allowing a programmable CFA phase to be applied to the TPG RGB
output image to create a CFA image. The TPG path is used for system testing
with or without sensors.
<br/>
<br/>

The second and third inputs into the Switch come from the 2 outputs of the
MIPI_In subsystem.
<br/>
<br/>



The final input into the Switch comes from the VVP Video Frame Reader IP path.
The Frame Reader function is able to inject an image or sequence of images that
have been uploaded into the FPGA DDR4 SDRAM. An additional VVP Throttle IP is
used to control the frame rate of the playback.

<br/>
<br/>

The HPS subsystem to ISP_In subsystem memory map is detailed in the
following table:
<br/>
<br/>

<center markdown="1">

**HPS Subsystem to ISP_In Subsystem Memory Map**



| Address Start | Address End | Module | Description |
|:----:|:----:| ---- | ---- |
| 0x4030_0000 | 0x4030_01FF | VVP Throttle IP | Test Pattern Generator Throttle (inst 20) |
| 0x4030_0400 | 0x4030_05FF | VVP TPG IP | RGB Test Pattern Generator |
| 0x4030_0600 | 0x4030_06FF | Remosaic IP | Remosaic (RGB to Bayer conversion) |
| 0x4030_0800 | 0x4030_09FF | VVP Switch IP | Bayer Switch |
| 0x4030_0A00 | 0x4030_0BFF | VVP Throttle IP | Frame Reader Throttle (inst 21) |
| 0x4030_0C00 | 0x4030_0FFF | VVP Video Frame Reader IP | Frame Reader |


</center>

<br/>




### **ISP Subsystem**

The ISP subsystem is the main image processing pipeline. Reference should be
made to the [ISP Functional Description.](./isp-funct-descr.md) when
reading this section in order to understand the exact IP function.


<br/>

<br/>
<center markdown="1">

![pd_isp](../camera_4k/images/HW/pd_isp.png)

**ISP Subsystem**

<br/>
<br/>

![pd_isp_block_diagram](../camera_4k/images/HW/pd_isp_block_diagram.png)

**ISP Subsystem Block Diagram**
</center>
<br/>

The input into the ISP is a Color Filter Array (CFA) image from the
ISP_In subsystem. For simplicity, the main block diagram doesn't show all the
IP within the ISP subsystem - just the main processing ISP.
<br/>
<br/>

The first ISP IP is the VVP Black Level Statistics (BLS) IP. This IP is used to
obtain statistics relating to the Optical Black Region (OBR) of the image
sensor - typically a shielded area within the sensor. The BLS is used to
continually set the coefficients for the VVP Black Level Correction (BLC) IP as
the black level can change, for example with temperature. However, the IMX 678
used in the Camera Solution System Example Design does not allow access to the
OBR and as such the BLS isn't used in normal operation. However, it is used
during calibration where the sensor lens can be covered to provide a black
level reading. Although not as accurate as continuous reading, it does
nonetheless provide a pretty good black level reading. As with all VVP ISP
Statistics IP. the BLS IP passes input image data to the output untouched.

Note that the BLS may be a build option for some Camera Solution Example Designs.
<br/>
<br/>

The output from BLS feeds into the VVP Clipper IP. The clipper is used to
remove the OBR from the image. However, since the OBR is not available for the
IMX 678, the input image simply passes through the clipper untouched.

Note that the BLS may be a build option for some Camera Solution Example Designs.
<br/>
<br/>


The RAW Frame Capture function allows a single snapshot image to be taken of
the raw sensor image before any ISP processing. The function uses a VVP
Broadcaster IP to pass a copy of the pipeline to the VVP Video Frame Writer IP
via a VVP Color Plane Manager (CPM) IP. The CPM is used to replicate the CFA
color of a given input pixel into all 3 RGB components of a given output pixel,
therefore producing a grayscale RGB image from the CFA raw image. The Frame
Writer uses a frame buffer within a 2GB area of the external 8GB FPGA DDR4
SDRAM (only the first 2GB of the DDR4 SDRAM is used in the Camera Solution
System Example Design). It interfaces to an Address Span Extender which
provides the 2GB window into the EMIF. The Software App can access the buffer
for downloading the captured images.

Note that EMIF performance for the FPGA speed grade Device fitted to the
Modular Development Kit SOM Board is somewhat limited and therefore the capture
function can temporarily interrupt the main output and cause the DP output to
temporarily flicker or go blank during the capture. The Capture feature is
intended for debug.
<br/>
<br/>


The VVP Defective Pixel Correction (DPC) IP is next in the ISP pipeline. The
function of the DPC is to effectively remove defective pixels from the sensor
image. Defective pixels manifests themselves as drastically different intensity
to their neighboring pixels.
<br/>
<br/>


The DPC feeds the VVP Adaptive Noise Reduction IP (ANR). ANR is an
edge-preserving smoothing filter that mainly reduces the independent pixel noise
of an image. It is configured with a 17x17 kernel size and was chosen to
maximize functionality with a sensible resource utilization footprint.
<br/>
<br/>



VVP Black Level Correction (BLC) IP comes next in the processing pipeline.
Based on the BLS results, the image black level can be compensated for by
firstly subtracting a pedestal value before scaling the result back to the full
dynamic range.
<br/>
<br/>

BLC feeds the VVP Vignette Correction (VC) IP. VC is used to compensate for
non-uniform intensity across the image, often caused by uneven light-gathering
of the sensor and optics. The compensation is achieved using a mesh of
coefficient scalers for each color plane and interpolating values between them
for any given pixel. The mesh coefficient generation is done using a
calibration process often with a white non-reflective image from the sensor.
Darker and lighter areas of the image can then be compensated for by boosting
or reducing pixel color values.
<br/>
<br/>

The next stage in the processing pipeline is the White Balance correction. Both
the VVP White Balance Statistics (WBS) IP and VVP White Balance Correction
(WBC) IP are used to eliminate color casts which occur due to lighting
conditions or difference in the light sensitivity of pixels of different color.
The WBS IP collects statistics relating to the red-green and blue-green ratios
within a region of interest (ROI). The Software App uses the WBS results to set
individual color scalers within the WBC IP to alter the balance between the
colors, therefore ensuring that whites really look white, and grays really look
gray without unwanted color tinting. In the Camera Solution System Example
Design, and not shown in the main block diagram, a WBS VVP Switch IP sits
in-line between the VC IP and the VVP Demosaic (DMS) IP. Both the WBS and WBC
are fully connected to the WBS Switch. The Software App allows WBS to come
before or after WBC in the processing pipeline.
<br/>

<br/>
<center markdown="1">

![pd_wbs_switch_block_diagram](../camera_4k_resources/images/HW/pd_wbs_switch_block_diagram.png)

**ISP Subsystem White Balance Switch Configuration Block Diagram**
</center>
<br/>

The Software App Auto-White Balance algorithm (AWB) (as used in the Camera
Solution System Example Design), switches continuously between the
configurations during normal operation to continually adjust the white balance.
<br/>
<br/>

VVP Demosaic (DMS) IP is the final processing IP in the CFA image data domain.
It is a color reconstructing IP and converts the CFA image into an RGB image.
The DMS interpolates missing colors for each pixel based on its neighboring
pixels.
<br/>
<br/>

The RGB image from DMS feeds into the VVP Histogram Statistics (HS) IP. This IP
produces light intensity histograms for both the entire image and a ROI. The
HPS Auto-Exposure algorithm (AE) uses the histograms to adjust the sensor's
exposure settings such as shutter speed and analog gain.
<br/>
<br/>

The Color Correction Matrix (CCM) is primarily used to correct undesired color
bleeding across color channels in the sensor, which is mainly caused by pixels
being sensitive to color spectrums other than their own intended color. The
functionality is provided by a VVP Color Space Converter (CSC) IP which is used
to multiply the input RGB values with a 3x3 CCM to produce the corrected output
RGB values. The CCM can also be used to provide many artistic effects.
<br/>
<br/>




To facilitate conversions between different color spaces and dynamic ranges, or
to simply apply artistic effects, the Camera Solution System Example Design
provides a combination chain of a VVP 1D LUT IP followed by 2 back to back VVP
3D LUT IPs. The 1D LUT can be used to apply an input to output transfer
function, for instance to convert between linear and non-linear color spaces.
The IP is configured as a 12-bit LUT with a 16-bit input lookup and a 14-bit
output value. The configuration was chosen to maximize functionality with a
sensible resource utilization footprint. The VVP 3D LUT IPs can be used to
support application specific combinations of color space conversions, such as
RGB to HLG followed by HLG to BT.709 for instance. Both 3D LUT IPs are
configured for 14-bit input with a LUT size of 17 cubed and 14-bit color depth.
The first 3D LUT has an output of 14-bits whereas the second has an output of
12-bit to match the maximum color depth of the VVP Tone Mapping Operator IP.
Unused LUTs can be placed in bypass mode when not required.
<br/>
<br/>




The VVP TMO IP implements a ROI based tone mapping algorithm to improve the
visibility of latent image detail across areas of the image. The IP feeds into
a VVP Pixel Adapter IP to reduce the output to 10-bits to match the maximum
color depth of the VVP Unsharp Mask (USM) IP.
<br/>
<br/>

The USM applies a sharpening algorithm to the input image by implementing an
unsharp mask. The input image passes through a low pass blur filter to create a
blurred image which is subtracted from the original input image to create a
high frequency component. This component is scaled using a positive (sharpen)
or negative (soften) strength value which is then multiplied against the
original input image before being output.
<br/>
<br/>

The Region of Interest (ROI) IP is next in the pipeline and can be used by the
SW App to highlight an area of interest in the image for use in the GUI.
<br/>
<br/>

The VVP Warp IP is used to apply arbitrary transforms and can correct for lens
distortions like fisheye for instance. Warp can also scale, rotate, and mirror
the image. The Warp uses 512MB of the external 8GB FPGA DDR4 SDRAM. 128MB is
allocated for the transform coefficients (which the HPS Software writes) while
the remainder is used as frame buffers. The Warp interfaces to an Address Span
Extender which provides the 512MB window into the EMIF.




The Warp output is fed to the [VID_Out Subsystem](#vid_out-subsystem).
<br/>
<br/>


The HPS subsystem to ISP subsystem memory map is detailed in the following
table:
<br/>
<br/>

<center markdown="1">

**HPS Subsystem to ISP Subsystem Memory Map**


| Address Start | Address End | Module | Description |
|:----:|:----:| ---- | ---- |
| 0x4020_0000 | 0x4020_7FFF | VVP Warp IP | Warp |
| 0x4020_8200 | 0x4020_83FF | VVP 3D-LUT IP | 3D LUT for HDR processing |
| 0x4020_8A00 | 0x4020_8BFF | VVP BLC IP | Black Level Correction |
| 0x4020_8C00 | 0x4020_8DFF | VVP DPC IP | Defective Pixel Correction |
| 0x4020_8E00 | 0x4020_8FFF | VVP Clipper IP | Clipper |
| 0x4020_9000 | 0x4020_91FF | VVP TMO IP | Tone Mapping Operator |
| 0x4020_9200 | 0x4020_93FF | VVP DMS IP | Demosaic |
| 0x4020_9400 | 0x4020_95FF | VVP WBC IP | White Balance Correction |
| 0x4020_9600 | 0x4020_97FF | VVP BLS IP | Black Level Statistics |
| 0x4020_9800 | 0x4020_99FF | VVP USM IP | Un-Sharp Mask Filter |
| 0x4020_9A00 | 0x4020_9BFF | VVP 3D-LUT IP | 3D LUT for HDR processing |
| 0x4020_9C00 | 0x4020_9DFF | VVP CSC IP | Color Correction Matrix |
| 0x4020_B000 | 0x4020_BFFF | VVP HS IP | Histogram Statistics |
| 0x4020_C000 | 0x4020_CFFF | VVP WBS IP | White Balance Statistics |
| 0x4020_D000 | 0x4020_D1FF | VVP Switch IP | White Balance Switch |
| 0x4020_D400 | 0x4020_D5FF | VVP VFW IP | Frame Writer |
| 0x4020_D800 | 0x4020_D8FF | ROI IP | Region of Interest |
| 0x4020_DA00 | 0x4020_DBFF | VVP CPM IP | Color Plane Manager |
| 0x4021_0000 | 0x4021_3FFF | VVP ANR IP | Adaptive Noise Reduction |
| 0x4022_0000 | 0x4022_FFFF | VVP 1D-LUT IP | 1D LUT for HDR processing |
| 0x4024_0000 | 0x4027_FFFF | VVP VC IP | Vignette Correction |




</center>

<br/>

The ISP subsystem also includes some additional processing scripts to allow
the 3D LUT to be preloaded with a cube file and built into the FPGA.
<br/>
<br/>





### **EMIF Subsystems**

An EMIF subsystem (External Memory Interface) provides an interface to one of
the external 8GB FPGA DDR4 SDRAMs (available on the Modular Development Kit SOM
Board). The local EMIF interface is 256-bit wide running at a clock frequency
of 200MHz. This provides just enough bandwidth to perform write and read of a
4K image at 60 FPS with 70% efficiency. The EMIF subsystem is part of the MDT
common subsystems.
<br/>

<br/>
<center markdown="1">

![pd_emif](../camera_4k/images/HW/pd_emif.png)

**EMIF Subsystem**
</center>
<br/>
<br/>

The FPGA DDR4 SDRAM memory maps are detailed in the following tables:
<br/>
<br/>

<center markdown="1">

**FPGA DDR4 SDRAM 1 Memory Map**


| Address Start | Address End | Size | Module | Description |
|:----:|:----:|:----:| ---- | ---- |
| 0x0_0000_0000 | 0x0_17FF_FFFF | 384MB | ISP subsystem | Warp Buffers |
| 0x0_1800_0000 | 0x0_1FFF_FFFF | 128MB | ISP subsystem | Warp Coefficients |
| 0x0_2000_0000 | 0x1_FFFF_FFFF | 7.5GB | - | Unused |




<br/>
<br/>

**FPGA DDR4 SDRAM 2 Memory Map**


| Address Start | Address End | Size | Module | Description |
|:----:|:----:|:----:| ---- | ---- |
| 0x0_0000_0000 | 0x0_0FFF_FFFF | 256MB | ISP/Vid_Out subsystems | Image Capture Buffer |
| 0x0_1000_0000 | 0x0_07FF_FFFF | 1.75GB | ISP_In subsystem | Frame Reader Buffer |
| 0x0_8000_0000 | 0x1_FFFF_FFFF | 6GB | - | Unused |




</center>
<br/>


### **VID_Out Subsystem**

The VID_Out subsystem provides a final set of functions before the DP_Tx
subsystem input.

<center markdown="1">

![pd_vid_out](../camera_4k/images/HW/pd_vid_out.png)

**VID_Out Subsystem**

<br/>
<br/>

![pd_vid_out_block_diagram](../camera_4k/images/HW/pd_vid_out_block_diagram.png)

**VID_Out Subsystem Block Diagram**
</center>

<br/>
<br/>

Firstly, the ISP output feeds into a VVP Mixer IP. The mixer uses a VVP TPG IP
to provide a solid black base layer (layer 0) for the ISP output to be overlaid
onto. This can be useful when the ISP output is of a smaller resolution than
the connected monitor as it can be "framed" over the TPG background. Also, the
solid black image can be used as the screensaver function. In addition, the TPG
is also configured to support color bars, which the Software App can use for
testing the DP output with an ISP unaltered image.
<br/>
<br/>

The mixer also has an additional layer for an HPS generated overlay image. This
is typically an ARGB8888 image (Alpha+RGB, 8-bit per color sample) and can be
anything useful like a logo.



A VVP Frame Reader IP is used to fetch the overlay image from the HPS DDR4
SDRAM via the F2SDRAM HPS interface. A VVP Scaler IP can be used to upscale the
image which can be placed anywhere in the final output image. A Pixel Adpater
IP is used to convert the ARGB image to 10-bit color samples to match the mixer
configuration. The opacity of the overlay image is controlled by the HPS.
<br/>
<br/>

The mixer output feeds into the Gamma LUT - a 1D LUT IP instance. The Gamma LUT
is used to implement input to output transfer functions (such as Opto-Optical
Transfer Function (OOTF), Opto-Electrical Transfer Function (OETF), and
Electrical-Optical Transfer Function (EOTF)) for video standards and
traditional gamma compression and decompression, as well as High Dynamic Range
Perceptual Quantizer (HDR PQ) and Hybrid Log-Gamma (HDR HLG) correction. The 1D
LUT is configured as a 9-bit LUT with a 10-bit input and output.
<br/>
<br/>


The Output Frame Capture function allows a single snapshot image to be taken of
the final output before the DP Egress. The function uses a VVP Broadcaster IP
to pass a copy of the pipeline to the VVP Video Frame Writer IP. The Frame
Writer uses a frame buffer within a 2GB area of the external 8GB FPGA DDR4
SDRAM (only the first 2GB of the DDR4 SDRAM is used in the Camera Solution
System Example Design). It interfaces to an Address Span Extender which
provides the 2GB window into the EMIF. The Software App can access the buffer
for downloading the captured images.

Note that EMIF performance for the FPGA speed grade Device fitted to the
Modular Development Kit SOM Board is somewhat limited and therefore the capture
function can temporarily interrupt the main output and cause the DP output to
temporarily flicker or go blank during the capture. The Capture feature is
intended for debug.
<br/>
<br/>




Finally the Vid_Out subsystem feeds the (2 PIP VVP
AXI4-S Lite), to the DP_Tx subsystem input (2
PIP VVP AXI4-S Full) via a VVP Protocol Converter IP.


The VID_Out subsystem also includes PIO IPs for the Software App to handshake
DP control and status with the DP Nios® V Software.
<br/>
<br/>

The HPS subsystem to VID_Out subsystem memory map is detailed in the
following table:
<br/>
<br/>

<center markdown="1">

**HPS Subsystem to VID_Out Subsystem Memory Map**


| Address Start | Address End | Module | Description |
|:----:|:----:| ---- | ---- |
| 0x4050_0000 | 0x4050_01FF | VVP TPG IP | Test Pattern Generator |
| 0x4050_0200 | 0x4050_03FF | VVP Scaler IP | Up Scaler |
| 0x4050_0400 | 0x4050_07FF | VVP Mixer IP | Mixer |
| 0x4050_0800 | 0x4050_080F | PIO IP | Input DP Rate Control |
| 0x4050_0810 | 0x4050_081F | PIO IP | Input DP status |
| 0x4050_0820 | 0x4050_082F | PIO IP | Input DP monitor supported formats |
| 0x4050_0830 | 0x4050_083F | PIO IP | Input DP current format |
| 0x4050_0840 | 0x4050_084F | PIO IP | Output DP new monitor format to override |
| 0x4050_0A00 | 0x4050_0BFF | VVP Protocol Converter IP | VVP AXI4-S Lite to VVP AXI4-S Full Protocol Converter |
| 0x4050_0C00 | 0x4050_0FFF | VVP VFR IP | Overlay Frame Reader |
| 0x4050_1200 | 0x4050_13FF | VVP VFW IP | Capture Frame Writer |
| 0x4050_2000 | 0x4050_3FFF | VVP 1D-LUT IP | 1D LUT for Gamma correction |




</center>
<br/>

### **DP Nios® V Subsystem**

The DP Nios® V subsystem (Nios® V CPU subsystem) is used to control the Display
Port Tx IP. In addition, it provides the EDID (Extended Display Identification
Data) processing and interfaces to the HPS for DP control and status
handshaking. The Nios® V subsystem is part of the MDT common CPU subsystems.
<br/>

<br/>
<center markdown="1">

![pd_niosv](../camera_4k_resources/images/HW/pd_niosv.png)

**DP Nios® V Subsystem**
</center>
<br/>

The Nios® V subsystem consists of the Nios® V/m soft processor IP along with
on-chip RAM and IRQ-management. The MDT flow compiles the DP Tx software during
the build step and generates the `.hex` file for the on-chip RAM such that the
Nios® automatically boots and runs on power up.

Note that the `settings.bsp` (normally generated as part of the MDT flow) has
been post modified to remove features to reduce the compiled code size and
therefore reduce the on-chip memory resource. The modified file is supplied
with the DP Tx software source and is explicitly used in the design `.xml`
file which bypasses the generation of a new file during the MDT flow.
Modifications made to the DP Tx software will need to remove the `.xml` setting
to generate a new `settings.bsp` file using the MDT flow. Note that the
compilation may fail due to the size of the new software, although the
`settings.bsp` will be generated correctly. The file can then be modified in a
similar manner, updated in the source software, and reintroduced into the
`.xml` if the User requires. Note the MDT flow should be run again when doing
this. Alternatively, the `.xml` can be modified to increase the memory size for
the new software and allow the `settings.bsp` to be generated every time the MDT
flow is run.

The DP Tx software determines the best resolution and color depth a connected
monitor/TV supports and configures the DP Tx IP accordingly. The Nios® V
subsystem memory map is detailed in the following table:
<br/>
<br/>

<center markdown="1">

**DP Nios® V Subsystem to FPGA Memory Map**


| Address Start | Address End | Module | Description |
|:----:|:----:| ---- | ---- |
| 0x0000_0000 | 0x0003_FFFF | CPU RAM | 256KB On-chip RAM (for the Nios® V CPU) |
| 0x0004_0000 | 0x0004_3FFF | Bridge | DP_Tx Subsystem Bridge |
| 0x0040_0000 | 0x0040_FFFF | Nios® V/m DM Agent | Nios® V/m Debug Module |
| 0x0041_0000 | 0x0041_003F | Nios® V/m Tmer | Nios® V/m Timer Module |
| 0x0041_0040 | 0x0041_005F | CPU Timer | Interval Timer IP (for the Nios® V CPU) |
| 0x0041_0060 | 0x0041_0067 | JTAG UART | JTAG UART IP (for the Nios® V CPU) |




</center>
<br/>


### **DP_Tx Subsystem**

The DP_Tx subsystem (Display Port Tx) provides the DP Tx output. It consists
of the DP Tx IP, I2C controllers for adjusting reference clock frequencies on
the development board, and PIO (Parallel Input/Output) IPs for the DP Nios® V
software to handshake DP control and status with the HPS software.
<br/>

<br/>
<center markdown="1">

![pd_dp_tx](../camera_4k/images/HW/pd_dp_tx.png)

**DP_Tx Subsystem**

<br/>
<br/>

![pd_dp_tx_block_diagram](../camera_4k_resources/images/HW/pd_dp_tx_block_diagram.png)

**DP_Tx Subsystem Block Diagram**
</center>
<br/>

The DP_Tx subsystem includes `.qsf` and `.sdc` files relating to the DP Tx IO
assignments and timing constraints. The MDT DP_Tx creation Tcl script also
includes top level Verilog code for instancing the DP GTS Tx Phy as well as CDC
(Cross Clock Domain) code for the PIO handshaking, and the rate control logic
for the DP multi-rate support.
<br/>

The Nios® V subsystem to DP_Tx subsystem memory map is detailed in the
following table:
<br/>
<br/>

<center markdown="1">

**Nios® V Subsystem to DP_Tx Subsystem Memory Map**

| Address Start | Address End | Module | Description |
|:----:|:----:| ---- | ---- |
| 0x0004_0000 | 0x0004_1FFF | DP Tx IP | DP Tx IP |
| 0x0004_2000 | 0x0004_203F | I2C Master IP | Not Used |
| 0x0004_2040 | 0x0004_207F | I2C Master IP | Development board DP reference clock chip reprogram |
| 0x0004_2080 | 0x0004_208F | PIO IP | Output DP Rate Control |
| 0x0004_2090 | 0x0004_209F | PIO IP | Output DP status |
| 0x0004_20A0 | 0x0004_20AF | PIO IP | Output DP monitor supported formats |
| 0x0004_20B0 | 0x0004_20BF | PIO IP | Output DP current format |
| 0x0004_20C0 | 0x0004_20CF | PIO IP | Input DP new monitor format to override |

</center>
<br/>

### Top Level

The Quartus® project top level Verilog file gets generated through the MDT
create step, along with all supporting files such as `.qsf` and `.sdc` files.
In addition, the Camera Solution System Example Design contains another
subsystem `system` which does not produce a PD subsystem, but instead produces
further supporting files such as `.qsf`, `.sdc`, `dawf`, and `.stp` (if enabled
in the `.xml` file). These are needed to ensure a successful build for the
Quartus® version and IP used and to work around any errata that may exist.
<br/>


### Build Results

The following screenshots show a set of typical build results for the Camera
Solution System Example Design Quartus® project:
<br/>

<br/>
<center markdown="1">

![qpds_flow_summary](../camera_4k/images/HW/qpds_flow_summary.png)

**Flow Summary**

<br/>
<br/>

![qpds_hierarchy](../camera_4k/images/HW/qpds_hierarchy.png)

**Hierarchy Resource Summary**

<br/>
<br/>

![qpds_fmax](../camera_4k/images/HW/qpds_fmax.png)

**FMAX Summary**

<br/>
<br/>

![qpds_design_closure](../camera_4k/images/HW/qpds_design_closure.png)

**Design Closure Summary**
</center>

<br>
<br>

***

<center markdown="1">

[BACK](../camera_4k/camera_4k.md#documentation)
</center>

***
<br>











[User flow 1]: ../camera_4k/camera_4k.md#pre-requisites
[User flow 2]: ../camera_4k/flow2-sof-mdt.md
[User flow 3]: ../camera_4k/flow3-rbf-mdt.md
[User flow 4]: ../camera_4k/flow4.md



[Release Repo]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1
[Release MDT]: https://github.com/altera-fpga/modular-design-toolkit/tree/rel/26.1
[meta-altera-fpga]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/sw/meta-altera-fpga
[meta-altera-fpga-ocs]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/sw/meta-altera-fpga-ocs
[meta-vvp-isp-demo]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/sw/meta-vvp-isp-demo
[agilex-ed-camera/sw]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/sw/



[Release Tag]: https://github.com/altera-fpga/agilex5-ed-camera/releases/tag/rel-26.1-isp_hdr-MDK_RevB_GrpB
[https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1
[hps-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_hdr-MDK_RevB_GrpB/hps-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz
[fpga-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_hdr-MDK_RevB_GrpB/fpga-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz
[fsbl_agilex5_modkit_vvpisp_time_limited.sof]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_hdr-MDK_RevB_GrpB/fsbl_agilex5_modkit_vvpisp_time_limited.sof
[top.core.jic]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_hdr-MDK_RevB_GrpB/top.core.jic
[top.core.rbf]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_hdr-MDK_RevB_GrpB/top.core.rbf



[AGX_5E_Modular_Devkit_ISP_FF_RD.xml]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/AGX_5E_Modular_Devkit_ISP_FF_RD.xml
[AGX_5E_Modular_Devkit_ISP_RD.xml]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/AGX_5E_Modular_Devkit_ISP_RD.xml
[Create microSD card image (.wic.gz) using YOCTO/KAS]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/sw/README.md
[SOF Modular Design Toolkit (MDT) Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/HDR_CAMERA.md#create-the-design-using-the-modular-design-toolkit-mdt
[RBF Modular Design Toolkit (MDT) Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/HDR_CAMERA.md#create-the-design-using-the-modular-design-toolkit-mdt
[Quartus® GUI Create Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/HDR_CAMERA.md#using-the-pregenerated-mdt-quartus-project
[Quartus® GUI Build Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/HDR_CAMERA.md#building-the-pregenerated-mdt-quartus-project


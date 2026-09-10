





# 4Kp60 Multi-Sensor Camera Stitch Solution System Example Design for Agilex™ 5 Devices

The design is compatible with
[Altera® Quartus® Prime Pro Edition version 26.1 Linux].

<br/>

> **Important Notes** <br/>
> **-** The following text can contain embedded links that aim to assist with
        navigation, pointers to useful references and resources, or to add
        clarity. <br/>
> **-** There may be slight variations between screenshots and diagrams used
        here and the actual Camera Solution System Example Design.

## Overview

The 4Kp60 Multi-Sensor Camera Stitch Solution System Example Design for Agilex™ 5 Devices demonstrates a practical glass-to-glass
camera stitch solution. The exclusive support for industry-standard MIPI
(Mobile Industry Processor Interface) D-PHY and MIPI CSI-2 interface on Agilex™
5 FPGAs provides a powerful tool for camera product development.

|<center markdown="1">An example of Image Stitching|
|-|
| ![stitch-overview](../camera_4k_stitch/images/Stitch_Overview.png) |

The MIPI interface supports up to 2.5Gbps per lane and up to 8x lanes per MIPI
interface, enabling seamless data reception from multiple 4K image sensors to
the FPGA fabric for further processing. Each MIPI CSI-2 IP instance converts
pixel data to AXI4-Streaming outputs, enabling connectivity to other IP cores
within Altera®'s Video and Vision Processing (VVP) Suite.

The design is a hardware-software co-design. The hardware component comprises
dual Image Signal Processors (ISPs) with Stitch, various VVP IPs, Hard
Processor Subsystem (HPS) and various connectivity IPs. The software stack is
Linux based and runs on the HPS.

The hardware includes a multi-sensor input video switch feeding into dual Image
Signal Processing (ISP) subsystems. Each ISP is a video processing pipeline
incorporating many VVP IP cores including a high performance Warp IP core for
geometric distortion correction and downscaling to 2Kp60. Each ISP processes
the raw sensor image data into RGB video data which feed into the Stitch video
processing pipeline to blend the images together, side by side to form a wide
4K x 1k output image. In addition, the pipeline also includes a 3D-LUT IP for
color transformations and conversions and the adaptive local tone mapping (TMO)
for handling wide dynamic range scenes. The design creates a full 4Kp60 output
letterbox style image from the stitched output image. The design drives the
resulting 4Kp60 streaming video output data through an Altera® DisplayPort IP.

The software stack consists of an application software binary running on the Linux
operating system with various layers of drivers. The backend part of the
application software interrogates the hardware, dynamically discovers the IP
components and configures them. Multiple feedback loops, in the application
software, monitor the hardware and keep various hardware components in
lockstep. Some of the notable feedback loops are Automatic White Balance (AWB),
Auto Exposure (AE), and Adaptive Noise Reduction (ANR) algorithms, reading
their relevant statistics and adjusting various coefficients and Look Up Tables
(LUTs) in real time. The frontend of the software creates a web-based Graphical
User Interface (GUI) and runs it over a web server.

The following diagram provides an overview of the interaction of the software
running in the Hard Processor Subsystem (HPS) and hardware components running
in the Programmable Logic parts of the device. (For more information on ARM HPS
in Altera® Agilex™ Devices refer to the
[Other Documentation and References](#other-documentation-and-references)
section.)

<br/>
<center markdown="1">

![VVP_ISP_ED_Top_level](../camera_4k_resources/images/VVP_ISP_ED_Top_level.png)

**High-Level Block Diagram of the Camera Solution System Example Design**
</center>
<br/>

> **Hints** <br/>
> **-** If you want to run the pre-built binaries, follow [User Flow 1]. <br/>
> **-** If you want to build your own binaries from source using Quartus® with
        OpenCore Plus IP Evaluation License for time limited and tethered
        camera solutions, follow [User Flow 2]. <br/>
> **-** If you want to just explore the Quartus® Project for [User Flow 2],
        follow [SOF Modular Design Toolkit (MDT) Flow](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#create-the-design-using-the-modular-design-toolkit-mdt). <br/>
> **-** If you want to build your own binaries from source using Quartus® with
        full IP License for turnkey microSD card camera solutions, follow
        [User Flow 3]. <br/>
> **-** If you want to just explore the Quartus® Project for [User Flow 3],
        follow [RBF Modular Design Toolkit (MDT) Flow](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#create-the-design-using-the-modular-design-toolkit-mdt). <br/>
> **-** If you want to just explore the Quartus® Project, follow [User Flow 4],
        (specifically [Quartus® GUI Create Flow](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#using-the-pregenerated-mdt-quartus-project)). <br/><br/>
> For more detail on the different user flows refer to the
  [Recommended user flows](#recommended-user-flows) section. <br/><br/>
> **Important** <br/>
> Take note of all [**Known Issues**.](./known_issues.md)

<br/>

## Pre-requisites

### **Hardware Requirements**

* [Agilex™ 5 FPGA E-Series 065B Modular Development Kit].

<br/>
<center markdown="1">

![Agx5-MDK](../common/images/Agx5-MDK.png)

**Agilex™ 5 FPGA E-Series 065B Modular Development Kit**
<br/>
(variant shown is for illustration purposes only)
</center>
<br/>

* 2 [Framos FSM:GO IMX678C Camera Modules], with [Wide 110deg HFOV Lens].
* Ideal Mount/Tripod Option (for accurate setup):
  * [openSCAD File - Multi-Camera Tripod Mount Adapter for Framos].
  * [Tripod].
  * [Alternative Tripod].
* Alternative Mount Option:
  * 2 [Framos Tripod Mount Adapter]s.
  * Alternative Framos Tripod Mounts: [openSCAD File - Camera Tripod Mount Adapter for Framos FSM:GO IMX678C].
  * 2 [Tripod]s.
  * 2 [Alternative Tripod]s.
* 2 Framos [150mm flex-cable]s for PixelMate MIPI-CSI-2 (for Modular Development Kit connection).
* (Optional) 2 [Framos GMSL3 5m] of which 1 requires [GMSL Modification](../common/gmsl-mod.md)
  * If using the Multi-Camera Tripod Mount Adapter:
    * 2 extra Framos [150mm flex-cable]s for PixelMate MIPI-CSI-2 (between Camera Module and GMSL Serializer).
  * (Ideally, but optional):
    * [openSCAD File - Fixed Camera Mount Adapter for Agilex™ 5 FPGA E-Series 065B Modular Development Kit].
* MicroSD card (minimum 8Gb).
* DP cable or HDMI cable with [DP to HDMI Adapter] (recommend 4Kp60 capable).
* USB Micro B cable x2 (for QSPI programming and HPS serial console access).
* Ethernet cable (for HPS network connection).
* PC monitor or TV: 4Kp60 Recommended.

<br/>


### **Software Requirements to run**

* Host PC with:
  * 8 GB of RAM (less if not rebuilding binaries).
  * Linux/Windows OS installed.
  * Serial terminal (such as GtkTerm or Minicom on Linux, and [TeraTerm] or
    [PuTTY] on Windows).
    * FTDI FT232R USB UART drivers (for a Windows host).
  * Tool to write images for removable USB drives or microSD cards such as
    [Win32DiskImager] on Windows or "dd" command on Linux.
  * [Altera® Quartus® Prime Pro Edition version 26.1 Linux Programmer and Tools] or
  * [Altera® Quartus® Prime Pro Edition version 26.1 Windows Programmer and Tools].
    * Ensure you have all the appropriate byteblaster drivers installed.
  * Ethernet connection (either direct from Host PC to development board, or
    via a switch or router).
    * Note, you may need to disconnected/disabled VPN if it is installed on the
      Host PC.
  * Web browser.

<br/>


## Getting Started - run with pre-built binaries

Follow the instructions provided in this section to run the Camera Solution
System Example Design on the Agilex™ 5 FPGA E-Series 065B Modular Development
Kit.

### **Download the pre-built Binaries**

* Download the pre-built Camera Solution System Example Design binaries for the
  Modular Development Kit:

<br/>

**Binaries**

| Source | Link | Description |
| ---- | ---- | ---- |
| QSPI | [top.core.jic] | Allows the Camera Solution System Example Design to be booted from the microSD card |
| microSD Card Image | [hps-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz] | The Camera Solution System Example Design |

<br/>



## Programming

### **Setting Up the Development Kit**

> **Warning** <br/>
> Handle ESD-sensitive equipment (boards, microSD cards, camera sensors, etc.)
  only when properly grounded and at an ESD-safe workstation.

<br/>

* Configure the Agilex™ 5 FPGA E-Series 065B Modular Development Kit switches
  to the factory default positions:

<br>
<center markdown="1">

![som-board-sw](../common/images/som-board-sw.png)

**Modular Development Kit - System On a Module (SOM) Switch Locations**

<br>
<br>

![board-1-som](../common/images/board-1-som.png)

**Modular Development Kit - System On a Module (SOM) Default Switch Positions**

<br>
<br>

![carrier-board-top-sw](../common/images/carrier-board-top-sw.png)

**Modular Development Kit - Carrier Topside Switch Locations**

<br>
<br>

![carrier-board-bot-sw](../common/images/carrier-board-bot-sw.png)

**Modular Development Kit - Carrier Underside Switch Locations**

<br>
<br>

![board-1-carrier](../common/images/board-1-carrier.png)

**Modular Development Kit - Carrier Default Switch Positions**

<br>

| Switch | Board | Position |
|:----:|:----:|:----:|
| SW1[1:2] | SOM | ON-ON |
| SW2[1:2] | SOM | ON-ON |
| SW2 (if fitted) | Carrier | ON |
| SW6[1:4] | Carrier | OFF-ON-OFF-OFF |
| S1[1:4] | Carrier | OFF-OFF-OFF-ON |
| SW5 | Carrier | OFF |
| S6[1:4] | Carrier (underside) | OFF-OFF-OFF-OFF |
| S11[1:4] | Carrier (underside) | OFF-OFF-OFF-OFF |
| SW13[1:4] | Carrier | OFF-ON-OFF-OFF |
| S7[1:4] | Carrier | OFF-OFF-OFF-OFF |
| SW1 | Carrier | OFF |
| SW4 | Carrier | OFF |
| SW3 | Carrier | ON |

**Modular Development Kit - Switch Positions**

</center>
<br>

* Make the required connections between the Host PC and the Modular Development
  Kit as shown in the following diagram:

<br/>
<center markdown="1">

![ed-conn](../camera_4k_resources/images/ed-conn.png)

**Host PC and Modular Development Kit Connections diagram**
</center>
<br/>

* Connect micro USB cable between the Modular Development Kit SOM Board (`J2`,
  HSP_UART and SOM JTAG) and the Host PC. This will be used for QSPI
  programming / FGPA configuration over JTAG and to access HPS serial console.
  Look at what ports are enumerated on your Host computer. The HPS should be
  the first one.
* Connect an RJ45 cable between the ethernet port on the Modular Development
  Kit SOM Board (`J6`, ETH 1G HPS) and make sure it is on the same network as
  your Host PC. You can check the `eth0` IP address after boot using the Linux
  `ip a` command.
* (Optional) Connect micro USB cable between the Modular Development Kit
  Carrier Board (`J35`) and the Host PC. Can be used for Max10 programming over
  JTAG.

<br>
<center markdown="1">

![Agx-MDK-Conn](../common/images/Agx5-MDK-Conn.png)

**Modular Development Kit Connector Locations**
</center>
<br>


### **Burn the microSD Card Image**

* Either use your own or download the pre-built `<name>.wic.gz` image.
* If required, extract `<name>.wic` image from the compressed download file.
  * On Linux, use the `dd` utility:

  ```bash
  tar -xzf `<name>.wic.gz`
  ```

  * On Windows, use the [7-Zip] program (or similar):
    * Right click `<name>.wic.gz` file, and select "Extract All..."

* Write the `<name>.wic` image to the microSD card using a USB writer:
  * On Linux, use the `dd` utility:

  ```bash
  # Determine the device associated with the SD card on the host computer.
  cat /proc/partitions
  # This will return for example /dev/sd<x>
  # Use dd to write the image in the corresponding device
  sudo dd if=<name>.wic of=/dev/sd<x> bs=1M
  # Flush the changes to the microSD card
  sync
  ```

  * On Windows, use the [Win32DiskImager] program (or similar):
    * Click browse icon and select "\*.\*" filter.
    * Write the image (note your Device may be different to that shown):

    <center markdown="1">

    ![disk-imager-browse](../common/images/disk-imager-browse.png)

    **Navigate to your download and select `<name>.wic` in the "Disk Imager" tool**

    <br>
    <br>

    ![disk-imager](../common/images/disk-imager.png)

    **Write the microSD Card using the "Disk Imager" tool**
    </center>

<br>

* Turn off the Modular Development Kit and insert the microSD card in the
  microSD card slot located on the Modular Development Kit SOM Board.

<br>





[Win32DiskImager]: https://sourceforge.net/projects/win32diskimager
[7-Zip]: https://www.7-zip.org
[TeraTerm]: https://github.com/TeraTermProject/teraterm/releases
[PuTTY]: https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html
[Jupyter Notebook]: https://jupyter.org/
[python]: https://www.python.org/
[GIMP]: https://www.gimp.org/
[GIT LFS]: https://git-lfs.com/
[GIT LFS Installing]: https://github.com/git-lfs/git-lfs?utm_source=gitlfs_site&utm_medium=installation_link&utm_campaign=gitlfs#installing


[Framos FSM:GO IMX678C Camera Modules]: https://www.framos.com
[Wide 110deg HFOV Lens]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FSMGO-IMX678C-M12-L110A-PM-A1Q1?qs=%252BHhoWzUJg4KQkNyKsCEDHw%3D%3D
[Medium 100deg HFOV Lens]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FSMGO-IMX678C-M12-L100A-PM-A1Q1?qs=%252BHhoWzUJg4IesSwD2ACIBQ%3D%3D
[Narrow 54deg HFOV Lens]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FSMGO-IMX678C-M12-L54A-PM-A1Q1?qs=%252BHhoWzUJg4L5yHZulKgVGA%3D%3D
[Framos Tripod Mount Adapter]: https://www.framos.com/en/products/fma-mnt-trp1-4-v1c-26333
[openSCAD File - Camera Tripod Mount Adapter for Framos FSM:GO IMX678C]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1/camera_mount_framos_imx678.scad
[Tripod]: https://thepihut.com/products/small-tripod-for-raspberry-pi-hq-camera
[Alternative Tripod]: https://www.amazon.co.uk/dp/B0DXDQVN73?ref=ppx_yo2ov_dt_b_fed_asin_title
[150mm flex-cable]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FMA-FC-150-60-V1A?qs=GedFDFLaBXGCmWApKt5QIQ%3D%3D&_gl=1*d93qim*_ga*MTkyOTE4MjMxNy4xNzQxMTcwMzQy*_ga_15W4STQT4T*MTc0MTE3MDM0Mi4xLjEuMTc0MTE3MDQ5OS40NS4wLjA
[300mm micro-coax cable]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FFA-MC50-Kit-0.3m?qs=%252BHhoWzUJg4K3LtaE207mhw%3D%3D
[DP to HDMI Adapter]: https://www.amazon.co.uk/gp/product/B01M6WK3KU/ref=ppx_yo_dt_b_asin_title_o02_s00?ie=UTF8&psc=1
[Framos GMSL3]: https://framos.com/news/framos-makes-next-generation-gmsl3-accessible-for-any-embedded-vision-application/
[Framos GMSL3 5m]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FFA-GMSL3-Kit-5m?qs=%252BHhoWzUJg4IkLHv%2F6fzsXQ%3D%3D
[Framos FFA-GMSL-SER-V2A Serializer]: https://www.framos.com/en/products/ffa-gmsl-ser-v2a-27617
[Framos FFA-GMSL-DES-V2A Deserializer]: https://www.framos.com/en/products/ffa-gmsl-des-v2a-27240
[openSCAD File - Fixed Camera Mount Adapter for Agilex™ 5 FPGA E-Series 065B Modular Development Kit]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1/gmsl_bracket_framos.scad
[openSCAD File - Multi-Camera Tripod Mount Adapter for Framos]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1/stitch_camera_mount.scad


[ultralytics YOLO]: https://docs.ultralytics.com
[ONNX]: https://onnx.ai/
[OpenVINO™ Toolkit]: https://storage.openvinotoolkit.org/repositories/openvino/packages/2024.6/linux
[openSCAD]: https://openscad.org/




[Agilex™ 5 E-Series Modular Development Board GSRD User Guide (26.1)]: https://altera-fpga.github.io/rel-26.1/embedded-designs/agilex-5/e-series/modular-065b/gsrd/ug-gsrd-agx5e-modular-065b/
[Agilex™ 5 E-Series Modular Development Board GSRD User Guide (26.1.1)]: https://altera-fpga.github.io/rel-26.1.1/embedded-designs/agilex-5/e-series/modular-065b/gsrd/ug-gsrd-agx5e-modular-065b/


[Agilex™ 5 SoC FPGA]: https://www.altera.com/products/fpga/agilex/5
[Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (25.1)]: https://docs.altera.com/r/docs/814346/25.1/hard-processor-system-technical-reference-manual-agilextm-5-socs/download-document
[Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (26.1)]:https://docs.altera.com/r/docs/814346/26.1/hard-processor-system-technical-reference-manual-agilextm-5-socs/agilextm-5-hard-processor-system-technical-reference-manual-revision-history
[Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (26.1.1)]:https://docs.altera.com/r/docs/814346/26.1.1/hard-processor-system-technical-reference-manual-agilextm-5-socs/agilextm-5-hard-processor-system-technical-reference-manual-revision-history
[NiosV Processor for Altera® FPGA]: https://www.altera.com/design/guidance/nios-v-developer
[Agilex™ 5 FPGA E-Series 065B Modular Development Kit]: https://www.altera.com/products/devkit/po-3274/agilex-5-fpga-and-soc-e-series-065b-modular-development-kit
[Agilex™ 5 FPGA E-Series Modular Development Kits - Product Brief]: https://docs.altera.com/v/u/docs/815178/agilex-5-fpga-e-series-065b-and-065a-modular-development-kit-product-brief
[Altera® FPGA AI Suite]: https://www.altera.com/products/development-tools/fpga-ai-suite


[altera-fpga GitHub site]: https://github.com/altera-fpga
[Software Development]: https://www.altera.com/design/agilex-5/design-hub/software-development#d1e387


[VVP IP Suite]: https://www.altera.com/products/ip/po-3150/video-and-vision-processing-suite
[High-performance Image Signal Processing and Camera Sensor Pipeline Design on FPGAs]:https://docs.altera.com/v/u/docs/827445/high-performance-image-signal-processing-and-camera-sensor-pipeline-design-on-fpgas-white-paper
[MIPI DPHY IP and MIPI CSI-2 IP]: https://www.altera.com/products/ip/po-3062/mipi-d-phy-ip
[DisplayPort IP]: https://www.altera.com/design/fpga-ip/displayport-support


[Altera® Quartus® Prime Pro Edition version 25.1 Linux]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-25-1-linux
[Altera® Quartus® Prime Pro Edition version 25.1 Windows]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-25-1-windows
[Altera® Quartus® Prime Pro Edition version 25.1 Linux Programmer and Tools]: https://www.altera.com/download-center/license-agreement/78566/a80f03fa51b274d2f439004f9f9120f1b867d2ac?filename=QuartusProProgrammerSetup-25.1.0.129-linux.run
[Altera® Quartus® Prime Pro Edition version 25.1 Windows Programmer and Tools]: https://www.altera.com/download-center/license-agreement/78351/af4088c123ab95ef1fa4d00cf30254f1d588cfda?filename=QuartusProProgrammerSetup-25.1.0.129-windows.exe


[Altera® Quartus® Prime Pro Edition version 26.1 Linux]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-26-1-linux
[Altera® Quartus® Prime Pro Edition version 26.1 Windows]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-26-1-windows
[Altera® Quartus® Prime Pro Edition version 26.1 Linux Programmer and Tools]: https://www.altera.com/download-center/license-agreement/127201/22b934d43e3642953f6fa5ea39911dcd3f535cf4?filename=QuartusProProgrammerSetup-26.1.0.110-linux.run
[Altera® Quartus® Prime Pro Edition version 26.1 Windows Programmer and Tools]: https://www.altera.com/download-center/license-agreement/127231/4e7f616c20e1954783e8d9971c0503cab69483c6?filename=QuartusProProgrammerSetup-26.1.0.110-windows.exe


[Altera® Quartus® Prime Pro Edition version 26.1.1 Linux]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-26-1-1-linux
[Altera® Quartus® Prime Pro Edition version 26.1.1 Windows]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-26-1-1-windows
[Altera® Quartus® Prime Pro Edition version 26.1.1 Linux Programmer and Tools]: https://www.altera.com/download-center/license-agreement/127201/22b934d43e3642953f6fa5ea39911dcd3f535cf4?filename=QuartusProProgrammerSetup-26.1.1.110-linux.run
[Altera® Quartus® Prime Pro Edition version 26.1.1 Windows Programmer and Tools]: https://www.altera.com/download-center/license-agreement/127231/4e7f616c20e1954783e8d9971c0503cab69483c6?filename=QuartusProProgrammerSetup-26.1.1.110-windows.exe




### **Program the QSPI Flash Memory**

This should only need to be done once. To program the QSPI flash memory:

* Ensure the Modular Development Kit is powered off. Set MSEL=JTAG by setting
  the **SW1** dip switch on the Modular Development SOM Board to **OFF-OFF**.
  * This prevents any bootloader from starting and leaves the JTAG chain in a
    default state.

* Power up the Modular Development Kit.

* Either use your own or download the pre-built `JIC` image, and write it to
  the QSPI Flash memory using either the command (targetting the SOM JTAG
  chain):

    ```bash
    quartus_pgm -c 1 -m jtag -o "pvi;top.core.jic" 
    ```

* or, optionally using the Quartus® Programmer GUI:

  * Launch the Quartus® Programmer and Configure the **"Hardware Setup..."**
    settings as following (note that the following set of screenshots are for
    illustration only and the exact details may vary slightly):

<br>
<center markdown="1">

![hw-setup-set](../common/images/hw-setup-set.png)

**Programmer - GUI Hardware Settings**
</center>
<br>
<br>

* Click "Auto Detect", select the device `A5EC065AB32A`, and press
  **"Change File.."**

<br>
<center markdown="1">

![programmer-agx5](../common/images/programmer-agx5.png)

**Programmer - After "Auto Detect"**
</center>
<br>
<br>

* Select your `top.core.jic` file. The device updates to `A5ED065BB32A` and the
  `MT25QU02G` device should now be shown (see below). Check the
  **"Program/Configure"** box and press the **"Start"** button. Wait until the
  programming has been completed (which can take several minutes).

<br>
<center markdown="1">

![programmer-agx5-2](../common/images/programmer-agx5-2.png)

**Programming the QSPI Flash with the JIC file**
</center>
<br>
<br>

* Power down the Modular Development Kit. Set MSEL=ASX4 (QSPI) by setting the
  **SW1** dip switch on the Modular Development SOM Board to **ON-ON**.
  * This starts the HPS bootloader and FPGA configuration from the microSD Card
    after power up.
<br>


## Running

### **Setting Up the Camera Solution**

> **Warning** <br/>
> **-** Handle ESD-sensitive equipment (boards, microSD cards, camera sensors, etc.)
        only when properly grounded and at an ESD-safe workstation. <br/>
> **-** Failing to ensure the MIPI connections are aligned pin 1 to pin 1 can cause
        equipment damage.

<br/>

* Make the required connections between the Host PC and the
  [Agilex™ 5 FPGA E-Series 065B Modular Development Kit] as detailed in
  [**Setting Up the Modular Development Kit**](../camera_4k_stitch/camera_4k_stitch.md#programming).
* Connect the Framos cable(s) between the Framos Camera Module(s) and the MIPI
  connector(s) on the Modular Development Kit Carrier Board taking care to
  align the cable(s) correctly with the connector(s) (pin 1 to pin 1). When
  using a single camera module, either MIPI connector can be used.

<br/>
<center markdown="1">

![board-mipi](../common/images/Agx5-MDK-MIPI.png)

**Modular Development Kit Carrier Board MIPI Connector Locations**
</center>
<br/>

<br/>
<center markdown="1">

![mipi-ribbon](../common/images/mipi-ribbon-connection.png)

**Modular Development Kit Carrier Board with Framos MIPI Flex Cable Connected**
</center>
<br/>

<br/>
<center markdown="1">

![camera-ribbon](../common/images/camera-ribbon-connection.png)

**Framos Camera Module with Framos MIPI Flex Cable Connected**
</center>
<br/>

<br/>

* If using the optional Framos GMSL3 solution:
  * Connect the [Framos FFA-GMSL-SER-V2A Serializer] module back-to-back to the
    Framos Camera module.
  * Using the Framos [150mm flex-cable] connect the
    [Framos FFA-GMSL-DES-V2A Deserializer] module to the MIPI0 connector on the
    Modular Development Kit Carrier Board taking care to align the cable
    correctly with the connector (pin 1 to pin 1).
  * Connect the serializer module to the deserializer module using the GMSL3 5m
    coax cable.
  * Connect the power supply to the deserializer module.
    * Note the GMSL3 deserializer module must be powered up before the Modular
      Development Kit.
  * Power up the Modular Development Kit and ensure the deserializer
    modules Lock LED is illuminated green.

<br/>
<center markdown="1">

![GMSL](../common/images/GMSL_1.png)

**Framos Camera Connected Directly to the GMSL Serializer with Coax Cable Connected**
</center>
<br/>

> **Notes on GMSL** <br/>
> **-** It is possible through [GMSL Modification](../common/gmsl-mod.md) to
        support a second GMSL solution in the MIPI1 connector. <br/>
> **-** [openSCAD models](../common/scad-models.md) exist to improve GMSL
        solution robustness.

<br/>

* If using the Multi-Camera Tripod Mount Adapter:
  * Mount the Framos Camera module and GMSL Serializer opposite each other on
    the mount and use an additional Framos flex-cable to connect them.
  * Use the Fixed Camera Mount Adapter for the Modular Devkit to aid robustness.

Note that if you do not use the Multi-Camera Tripod Mount Adapter, then you
will need to manually set up the position of the Cameras. The left view Camera
connects to MIPI0.

<br/>
<center markdown="1">

![dual-gmsl](../common/images/GMSL_2.png)

**Dual GMSL Solution using openSCAD Mounts**

(Each Framos Camera connects to a GMSL Serializer on the opposite side via a Flex Cable in the multi-camera tripod mount)
</center>
<br/>

* Connect the Modular Development Kit Carrier Board DisplayPort Tx connector to
  the Monitor using a suitable cable (and the adapter if you are using an HDMI cable).

<br/>
<center markdown="1">

![full-system](../common/images/full-system.png)

**Modular Development Kit with Connections**
</center>
<br/>





[Win32DiskImager]: https://sourceforge.net/projects/win32diskimager
[7-Zip]: https://www.7-zip.org
[TeraTerm]: https://github.com/TeraTermProject/teraterm/releases
[PuTTY]: https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html
[Jupyter Notebook]: https://jupyter.org/
[python]: https://www.python.org/
[GIMP]: https://www.gimp.org/
[GIT LFS]: https://git-lfs.com/
[GIT LFS Installing]: https://github.com/git-lfs/git-lfs?utm_source=gitlfs_site&utm_medium=installation_link&utm_campaign=gitlfs#installing


[Framos FSM:GO IMX678C Camera Modules]: https://www.framos.com
[Wide 110deg HFOV Lens]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FSMGO-IMX678C-M12-L110A-PM-A1Q1?qs=%252BHhoWzUJg4KQkNyKsCEDHw%3D%3D
[Medium 100deg HFOV Lens]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FSMGO-IMX678C-M12-L100A-PM-A1Q1?qs=%252BHhoWzUJg4IesSwD2ACIBQ%3D%3D
[Narrow 54deg HFOV Lens]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FSMGO-IMX678C-M12-L54A-PM-A1Q1?qs=%252BHhoWzUJg4L5yHZulKgVGA%3D%3D
[Framos Tripod Mount Adapter]: https://www.framos.com/en/products/fma-mnt-trp1-4-v1c-26333
[openSCAD File - Camera Tripod Mount Adapter for Framos FSM:GO IMX678C]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1/camera_mount_framos_imx678.scad
[Tripod]: https://thepihut.com/products/small-tripod-for-raspberry-pi-hq-camera
[Alternative Tripod]: https://www.amazon.co.uk/dp/B0DXDQVN73?ref=ppx_yo2ov_dt_b_fed_asin_title
[150mm flex-cable]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FMA-FC-150-60-V1A?qs=GedFDFLaBXGCmWApKt5QIQ%3D%3D&_gl=1*d93qim*_ga*MTkyOTE4MjMxNy4xNzQxMTcwMzQy*_ga_15W4STQT4T*MTc0MTE3MDM0Mi4xLjEuMTc0MTE3MDQ5OS40NS4wLjA
[300mm micro-coax cable]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FFA-MC50-Kit-0.3m?qs=%252BHhoWzUJg4K3LtaE207mhw%3D%3D
[DP to HDMI Adapter]: https://www.amazon.co.uk/gp/product/B01M6WK3KU/ref=ppx_yo_dt_b_asin_title_o02_s00?ie=UTF8&psc=1
[Framos GMSL3]: https://framos.com/news/framos-makes-next-generation-gmsl3-accessible-for-any-embedded-vision-application/
[Framos GMSL3 5m]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FFA-GMSL3-Kit-5m?qs=%252BHhoWzUJg4IkLHv%2F6fzsXQ%3D%3D
[Framos FFA-GMSL-SER-V2A Serializer]: https://www.framos.com/en/products/ffa-gmsl-ser-v2a-27617
[Framos FFA-GMSL-DES-V2A Deserializer]: https://www.framos.com/en/products/ffa-gmsl-des-v2a-27240
[openSCAD File - Fixed Camera Mount Adapter for Agilex™ 5 FPGA E-Series 065B Modular Development Kit]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1/gmsl_bracket_framos.scad
[openSCAD File - Multi-Camera Tripod Mount Adapter for Framos]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1/stitch_camera_mount.scad


[ultralytics YOLO]: https://docs.ultralytics.com
[ONNX]: https://onnx.ai/
[OpenVINO™ Toolkit]: https://storage.openvinotoolkit.org/repositories/openvino/packages/2024.6/linux
[openSCAD]: https://openscad.org/




[Agilex™ 5 E-Series Modular Development Board GSRD User Guide (26.1)]: https://altera-fpga.github.io/rel-26.1/embedded-designs/agilex-5/e-series/modular-065b/gsrd/ug-gsrd-agx5e-modular-065b/
[Agilex™ 5 E-Series Modular Development Board GSRD User Guide (26.1.1)]: https://altera-fpga.github.io/rel-26.1.1/embedded-designs/agilex-5/e-series/modular-065b/gsrd/ug-gsrd-agx5e-modular-065b/


[Agilex™ 5 SoC FPGA]: https://www.altera.com/products/fpga/agilex/5
[Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (25.1)]: https://docs.altera.com/r/docs/814346/25.1/hard-processor-system-technical-reference-manual-agilextm-5-socs/download-document
[Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (26.1)]:https://docs.altera.com/r/docs/814346/26.1/hard-processor-system-technical-reference-manual-agilextm-5-socs/agilextm-5-hard-processor-system-technical-reference-manual-revision-history
[Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (26.1.1)]:https://docs.altera.com/r/docs/814346/26.1.1/hard-processor-system-technical-reference-manual-agilextm-5-socs/agilextm-5-hard-processor-system-technical-reference-manual-revision-history
[NiosV Processor for Altera® FPGA]: https://www.altera.com/design/guidance/nios-v-developer
[Agilex™ 5 FPGA E-Series 065B Modular Development Kit]: https://www.altera.com/products/devkit/po-3274/agilex-5-fpga-and-soc-e-series-065b-modular-development-kit
[Agilex™ 5 FPGA E-Series Modular Development Kits - Product Brief]: https://docs.altera.com/v/u/docs/815178/agilex-5-fpga-e-series-065b-and-065a-modular-development-kit-product-brief
[Altera® FPGA AI Suite]: https://www.altera.com/products/development-tools/fpga-ai-suite


[altera-fpga GitHub site]: https://github.com/altera-fpga
[Software Development]: https://www.altera.com/design/agilex-5/design-hub/software-development#d1e387


[VVP IP Suite]: https://www.altera.com/products/ip/po-3150/video-and-vision-processing-suite
[High-performance Image Signal Processing and Camera Sensor Pipeline Design on FPGAs]:https://docs.altera.com/v/u/docs/827445/high-performance-image-signal-processing-and-camera-sensor-pipeline-design-on-fpgas-white-paper
[MIPI DPHY IP and MIPI CSI-2 IP]: https://www.altera.com/products/ip/po-3062/mipi-d-phy-ip
[DisplayPort IP]: https://www.altera.com/design/fpga-ip/displayport-support


[Altera® Quartus® Prime Pro Edition version 25.1 Linux]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-25-1-linux
[Altera® Quartus® Prime Pro Edition version 25.1 Windows]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-25-1-windows
[Altera® Quartus® Prime Pro Edition version 25.1 Linux Programmer and Tools]: https://www.altera.com/download-center/license-agreement/78566/a80f03fa51b274d2f439004f9f9120f1b867d2ac?filename=QuartusProProgrammerSetup-25.1.0.129-linux.run
[Altera® Quartus® Prime Pro Edition version 25.1 Windows Programmer and Tools]: https://www.altera.com/download-center/license-agreement/78351/af4088c123ab95ef1fa4d00cf30254f1d588cfda?filename=QuartusProProgrammerSetup-25.1.0.129-windows.exe


[Altera® Quartus® Prime Pro Edition version 26.1 Linux]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-26-1-linux
[Altera® Quartus® Prime Pro Edition version 26.1 Windows]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-26-1-windows
[Altera® Quartus® Prime Pro Edition version 26.1 Linux Programmer and Tools]: https://www.altera.com/download-center/license-agreement/127201/22b934d43e3642953f6fa5ea39911dcd3f535cf4?filename=QuartusProProgrammerSetup-26.1.0.110-linux.run
[Altera® Quartus® Prime Pro Edition version 26.1 Windows Programmer and Tools]: https://www.altera.com/download-center/license-agreement/127231/4e7f616c20e1954783e8d9971c0503cab69483c6?filename=QuartusProProgrammerSetup-26.1.0.110-windows.exe


[Altera® Quartus® Prime Pro Edition version 26.1.1 Linux]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-26-1-1-linux
[Altera® Quartus® Prime Pro Edition version 26.1.1 Windows]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-26-1-1-windows
[Altera® Quartus® Prime Pro Edition version 26.1.1 Linux Programmer and Tools]: https://www.altera.com/download-center/license-agreement/127201/22b934d43e3642953f6fa5ea39911dcd3f535cf4?filename=QuartusProProgrammerSetup-26.1.1.110-linux.run
[Altera® Quartus® Prime Pro Edition version 26.1.1 Windows Programmer and Tools]: https://www.altera.com/download-center/license-agreement/127231/4e7f616c20e1954783e8d9971c0503cab69483c6?filename=QuartusProProgrammerSetup-26.1.1.110-windows.exe




### **Connecting with a Web Browser**

* Power up the Modular Development Kit (if not already powered) and set up the
  serial terminal emulator (minicom, [TeraTerm], [PuTTY], etc.):
  * Select the correct `COMx` port. Set the port configuration as follows:
    * 115200 baud rate, 8 Data bits, 1 Stop bit, CRC and Hardware flow control
      disabled.
* The Linux OS will boot and the Camera Solution System Example Design Software
  Application should run automatically.
* A few seconds after Linux boots, the Software Application will detect the
  attached Monitor and the ISP processed output will be displayed using the
  best supported format.
* Take note of the Modular Development Kit's IP address.
  * The IP address can also be found using the terminal by logging in as `root`
    (no password required) and querying the Ethernet controller:

    ```bash
    root
    ifconfig
    ```

  * `eth0` provides the IPv4 or IPv6 address to connect to.

<br/>
<center markdown="1">

![ed-conn](../camera_4k_resources/images/Setup/ifconfig-dhcp.png)

**An Example ifconfig Output for a DHCP Network**
</center>

<br/>
<center markdown="1">

![ed-conn](../camera_4k_resources/images/Setup/ifconfig-direct.png)

**An Example ifconfig Output for a Network with no DHCP support or is using a direct connection**
</center>

<br/>

* Connect your web browser to the boards IP address so you can interact with
  the Camera Solution System Example Design using the GUI.
  * To connect using IPv6 in the example address shown above, you would use
    `http://[fe80::a8bb:ccff:fe55:6688]` (note the square brackets)
  * To connect using IPv4 for the DHCP example shown above, you would use
    `http://192.168.0.1`

<br/>
<center markdown="1">

![ed-conn](../camera_4k_resources/images/Setup/Browser-IPv6.png)

**An Example Web Browser URL for an IPv6 Address**

<br/>
<br/>

![ed-conn](../camera_4k_resources/images/Setup/Browser-IPv4.png)

**An Example Web Browser URL for an IPv4 address**
</center>

<br/>

* During connection, you will see the Altera® splash screen, after which you
  will be presented with the Web GUI.

<br/>
<center markdown="1">

![ed-conn](../camera_4k_resources/images/Setup/UI-screen.png)

**An Example Camera Solution System Example Design GUI**
</center>

<br/>


## Recommended User Flows

### **Sources**

The sources listed in this table are the most current and highly recommended
for [Altera® Quartus® Prime Pro Edition version 26.1 Linux] builds. Users are
advised to utilize the updated versions of these building blocks in production
environments. Please note that this is a demonstration design and is not
suitable for production or final deployment.

<br/>

**Camera Solution System Example Design Source Repository**

| Component | Location | Branch |
|-|-|-|
| Assets Release Tag | [Release Tag](https://github.com/altera-fpga/agilex5-ed-camera/releases/tag/rel-26.1-isp_stitch-MDK_RevC_GrpB) | rel/26.1 |
| Repository | [Release Repo](https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1) | rel/26.1 |

<br/>

> **Important Notes** <br/>
> **-** Ensure [GIT LFS] is installed prior to cloning this repository.
        See [GIT LFS Installing]. <br/>
> **-** This repository contains a git submodule.
        Use ```--recurse submodules``` option when cloning.<br/>

<br/>
<br/>

With the available source, you can create and modify, build and compile, and
execute the Camera Solution System Example Design. There are 4 recommended
flows that you can explore:

* [User Flow 1]: [Getting Started - Running with pre-built binaries.](#getting-started-run-with-pre-built-binaries)
* [User Flow 2]: Use the [SOF Modular Design Toolkit (MDT) Create Flow](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#create-the-design-using-the-modular-design-toolkit-mdt) and
  [SOF Modular Design Toolkit (MDT) Build Flow](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#build-the-design-using-the-modular-design-toolkit-mdt) - typically used for Quartus®
  with OpenCore Plus IP Evaluation License for time limited and tethered camera
  solutions.
* [User Flow 3]: Use the [RBF Modular Design Toolkit (MDT) Create Flow](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#create-the-design-using-the-modular-design-toolkit-mdt) and
  [RBF Modular Design Toolkit (MDT) Build Flow](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#build-the-design-using-the-modular-design-toolkit-mdt) - typically used for Quartus®
  with full IP License for turnkey microSD card camera solutions.
* [User Flow 4]: Use the [Quartus® GUI Create Flow](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#using-the-pregenerated-mdt-quartus-project) and
  [Quartus® GUI Build Flow](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#building-the-pregenerated-mdt-quartus-project) - typically used to explore the Camera solution.

<br/>

> **Notes** <br/>
> **-** The free OpenCore Plus feature allows you to evaluate licensed IP cores
        in simulation and hardware before purchase. OpenCore Plus evaluation
        supports the following two operation modes: <br/>
> *Untethered* — run the design containing the licensed IP for a limited
        time. <br/>
> *Tethered* — run the design containing the licensed IP for a longer time
        or indefinitely. This operation requires a JTAG connection between your
        Development Kit and the Host computer. <br/>
> **-** All video IP cores that use OpenCore Plus time out simultaneously when
        any one video IP core in the design times out.

<br/>

### **Flows**

<center markdown="1">
**Recommended User Flows**

 | User Flow | Description | [User Flow 1] | [User Flow 2] | [User Flow 3] | [User Flow 4] |
 | --------- | --------- |:---------:|:---------:|:---------:|:---------:|
 | Pre-requisites | Hardware Requirements | [&check;](../camera_4k_stitch/camera_4k_stitch.md#hardware-requirements) | [&check;](../camera_4k_stitch/flow2-sof-mdt.md#hardware-requirements) | [&check;](../camera_4k_stitch/flow3-rbf-mdt.md#hardware-requirements) | &cross; |
 | | Software Requirements to build. | &cross; | [&check;](../camera_4k_stitch/flow2-sof-mdt.md#software-requirements-to-build) | [&check;](../camera_4k_stitch/flow3-rbf-mdt.md#software-requirements-to-build) | [&check;](../camera_4k_stitch/flow4.md#software-requirements-to-build) |
 | | License Requirements to build. | &cross; | [&check;](../camera_4k_stitch/flow2-sof-mdt.md#license-requirements-to-build) | [&check;](../camera_4k_stitch/flow3-rbf-mdt.md#license-requirements-to-build) | [&check;](../camera_4k_stitch/flow4.md#license-requirements-to-build) |
 | | Software Requirements to run | [&check;](../camera_4k_stitch/camera_4k_stitch.md#software-requirements-to-run) | [&check;](../camera_4k_stitch/flow2-sof-mdt.md#software-requirements-to-run) | [&check;](../camera_4k_stitch/flow3-rbf-mdt.md#software-requirements-to-run) | &cross; |
 | | Download the Pre-built Binaries | [&check;](../camera_4k_stitch/camera_4k_stitch.md#download-the-pre-built-binaries) | &cross; |&cross; | &cross; |
 | HW-Compilation | Creating and Building the Design based on the [SOF Modular Design Toolkit (MDT) Create Flow](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#create-the-design-using-the-modular-design-toolkit-mdt) and [SOF Modular Design Toolkit (MDT) Build Flow](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#build-the-design-using-the-modular-design-toolkit-mdt) | &cross; | &check; | &cross; | &cross; |
 | | Creating and Building the Design based on the [RBF Modular Design Toolkit (MDT) Create Flow](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#create-the-design-using-the-modular-design-toolkit-mdt) and [RBF Modular Design Toolkit (MDT) Build Flow](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#build-the-design-using-the-modular-design-toolkit-mdt) | &cross; | &cross; | &check; | &cross; |
 | | Creating and Building the Design based on the [Quartus® GUI Create Flow](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#using-the-pregenerated-mdt-quartus-project) and [Quartus® GUI Build Flow](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#building-the-pregenerated-mdt-quartus-project) | &cross; | &cross; | &cross; | &check; |
 | SW-Compilation | [Create microSD card image (.wic.gz) using YOCTO/KAS](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/sw/README.md) <br/> NOTE: use **KAS_MACHINE=agilex5_mk_a5e065bb32aes1** and **kas/agilex_camera_ff.yml** configuration | &cross; | &check; | &cross; | &cross; |
 | | [Create microSD card image (.wic.gz) using YOCTO/KAS](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/sw/README.md) <br/> NOTE: use **KAS_MACHINE=agilex5_mk_a5e065bb32aes1** and **kas/agilex_camera.yml** configuration | &cross; | &cross; | &check; | &cross; |
 | Programming | Setting Up the Development Kit | [&check;](../camera_4k_stitch/camera_4k_stitch.md#setting-up-the-development-kit) | [&check;](../camera_4k_stitch/flow2-sof-mdt.md#setting-up-the-development-kit) | [&check;](../camera_4k_stitch/flow3-rbf-mdt.md#setting-up-the-development-kit) | &cross; |
 | | Burn the microSD card image. | [&check;](../camera_4k_stitch/camera_4k_stitch.md#burn-the-microsd-card-image) | [&check;](../camera_4k_stitch/flow2-sof-mdt.md#burn-the-microsd-card-image) | [&check;](../camera_4k_stitch/flow3-rbf-mdt.md#burn-the-microsd-card-image) | &cross; |
 | | Program the QSPI Flash Memory | [&check;](../camera_4k_stitch/camera_4k_stitch.md#program-the-qspi-flash-memory) | &cross; | [&check;](../camera_4k_stitch/flow3-rbf-mdt.md#program-the-qspi-flash-memory) | &cross; |
 | Running | Setting Up the Camera Solution | [&check;](../camera_4k_stitch/camera_4k_stitch.md#setting-up-the-camera-solution) | [&check;](../camera_4k_stitch/flow2-sof-mdt.md#setting-up-the-camera-solution) | [&check;](../camera_4k_stitch/flow3-rbf-mdt.md#setting-up-the-camera-solution) | &cross; |
 | | Program the FPGA SOF | &cross; | [&check;](../camera_4k_stitch/flow2-sof-mdt.md#program-the-fpga-sof) | &cross; | &cross; |
 | | Connecting with a Web Browser | [&check;](../camera_4k_stitch/camera_4k_stitch.md#connecting-with-a-web-browser) | [&check;](../camera_4k_stitch/flow2-sof-mdt.md#connecting-with-a-web-browser) | [&check;](../camera_4k_stitch/flow3-rbf-mdt.md#connecting-with-a-web-browser) | &cross; |

</center>
<br/>

## Resources

### **Documentation**
* [Features.](./features.md)
* [ISP IP Design Functional Description.](./isp-funct-descr.md)
* [Hardware Design Functional Description.](./hw-funct-descr.md)
* [Software Functional Description.](./sw-funct-descr.md)
* [Web GUI Functional Description.](./ui-funct-descr.md)
* [Design Security Considerations.](./design-security-considerations.md)
* [Acronyms and Terminology.](./glossary.md)
* [A Guide to Lens Correction and Projection.](./stitch.md)
* [Known Issues.](./known_issues.md)
* [GMSL Modification.](../common/gmsl-mod.md)
* [openSCAD models.](../common/scad-models.md)

### **References**
* [Agilex™ 5 FPGA E-Series 065B Modular Development Kit].
* [Framos FSM:GO IMX678C Camera Modules](https://www.framos.com).
* [Framos FFA-GMSL-SER-V2A Serializer](https://www.framos.com/en/products/ffa-gmsl-ser-v2a-27617).
* [Framos FFA-GMSL-DES-V2A Deserializer](https://www.framos.com/en/products/ffa-gmsl-des-v2a-27240).
* [VVP IP Suite].
* [Warp](https://www.altera.com/products/ip/po-3156/warp-fpga-ip).
* [Tone Mapping Operator](https://www.altera.com/products/ip/po-3151/tone-mapping-operator-fpga-ip).
* [3D LUT](https://www.altera.com/products/ip/po-3152/3d-lut-altera-fpga-ip).
* [MIPI DPHY IP and MIPI CSI-2 IP].
* [DisplayPort IP].

### **Other Repositories Used**
| Component | Location | Branch |
|----|----|----|
|Modular Design Toolkit|[Release MDT](https://github.com/altera-fpga/modular-design-toolkit/tree/rel/26.1)|rel/26.1|
|Linux|[https://github.com/altera-opensource/linux-socfpga](https://github.com/altera-opensource/linux-socfpga)|socfpga-6.6.22-lts|
|Arm Trusted Firmware|[https://github.com/ARM-software/arm-trusted-firmware](https://github.com/ARM-software/arm-trusted-firmware)|socfpga_v2.11.0|
|U-Boot|[https://github.com/altera-opensource/u-boot-socfpga](https://github.com/altera-opensource/u-boot-socfpga)|v2024.01|
|Yocto Project: poky|[https://git.yoctoproject.org/poky](https://git.yoctoproject.org/poky)|scarthgap|

### **Other Documentation and References**
* [High-performance Image Signal Processing and Camera Sensor Pipeline Design on FPGAs].
* [altera-fpga GitHub site].
* [Software Development].
* [Agilex™ 5 SoC FPGA].
* [Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (26.1)].
* [NiosV Processor for Altera® FPGA].
* [openSCAD](https://openscad.org/).

<br/>





[User flow 1]: ../camera_4k_stitch/camera_4k_stitch.md#pre-requisites
[User flow 2]: ../camera_4k_stitch/flow2-sof-mdt.md
[User flow 3]: ../camera_4k_stitch/flow3-rbf-mdt.md
[User flow 4]: ../camera_4k_stitch/flow4.md



[Release Repo]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1
[Release MDT]: https://github.com/altera-fpga/modular-design-toolkit/tree/rel/26.1
[meta-altera-fpga]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/sw/meta-altera-fpga
[meta-altera-fpga-ocs]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/sw/meta-altera-fpga-ocs
[meta-vvp-isp-demo]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/sw/meta-vvp-isp-demo
[agilex5-ed-camera/sw]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/sw/



[Release Tag]: https://github.com/altera-fpga/agilex5-ed-camera/releases/tag/rel-26.1-isp_stitch-MDK_RevC_GrpB
[https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1
[hps-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_stitch-MDK_RevC_GrpB/hps-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz
[fpga-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_stitch-MDK_RevC_GrpB/fpga-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz
[fsbl_agilex5_modkit_vvpisp_time_limited.sof]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_stitch-MDK_RevC_GrpB/fsbl_agilex5_modkit_vvpisp_time_limited.sof
[top.core.jic]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_stitch-MDK_RevC_GrpB/top.core.jic
[top.core.rbf]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_stitch-MDK_RevC_GrpB/top.core.rbf



[AGX_5E_Modular_Devkit_ISP_Stitch_FF_RD.xml]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/AGX_5E_Modular_Devkit_ISP_Stitch_FF_RD.xml
[AGX_5E_Modular_Devkit_ISP_Stitch_RD.xml]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/AGX_5E_Modular_Devkit_ISP_Stitch_RD.xml
[Create microSD card image (.wic.gz) using YOCTO/KAS]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/sw/README.md
[SOF Modular Design Toolkit (MDT) Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#create-the-design-using-the-modular-design-toolkit-mdt
[SOF Modular Design Toolkit (MDT) Create Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#create-the-design-using-the-modular-design-toolkit-mdt
[SOF Modular Design Toolkit (MDT) Build Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#build-the-design-using-the-modular-design-toolkit-mdt
[RBF Modular Design Toolkit (MDT) Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#create-the-design-using-the-modular-design-toolkit-mdt
[RBF Modular Design Toolkit (MDT) Create Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#create-the-design-using-the-modular-design-toolkit-mdt
[RBF Modular Design Toolkit (MDT) Build Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#build-the-design-using-the-modular-design-toolkit-mdt
[Quartus® GUI Create Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#using-the-pregenerated-mdt-quartus-project
[Quartus® GUI Build Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#building-the-pregenerated-mdt-quartus-project





[Win32DiskImager]: https://sourceforge.net/projects/win32diskimager
[7-Zip]: https://www.7-zip.org
[TeraTerm]: https://github.com/TeraTermProject/teraterm/releases
[PuTTY]: https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html
[Jupyter Notebook]: https://jupyter.org/
[python]: https://www.python.org/
[GIMP]: https://www.gimp.org/
[GIT LFS]: https://git-lfs.com/
[GIT LFS Installing]: https://github.com/git-lfs/git-lfs?utm_source=gitlfs_site&utm_medium=installation_link&utm_campaign=gitlfs#installing


[Framos FSM:GO IMX678C Camera Modules]: https://www.framos.com
[Wide 110deg HFOV Lens]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FSMGO-IMX678C-M12-L110A-PM-A1Q1?qs=%252BHhoWzUJg4KQkNyKsCEDHw%3D%3D
[Medium 100deg HFOV Lens]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FSMGO-IMX678C-M12-L100A-PM-A1Q1?qs=%252BHhoWzUJg4IesSwD2ACIBQ%3D%3D
[Narrow 54deg HFOV Lens]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FSMGO-IMX678C-M12-L54A-PM-A1Q1?qs=%252BHhoWzUJg4L5yHZulKgVGA%3D%3D
[Framos Tripod Mount Adapter]: https://www.framos.com/en/products/fma-mnt-trp1-4-v1c-26333
[openSCAD File - Camera Tripod Mount Adapter for Framos FSM:GO IMX678C]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1/camera_mount_framos_imx678.scad
[Tripod]: https://thepihut.com/products/small-tripod-for-raspberry-pi-hq-camera
[Alternative Tripod]: https://www.amazon.co.uk/dp/B0DXDQVN73?ref=ppx_yo2ov_dt_b_fed_asin_title
[150mm flex-cable]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FMA-FC-150-60-V1A?qs=GedFDFLaBXGCmWApKt5QIQ%3D%3D&_gl=1*d93qim*_ga*MTkyOTE4MjMxNy4xNzQxMTcwMzQy*_ga_15W4STQT4T*MTc0MTE3MDM0Mi4xLjEuMTc0MTE3MDQ5OS40NS4wLjA
[300mm micro-coax cable]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FFA-MC50-Kit-0.3m?qs=%252BHhoWzUJg4K3LtaE207mhw%3D%3D
[DP to HDMI Adapter]: https://www.amazon.co.uk/gp/product/B01M6WK3KU/ref=ppx_yo_dt_b_asin_title_o02_s00?ie=UTF8&psc=1
[Framos GMSL3]: https://framos.com/news/framos-makes-next-generation-gmsl3-accessible-for-any-embedded-vision-application/
[Framos GMSL3 5m]: https://www.mouser.co.uk/ProductDetail/FRAMOS/FFA-GMSL3-Kit-5m?qs=%252BHhoWzUJg4IkLHv%2F6fzsXQ%3D%3D
[Framos FFA-GMSL-SER-V2A Serializer]: https://www.framos.com/en/products/ffa-gmsl-ser-v2a-27617
[Framos FFA-GMSL-DES-V2A Deserializer]: https://www.framos.com/en/products/ffa-gmsl-des-v2a-27240
[openSCAD File - Fixed Camera Mount Adapter for Agilex™ 5 FPGA E-Series 065B Modular Development Kit]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1/gmsl_bracket_framos.scad
[openSCAD File - Multi-Camera Tripod Mount Adapter for Framos]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1/stitch_camera_mount.scad


[ultralytics YOLO]: https://docs.ultralytics.com
[ONNX]: https://onnx.ai/
[OpenVINO™ Toolkit]: https://storage.openvinotoolkit.org/repositories/openvino/packages/2024.6/linux
[openSCAD]: https://openscad.org/




[Agilex™ 5 E-Series Modular Development Board GSRD User Guide (26.1)]: https://altera-fpga.github.io/rel-26.1/embedded-designs/agilex-5/e-series/modular-065b/gsrd/ug-gsrd-agx5e-modular-065b/
[Agilex™ 5 E-Series Modular Development Board GSRD User Guide (26.1.1)]: https://altera-fpga.github.io/rel-26.1.1/embedded-designs/agilex-5/e-series/modular-065b/gsrd/ug-gsrd-agx5e-modular-065b/


[Agilex™ 5 SoC FPGA]: https://www.altera.com/products/fpga/agilex/5
[Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (25.1)]: https://docs.altera.com/r/docs/814346/25.1/hard-processor-system-technical-reference-manual-agilextm-5-socs/download-document
[Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (26.1)]:https://docs.altera.com/r/docs/814346/26.1/hard-processor-system-technical-reference-manual-agilextm-5-socs/agilextm-5-hard-processor-system-technical-reference-manual-revision-history
[Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (26.1.1)]:https://docs.altera.com/r/docs/814346/26.1.1/hard-processor-system-technical-reference-manual-agilextm-5-socs/agilextm-5-hard-processor-system-technical-reference-manual-revision-history
[NiosV Processor for Altera® FPGA]: https://www.altera.com/design/guidance/nios-v-developer
[Agilex™ 5 FPGA E-Series 065B Modular Development Kit]: https://www.altera.com/products/devkit/po-3274/agilex-5-fpga-and-soc-e-series-065b-modular-development-kit
[Agilex™ 5 FPGA E-Series Modular Development Kits - Product Brief]: https://docs.altera.com/v/u/docs/815178/agilex-5-fpga-e-series-065b-and-065a-modular-development-kit-product-brief
[Altera® FPGA AI Suite]: https://www.altera.com/products/development-tools/fpga-ai-suite


[altera-fpga GitHub site]: https://github.com/altera-fpga
[Software Development]: https://www.altera.com/design/agilex-5/design-hub/software-development#d1e387


[VVP IP Suite]: https://www.altera.com/products/ip/po-3150/video-and-vision-processing-suite
[High-performance Image Signal Processing and Camera Sensor Pipeline Design on FPGAs]:https://docs.altera.com/v/u/docs/827445/high-performance-image-signal-processing-and-camera-sensor-pipeline-design-on-fpgas-white-paper
[MIPI DPHY IP and MIPI CSI-2 IP]: https://www.altera.com/products/ip/po-3062/mipi-d-phy-ip
[DisplayPort IP]: https://www.altera.com/design/fpga-ip/displayport-support


[Altera® Quartus® Prime Pro Edition version 25.1 Linux]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-25-1-linux
[Altera® Quartus® Prime Pro Edition version 25.1 Windows]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-25-1-windows
[Altera® Quartus® Prime Pro Edition version 25.1 Linux Programmer and Tools]: https://www.altera.com/download-center/license-agreement/78566/a80f03fa51b274d2f439004f9f9120f1b867d2ac?filename=QuartusProProgrammerSetup-25.1.0.129-linux.run
[Altera® Quartus® Prime Pro Edition version 25.1 Windows Programmer and Tools]: https://www.altera.com/download-center/license-agreement/78351/af4088c123ab95ef1fa4d00cf30254f1d588cfda?filename=QuartusProProgrammerSetup-25.1.0.129-windows.exe


[Altera® Quartus® Prime Pro Edition version 26.1 Linux]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-26-1-linux
[Altera® Quartus® Prime Pro Edition version 26.1 Windows]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-26-1-windows
[Altera® Quartus® Prime Pro Edition version 26.1 Linux Programmer and Tools]: https://www.altera.com/download-center/license-agreement/127201/22b934d43e3642953f6fa5ea39911dcd3f535cf4?filename=QuartusProProgrammerSetup-26.1.0.110-linux.run
[Altera® Quartus® Prime Pro Edition version 26.1 Windows Programmer and Tools]: https://www.altera.com/download-center/license-agreement/127231/4e7f616c20e1954783e8d9971c0503cab69483c6?filename=QuartusProProgrammerSetup-26.1.0.110-windows.exe


[Altera® Quartus® Prime Pro Edition version 26.1.1 Linux]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-26-1-1-linux
[Altera® Quartus® Prime Pro Edition version 26.1.1 Windows]: https://www.altera.com/downloads/fpga-development-tools/quartus-prime-pro-edition-design-software-version-26-1-1-windows
[Altera® Quartus® Prime Pro Edition version 26.1.1 Linux Programmer and Tools]: https://www.altera.com/download-center/license-agreement/127201/22b934d43e3642953f6fa5ea39911dcd3f535cf4?filename=QuartusProProgrammerSetup-26.1.1.110-linux.run
[Altera® Quartus® Prime Pro Edition version 26.1.1 Windows Programmer and Tools]: https://www.altera.com/download-center/license-agreement/127231/4e7f616c20e1954783e8d9971c0503cab69483c6?filename=QuartusProProgrammerSetup-26.1.1.110-windows.exe




[Test Pattern Generator IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/test-pattern-generator-ip
[Switch IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/switch-ip
[Black Level Statistics IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/black-level-statistics-ip
[Clipper IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/clipper-ip
[Defective Pixel Correction IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/defective-pixel-correction-ip
[Adaptive Noise Reduction IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/adaptive-noise-reduction-ip
[Black Level Correction IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/black-level-correction-ip
[Vignette Correction IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/vignette-correction-ip
[White Balance Statistics IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/white-balance-statistics-ip
[White Balance Correction IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/white-balance-correction-ip
[Demosaic IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/demosaic-ip
[Histogram Statistics IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/histogram-statistics-ip
[Color Space Converter IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/color-space-converter-ip
[1D LUT]: https://www.altera.com/products/ip/a1jui000004r4gnmas/1d-lut-altera-fpga-ip
[1D LUT IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/d-lut-ip?tocId=aPezyn4lPf1RdBu%7EAlXEEQ
[3D LUT]: https://www.altera.com/products/ip/po-3152/3d-lut-altera-fpga-ip
[3D LUT IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/d-lut-ip?tocId=AMpTCaguLedw3wZdHM_3Bg
[LUTCalc GitHub page]: https://github.com/cameramanben/LUTCalc
[Tone Mapping Operator]: https://www.altera.com/products/ip/po-3151/tone-mapping-operator-fpga-ip
[Tone Mapping Operator IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/tone-mapping-operator-ip
[Unsharp Mask IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/unsharp-mask-ip
[Warp]: https://www.altera.com/products/ip/po-3156/warp-fpga-ip
[Warp IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/warp-ip
[Mixer IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/mixer-ip
[Video Frame Writer IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/video-frame-writer-ip
[Video Frame Reader IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/video-frame-reader-ip
[Color Plane Manager IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/color-plane-manager-ip
[Bits per Color Sample Adapter IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/bits-per-color-sample-adapter-ip
[Protocol Converter IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/protocol-converter-ip
[Pixels in Parallel Converter IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/pixels-in-parallel-converter-ip
[Video and Vision Processing Suite Altera® FPGA IP User Guide]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/about-the-video-and-vision-processing-suite
[Altera® FPGA Streaming Video Protocol Specification]: https://docs.altera.com/r/docs/683397/current/altera-streaming-video-protocol-specification/about-the-altera-streaming-video-protocol
[AMBA 4 AXI4-Stream Protocol Specification]: https://developer.arm.com/documentation/ihi0051/a/
[Avalon® Interface Specifications – Avalon® Streaming Interfaces]: https://docs.altera.com/r/docs/683091/22.3/avalon-interface-specifications/introduction-to-the-avalon-interface-specifications
[KAS]: https://kas.readthedocs.io/en/latest/
[EMIF]: https://www.altera.com/design/guidance/emif-support
[Scaler IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/scaler-ip
[MSGDMA IP]: https://docs.altera.com/r/docs/683130/26.1/embedded-peripherals-ip-user-guide/modular-scatter-gather-dma-core
[Broadcaster IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/axi-stream-broadcaster-ip
[Video and Vision Monitor IP]: https://docs.altera.com/r/docs/683329/25.1/video-and-vision-processing-suite-ip-user-guide/video-and-vision-monitor-ip
[Region Of Interest IP]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/docs/camera/common/non-qpds-ip/Region_of_Interest_basic_guide.pdf
[Remoasaic IP]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/docs/camera/common/non-qpds-ip/Remosaic_basic_guide.pdf
[Throttle IP]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/docs/camera/common/non-qpds-ip/Throttle_basic_guide.pdf
[Alpha Channel IP]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/docs/camera/common/non-qpds-ip/Alpha_Channel_basic_guide.pdf


<br/>

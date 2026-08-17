# GMSL Modification

The [Framos GMSL3](https://framos.com/news/framos-makes-next-generation-gmsl3-accessible-for-any-embedded-vision-application/) solution does not use the I2C Slave Mode pins (`SLAMODE`)
available on the Framos MIPI connector and instead hardcodes the I2C device
addresses on both the Framos Serializer and Deserializer boards. Since the
Agilex™ 5 FPGA E-Series 065B Modular Development Kit only uses a single I2C
bus for both the Framos MIPI connectors, using the same Framos GMSL3 solution
in each MIPI connector creates I2C conflicts and therefore does not work.

The I2C device addresses are hardcoded using pull up and pull down resistor
networks which sets 1 of 2 available I2C slave addresses for each of the
devices on the Serializer and Deserializer boards. Modifying these networks
will set the second I2C slave address and therefore will then allow both an
unmodified and modified GMSL solution to be used on the same I2C bus. The
Camera Solution Example Designs support automatic detection of an unmodified
GMSL solution in MIPI 0 and a modified GMSL solution in MIPI 1. All other
combinations, including mix and match modified and unmodified Serializer and
Deserializer boards, are not supported by the automatic detection software.

<br/>

> **Important Warning** <br/>
> **-** Modification will invalidate any warranty and is undertaken entirely at
        your own risk. <br/>
> **-** Specialist 0402 package surface mount Hardware skills, equipment, and
        components will be required. <br/>
> **-** Handle ESD-sensitive equipment (boards, microSD cards, camera sensors,
        etc.) only when properly grounded and at an ESD-safe workstation.

<br/>

> **Disclaimer** <br/>
> Altera disclaims all express and implied warranties, including without
  limitation, the implied warranties of merchantability, fitness for a
  particular purpose, and non-infringement, as well as any warranty arising
  from course of performance, course of dealing, or usage in trade. You are
  responsible for safety of the overall system, including compliance with
  applicable safety-related requirements or standards.

<br/>
<br/>

## GMSL Serializer

The [Framos FFA-GMSL-SER-V2A Serializer](https://www.framos.com/en/products/ffa-gmsl-ser-v2a-27617) contains 2 addressable I2C devices
which can be modified as circled twice in blue below:
<br/>
<br/>

|<center markdown="1">Serializer Underside View</center>|<center markdown="1">Serializer Underside PCB Overlay View</center>|
|-|-|
| ![Serializer 3D](../common/images/GMSL/gmsl_serializer_3d.png) | ![Serializer PCB](../common/images/GMSL/gmsl_serializer_pcb.png) |

<br/>

| Device | Resister Network | Unmodified (Default) Values | Unmodified (Default) I2C Address | Modified Values | Modified I2C Address |
| ---- | ---- | ---- | ---- | ---- | ---- |
| Serializer IC | R18 and R21 | 56.2k and 44.2k | 0x42 | 68.1k and 32.4k | 0x40 |
| GPIO Extender IC | R1 and R8 | 1k and DNF (Do Not Fit) | 0x21 | DNF and 1k  | 0x20 |

<br/>
<br/>

## GMSL Deserializer

The [Framos FFA-GMSL-DES-V2A Deserializer](https://www.framos.com/en/products/ffa-gmsl-des-v2a-27240) contains a single addressable I2C
device which can be modified as circled in red below:
<br/>
<br/>


|<center markdown="1">Deserializer Underside View</center>|<center markdown="1">Deserializer Underside PCB Overlay View</center>|
|-|-|
| ![Deserializer 3D](../common/images/GMSL/gmsl_deserializer_3d.png) | ![Deserializer PCB](../common/images/GMSL/gmsl_deserializer_pcb.png) |

<br/>
<br/>


| Device | Resister Network | Unmodified (Default) Values | Unmodified (Default) I2C Address | Modified Values | Modified I2C Address |
| ---- | ---- | ---- | ---- | ---- | ---- |
| Deserializer IC | R9 and R12 | 56.2k and 44.2k | 0x6A | 68.1k and 32.4k | 0x4C |

<br>
<br>

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

${{ env_local.CAMERA_4K_REV_B_GRP_B }}

[Agilex™ 5 E-Series Modular Development Board GSRD User Guide (25.1)]: https://altera-fpga.github.io/rel-25.1/gsrd/ug-gsrd-agx5e-modular/
[Agilex™ 5 E-Series Modular Development Board GSRD User Guide (26.1)]: https://altera-fpga.github.io/rel-26.1/gsrd/ug-gsrd-agx5e-modular/


[Agilex™ 5 SoC FPGA]: https://www.altera.com/products/fpga/agilex/5
[Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (25.1)]: https://docs.altera.com/r/docs/814346/25.1/hard-processor-system-technical-reference-manual-agilextm-5-socs/download-document
[Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (26.1)]:https://docs.altera.com/r/docs/814346/26.1/hard-processor-system-technical-reference-manual-agilextm-5-socs/agilextm-5-hard-processor-system-technical-reference-manual-revision-history
[NiosV Processor for Altera® FPGA]: https://www.altera.com/design/guidance/nios-v-developer
[Agilex™ 5 FPGA E-Series 065B Modular Development Kit]: https://www.altera.com/products/devkit/po-3001/agilex-5-fpga-and-soc-e-series-modular-development-kit-es
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

${{ env_local.CAMERA_4K_END_REV_B_GRP_B }}
${{ env_local.CAMERA_4K_REV_C_GRP_B }}

[Agilex™ 5 E-Series Modular Development Board GSRD User Guide (26.1)]: https://altera-fpga.github.io/rel-26.1/embedded-designs/agilex-5/e-series/modular-065b/gsrd/ug-gsrd-agx5e-modular-065b/


[Agilex™ 5 SoC FPGA]: https://www.altera.com/products/fpga/agilex/5
[Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (25.1)]: https://docs.altera.com/r/docs/814346/25.1/hard-processor-system-technical-reference-manual-agilextm-5-socs/download-document
[Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (26.1)]:https://docs.altera.com/r/docs/814346/26.1/hard-processor-system-technical-reference-manual-agilextm-5-socs/agilextm-5-hard-processor-system-technical-reference-manual-revision-history
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

${{ env_local.CAMERA_4K_END_REV_C_GRP_B }}

# openSCAD Models

[openSCAD](https://openscad.org/) is a free, open-source software that has been used to create solid
3D CAD (Computer-Aided Design) models for camera mounts and brackets that can
be used with the Camera Solution Example Designs. The models can be exported to
an STL (Standard Triangle Language) file supported by many 3D printers.

<br/>

> **Disclaimer** <br/>
> Altera disclaims all express and implied warranties, including without
  limitation, the implied warranties of merchantability, fitness for a
  particular purpose, and non-infringement, as well as any warranty arising
  from course of performance, course of dealing, or usage in trade. You are
  responsible for safety of the overall system, including compliance with
  applicable safety-related requirements or standards.

<br/>

## Camera Tripod Mount for [Framos FSM:GO IMX678C Camera Modules](https://www.framos.com)

|<center markdown="1">openSCAD Camera Tripod Mount</center>|<center markdown="1">Tripod Camera Mount</center>|
|-|-|
| ![scad-camera-mount-678](../common/images/scad/scad_camera_mount_framos_imx678.png) | ![camera-mount-678](../common/images/scad/camera_mount_framos_imx678.png) |

[openSCAD File - Camera Tripod Mount Adapter for Framos FSM:GO IMX678C](https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1/camera_mount_framos_imx678.scad)

<br/>
<br/>

## Agilex™ 5 FPGA E-Series 065B Modular Development Kit Camera Mount for [Framos FSM:GO IMX678C Camera Modules](https://www.framos.com)

|<center markdown="1">openSCAD MDK Fixed Camera Mount</center>|<center markdown="1">MDK Fixed Camera Mount</center>|
|-|-|
| ![scad-fixed-camera-mount-678](../common/images/scad/scad_fixed_camera_mount_framos_imx678.png) | ![fixed-camera-mount-678](../common/images/scad/fixed_camera_mount_framos_imx678.png) |

[openSCAD File - Fixed Camera Mount Adapter for Agilex™ 5 FPGA E-Series 065B Modular Development Kit](https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1/gmsl_bracket_framos.scad)
<br/>

Specifically designed for the
Agilex™ 5 FPGA E-Series 065B Modular Development Kit to align with the MIPI
connectors to minimize bends in the Framos [150mm flex-cable](https://www.mouser.co.uk/ProductDetail/FRAMOS/FMA-FC-150-60-V1A?qs=GedFDFLaBXGCmWApKt5QIQ%3D%3D&_gl=1*d93qim*_ga*MTkyOTE4MjMxNy4xNzQxMTcwMzQy*_ga_15W4STQT4T*MTc0MTE3MDM0Mi4xLjEuMTc0MTE3MDQ5OS40NS4wLjA)s, it can be used
to hold either [Framos FSM:GO IMX678C Camera Modules](https://www.framos.com) or
[Framos FFA-GMSL-DES-V2A Deserializer](https://www.framos.com/en/products/ffa-gmsl-des-v2a-27240) boards (as shown in the picture).

<br/>
<br/>

## Multi-Camera Tripod Mount for [Framos FSM:GO IMX678C Camera Modules](https://www.framos.com)

|<center markdown="1">openSCAD Multi-Camera Tripod Mount</center>|<center markdown="1">Multi-Camera Tripod Mount</center>|
|-|-|
| ![scad-multi-camera-mount-678](../common/images/scad/scad_multi-camera_mount_framos_imx678.png) | ![multi-camera-mount-678](../common/images/scad/multi-camera_mount_framos_imx678.png) |

[openSCAD File - Multi-Camera Tripod Mount Adapter for Framos](https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1/stitch_camera_mount.scad)
<br/>

Can be used to hold [Framos FSM:GO IMX678C Camera Modules](https://www.framos.com) and
[Framos FFA-GMSL-SER-V2A Serializer](https://www.framos.com/en/products/ffa-gmsl-ser-v2a-27617) boards. Picture shows a pair of Camera
Modules connected to GMSL Serializers (using additional Framos
[150mm flex-cable](https://www.mouser.co.uk/ProductDetail/FRAMOS/FMA-FC-150-60-V1A?qs=GedFDFLaBXGCmWApKt5QIQ%3D%3D&_gl=1*d93qim*_ga*MTkyOTE4MjMxNy4xNzQxMTcwMzQy*_ga_15W4STQT4T*MTc0MTE3MDM0Mi4xLjEuMTc0MTE3MDQ5OS40NS4wLjA)s). The mount allows the pair of Cameras to be separated by
90° for Camera Solution Example Designs supporting multiple camera pipelines.

<br/>
<br/>

> **Notes** <br/>
> **-** Tripod mounting holes will require tapping to 1/4 inch/0.64mm. <br/>
> **-** All other mounting holes are designed to take M2 screws. Some holes may
        self tap using screws. Other holes may require further drilling to
        enlarge or to remove burrs from the printing process. M2 nuts may be
        required for common securing screws in some holes.

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


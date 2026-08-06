

# 4Kp60 Multi-Sensor Camera Stitch Solution System Example Design for Agilex™ 5 Devices - Acronyms and Terminology

<center markdown="1">

| **Term**  | **Description**                                           |
| :------:  | :-------------------------------------------------------- |
| AE        | Auto Exposure                                             |
| ANR       | Adaptive Noise Reduction                                  |
| API       | Application Programming Interface                         |
| AWB       | Auto White Balance                                        |
| AXI       | Advance eXtensible Interface bus protocol                 |
| BLC       | Black Level Correction                                    |
| BLS       | Black Level Statistics                                    |
| BPS       | Bits Per-Symbol                                           |
| BSP       | Board Support Package                                     |
| CCM       | Color Correction Matrix                                   |
| CFA       | Color Filter Array                                        |
| CPU       | Central processing unit                                   |
| CSC       | Color Space Converter                                     |
| CSR       | Control/Status Registers                                  |
| CDC       | Cross Clock Domain                                        |
| DDR       | Double Data-Rate                                          |
| DHCP      | Dynamic Host Configuration Protocol                       |
| DMA       | Direct Memory Access                                      |
| DMS       | Demosaic                                                  |
| DP        | DisplayPort                                               |
| DPC       | Defective Pixel Correction                                |
| DRAM      | Dynamic RAM                                               |
| DSP       | Digital Signal Processing                                 |
| DTB       | Device Tree Blob                                          |
| DTS       | Device Tree Source                                        |
| EDID      | Extended Display Identification Data                      |
| EMIF      | External Memory Interface                                 |
| EOTF      | Electrical-Optical Transfer Function                      |
| FBD       | Function Block Diagram                                    |
| FMC       | FPGA Mezzanine Card                                       |
| FPGA      | Field Programmable Gate Array                             |
| FPS       | Frames Per Second                                         |
| FW        | Firmware                                                  |
| GMSL      | Gigabit Multimedia Serial Link                            |
| GPIO      | General Purpose Input-Output                              |
| GUI       | Graphical User Interface                                  |
| HDR       | High-Dynamic Range                                        |
| HDR HLG   | HDR Hybrid Log Gamma                                      |
| HDR PQ    | HDR Perceptual Quantization                               |
| HPS       | Hard Processor System                                     |
| HPS2FPGA  | Hard Processor System to FPGA, usually referring to a hardened memory-mapped bridge |
| HS        | Histogram Statistics                                      |
| HW        | Hardware                                                  |
| IP        | Intellectual Property (Hardware)                          |
| IO, I/O   | Input/Output                                              |
| IRQ       | Interrupt Request                                         |
| ISP       | Image Signal Processing                                   |
| ISR       | Interrupt Service Routine                                 |
| JTAG      | Joint Test Action Group (industry standard for testing)   |
| KAS       | Setup tool for Yocto projects                             |
| LAN       | Local Area Network                                        |
| LED       | Light Emitting Diode                                      |
| LSB       | Least Significant Bit                                     |
| LUMA      | Luminance                                                 |
| LUT       | Look Up Table                                             |
| MDT       | Modular Development Toolkit                               |
| MIPI      | Mobile Industry Processor Interface                       |
| MP        | Mega Pixel                                                |
| MSB       | Most Significant Bit                                      |
| MSGDMA    | Modular Scatter-Gather DMA                                |
| NGPD      | Next Generation Platform Designer                         |
| OBR       | Optical Black Region                                      |
| OETF      | Opto-Electrical Transfer Function                         |
| OOTF      | Opto-Optical Transfer Function                            |
| PD        | Usually used to refer to "Platform Designer"              |
| PIP       | Pixels In Parallel                                        |
| PLL       | Phase Locked Loop                                         |
| PPI       | PHY Protocol Interface                                    |
| QPDS      | Quartus® Prime Design Software                            |
| RAM       | Random Access Memory                                      |
| RGB       | Red, Green, and Blue                                      |
| RMS       | Remosaic                                                  |
| ROI       | Region Of Interest                                        |
| ROM       | Read Only Memory                                          |
| RTL       | Register Transfer Level                                   |
| RX        | Receive                                                   |
| SD        | Secure Digital                                            |
| SDRAM     | Synchronous DRAM                                          |
| SFC       | Sequential Function Charts                                |
| SoC       | System-on-Chip                                            |
| SSH       | Secure Shell                                              |
| SW, S/W   | Software                                                  |
| SW1, SW2, ... | Switch 1, Switch 2, …                                 |
| TMO       | Tone Mapping Operator                                     |
| TPG       | Test Pattern Generator                                    |
| TX        | Transmit                                                  |
| UART      | Universal Asynchronous Receiver-Transmitter               |
| UHD       | Ultra-high definition                                     |
| UI        | User Interface                                            |
| UIO       | User Space I/O System                                     |
| USM       | Unsharp Mask                                              |
| VC        | Vignette Correction                                       |
| VVP       | Video and Vision Processing                               |

**Acronyms and Terminology**

</center>

<br>
<br>

***

<center markdown="1">

[BACK](../camera_4k_stitch/camera_4k_stitch.md#documentation)
</center>

***
<br>





[User flow 1]: ../camera_4k_stitch/camera_4k_stitch.md#pre-requisites
[User flow 2]: ../camera_4k_stitch/flow2-sof-mdt.md
[User flow 3]: ../camera_4k_stitch/flow3-rbf-mdt.md
[User flow 4]: ../camera_4k_stitch/flow4.md



[Release Repo]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1
[Release MDT]: https://github.com/altera-fpga/modular-design-toolkit/tree/rel/26.1
[meta-altera-fpga]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/sw/meta-altera-fpga
[meta-altera-fpga-ocs]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/sw/meta-altera-fpga-ocs
[meta-vvp-isp-demo]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/sw/meta-vvp-isp-demo
[agilex-ed-camera/sw]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/sw/



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
[RBF Modular Design Toolkit (MDT) Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#create-the-design-using-the-modular-design-toolkit-mdt
[Quartus® GUI Create Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#using-the-pregenerated-mdt-quartus-project
[Quartus® GUI Build Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/STITCH_CAMERA.md#building-the-pregenerated-mdt-quartus-project


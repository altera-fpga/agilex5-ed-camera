# GMSL Modification

The [Framos GMSL3] solution does not use the I2C Slave Mode pins (`SLAMODE`)
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

The [Framos FFA-GMSL-SER-V2A Serializer] contains 2 addressable I2C devices
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

The [Framos FFA-GMSL-DES-V2A Deserializer] contains a single addressable I2C
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

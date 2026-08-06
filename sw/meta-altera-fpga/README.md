# Altera® FPGA Yocto Layers

Altera® Yocto layers to support Altera® SoC FPGA devices.


## Board Support
| Identifier | Layer | Description |
| --- | --- | --- |
| `agilex3` | meta-altera-devkit | Agilex 3 SoCFPGA |
| `agilex5_dk_a5e065bb32aes` | meta-altera-devkit | Agilex 5 FPGA E-Series Premium Development Kit |
| `agilex5_mk_a5e065bb32aes1` | meta-altera-devkit | Agilex 5 FPGA E-Series Modular Development Kit |
| `agilex7_dk_dev_agf027f1es` | meta-altera-devkit | Agilex 7 FPGA F-Series Development Kit |
| `agilex7_dk_dev_agm039fes` | meta-altera-devkit | Agilex 7 FPGA M-Series Development Kit |
| `agilex7_dk_si_agf014ea` | meta-altera-devkit | Agilex 7 FPGA F-Series Transceiver-SoC Development Kit |
| `agilex7_dk_si_agi027fa` | meta-altera-devkit | Agilex 7 FPGA I-Series Transceiver-SoC Development Kit |
| `agilex5_sulfur` | meta-altera-vendor | Macnica Sulfur - Agilex™ 5 FPGAs E-Series SOM Development Kit |
| `agilex5_axe5_eagle` | meta-altera-vendor | Arrow AXE5-Eagle Development Kit |

## Dependencies

### Kas
[kas](https://github.com/siemens/kas) is recommended to configure a build.

`kas` is a Python based tool and can be installed using `pip` directly or in a [Docker](https://www.docker.com/) container which avoids the need to also install required [Yocto build dependencies](https://docs.yoctoproject.org/2.2.2/ref-manual/ref-manual.html#required-packages-for-the-host-development-system).

We recommend using `kas` in a Docker container. If you don't currently have Docker installed follow the [official documentation](https://docs.docker.com/engine/install/) before running the below commands to pull down the `kas` helper script:

```bash
sudo wget https://github.com/siemens/kas/raw/refs/heads/master/kas-container -O /usr/bin/kas
sudo chmod +x /usr/bin/kas
```

[Read the official documentation](https://kas.readthedocs.io/en/latest/userguide/getting-started.html) for more information.


## Building A Minimal Image
To build a minimal SD card image for a given target run the following commands:

```bash
KAS_MACHINE=${MACHINE} kas build kas-example.yml
```

For example:

```bash
KAS_MACHINE=agilex5_mk_a5e065bb32aes1 kas build kas-example.yml
```

See above for a list of supported `MACHINE` identifiers.

Once the build has finished successfully you will have a `.wic.gz` image available in `build/tmp/deploy/images/${MACHINE}` which can be [written to an SD card](docs/flash_sd_card.md).

## Building A Custom Image
To build your own custom image it is recommended to create a `kas` configuration file for your project. You can use [kas-example.yml](../kas-example.yml) provided in this repository as a starting point.
For more advanced projects you may also want to [create a dedicated Yocto layer](https://docs.yoctoproject.org/dev/dev-manual/layers.html) for your project.

### Setting Up Your Project

The `kas` configuration YAML file is where you will add all of your project specfic configuration to customize your image. The most common configuration options are outlined in the following sections.

See [kas documentation](https://kas.readthedocs.io/en/latest/userguide/project-configuration.html) for more information.

### Updating Project Name
You can rename `kas-example.yml` to reflect the name of your project. For example `kas-${my_project_name}.yml`

Open your projects kas config file in your preferred text editor and update the `example_project` identifier in the `local_conf_header` section to reflect your project name.

## Configuration
The `altera-fpga` layers are designed to be modular and configurable enabling you to build a bespoke image for your project easily and without additional bloat. The most common configuration options are outlined in the following sections.

### Layers
The `altera-bsp` and `altera-core` layers should always be present in your configuration. The `altera-bsp` layer provides the foundational components to build an image for an Altera® SoC FPGA board.

```
repos:
  meta-altera-fpga:
    path: .
    layers:
      meta-altera-bsp:
      meta-altera-core:
```

If you intend to build an image for an Altera® Soc FPGA Devkit or third-party vendor board you will also need to add the `altera-devkit` or `altera-vendor` layer to your configuration.

```
repos:
  meta-altera-fpga:
    path: .
    layers:
      meta-altera-devkit:
      meta-altera-vendor:
```

### Required Configuration
The `base` config provided by [kas/base.yml](kas/base.yml) is required and should always be present in you project config `includes` section. This configuration file adds essential BSP related packages and other build environment parameters.

```
  includes:
    - repo: meta-altera-fpga
      file: kas/base.yml
```

You will also need to choose your base distro. [Poky](https://git.yoctoproject.org/poky) is a common reference distro and can be enabled by including [kas/poky.yml](kas/poky.yml).

```
  includes:
    - repo: meta-altera-fpga
      file: kas/poky.yml
```

### Target Machine
To configure your target machine add the following to your projects kas config file substituting `${MACHINE}` with one of the supported machine identifiers listed above:

```
machine: ${MACHINE}
```

For example to build for Agilex 5 E-Series Modular Devkit:

```
machine: agilex5_mk_a5e065bb32aes1
```

> You can omit `machine` from your config file and simply use `KAS_MACHINE=${MACHINE}` when running `kas` commands if preferred

### SoC FPGA Features
Optional build features can be enabled using the `SOCFPGA_FEATURES` variable in your build config. The list of supported features can be found below.

| Feature | Description |
| --- | --- |
| bitstream | If enabled the `fpga-bitstream` recipe will be built and is expected to populate a core bitstream in the `boot` partition to be programmed on boot. The FPGA core bitstream `.rbf` file should be provided in your own layer using a `fpga-bitstream.bbappend` file or in your build config by setting `FPGA_BST_SRC_URI` and `FPGA_BST_SHA256SUM` as applicable. |
| jffs2 | Enable JFFS2 (Journaling Flash File System version 2) file system support commonly used for NAND flash devices. |
| gpio_sys | Enable sysfs GPIO support in the Linux kernel. |
| usb_adapter | Enable Linux kernel support for common USB peripherals including network adapters and mass storage devices. |
| uio | Enable Userspace I/O support in the Linux kernel. |
| rt | Enable real-time Linux kernel support. |

An example of enabling `bitstream` and `rt` features in your build config:
```
SOCFPGA_FEATURES:append = " bitstream rt"
```

### Optional Configuration
Some optional `kas` configurations are provided which can be included in your project config to add additional functionality to your image.

The current optional configs are detailed below.

#### Dev
The `Dev` configuration provides some essential tools for a developer focussed image including SSH access out-of-the-box and tools such as `git` and `devmem2` for probing memory addresses.

```
  includes:
    - repo: meta-altera-fpga
      file: kas/dev.yml
```

#### Docker
The `Docker` configuration adds full [Docker](https://www.docker.com/) support to an image including the [containerd](https://containerd.io/) runtime and [Docker Compose](https://docs.docker.com/compose/) plugin.

```
  includes:
    - repo: meta-altera-fpga
      file: kas/docker.yml
```

#### Intel® RealSense™
The `RealSense™` configuration adds support for [Intel® RealSense™](https://www.intelrealsense.com/) cameras to an image including the [Intel® RealSense™ SDK](https://github.com/IntelRealSense/librealsense) and tools.

```
  includes:
    - repo: meta-altera-fpga
      file: kas/realsense.yml
```

### Boot Arguments
If you wish to add additional boot arguments you can append the `IMAGE_BOOT_ARGS` variable. For example to enable the `generic-uio` kernel module on boot:

```
IMAGE_BOOT_ARGS:append = " uio_pdrv_genirq.of_id=generic-uio"
```

### Expand Root Partition
There is a recipe in the `core` layer called `expand-root-partition` which installs a script and `systemd` service in the image which will expand the root partition to fill any remaining disk space on first boot.

To include this functionality add the following line to your build config:

```
IMAGE_INSTALL:append = " expand-root-partition"
```

## Supported Image Types

 - `mmc` (SD Card)


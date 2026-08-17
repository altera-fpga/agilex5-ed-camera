

# 4Kp60 Multi-Sensor HDR Camera Solution System Example Design for Agilex™ 5 Devices - A Guide to Image Sensor Calibration

The 4Kp60 Multi-Sensor HDR Camera Solution System Example Design for Agilex™ 5 Devices features built-in calibration tools that,
when used with the additional offline calibration tools, can help calibrate the
Image Signal Processing (ISP) pipeline for any given Image Sensor.

Note that Image Sensor ingest can be via a live Image Sensor as connected to
the Development Kit, or raw bayer images uploaded and played using the Input
Frame Reader function.


<br/>

> **Important Notes** <br/>
> The following text can contain embedded links that aim to assist with
> navigation, pointers to useful references and resources, or to add clarity. <br/>
> There may be slight variations between screenshots and diagrams used here
> and the actual Camera Solution System Example Design.

<br/>

## Overview
This guide describes the end-to-end image sensor calibration flow using the
4Kp60 Multi-Sensor HDR Camera Solution System Example Design for Agilex™ 5 Devices. Calibration ensures correct black level
subtraction, adaptive noise reduction (ANR), and color reproduction across a
range of illuminants. The procedure is performed using the Camera Solution
System Example Designs web GUI and is supplemented by offline
[Jupyter Notebook] processing for color correction matrix (CCM) and white
balance correction (WBC) coefficients.

<br/>

### Calibration Phases

| Phase | Purpose | Environment |
| ---- | ---- | ---- |
| Black Level | Measure CFA pedestal and scaler values at each analog gain step | Dark box or dark room |
| ANR | Characterize dark noise and combined gain for adaptive noise reduction | Controlled static scene and dark cover |
| Colorimetry | Capture raw images at multiple color temperatures; compute WBC and CCM offline | Light box or gray wall with color chart |

> **Important** <br/>
> The phases must be performed in order as each phase builds on parameters
  established in the previous phase.

<br/>

## Prerequisites

### Hardware
* Dark box or light-tight dark room.
* Dark anti-static fabric for covering the sensor.
* Macbeth ColorChecker or equivalent 24-patch color chart.
* Neutral gray card (or use gray patches on the color chart).
* Multiple adjustable light sources with CRI 95+ covering 2700 K – 9990 K.
* Spectrometer (to verify color temperature).
* Lux meter (to verify uniform illumination at 1000 lux).
* Narrow-angle lens with minimal distortion and lens shading (recommended for
  colorimetry).
* Monitor connected via DisplayPort for preview.

<br/>

> **Note** <br/>
> The guide assumes a live image sensor is being used such as the Sony IMX678
  sensor (as used in the examples below). However the Input Frame Reader
  function of the Camera Solution System Example Design can be equally used
  as long as the actual raw bayer images/sequence of images being used
  conform to the requirements being asked for by the calibration process. For
  example, dark scene capture images taken with different analog gain
  settings. Some calibration steps will only require single input images,
  whereas others (like ANR calibration) will require a sequence of input
  images.

<br/>

### Software
* [GIMP] (or equivalent) for raw image inspection and grayscale conversion.
* [Jupyter Notebook] with [python] 3.11+ and package dependencies for
  [`color_calibration.ipynb`](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/calibration/color_calibration.ipynb).

<br/>

### Local Calibration Assets

| Asset | Location |
| ---- | ---- |
| Color calibration notebook | [`color_calibration.ipynb`](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/calibration/color_calibration.ipynb) |
| IMX678 example captures | [`imx678_calibration/wb_ccm/`](https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/calibration/imx678_calibration/wb_ccm) and [`imx678_calibration/anr/`](https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/calibration/imx678_calibration/anr) |

<br/>

## System Setup

* Place the system in a dark box or dark room. Follow all the Camera Solution
  System Example Design instructions to setup and run the demo. Do not exclude
  any steps as this guide requires the terminal, Web browser, and output
  monitor connections. Remember to take note of the Modular Development Kit's IP address.

<br/>

> **Note** <br/>
> The IP address can also be found using the terminal by logging in as `root`
  (no password required) and querying the Ethernet controller: <br/>
> ```bash
>  root
>  ifconfig
>  ```
> `eth0` provides the IPv4 or IPv6 address to connect to.

<br/>

* Ensure you are logged in as `root` and stop any running instance of the demo application:

    ```bash
    killall VvpIspDemo
    ```

* Remove any saved settings to ensure a clean default state:

    ```bash
    rm settings.json
    rm awb_profile.json
    ```

* Launch the application in power-user mode:

    ```bash
    ./VvpIspDemo -pu
    ```

* Connect your web browser to the boards IP address so you can interact with
  the Camera Solution System Example Design using the GUI.
  * To connect using IPv6 you would use the format `http://[IP6-address]`
    (note the square brackets).
  * To connect using IPv4 you would use the format `http://<IP4-address>`.

* Confirm that live video is displayed on the output monitor and that the
  `Histogram` Tile in the `Input Config` Tab is constantly updating with
  movement in front of the sensor.

<br/>
<center markdown="1">

![gui-calibration-tab.](../camera_4k_resources/images/Calibration/Input_Config_Tab.png)

**Input Config Tab**
</center>

<br/>
The `Input Config` Tab will be used to set `Shutter Speed` and `Analog Gain`
(in the `Camera` Tile), and to disable auto exposure (AE) and auto white
balance (AWB) functions (within their respective Tiles) during the calibration
process.

<br/>

The `Sensor Calibration` Tab is is the primary workspace for calibrating the
Black Level, ANR, and color temperature (colorimetry).

<br/>
<center markdown="1">

![sensor-calibration-tab-full.](../camera_4k_resources/images/Calibration/Sensor_Calibration_Tab_Full.png)

**Sensor Calibration Tab**
</center>

<br/>

## Black Level Calibration

Black level calibration measures the sensor offset (pedestal) and scaling
values for each CFA channel at every analog gain step. Accurate black level is
required before ANR and colorimetry can produce valid results.

<br/>

> **Important** <br/>
> Do not leave the sensor covered for extended periods as overheating can
  affect measurements. Keep the cover on only long enough to capture each
  reading.

<br/>

### Prepare the Dark Environment

* Cover the sensor with dark anti-static fabric.
* Switch off all lights in the dark box or dark room.
* Navigate to the `Input Config` Tab:
  * Disable AE (in the `Auto Exposure` Tile).
  * Disable AWB (in the `Auto White Balance` Tile).
  * In the `Camera` Tile:
    * Set `Shutter Speed` to maximum.
    * Set `Analog Gain` to **1**.
    * Set `Digital Gain` to **1** (if applicable).

<br/>

### Read Black Level at Gain = 1

* Navigate to the `Sensor Calibration` Tab:
  * In the `Black Level Calibration` Tile, confirm that `Gain` is set to **1**.
  * Click `Read BLS`. The `CFA Pedestal` and `CFA Scalar` fields will populate
    automatically and the values will be recorded internally.

<br/>
<center markdown="1">

![blc-parameters.](../camera_4k_resources/images/Calibration/BLC_Parameters.png)

**BLC Parameters after clicking Read BLS**
</center>

<br/>

> **Note** <br/>
> All four CFA channels (cfa_00, cfa_01, cfa_10, cfa_11) should show
  consistent `CFA Pedestal` and `CFA Scalar` values.

<br/>

### Read Black Level at Gain = N

Repeat the Read Black Level process [above](#read-black-level-at-gain-1) for
analog gains of N, where N should be integer powers of two (i.e. **2**, **4**,
**8**, **16**, **32**, and so on), up to the maximum allowed gain for your
sensor. Note that the final gain value may be arbitrary and may not follow the
power-of-two sequence:

* Navigate to the `Input Config` Tab:
  * In the `Camera` Tile, set the `Analog Gain` to N using the slider or
  manually typing it into the free-text field.

<br/>
<center markdown="1">

![input-config-tab-ag.](../camera_4k_resources/images/Calibration/Input_Config_Tab_AG.png)

**Set Analog Gain to N in the Camera Tile using the Input Config Tab**
</center>

<br/>

* Navigate to the `Sensor Calibration` Tab:
  * In the `Black Level Calibration` Tile, select the matching gain N using the
    `Select Gain` dropdown. **Do not use the `Gain` free-text field**:

<br/>
<center markdown="1">

![sensor-calib-tab.](../camera_4k_resources/images/Calibration/Sensor_Calibration_Tab.png)

**Verify the correct gain is selected in the Select Gain dropdown.**
</center>

<br/>

> **Note** <br/>
> If the dropdown gain does not match the `Analog Gain` slider, go back to
  the `Input Config` Tab and click `Resync` in the `Camera` Tile. Go to the
  `Sensor Calibration` Tab and re-select the matching gain from the
  `Select Gain` dropdown.

* Re-cover the sensor if needed, then click `Read BLS`. The `CFA Pedestal` and
  `CFA Scalar` fields will populate automatically and the values will be
  recorded internally.

<br/>

### Complete Black Level Calibration

After all gain steps are captured, remove the sensor cover and proceed to the
ANR calibration phase.

<br/>

## ANR Calibration

Adaptive Noise Reduction (ANR) calibration characterizes the sensors dark
noise and combined gain at each analog gain setting. The quality of ANR
calibration directly affects image detail and noise appearance. Dark frames
are captured with the sensor covered whereas scene frames are captured
uncovered.

<br/>

> **Critical - Scene Requirements** <br/>
>   ANR calibration is extremely sensitive to scene conditions. Deviating from
    the requirements below — even slightly — can produce drastically incorrect
    noise models and severely degrade picture quality. Specifically: <br/>
>   * Use absolutely non-flickering light sources. <br/>
>   * Ensure zero movement in the scene. Any movement - even by a single pixel,
      is sufficient to invalidate the calibration. <br/>
>   * Try not not move the camera between dark-frame and scene-frame captures
      at the same gain. <br/>

<br/>

### Prepare the Static Scene

* Set up a controlled static scene with uniform, non-flickering illumination.
* Focus the lens (if applicable).
* In the `Camera` Tile under the `Input Config` Tab:
  * Set `Analog Gain` to **1**.
  * Adjust the exposure using the `Shutter Speed` so that the `Histogram` Tile
    spans from black level to approximately **90%** of the dynamic range. The
    histogram does not need to be uniform, but it must not contain gaps or
    dips.
    * If maximum `Shutter Speed` is insufficient, reduce the `Frame Rate` to
    achieve the required exposure.

<br/>

### Capture Dark and Scene Frames at Gain = 1

* Switch off the lights and cover the sensor with dark anti-static fabric.
* Navigate to the `Sensor Calibration` Tab:
  * In the `Black Level Calibration` Tile select `Gain` **1** from the
  `Select Gain` dropdown.
  * Click `Read Dark Frame`:

<br/>
<center markdown="1">

![anr-parameters.](../camera_4k_resources/images/Calibration/ANR_Parameters.png)

**ANR Capture Controls**
</center>

<br/>

* Switch on the lights, uncover the sensor, and only if absolutely necessary,
  reposition the camera.
* Confirm the scene is completely static with no movement or flickering.
* Click `Read Scene Frames`.
* Click `Calculate Params`. The `Combined Gain` and `Dark Noise` fields will
  update and the values will be recorded internally.

<br/>

### Capture Dark and Scene Frames at Gain = N

Repeat the Capture Dark and Scene Frames process
[above](#capture-dark-and-scene-frames-at-gain-1) for analog gains of N, where
N are the same values used during the
[Read Black level](#read-black-level-at-gain-1) process (i.e. integer powers
of two: **2**, **4**, **8**, **16**, **32**, and so on). Use the same static
scene for all gain steps.

<br/>

> **Example Reference Data** <br/>
> Example ANR capture files for the Sony IMX678 are provided in
  [`imx678_calibration/anr/`](https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/calibration/imx678_calibration/anr)
  (e.g., black_1.tif, scene1_1.tif at each Gain step).

<br/>

## Colorimetry Calibration

Colorimetry calibration captures raw sensor images at multiple color
temperatures. White balance statistics are collected within the GUI. Color
Correction Matrix (CCM) and refined White Balance Correction (WBC) coefficients
are computed offline using a [Jupyter Notebook].

<br/>

### Scene and Lighting Setup

* Mount a color chart on a neutral gray wall or position it in a light box.
* Place a gray card directly below the color chart or use the gray row of
  patches on the chart itself.
* Attach a narrow-angle lens with minimal distortion and lens shading.
* Center the camera on the color chart so the chart appears rectangular and
  undistorted in the monitor output. If fisheye distortion is unavoidable, move
  the camera farther away so the chart occupies a smaller, undistorted central
  region of the frame.
* Prepare light sources covering these color temperatures: **2700 K**,
  **3200 K**, **4000 K**, **5000 K**, **6000 K**, **6500 K**, **8000 K**,
  and **9990 K**. Use sources with **CRI 95** or higher.
* Illuminate the chart from left and right of the camera using two light
  sources. Avoid casting shadows on the chart.
* Ensure no other light sources illuminate the chart — including indicator LEDs
  on the Modular Development Kit itself, control panels, fire alarms, etc. Even
  a small stray light source can invalidate calibration.

<br/>

> **Important - Do not move the camera or scene** <br/>
> When changing color temperatures, the camera and chart must remain fixed.
  The calibration process tolerates only a few pixels of scene shift between
  captures.

<br/>

### Per-Color-Temperature Capture Procedure

Repeat the following steps for each of the **2700 K**, **3200 K**, **4000 K**,
**5000 K**, **6000 K**, **6500 K**, **8000 K**, and **9990 K** color
temperatures:

* Set the light source to the target color temperature.
* Verify the color temperature on the chart with a spectrometer.
  * LED sources may drift for the first few minutes after power-on, so allow
    some time for stabilization.
* Measure illumination at the four corners and center of the chart with a lux
  meter. Adjust light position and intensity to achieve **1000 lux** at all
  five measurement points. **Do not reposition the camera while adjusting
  lights**.
* Navigate to the `Input Config` tab and in the `Camera` Tile: 
  * Set the `Analog Gain` and the `Digital Gain` (if applicable) to **1**.
  * Adjust the exposure using the `Shutter Speed` so that no pixels clip in the
    `Histogram`. Slight underexposure is preferable to overexposure.
* Navigate to the `Sensor Calibration` Tab and select a Region Of Interest
  (ROI) in the `Profile Global Region Select` Tile that only covers a portion
  of the gray card:

<br/>
<center markdown="1">

![roi-select.](../camera_4k_resources/images/Calibration/ROI_Select.png)

**Select an ROI covering the gray card in Profile Global Region Select**

</center>

<br/>

* Add the color temperature entry in the `White Level Calibration` Tile and
click `Read Color Temp Data` to capture the white balance statistics:

<br/>
<center markdown="1">

![sensor-calib-wl-tab.](../camera_4k_resources/images/Calibration/Sensor_Calibration_WL_Tab.png)

**Add the color temperature**
</center>

<br/>

* Navigate to the `Output Config` Tab and in the `Frame Writer` Tile: 
  * Click `Raw Camera Snapshot`.
  * Click `Download Image`.
  * Rename the file with the color temperature for future reference (e.g., 2700K.tif).
* Open the image in [GIMP] and verify the exposure:
  * The brightest pixels in the white patch in the color chart should ideally
    fall between **50,000** and **58,000** in 16-bit range.
  * Re-adjust the exposure and recapture if necessary.

<br/>

### Export Calibration Profile

* After all color temperatures are captured, return to the `Sensor Calibration`
  Tab.
* Click `Export` in the `Profile Overview` Tab to download the
  calibration `.json` file.
* Keep this file as the offline CCM/WBC coefficients obtained in the next
  section, will need to be merged into it.

<br/>

> **Example Reference Data** <br/>
> Example color captures for the Sony IMX678 are in
  [`imx678_calibration/wb_ccm/`](https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/calibration/imx678_calibration/wb_ccm)
  (`2700K.tif` through `9990K.tif`).

<br/>

## Offline CCM / WBC Processing

The [`color_calibration.ipynb`](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/calibration/color_calibration.ipynb)
notebook computes White Balance Correction (WBC) and Color Correction Matrix
(CCM) coefficients from the downloaded raw TIFF captures. These coefficients
replace the corresponding fields in the exported calibration `.json` file.

<br/>

### Prepare Raw Images

* Open all captured TIFF images in [GIMP].
* Convert each image to **grayscale** mode and overwrite the original files.
  The [Jupyter Notebook] expects single-channel CFA raw data.
* Determine the corner coordinates of the color chart in each image. Record as
  `vertical_start`, `vertical_end`, `horizontal_start`, and `horizontal_end`.
* Copy the images to the [Jupyter Notebook] image folder, following the same
  directory layout as existing sensors -
  `../images/<sensor>_calibration/wb_ccm/`

<br/>

> **Example Reference Data** <br/>
> Example TIFF images for the Sony IMX678 are in
[`imx678_calibration/wb_ccm/.`](https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1/calibration/imx678_calibration/wb_ccm)

<br/>

### Configure the [Jupyter Notebook]

Open [`color_calibration.ipynb`](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/calibration/color_calibration.ipynb)
and verify the sensor-specific parameters:

| Parameter | Description | IMX678 Example |
| ---- | ---- | ---- |
| `sensor` | Sensor identifier string | `'imx678'` |
| `bits_capture` | Sensor ADC bit depth | `12` |
| `bits_file` | TIFF storage bit depth | `16` |
| `black_level_pedestals` | [Calculated CFA black level](#black-level-calibration) | `[[200, 200, 200, 200]]`|
| `cfa_pattern` | Bayer pattern | `'RGGB'` |
| `cbox` | Color chart crop coordinates | `(794, 1195, 1616, 2223)` |
| `rotate180` | Rotate captured image 180° if needed | `True` |

<br/>

> **Important - CFA Alignment** <br/>
> When specifying `cbox` ensure start coordinates are on odd pixel indices
  and that they end on even pixel indices to maintain correct CFA **RGGB**
  alignment after rotation.

<br/>

### Setup the Python Environment and Launch the [Jupyter Notebook]

* Install required packages:

    ```bash
      python3 -m pip install --user colour colour-demosaicing ipympl \
      opencv-contrib-python scikit-image lazy_loader
    ```

* Launch the notebook:

    ```bash
      jupyter notebook color_calibration.ipynb
    ```

<br/>

### Run and Extract Coefficients

* Run all cells in the [Jupyter Notebook].
* For each color temperature, the [Jupyter Notebook] prints:
  * `rgb_scalars` — white balance correction (WBC) coefficients.
  * `ccm_coeffs` — 3×4 color correction matrix (CCM) coefficients.
  * Delta-E error metrics for quality assessment.
* Open the [exported calibration `.json`](#export-calibration-profile) file.
* Manually copy the printed WBC and CCM values into the `.json` file, replacing
  the existing fields. **Do not modify any other fields in the file.**

<br/>

### Apply and Validate

* In the Camera Solution GUI, navigate to the `Sensor Calibration` Tab.
* In the `Profile Overview` Tile, click `Import camera profile` and load the
  updated calibration `.json` file.
* Navigate to the `Input Config` Tab and enable the AWB in the `Auto White
  Balance` Tile.
* Verify the following across all calibrated color temperatures:
  * Gray patches on the color chart appear neutral (i.e. no color cast).
  * Color patches match expected reference colors.
  * AWB locks to the correct color temperature for each illuminant.
* Inspect the live video monitor output under typical operating conditions to
  confirm noise reduction and overall image quality.

<br/>

**Validation checklist**

* Black level: no visible color cast in a lens-capped or dark scene.
* ANR: fine detail preserved without excessive smoothing or residual noise.
* Color: average delta-E from the notebook should be low; visually inspect
  worst-case patches.

<br/>

## References

**Calibration Flow Summary**

| Step | Tab/Tool | Tab Tile | Key Action |
| ---- | ---- | ---- | ---- |
| System setup | Terminal + GUI | - | `./VvpIspDemo -pu` |
| Black level | Sensor Calibration | Black Level Calibration | Read BLS at each gain
| ANR dark frame | Sensor Calibration | Black Level Calibration | Read Dark Frame |
| ANR scene frames | Sensor Calibration | Black Level Calibration | Read Scene Frames → Calculate Params |
| Color WB stats | Sensor Calibration | White Level Calibration | Read Color Temp Data |
| Raw capture | Output Config | Frame Writer | Raw Camera Snapshot → Download Image |
| CCM / WBC | [`color_calibration.ipynb`](https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/calibration/color_calibration.ipynb) | - | Run notebook, copy coefficients to `.json` |
| Load profile | Input Config | Sensor Profile | Import camera profile |

<br/>

**Directory Layout**

    Calibration/
    ├── Images/                         ← GUI screenshots (01–08.PNG)
    ├── imx678_calibration/
    │   ├── anr/                        ← example ANR captures
    │   └── wb_ccm/                     ← example color captures
    ├── color_calibration.ipynb         ← offline CCM/WBC notebook

<br>
<br>

***

<center markdown="1">

[BACK](../camera_4k/camera_4k.md#documentation)
</center>

***
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
[Region Of Interest IP]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/docs/camera/common/non-qpds-ip/Region_of_interest_basic_guide.pdf
[Remoasaic IP]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/docs/camera/common/non-qpds-ip/Remosaic_basic_guide.pdf
[Throttle IP]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/docs/camera/common/non-qpds-ip/Throttle_basic_guide.pdf
[Alpha Channel IP]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/docs/camera/common/non-qpds-ip/Alpha_Channel_basic_guide.pdf






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



[Release Tag]: https://github.com/altera-fpga/agilex5-ed-camera/releases/tag/rel-26.1-isp_hdr-MDK_RevC_GrpB
[https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1
[hps-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_hdr-MDK_RevC_GrpB/hps-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz
[fpga-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_hdr-MDK_RevC_GrpB/fpga-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz
[fsbl_agilex5_modkit_vvpisp_time_limited.sof]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_hdr-MDK_RevC_GrpB/fsbl_agilex5_modkit_vvpisp_time_limited.sof
[top.core.jic]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_hdr-MDK_RevC_GrpB/top.core.jic
[top.core.rbf]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_hdr-MDK_RevC_GrpB/top.core.rbf



[AGX_5E_Modular_Devkit_ISP_FF_RD.xml]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/AGX_5E_Modular_Devkit_ISP_FF_RD.xml
[AGX_5E_Modular_Devkit_ISP_RD.xml]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/AGX_5E_Modular_Devkit_ISP_RD.xml
[Create microSD card image (.wic.gz) using YOCTO/KAS]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/sw/README.md
[SOF Modular Design Toolkit (MDT) Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/HDR_CAMERA.md#create-the-design-using-the-modular-design-toolkit-mdt
[RBF Modular Design Toolkit (MDT) Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/HDR_CAMERA.md#create-the-design-using-the-modular-design-toolkit-mdt
[Quartus® GUI Create Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/HDR_CAMERA.md#using-the-pregenerated-mdt-quartus-project
[Quartus® GUI Build Flow]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/AGX_5E_Altera_Modular_Dk_ISP_designs/HDR_CAMERA.md#building-the-pregenerated-mdt-quartus-project





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
[Region Of Interest IP]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/docs/camera/common/non-qpds-ip/Region_of_interest_basic_guide.pdf
[Remoasaic IP]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/docs/camera/common/non-qpds-ip/Remosaic_basic_guide.pdf
[Throttle IP]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/docs/camera/common/non-qpds-ip/Throttle_basic_guide.pdf
[Alpha Channel IP]: https://github.com/altera-fpga/agilex5-ed-camera/blob/rel/26.1/docs/camera/common/non-qpds-ip/Alpha_Channel_basic_guide.pdf


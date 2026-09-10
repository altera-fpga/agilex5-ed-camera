


# 4Kp60 Multi-Sensor Camera Stitch Solution System Example Design for Agilex™ 5 Devices - Web Graphical User Interface Functional Description

> **Important Notes** <br/>
> **-** The following text can contain embedded links that aim to assist with
>       navigation, pointers to useful references and resources, or to add
        clarity. <br/>
> **-** There may be slight variations between screenshots and diagrams used
        here and the actual Camera Solution System Example Design.

## Running the Graphical User Interface (GUI)

The application software of the design has a web-server GUI that is used to
control and demonstrate various hardware and software features of the Camera
Solution System Example Design. Follow the
[**instructions**](../camera_4k_stitch/camera_4k_stitch.md#setting-up-the-camera-solution) to
run the GUI on a web browser. The GUI is optimized for displaying the full
screen on a 1920x1080 resolution screen. Press F11 in your browser to go full
screen.
<br/>

The Software Application is flexible in discovering available hardware
components and only drawing the components present in the design on the GUI.
Therefore, some parts of the GUI elements can be missing or different across
different camera solution designs. Also note that there may be slight
variations in the GUI compared to the example images shown in this section.
<br/>


## Connecting to the Demo Application Web GUI

Take note of the Modular Development Kit IP address. This will be displayed on
the HPS console serial port when the demo application starts. Alternatively,
use the following command to list IP addresses:

```
ip a
```

Use a web browser of your choice (such as Chrome) on any device connected to
the same network as the Modular Development Kit. If connecting directly without
a switch or router, the self-assigned IPv6 address can be used, or assign a
[**static IPv4 Address**](./sw-funct-descr.md#using-a-static-ip-address).
<br/>


## Descriptions of the Tabs

This section summarizes the GUI tabs in the Camera Solution System Example
Design.

Note that the 4Kp60 Multi-Sensor Camera Stitch Solution System Example Design for Agilex™ 5 Devices replicates controls for both
left (camera 0) and right (camera 1) camera sensors.
<br/>

* [Input Config Tab](#input-config-tab)
* [ISP Input Pipeline 0 Tab](#isp-input-pipeline-tab)
* [ISP Input Pipeline 1 Tab](#isp-input-pipeline-tab)
* [ISP Pipeline 0 Tab](#isp-pipeline-tab)
* [ISP Pipeline 1 Tab](#isp-pipeline-tab)
* [Output Config Tab](#output-config-tab)
* [Pipeline Statistics 0 Tab](#pipeline-statistics-tab)
* [Pipeline Statistics 1 Tab](#pipeline-statistics-tab)
* [Pipeline Stitch Config Tab](#pipeline-stitch-config-tab)

<br/>


### Input Config Tab

<br/>
<center markdown="1">

![InputConfigTab.](../camera_4k_stitch/images/SW/InputConfigTab.png)

**Input Config Tab**
</center>
<br/>

The Input Config Tab controls two separate parts of the application:

* The Input Stage which corresponds to selecting which input to use and the
  parameters for those input sources.
* The Multi-Channel Control Loops, corresponding to the Auto White Balance and
  Auto Exposure functions.
<br/>
<br/>

This section summarizes the GUI controls in the Input Config Tab:

* [Input Source](#input-source)
* [Test Pattern Generator](#test-pattern-generator)
* [Camera Control 0](#camera-control)
* [Camera Control 1](#camera-control)
* [Multi-Channel Auto Exposure](#auto-exposure)
* [Multi-Channel Auto White Balance](#auto-white-balance)

<br/>


#### Input Source

<br/>
<center markdown="1">

![InputSelect.](../camera_4k_stitch/images/SW/InputSelect.png)

**Input Source**
</center>
<br/>

The Input Source UI controls which input is selected:

* Input TPG corresponds to the
[**Test Pattern Generator (TPG)**](#test-pattern-generator). The TPG RGB output
image is passed through a Remosaic IP to convert it to a Color Filter Array
(CFA) format image (also known as Bayer) that is consistent with the camera
input. It can be used to verify the system with or without the sensor modules.
The same TPG image is used for both left and right images.
* Cameras - the left (camera 0) and right (camera 1) camera sensor modules
connected to the development board. This is the normal selection.
* Camera N, where N corresponds to the sensor modules connected to the board.
The same sensor image is used for both left and right images.

<br/>


#### Test Pattern Generator

<br/>
<center markdown="1">

![TestPatternGenerator.](../camera_4k_resources/images/SW/TestPatternGenerator.png)

**Test Pattern Generator**
</center>
<br/>

The Test Pattern Generator UI allows you to control the Input Test Pattern
Generator operation mode:

* Color bars.
* Solid Color. Use the Color Select dropdown box to set the color from the list
(including color bar colors, and a mid-level gray).
* Rainbow. Cycles every few seconds the color selection from the Solid Color
mode.

<br/>


#### Camera Control

<br/>
<center markdown="1">

![CameraControl.](../camera_4k_resources/images/SW/CameraControl.png)

**Camera Control - FSM-IMX678 Sensor**
</center>
<br/>

The camera control provides manual exposure controls for the corresponding
imaging sensor. The UI presents a camera control tile for each sensor module
connected to the board. The model field shows the model of the optical sensor
module detected. Note that (GMSL) will appear after the model designation if
the Application Software has detected the optional GMSL link.
<br/>

The Frame Rate slider adjusts the sensor frame rate. Reducing the frame rate
gives more time to accumulate time, while increasing it reduces motion blurring
artifacts. For flickering light sources, we recommend setting the frame rate to
an integer multiple or reciprocal integer multiple of the electrical grid
frequency. For example, for 50 Hz grid frequency you may set the frame rate to
50, 25, 12.5 or 6.25 Hz to mitigate flickering.
<br/>

The Shutter Speed slider controls the light integration time of the sensor,
allowing the image to be brightened or dimmed. The maximum shutter speed is
inversely proportional to the frame rate, where a lower frame rate gives more
time for the sensor to integrate more light.
<br/>

The Analog Gain slider adjusts the light sensitivity of the sensor, allowing
the image to be brightened or dimmed. However, increasing exposure via analog
gain has a tradeoff of reducing signal-to-noise ratio resulting in a grainier
image.
<br/>

We recommend the following flow for manually adjusting the total exposure of a
scene:

* Minimize the analog gain.
* If the image is flickering adjust frame rate to 50 or 60 Hz depending on the
  power line frequency.
* Adjust shutter speed for optimum exposure.
* If the image is underexposed even at maximum shutter speed, do one or both of
  the following, depending on your motion artifact vs. noise preference, until
  satisfied:
  * Reduce the frame rate, halving it if flickering, and further increase the
  shutter speed. This will reduce the responsiveness of the video and increase
  motion artifacts without increasing graininess.
  * Increase the analog gain. This will increase the graininess of the video
  without changing responsiveness.

<br/>


#### Multi-Channel Auto Exposure

<br/>
<center markdown="1">

![MultiAutoexposureControl.](../camera_4k_stitch/images/SW/MultiChannelAutoExposure.png)

**Multi-Channel Auto Exposure Control**
</center>
<br/>

An Auto Exposure function is built into the camera solution demonstration.

In order to obtain a uniform result, both sensors contribute equally to the
Auto Exposure function. The Multi-Channel Auto Exposure control function can be
used to fine tune the contribution algorithm.

<br/>


#### Multi-Channel Auto White Balance

<br/>
<center markdown="1">

![MultiAutoWhiteBalanceControl.](../camera_4k_stitch/images/SW/MultiChannelAutoWhiteBalance.png)

**Multi-Channel Auto White Balance Control**
</center>
<br/>

An Auto White Balance function is built into the camera solution demonstration.

In order to obtain a uniform result, both sensors contribute equally to the
Auto White Balance function. The Multi-Channel Auto White Balance control
function can be used to fine tune the contribution algorithm.

<br/>


### ISP Input Pipeline Tab

<br/>
<center markdown="1">

![ISPInputPipelineTab.](../camera_4k_stitch/images/SW/IspInputPipelineTab.png)

**ISP Input Pipeline Tab**
</center>
<br/>

The ISP Input Pipeline tab contains the control parameters for the input source.

* [Sensor Profile](#sensor-profile)
* [Histogram](#histogram)
* [Auto Exposure](#auto-exposure)
* [Auto White Balance](#auto-white-balance)

<br/>


#### Sensor Profile

<br/>
<center markdown="1">

![SensorProfile.](../camera_4k_resources/images/SW/SensorProfile.png)

**Sensor Profile**
</center>
<br/>

The Import Camera Profile button loads a sensor profile (a JSON format
pre-calibrated sensor settings file) into the application. The application
comes with a default calibration profile preloaded on the microSD card image
for the IMX678 as featured in the Camera Solution System Example Design.
Therefore, is not necessary to load a new profile in normal operation. The
IMX678 profile was produced using the Calibration Tab in the UI, as well as
some offline processes for best-in-class color reproduction. Note that using
the calibration tab requires expert knowledge and is therefore hidden in the UI
by default.

<br/>


#### Histogram

<br/>
<center markdown="1">

![HistogramStatistics.](../camera_4k_resources/images/SW/HistogramStatistics.png)

**Histogram Statistics**
</center>
<br/>

The Histogram Statistics displays the real-time output of the Histogram
Statistics IP. It is intended to be used as a visual aid while adjusting
exposure controls.

<br/>


#### Auto Exposure

<br/>
<center markdown="1">

![AutoexposureControls.](../camera_4k_resources/images/SW/AutoexposureControls.png)

**Auto Exposure Controls**
</center>
<br/>

The Auto Exposure function built into the camera solution demonstration, uses
data from the Histogram Statistics IP to control the sensor analog gain and
shutter speed. When using the ROI, the algorithm will prioritize the region
specified when calculating the exposure.

<br/>
<center markdown="1">

![AutoexposureROI.](../camera_4k_resources/images/SW/AutoexposureROI.png)

**Auto Exposure - Region of Interest Editor**
</center>
<br/>

The Region of Interest (ROI) editor allows you to select a region of the image
to use for the Auto Exposure function. Drag the region with a mouse stretch and
drag the window freely, or enter starting coordinates and the size of the
region manually. Highlighting ROI makes the region visible on the ISP output
as an overlay. Clicking the circular arrow on the tile resets the configuration
to its default values.

<br/>


#### Auto White Balance

The Auto White Balance UI allows control of the Automatic White Balance (AWB)
function built into the camera solution demonstration. It uses the calibration
settings loaded via the Sensor Profile UI.

<br/>
<center markdown="1">

![AutoWhiteBalanceControls.](../camera_4k_resources/images/SW/AutoWhiteBalanceControls.png)

**Auto White Balance Controls**
</center>
<br/>

The White Balance control has five modes of operation:

* Disabled. This mode turns off the white balance function by bypassing the
White Balance Correction and Black Level Correction IPs, as well as unlocking
the controls of those IPs in the UI in the
[**ISP Pipeline Tab**](#isp-pipeline-tab). You may then enable those IPs and
manually change their coefficients. This mode also disables the Color
Correction Matrix function by programming it to unity.
* Choose Temperature. This mode lets you enter a color temperature freely using
the Temperature (K) slider. The temperature must be set correctly otherwise
incorrect coefficients will be used causing the image to look incorrect. As a
guide, scenes generally require a color temperature ranging from 4000 to 6500
Kelvin.
* Choose Lighting. This mode operates similarly to the Choose Temperature mode.
However, in this mode Temperature (K) slider is disabled. Instead, you can
choose the lighting conditions from the Lighting dropdown menu. Note that the
real temperature varies among the family of light sources.
* Automatic. Automatic mode turns off all user input and instead uses the White
Balance Statistics IP pre- and post- White Balance Correction to attempt to guess
the scene's color temperature.
* Custom Preset (Spot). This is a one-shot automatic white balance mode which
runs the AWB function for 2 seconds before locking in the temperature. Clicking
the Snapshot button or changing the ROI, resets the mode and triggers another 2
seconds one-shot. This mode is useful where the scene is too colorful. You can
position a gray card or position the camera to focus on a gray or white object
in the scene, and using the ROI to focus in on just that area, snapshot the
temperature to keep it constant for the entire scene.

The Tint Adjustment slider is unlocked across all modes. This control enables
you to adjust the magenta-green color balance when the light source illuminating
the scene is not an ideal black light source following the black body radiation
curve (Planckian locus).
<br/>

The CCM strength slider controls the color vibrancy. A unity value of 1.0 gives
the most accurate color reproduction following the sensor profile. If the light
source is not ideal, then a value of 1.0 might over-saturate some colors, which
requires you to lower it until satisfied. Conversely, you may increase the color
vibrancy for artistic purposes.
<br/>

Automatic and Custom Preset (Spot) modes unlock the ROI editor. The selected
region defines the area over which the White Balance Statistics IP performs its
calculations. Resizing or moving the active region automatically highlights the
region on the ISP output as an overlay for 2 seconds. For mixed scenes with
plenty of gray content, a full-screen statistics gives decent performance. For
mixed scenes, it is beneficial to localize the region of interest onto gray
parts of the image as described above.
<br/>

Clicking the circular arrow on the tile resets the configuration to its default
values.

<br/>





## ISP Pipeline Tab

<br/>
<center markdown="1">

![ISPPipeline.](../camera_4k_stitch/images/SW/IspPipelineTab.png)

**ISP Pipeline Tab**
</center>
<br/>

The ISP Pipeline tab contains the controls corresponding to the core ISP
functions and are summarized as follows:

* [Defective Pixel](#defective-pixel-correction)
* [Adaptive Noise Reduction](#adaptive-noise-reduction)
* [Black Level Correction](#black-level-correction)
* [Vignette Correction](#vignette-correction)
* [White Balance Correction](#white-balance-correction)
* [Demosaic](#demosaic)
* [Color Correction Matrix](#color-correction-matrix)

<br/>


### Defective Pixel Correction

<br/>
<center markdown="1">

![DefectivePixelCorrection.](../camera_4k_resources/images/SW/DefectivePixelCorrection.png)

**Defective Pixel Correction**
</center>
<br/>

The Defective Pixel Correction UI provides controls for bypassing the Defective
Pixel Correction (DPC) IP functionality, and when enabled setting the DPC
strength. The default strength is Very Weak which provides a good balance
between mitigating defective pixels, and preserving details and specular
highlights. Increasing the strength mitigates more defective pixels with a
tradeoff of softening the image.

<br/>


### Adaptive Noise Reduction

<br/>
<center markdown="1">

![AdaptiveNoiseReduction.](../camera_4k_resources/images/SW/AdaptiveNoiseReduction.png)

**Adaptive Noise Reduction**
</center>
<br/>

The Adaptive Noise Reduction UI provides controls for bypassing or setting the
strength of the denoising algorithm of the Adaptive Noise Reduction (ANR) IP. A
higher strength provides better denoising with a tradeoff of softening the
image.

Clicking the circular arrow on the tile resets the ANR configuration to its
default values.

<br/>


### Black Level Correction

<br/>
<center markdown="1">

![BlackLevelCorrection.](../camera_4k_resources/images/SW/BlackLevelCorrection.png)

**Black Level Correction**
</center>
<br/>

The Black Level Correction UI provides access to low level Black Level
Correction (BLC) IP controls. This tile is locked, displaying live values for
most of the white balance modes in the
[**Auto White Balance Control**](#auto-white-balance) as the settings are
controlled by the Application software. Note that disabling Auto White Balance
to enable manual BLC controls is not implemented by default. This is because
the SW App does not configure the imaging sensor and the MIPI connectivity IPs
to pass the Optical Black Region (OBR) required for processing the black level
statistics in real time. Instead, it uses offline calibrated values.

<br/>


### Vignette Correction

<br/>
<center markdown="1">

![VignetteCorrection.](../camera_4k_resources/images/SW/VignetteCorrection.png)

**Vignette Correction Controls**
</center>
<br/>

Vignette Correction (VC) is used to compensate for non-uniform intensity across
the image caused by the uneven light gathering limitations of the sensor and
optics. The most common non-uniformity is where the center of the lens gathers
more light compared to the outer regions. The VC IP corrects uniformity using a
runtime mesh of pre-calibrated coefficients and interpolating coefficients for
any given pixel.
<br/>

The Vignette Correction UI controls the Vignette Correction (VC) IP and
provides controls for bypassing as well as radial and lens corona correction.
Inverting the functionality allows adding vignetting to the image for artistic
purposes or showcasing the effect. The Radial Vignette slider simulates radial
vignetting as commonly seen on fisheye lenses. Likewise, the Lens Corona
simulates the effects of a strong light source on an imperfect lens, showing as
a bright spot in the center. The Application software translates the controls
into a mesh of coefficients that the VC IP uses.
<br/>

Clicking the circular arrow on the tile resets the configuration to its default
values.

<br/>


### White Balance Correction

<br/>
<center markdown="1">

![WhiteBalanceCorrection.](../camera_4k_resources/images/SW/WhiteBalanceCorrection.png)

**White Balance Correction Controls**
</center>
<br/>

The White Balance Correction UI provides access to low level White Balance
Correction (WBC) IP controls. This tile is locked, displaying live values for
most of the white balance modes in the
[**Auto White Balance Control**](#auto-white-balance) as the settings are
controlled by the Application software. Disable Auto White Balance to enable
manual WBC controls.
<br/>

Clicking the circular arrow on the tile resets the configuration to its default
values.

<br/>


### Demosaic

<br/>
<center markdown="1">

![Demosaic.](../camera_4k_resources/images/SW/Demosaic.png)

**Demosaic Controls**
</center>
<br/>

The bypass control is the only option in the Demosaic UI controls. In bypass
mode the Demosaic IP generates an RGB monochrome output image by simply
replicating the single color of a given input CFA pixel to all 3 color planes
to generate the given output RGB pixel.

<br/>


### Color Correction Matrix

<br/>
<center markdown="1">

![ColorCorrectionMatrix.](../camera_4k_resources/images/SW/ColorCorrectionMatrix.png)

**Color Correction Matrix Control**
</center>
<br/>

The Color Correction Matrix (CCM) UI is an artistic control interface for
tuning the CCM (which is implemented by the Color Space Converter (CSC) IP).
The white balance function of the application software calculates a base CCM
targeting an accurate representation of the scene. The application software
calculates the final CCM based on the control settings on the Color Correction
Matrix UI:

* Hue, Saturation, and Value (Brightness) controls enable transformations in
the HSV color space.
* Enabling color temperature post-processing enables further control for the
color temperature and tint on top of the values set by the white balance
controls.
* Red, Green, Blue Channel Strength sliders enable transformations in the RGB
color space.
* Contrast adjustment and raising image floor change the dynamic range of the
image.

Clicking the circular arrow on the tile of the UI resets the functionality of
these controls to their default values.

<br/>

## Output Config Tab

<br/>
<center markdown="1">

![OutputConfigTab.](../camera_4k_stitch/images/SW/OutputConfigTab.png)

**Output Config Tab**
</center>
<br/>

The Output Config tab contains UI controls for IP Cores typically located after
the ISP core pipeline, right up to and including the output.
<br/>
<br/>

This section summarizes the GUI controls in the Output Config Tab:

* [Unsharp Mask Filter](#unsharp-mask-filter)
* [3D LUT](#3d-lut)
* [Tone Mapping Operator](#tone-mapping-operator)
* [1D LUT](#1d-lut)
* [Logo](#logo)
* [Output](#output)

<br/>


### Unsharp Mask Filter

<br/>
<center markdown="1">

![UnsharpMaskFilter.](../camera_4k_resources/images/SW/UnsharpMaskFilter.png)

**Unsharp Mask Filter Controls**
</center>
<br/>

The Unsharp Mask Filter UI controls the strength of the sharpening applied to
the image. Positive strengths will sharpen the image, while negative strengths
will soften the image.
<br/>

Clicking the circular arrow on the tile will reset the configuration of the IP
to its default values.

<br/>


### 3D LUT

<br/>
<center markdown="1">

![3DLUT.](../camera_4k_resources/images/SW/3DLUT.png)

**3D LUT Control**
</center>
<br/>

The 3D LUT IP can be used for color space conversion, chroma keying, dynamic
range conversion (standard to high and high to standard), and artistic effects
(sepia, hue rotation, color volume adjustment, etc.).

The 3D LUT IP can support dual buffer mode to allow 2 LUT tables to be loaded
and switched between. However, for resource utilization reasons, the Camera
Solution System Example Designs only single buffer mode is supported and so 
the LUT 2 section of the UI is grayed out.

Note that 3D LUT files are not supplied with the Camera Solution System Example
Design.

<br/>


### Tone Mapping Operator

The Tone Mapping Operator (TMO) UI has controls to bypass the tone mapping and
sliders to adjust the threshold and level. The threshold slider controls the
strength of the tone mapping. The level slider controls the alpha blending
percentage between fully processed TMO output and its original input. The tone
mapping feature can use an ROI which can be adjusted from a pop-up panel.

<br/>
<center markdown="1">

![TMOControls.](../camera_4k_resources/images/SW/TMOControls.png)

**TMO Controls**

<br/>

![TMOControls-ROI.](../camera_4k_resources/images/SW/TMOControls-ROI.png)

**TMO ROI Editor**
</center>
<br/>


### 1D LUT

<br/>
<center markdown="1">

![1DLUT.](../camera_4k_resources/images/SW/1DLUT.png)

**1D LUT Control**
</center>
<br/>

The 1D LUT UI allows you to apply a transfer function to the output video image
to match the video sink. You may bypass the functionality or use it in one of
two modes:

* Enable Gamma Layer calculates industry standard OETF, EOTF and OOTF LUTs from
the options provided. The default transfer function is an OETF Gamma Curve
compliant with BT.709 standard.
* Enable Custom Curve allows you to draw a custom transfer function curve using
the Custom Curve Editor popup window. The background histogram allows you to
guide shaping the transfer function, and you have the option of using the full
screen or ROI histograms. You adjust the ROI using the UI pane provided in the
bottom left corner of the popup window. The number of segments allows you to
control more points on the transfer function window. Resetting the curve
changes the curve to a unity transfer function.

Clicking the circular arrow on the title of the UI resets the functionality of
these controls to their default values.

<br/>
<center markdown="1">

![CustomCurveEditor.](../camera_4k_resources/images/SW/CustomCurveEditor.png)

**Custom Curve Editor**
</center>
<br/>
<br/>


### Logo

<br/>
<center markdown="1">

![Logo.](../camera_4k_resources/images/SW/Logo.png)

**Logo Control**
</center>
<br/>

The Logo UI controls the position and opacity of the Altera® logo on the
screen. The UI also lets you adjust the location of the logo which can be
positioned in each corner or configured to bounce across the screen. The
opacity slider determines how transparent the Logo is, where 1 is fully opaque
and 0 is fully transparent. The Logo also has a configurable screensaver
function to protect screen burn-in where applicable. If you enable the
screensaver, the screen output will drop to a black background with the logo
bouncing across the screen. Interact with the GUI on your web browser to get
out of the screensaver.

<br/>


### Output

<br/>
<center markdown="1">

![OutputSource.](../camera_4k_resources/images/SW/OutputSource.png)

**Output Control**
</center>
<br/>

The Output UI controls which output source is selected:

* Select the ISP option to output the fully processed final image.
* Select the Bars option to output an unprocessed colorbars image.
This option allows you to test the video connection to the video sink.

<br/>


## Pipeline Statistics Tab

<br/>
<center markdown="1">

![PipelineStatisticsTab.](../camera_4k_stitch/images/SW/PipelineStatisticsTab.png)

**Pipeline Statistics Tab**
</center>
<br/>

The Pipeline Statistics Tab contains UI controls to help visualize the
statistics IPs within the ISP pipeline.
<br/>
<br/>

This section summarizes the GUI controls in the Pipeline Statistics Tab:

* [White Balance Statistics](#white-balance-statistics)
* [Histogram Statistics](#histogram-statistics)

<br/>


### White Balance Statistics

<br/>
<center markdown="1">

![WhiteBalanceStatistics.](../camera_4k_resources/images/SW/WhiteBalanceStatistics.png)

**White Balance Statistics**
</center>
<br/>

The White Balance Statistics UI shows a color ratio representation of a
captured scene from the camera input. The scene is split into 7x7 zones and
clicking on a grid square will select and visualize the ratios below.
<br/>

It is important to note that the color grid does not capture luminosity, only
color ratios. Therefore, results in some zones may appear unusual. For example,
dark gray and light gray sections will be indistinguishable.
<br/>

The UI works on a captured scene. You can use the "Update" button to force a
scene capture update.

<br/>


### Histogram Statistics

<br/>
<center markdown="1">

![HistogramStatisticsFull.](../camera_4k_resources/images/SW/HistogramStatisticsFull.png)

**Histogram Statistics**
</center>
<br/>

The Histogram Statistics UI shows a luminosity histogram of the current scene.
The Auto Exposure algorithm uses the histogram statistics in the background as
part of the brightness control function.
<br/>

The histogram automatically updates at a frequency of 1Hz, which you may
disable using the Auto Update checkbox. Read Stats button triggers a manual
update of the histogram.
<br/>

The Histogram Statistics IP calculates two histograms, one for the entire frame
and the other for an ROI. The display mode dropdown box allows you to display
either histogram statistics separately, or overlays them on the UI.
<br/>

Finally, the "Edit ROI" button opens a popup window for adjusting the ROI.

<br/>


## Pipeline Stitch Config Tab

<br/>
<center markdown="1">

![PipelineStitchConfigTab.](../camera_4k_stitch/images/SW/PipelineStitchConfigTab.png)

**Pipeline Stitch Config Tab**
</center>
<br/>

The Pipeline Stitch Config tab contains UI controls for the Stitch function and
associated IP Cores. Note that some controls are replicated for both left and
right sensors.

In order to stitch two cameras together, each input image must be projected
onto the same output image. To preserve horizontal lines across the stitch
point and prevent unnatural artifacts, the geometric normal of the two camera
projection at the stitch point, must be equal. This can be achieved using a
cylindrical projection.

<br/>

This section summarizes the GUI controls in the Pipeline Stitch Config Tab:

* [Stitch](#stitch)
* [Warp](#warp)

<br/>

### Stitch

<br/>
<center markdown="1">

![StitchControls.](../camera_4k_stitch/images/SW/StitchControls.png)

**Stitch Controls**
</center>
<br/>

As per [**A Guide to Lens correction and Projection**](./stitch.md)
these controls allow you to control the precise stitch function. The controls
also automatically create and apply the projection mesh for the
[Warp](#warp). Note that changes made to these controls will always
overwrite any manual changes made to the warp projection mesh.

<br/>


### Warp

The main Warp UI is a popup interface you open by clicking the Show Controls
button in the Warp tile. Note that these controls allow you to make further
changes to the projection mesh as generated by the [Stitch](#stitch) controls.
However, they will get overwritten if you make further changes using the
[Stitch](#stitch) controls. The detail that follows is general detail and not
specific to the stitch function.

<br/>
<center markdown="1">

![WarpControls.](../camera_4k_resources/images/SW/Warp.png)

**Warp Controls**
</center>
<br/>

The dialog box that pops up has blue buttons at the center/bottom which
switch between the warp editing modes: Fixed, Corners, Arbitrary and Fisheye.
<br/>

If any transformations go outside the warp IP's capability, the mesh will
change color from blue to red indicating an invalid configuration. When the
parameterization ranges become valid again, the mesh color will change back to
blue. The Scale View slider zooms out the Mesh Editor which makes it possible
to view transforms that could extend outside the target window. The Show
alignment guide box lets you turn the 16x9 grid of squares on and off.

<br/>
<center markdown="1">

![WarpFullFixedControls.](../camera_4k_resources/images/SW/WarpFullFixed.png)

**Fixed Warp Controls**
</center>
<br/>

The Fixed Controls panel has high-level controls for specifying a warp mesh
based on various mathematical transforms. Clicking on the 3x3 mesh button on
Fixed Controls title page snapshots the current mesh and changes the warp
editing mode to arbitrary mesh control mode. Clicking the circular arrow next
to it resets the functionality of these controls to their default values.

<br/>
<center markdown="1">

![WarpFullCorners.](../camera_4k_resources/images/SW/WarpFullCorners.png)

**Corner Warp Controls**
</center>
<br/>

Corner Controls mode allows you to drag 4 corners of the transformation mesh
rectangle. There is also a control to apply additional radial distortion.

<br/>
<center markdown="1">

![WarpFullArbitrary.](../camera_4k_resources/images/SW/WarpFullArbitrary.png)

**Arbitrary Warp Controls**
</center>
<br/>

The mesh editor in Arbitrary Controls mode lets you manually adjust the mesh
by dragging any control points. The number of control points can be changed
with the slider. The interface also allows you to export and import mesh files
to and from your host device.

<br/>
<center markdown="1">

![WarpFullFisheye.](../camera_4k_resources/images/SW/WarpFullFisheye.png)

**Fisheye Lens Warp Controls**
</center>
<br/>

Fisheye Lens Controls mode lets you configure Fisheye to Panorama or Fisheye to
Equirectangular image mapping. The outline in the Mesh Editor shows which area
of the input image will be mapped to the output.

<br/>
<br/>

***

<center markdown="1">

[BACK](../camera_4k_stitch/camera_4k_stitch.md#documentation)
</center>

***
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



[Agilex™ 5 E-Series Modular Development Board GSRD User Guide (25.1)]: https://altera-fpga.github.io/rel-25.1/gsrd/ug-gsrd-agx5e-modular/
[Agilex™ 5 E-Series Modular Development Board GSRD User Guide (26.1)]: https://altera-fpga.github.io/rel-26.1/gsrd/ug-gsrd-agx5e-modular/
[Agilex™ 5 E-Series Modular Development Board GSRD User Guide (26.1.1)]: https://altera-fpga.github.io/rel-26.1.1/gsrd/ug-gsrd-agx5e-modular/


[Agilex™ 5 SoC FPGA]: https://www.altera.com/products/fpga/agilex/5
[Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (25.1)]: https://docs.altera.com/r/docs/814346/25.1/hard-processor-system-technical-reference-manual-agilextm-5-socs/download-document
[Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (26.1)]:https://docs.altera.com/r/docs/814346/26.1/hard-processor-system-technical-reference-manual-agilextm-5-socs/agilextm-5-hard-processor-system-technical-reference-manual-revision-history
[Hard Processor System Technical Reference Manual: Agilex™ 5 SoCs (26.1.1)]:https://docs.altera.com/r/docs/814346/26.1.1/hard-processor-system-technical-reference-manual-agilextm-5-socs/agilextm-5-hard-processor-system-technical-reference-manual-revision-history
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



[Release Tag]: https://github.com/altera-fpga/agilex5-ed-camera/releases/tag/rel-26.1-isp_stitch-MDK_RevB_GrpB
[https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1]: https://github.com/altera-fpga/agilex5-ed-camera/tree/rel/26.1
[hps-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_stitch-MDK_RevB_GrpB/hps-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz
[fpga-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_stitch-MDK_RevB_GrpB/fpga-first-vvp-isp-demo-image-agilex5_mk_a5e065bb32aes1.wic.gz
[fsbl_agilex5_modkit_vvpisp_time_limited.sof]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_stitch-MDK_RevB_GrpB/fsbl_agilex5_modkit_vvpisp_time_limited.sof
[top.core.jic]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_stitch-MDK_RevB_GrpB/top.core.jic
[top.core.rbf]: https://github.com/altera-fpga/agilex5-ed-camera/releases/download/rel-26.1-isp_stitch-MDK_RevB_GrpB/top.core.rbf



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


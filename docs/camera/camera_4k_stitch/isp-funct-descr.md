

# 4Kp60 Multi-Sensor Camera Stitch Solution System Example Design for Agilex™ 5 Devices - ISP Functional Description

The 4Kp60 Multi-Sensor Camera Stitch Solution System Example Design
demonstrates a practical glass-to-glass camera solution using standard Altera®
Connectivity and Video and Vision Processing (VVP) Suite IP Cores available
through the Quartus® Prime Design Software (QPDS).

The design ingests the video input through industry-standard MIPI directly
connected to each sensor. The dual sensor video is then processed through dual
Image Signal Processing (ISP) pipelines before being stitched together, and
output through DisplayPort (DP). The design runs an embedded Linux Software
Application (SW App) on the Hard Processor System (HPS) to provide real-time
auto white balance (AWB) and auto exposure (AE) functions.

<br/>

> **Important Notes** <br/>
> **-** The following text can contain embedded links that aim to assist with
>       navigation, pointers to useful references and resources, or to add
        clarity. <br/>
> **-** There may be slight variations between screenshots and diagrams used
        here and the actual Camera Solution System Example Design.

<br/>

The following block diagram shows the main components and subsystems of the
Camera Solution System Example Design. Note that the IP instance number is
shown (bottom right corner of the IP) and is used by the Application Software
to indentify the unique instance.

<br/>
<center markdown="1">

![top-block-diagram.](../camera_4k_stitch/images/ISP/top-block-diagram.png)

**4Kp60 Multi-Sensor Camera Stitch Solution System Example Design for Agilex™ 5 Devices Top Block Diagram**
</center>
<br/>


## MIPI Ingest

The Framos FSM:GO IMX678 optical sensor module with PixelMate MIPI-CSI-2
connection uses a 4-lane MIPI interface. The sensor is a Sony Starvis2 8MP
IMX678 that outputs a Color Filter Array (CFA) image (also known as a Bayer
image) and can support up to a UHD 4K resolution at 60 FPS.

A CFA is typically a 2x2 mosaic of tiny colored filters (usually red, green,
and blue) placed over a monochromatic image sensor to effectively capture
single color pixels. The CFA typically contains twice the number of green
filters to align with human vision which is more sensitive to light in the
yellow-green part of the spectrum. The example below shows a typical RGGB (Red,
Green, Green, Blue) CFA pattern which repeats over the entire image (an 8x8
image in this example). Pixels arrive left to right, top to bottom, as
alternating Red and Green pixels on the first line, and then alternating Green
and Blue pixels on the next line. This pattern repeats on the next pair of
lines, and so on.

<br/>
<center markdown="1">

![bayer_diagram.](../camera_4k_resources/images/bayer_diagram.png)

**8x8 RGGB Color Filter Array (Bayer) Image Example**
</center>
<br/>

Using single color pixels reduces the overall bandwidth requirements of the
sensor. A demosaic algorithm can be used to rebuild the full color image.
Note that in the CFA domain, 4 color channels actually exist (in our example
they are Red, Green1, Green2, and Blue). Therefore, it can be seen that any
given pixel belongs to just one of these color channels when processing. Each
color channel is sometimes referred to as a CFA phase.


The Altera® MIPI D-PHY IP interfaces the FPGA directly to 2 Framos optical
sensor modules via Framos connectors on the Modular Development Kit Carrier
Board and PixelMate CSI-2 Flex-Cables. The design showcases a 4K (3840x2160)
sensor that can process images up to 60 FPS using 12-bit Bayer pixel samples.
The MIPI D-PHY is configured for 2 links (one per sensor) of x4 lanes at 1782
Mbps (which provides sufficient bandwidth) with no skew calibration and
non-continuous clock mode. Each sensor module has additional pins, including
Power on Reset, Master/Slave mode, as well as sync signals and a slave I2C
interface for control and status. The MAX10 device on the Modular Development
Kit Carrier Board drives some of these signals which can be controlled via the
FPGA using an additional I2C interface. All the I2C interfaces are connected
to the HPS I2C controllers. The SW App auto-detects and configures all of the
detected sensor modules, including GMSL3 links if being used.

The design connects an Altera® MIPI CSI-2 IP to each of the MIPI D-PHY IP Rx
links using a 4-lane 16-bit PHY Protocol Interface (PPI) bus. The design
configures each CSI-2 IP output at 4 Pixels In Parallel (PiP) using a 297MHz
clock and minimal internal buffering. A VVP Monitor IP is then used to
determine if the sensor is passing valid video. The SW will not allow a switch
to an invalid sensor input. Since all ISP IP only support VVP AXI4-S
Lite protocol, A VVP Protocol Converter IP is used on each CSI-2 IP output.


To reduce FPGA resources, a VVP PIP Converter IP is then used to reduce the PIP
from 4 to 2 (which still provides sufficient bandwidth to process the video
image). The sensor modules cannot be stalled. So the PIP Converter contains 2
lines of video buffer to accommodate small amounts of back-pressure from
downstream IPs.

> **Related Information** <br/>
> [Video and Vision Monitor IP] <br/>
> [Protocol Converter IP] <br/>
> [Pixels in Parallel Converter IP]

<br/>

## ISP Ingest
<br/>
<center markdown="1">

![isp_ingest.](../camera_4k_stitch/images/ISP/isp_ingest.png)

**ISP Ingest**
</center>
<br/>

The Input TPG IP allows you to test the ISP parts of the design without a
sensor module input. It uses the VVP Test Pattern Generator IP, a VVP Throttle
IP, and a non-QPDS IP called Remosaic (RMS) (supplied with the source project).
Since the TPG only outputs the active picture in RGB pixels, a Throttle is used
to set either 30 or 60 FPS depending on the Camera Solution Example Design. The
RGB output cannot be processed by the ISP IP either (as they only support CFA
images). So the RMS is used to convert the RGB image to a CFA image (with the
same phase as the sensor input), by simply discarding color information. The
TPG features several modes, including color bars and solid colors.


The Bayer Switch (a VVP Switch IP) is then used to select the Input source for
a given output.

Note that the 4Kp60 Multi-Sensor Camera Stitch Solution System Example Design for Agilex™ 5 Devices has dual outputs.




> **Related Information** <br/>
> [Test Pattern Generator IP] <br/>
> [Throttle IP] <br/>
> [Remoasaic IP] <br/>
> [Switch IP]


<br/>

## ISP Processing

This section summarizes notable ISP processing functions and IPs used in the
Camera Solution System Example Design:

* [Black Level Statistics](#black-level-statistics)
* [Clipper](#clipper)
  
* [Defective Pixel Correction](#defective-pixel-correction)
* [Adaptive Noise Reduction](#adaptive-noise-reduction)
* [Black Level Correction](#black-level-correction)
* [Vignette Correction](#vignette-correction)
* [White Balance Statistics](#white-balance-statistics)
* [White Balance Correction](#white-balance-correction)
* [Demosaic](#demosaic)
* [Histogram Statistics](#histogram-statistics)
* [Color Correction Matrix](#color-correction-matrix)
  
* [3D LUT](#3d-lut)
* [Tone Mapping Operator](#tone-mapping-operator)
* [Unsharp Mask Filter](#unsharp-mask-filter)
* [Region Of Interest](#region-of-interest)
* [Warp](#warp)
  
* [Stitch](#stitch)
  

<br/>


### Black Level Statistics

<center markdown="1">

![BLS_diagram.](../camera_4k_resources/images/ISP/BLS_diagram.png)

**Black Level Statistics Block Diagram**
</center>
<br/>

The Black Level Statistics (BLS) IP accumulates pixel values in a Region of
Interest (ROI), which is usually associated with a shielded area of an imaging
sensor called an Optical Black Region (OBR). The SW App, using these 
statistics, may choose to keep the sensor's black-level value constant and
compensate for any deviations caused by various external factors like 
temperature changes, voltage drifts, or aging.

The BLS calculates statistics across an OBR of the 2x2 CFA input from the
sensor. The IP has dedicated accumulators for each CFA channel that calculates
4 independent pixel sums. The SW App uses the BLS IP to set the offset and
scalar coefficients of the Black Level Correction (BLC) IP. The BLS passes the
input image to its output unchanged.

Note that although the hardware design includes BLS IP as part of the video
pipeline, the SW App does not configure the imaging sensor and the MIPI
connectivity IPs to pass the OBR for processing the statistics real time. The
SW App reads these statistics as part of a hidden and unsupported offline
calibration flow.

Note that this IP is optional and may not be part of the design.

> **Related Information** <br/>
> [Black Level Statistics IP]

<br/>


### Clipper

The Clipper IP crops an active area from an input image and discards the
remainder. The SW App may choose to use this IP to discard unwanted regions of
the video like OBR from the sensor input or perform digital zoom. Currently,
the SW App configures the sensor to output 4K video at the input, therefore it
configures the Clipper IP to bypass the input image.

Note that this IP is optional and may not be part of the design.

> **Related Information** <br/>
> [Clipper IP]

<br/>





### Defective Pixel Correction

The Defective Pixel Correction (DPC) IP removes impulse noise associated with
defective pixels in the sensor image. Such impulse noise is usually the result
of defective pixel circuitry within image sensor for a given pixel, and it
manifests itself in those pixels to respond to light drastically differently
compared to their neighboring pixels. The DPC IP operates on 2x2 Bayer CFA
images, identifying defective pixels using a configurable non-linear filter, and
then corrects them.

<br/>
<center markdown="1">

![DPC_diagram.](../camera_4k_resources/images/ISP/DPC_diagram.png)

**Defective Pixel Correction Block Diagram**
</center>

<br/>

The DPC IP works on a 5x5 pixel neighborhood. The IP gathers the 9 pixels of
the same color channel closest to the current pixel (the center pixel in the
neighborhood) and sorts them according to their pixel values. A non-linear
filter calculates a corrected pixel value from the sorted group of pixels
depending on the sensitivity setting.

<br/>
<center markdown="1">

![DPC_Bayer_CFA_diagram.](../camera_4k_resources/images/ISP/DPC_Bayer_CFA_diagram.png)

**A Bayer CFA for a 6x6 section of an image and an example pixel neighborhood for green, red, and blue pixels**
</center>

<br/>

The DPC IP dynamically identifies and filters defect pixels and does not
support static defective pixel correction of predetermined pixels. When you set
the sensitivity level to the weakest value, the pixel value is altered only if
its original value falls outside the value range of its whole pixel
neighborhood. As the sensitivity increases the IP approximates a class of
median filter.

> **Related Information** <br/>
> [Defective Pixel Correction IP]

<br/>


### Adaptive Noise Reduction

The Adaptive Noise Reduction (ANR) IP is an edge-preserving smoothing filter
that mainly reduces the independent pixel noise of an image. The IP operates on
2x2 Bayer CFA images.

<br/>
<center markdown="1">

![ANR_diagram.](../camera_4k_resources/images/ISP/ANR_diagram.png)

**Adaptive Noise Reduction Block Diagram**
</center>

<br/>

The ANR IP uses a spatial weighted averaging filter that analyzes the scene and
correlates similar pixels dynamically while generating the weights on the fly.
The IP utilizes two LUTs when correlating the pixels, one for correlating pixel
intensities and the other for correlating the spatial distance between the
pixels.

The design provides the intensity range LUT pre-calibrated offline using the
difference in noise level between two video images with identical content but
different temporal noise. The noise level is a function of the sensors analog
gain. Therefore, the calibration file contains a table of LUT parameters for a
range of sensor analog gain settings. Using a LUT allows you to program
different denoising strengths across the pixel intensities. For example, you
may opt for stronger denoising of dark content to reduce shadow noise more 
aggressively while preserving the details on the mid-tones and highlights.

The spatial distance LUT is used to make the ANR more versatile. It is
programmed to create a weight distribution from the center pixel to the
neighboring pixels. Traditional distributions like a Hamming or Hanning window
can be used to reduce ringing artifacts, or the same value entries can be used
for a rectangular distribution to maximize denoising capability. By default,
the software configures a Gaussian distribution into the spatial distance LUT.

> **Related Information** <br/>
> [Adaptive Noise Reduction IP]

<br/>


### Black Level Correction

The Black Level Correction (BLC) IP operates on a 2x2 CFA input image and
adjusts the minimum brightness level of the image, ensuring an actual black
value is represented by the minimum pixel intensity. A camera system typically
adds a pedestal value as an offset to the image at the sensor side.
Artificially increasing black level creates foot room for preserving noise
distribution of the black pixels and prevents artifacts in the final image. The
design positions the BLC IP after the ANR IP where the noise is reduced as much
as desired.

<br/>
<center markdown="1">

![BLC_diagram.](../camera_4k_resources/images/ISP/BLC_diagram.png)

**Black Level Correction Block Diagram**
</center>

<br/>

The BLC IP subtracts the pedestal value from the input video stream and scales
the result back to the full dynamic range. The scaler part of BLC multiplies
the pedestal remover value by the scaler coefficient, clipping to the maximum
output pixel value should the calculation overflow.

<br/>
<center markdown="1">

![BLC_Function_diagram.](../camera_4k_resources/images/ISP/BLC_Function_diagram.png)

**BLC Function**
</center>

<br/>

The SW App sets the pedestal and scaler coefficients for each of the 2x2 CFA
color channels dynamically during runtime. These values can be pre-calibrated,
or calculated dynamically from statistics obtained from the BLS IP using OBR of
the sensor. You can configure the BLC IP to reflect negative values around zero
or clip them to zero.

<br/>
<center markdown="1">

![BLC_RAZ_diagram.](../camera_4k_resources/images/ISP/BLC_RAZ_diagram.png)

**Effects of Reflection Around Zero**
</center>

<br/>

Note that for this design, the SW App provided does not utilize the BLS IP to
read the OBR of the sensor and relies on pre-calibrated coefficients as a
function of analog gain of the sensor.

> **Related Information** <br/>
> [Black Level Correction IP]

<br/>


### Vignette Correction

The Vignette Correction (VC) IP compensates for non-uniform intensity across
the image caused by uneven light-gathering limitation of the sensor and optics.
In the usual case, the non-uniformity can be caused by lens geometry transforms
such as zoom, aperture, etc., where the center of the lens gathers more light
compared to the outer regions. The VC IP corrects uniformity using a runtime
mesh of coefficients and interpolating coefficients for any given pixel. 

<br/>
<center markdown="1">

![Vignette_diagram.](../camera_4k_resources/images/ISP/Vignette_diagram.png)

**Vignette Correction Block Diagram**
</center>

<br/>

The VC IP uses a rectangular mesh of coefficients to adjust the pixel
intensities across an image, therefore tuning out vignetting and other sensor
non-uniformities. The VC IP operates independently on the 4 color channels of
the 2x2 CFA input image. For all color channels, you may calculate the mesh
coefficients with a calibration process in a controlled imaging environment
and configure the SW App to write them to the VC IP. The precision of a mesh
coefficient is fixed-point unsigned 8.11, with 8 integer bits and 11 fractional
bits. The mesh divides the image into rectangular zones, with mesh points
residing at the corners of the zones. For each pixel, the IP selects the 4 mesh
points corresponding to the corners of the pixel's zone and interpolates them
using the distance from the pixel location to the mesh points. A multiplier
scales the input pixels using this interpolated coefficient.

Note that the vignetting effect is very low for the sensor module used in this
design, therefore, the design does not contain a pre-calibrated mesh of
coefficients.

<br/>
<center markdown="1">

![Vignette_Mesh_Zone_diagram.](../camera_4k_resources/images/ISP/Vignette_Mesh_Zone_diagram.png)

**A Mesh Zone with a Pixel of Interest**
</center>

<br/>

> **Related Information** <br/>
> [Vignette Correction IP]

<br/>


### White Balance Statistics
<center markdown="1">

![WBS_diagram.](../camera_4k_resources/images/ISP/WBS_diagram.png)

**White Balance Statistics Block Diagram**
</center>

<br/>

The White Balance Statistics IP (WBS) operates on a 2x2 Bayer CFA image and
calculates red-green and blue-green ratios within a Region of Interest (ROI).
The ROI is divided into 7x7 zones, and the WBS calculates independent
statistics for all 49 zones. The SW App uses the WBS to set the White Balance
Correction IP.

<br/>
<center markdown="1">

![WBS-packing-order-diagram.](../camera_4k_resources/images/ISP/WBS-packing-order-diagram.png)

**Packing Order of the Zones within a Region of Interest**
</center>

<br/>

Each 2x2 Bayer region creates a virtual pixel from grouping 4 pixels. The IP
then calculates a red-green and a blue-green ratio for each virtual pixel
within the ROI.

<br/>
<center markdown="1">

![WBS_example_diagram.](../camera_4k_resources/images/ISP/WBS_example_diagram.png)

**Example of ratio calculation for 2x2 virtual pixels for a 6x6 Section of Image**
</center>

<br/>

The WBS IP checks both ratios against runtime programmable lower and upper
range thresholds and increments a zone virtual pixel count only if both ratios
fall between their respective threshold values. If at least one of the ratios
is out of range, the IP discards both ratios for that 2x2 virtual pixel and
therefore do not contribute to the final statistics of that zone. Ratios of all
virtual pixels that are not marked out of range are accumulated, and along with
the zone virtual pixel count are transferred to the zone memory at the end of
the zone.

The SW App reads each zone memory and calculates an average ratio by dividing
the accumulated ratio by the virtual pixel count. If the number of counted
pixels is too low, it indicates a zone with mixed content and is therefore
unsuitable for inclusion in calculating white imbalance of the image. The Auto
White Balance (AWB) algorithm is a feedback loop inside the SW App that
continuously reads the white balance statistics and guesses the color
temperature of the image scene.

The WBS IP passes its input image to its output unchanged.

> **Related Information** <br/>
> [White Balance Statistics IP]

<br/>


### White Balance Correction

The White Balance Correction (WBC) IP adjusts colors in a CFA image to
eliminate color casts, which occur due to lighting conditions or differences
in the light sensitivity of the pixels of different color. The IP ensures that
gray and white objects appear truly gray and white without, unwanted color
tinting.

<br/>
<center markdown="1">

![WBC_diagram.](../camera_4k_resources/images/ISP/WBC_diagram.png)

**White Balance Correction Block Diagram**
</center>

<br/>

The WBC IP multiplies the color channels of a 2x2 CFA input image by scalar
coefficients per color channel, clipping to the maximum output pixel value
should the calculation overflow.

The design provides a table of WBC scalars pre-calibrated for the sensor for a
range of color temperatures. The white balance algorithm in the SW App uses
color temperature information of the scene to look up WBC scalars from the
calibration table and configures the WBC IP over the Avalon® memory-mapped
interface. The SW App uses AWB to guess the color temperature in automatic
mode. The SW App also supports many fixed color temperature options.

> **Related Information** <br/>
> [White Balance Correction IP]

<br/>


### Demosaic

<center markdown="1">

![Demosaic_diagram.](../camera_4k_resources/images/ISP/Demosaic_diagram.png)

**Demosaic Block Diagram**
</center>

<br/>

The Demosaic IP (DMS) is a color reconstruction IP for converting a 2x2 Bayer
CFA input image to an RGB output image. The DMS interpolates missing colors
for each pixel based on its neighboring pixels.

<br/>
<center markdown="1">

![Demosaic-example-diagram.](../camera_4k_resources/images/ISP/Demosaic-example-diagram.png)

**An example of a 2x2 RGGB Bayer Color Filter Array (for an 8x8 pixel section of the image)**
</center>

<br/>

The DMS analyzes the neighboring pixels for every CFA input pixel and
interpolates the missing colors to produce an RGB output pixel. The IP uses
line buffers to construct pixel neighborhood information, maps the pixels in
the neighborhood depending on the position on the 2x2 CFA pattern, and
interpolates missing colors to calculate the RGB output.

> **Related Information** <br/>
> [Demosaic IP]

<br/>


### Histogram Statistics

The Histogram Statistics (HS) IP operates on RGB images. It analyzes the pixel
values for every frame to collects data to form a histogram of light intensity.

<br/>
<center markdown="1">

![HS_diagram.](../camera_4k_resources/images/ISP/HS_diagram.png)

**Histogram Statistics Block Diagram**
</center>

<br/>

The HS IP calculates two light intensity histograms on the RGB input image - a
whole image histogram and a ROI image histogram. The RGB to intensity
conversion is performed according to the ITU-R BT.709 standard.

The Automatic Exposure (AE) algorithm is a feedback loop in the SW App that
guesses the optimum exposure of the scene. It continuously reads the HS IP and
guesses whether the capture is underexposed or overexposed and adjust camera
sensor exposure settings accordingly.

The HS IP passes its input to its output unmodified.

> **Related Information** <br/>
> [Histogram Statistics IP]

<br/>


### Color Correction Matrix

The Color Correction Matrix (CCM) functionality is provided by the VVP Color
Space Converter (CSC) IP. A CCM correction is necessary to untangle the
undesired color bleeding across CFA color channels on the sensor. This is
mainly caused by each colored pixel being sensitive to color spectrums other
than their intended color.

The design configures the CSC IP to multiply the input RGB values of each pixel
with a 3x3 CCM to obtain the color corrected output RGB values.

The design provides a table of CCM coefficients pre-calibrated for the sensor
for a range of color temperatures. The AWB algorithm in the SW App uses color
temperature information of the scene to look up the CCM coefficients from the
calibration table and configures the CSC IP over the Avalon® memory-mapped
interface. The SW App uses AWB to guess the color temperature in automatic
mode. The SW App also supports many fixed color temperature options.

The SW App also provides many post-processing options to modulate the CCM
coefficients for adding an artistic effect on top of the pre-calibrated
accurate representation of the scene.

> **Related Information** <br/>
> [Color Space Converter IP]

<br/>



### 3D LUT

The 3D LUT IP maps an image's color space to another using interpolated values
from a lookup table.

Typical applications include:

* Color space conversion
* Chroma keying
* Dynamic range conversion (standard to high and high to standard)
* Artistic effects (sepia, hue rotation, color volume adjustment, etc.)

<br/>
<center markdown="1">

![3DLUT_colour_transform_examples_diagram.](../camera_4k_resources/images/ISP/3DLUT_colour_transform_examples_diagram.png)

**3D LUT Color Transform Examples**
(From top left to right: original, saturation, brightness increase, colorize (purple), colorize (green), desaturation)

<br/>
<br/>

![3DLUT_diagram.](../camera_4k_resources/images/ISP/3DLUT_diagram.png)

**3D LUT Block Diagram**
</center>

<br/>

The 3D LUT uses the most significant bits (MSBs) of the 3 RGB color component
inputs to retrieve data values from the LUT and the least significant bits
(LSBs) to interpolate the final output value. The SW App connected to the
Avalon® memory-mapped interface handles runtime control and LUT programming.



#### Generating LUT Files

You are responsible for sourcing or generating LUTs for the example design.
LUTs are generally developed based on input (optical system, sensor, ISP, etc.)
and display parameters. Various tools are available under open-source licenses
to produce LUTs. One such tool is LUTCalc, which can be used online. When using
it to generate LUTs for the 3D LUT IP, ensure that:

* The LUT is set to 3D
* Size matches the 3D LUT IP parameters (i.e. 17, 33, or 65 cube)
* Input and Output Range are 100%
* LUT Type is "General cube LUT (.cube)"

<br/>
<center markdown="1">

![3DLUT-LUTCalc-format-settings-diagram.](../camera_4k_resources/images/ISP/3DLUT-LUTCalc-format-settings-diagram.png)

**LUTCalc Format Settings**
</center>

<br/>

In general, any LUT file used with this example design must follow these
formatting conventions:

* RGB component order
* Components change first from left to right, i.e., R first, G second, B third
* The data type must match the IP GUI parameter and may either be:
  * normalized fixed- or floating-point numbers between 0.0 to 1.0
  * integers between 0 and 2LUT_DEPTH-1 (for example, 10-bit: 0 to 1023)
* The data type must be the same for the whole file

> **Related Information** <br/>
> [3D LUT] <br/>
> [3D LUT IP] <br/>
> [LUTCalc GitHub page]

<br/>


### Tone Mapping Operator

The Tone Mapping Operator (TMO) IP implements a tile-based local tone mapping
algorithm. It improves the visibility of latent image details and enhances the
overall viewing experience.

<br/>

![TMO_example_diagram.](../camera_4k_resources/images/ISP/TMO_example_diagram.png)
<center markdown="1">

**Before (left) and after (right) TMO is applied to an example image**

<br/>
<br/>

![TMO_diagram.](../camera_4k_resources/images/ISP/TMO_diagram.png)

**TMO Block Diagram**
</center>

<br/>

The luminance extractor converts the RGB input to LUMA. The image statistics
calculator uses LUMA to calculate a set of global and local statistics
regarding the contrast of the input image over a 4x4 grid. The LUT generator
software runs on an embedded Nios® V CPU which analyzes the statistics,
generates a set of mapping transfer functions, and converts them to LUTs. The
contrast enhancement engine applies mapping transfer functions locally for
better granularity. The image enhancer combines the information and calculates
a set of weights that are applied to the input to generate the contrast-enhanced
output.

The TMO does not use image buffers and therefore the statistics collected from
the previous image are used to enhance the current image.

The SW App configures the TMO IP over the Avalon® memory-mapped interface.

> **Related Information** <br/>
> [Tone Mapping Operator] <br/>
> [Tone Mapping Operator IP]

<br/>


### Unsharp Mask Filter

The Unsharp Mask (USM) IP applies a sharpening algorithm to the input image
by implementing an unsharp mask filter.

The IP firstly converts the RGB input to LUMA. The LUMA input is then passed
through a low-pass Gaussian blur filter. The IP subtracts the blurred input
from the original LUMA input to generate a high frequency component. A strength
scaler is applied to the high frequency component which is then used to scale
the input RGB image to generate the output RGB image.

The unsharp mask has an agent Avalon® memory-mapped interface to allow runtime
control for changing the sharpening strength. You can configure a positive or
negative strength value for sharpening or blurring the image. Setting the
strength to 0 is equivalent to bypass i.e. passing the input to the output
unmodified.

> **Related Information** <br/>
> [Unsharp Mask IP]

<br/>


### Region Of Interest

The Region Of Interest (ROI) IP is a non-QPDS IP (supplied with the source
project) that allows the Software Application to define both a brightness
reduction strength and an area within the image to darken the pixels therefore
highlighting a ROI. The ROI is used in the GUI for Users to highlight areas of
interest for controls like Auto White Balance and Auto Exposure.

> **Related Information** <br/>
> [Region Of Interest IP]

<br/>



### Warp

The Warp IP applies an arbitrary warp (or image transform) to an input image.
It allows for lens distortion corrections (fisheye for instance) and the
ability to scale, rotate, and mirror the image.

<br/>

![Warp_transform_examples_diagram.](../camera_4k_resources/images/ISP/Warp_transform_examples_diagram.png)
<center markdown="1">

**Warp Transform Examples**
(From left: arbitrary warp with a 5x5 array of control points, four corner warp with some radial distortion.)

<br/>
<br/>

![Warp_rotation_examples_diagram.](../camera_4k_resources/images/ISP/Warp_rotation_examples_diagram.png)

**Warp Mirror and Rotation Examples**
(From top left clockwise: original image, mirrored, 90° rotate and 180° rotate.)

<br/>
<br/>

![Warp_diagram.](../camera_4k_resources/images/ISP/Warp_diagram.png)

**Warp Block Diagram**
</center>

<br/>

The block diagram shows an external memory which is used to buffer the incoming
and outgoing image data. The external memory also stores the coefficient tables
used to control the Warp IPs operation and define the required image transform.

The IP uses its Warp engines to operate on the input images stored in the input
buffers and to construct warped, output images from the output buffers.

Depending on the Camera Solution, the Warp IP can be configured for 1 or 2 PiP
(using 1 or 2 engines), single or double bounce (single bounce has less latency
and less DDR SDRAM bandwidth requirements than double bounce, but requires
on-chip cache memory - which can limit the transforms available), and Mipmap
support (to perform 2:1, 4:1, 8:1 downscale ratios and allow more bandwidth for
transforms). Warp is also used to produce lower output resolutions like Full HD
and HD Ready, as well as support UHD. However some Camera Solutions always
require the output to be a scaled version of the input. In this instance, and
depending on the required scaling ratio, using a pair of VVP Scaler IPs to
perform scaling firstly in the Horizontal domain then the Vertical domain
before a reduced configuration Warp IP, can consume less overall logic
resources.

> **Related Information** <br/>
> [Warp] <br/>
> [Warp IP] <br/>
> [Scaler IP]

<br/>




### Stitch
The image stitch is a side-by-side stitch operation where the right most edge
of the left image is blended with the left most edge of the right image. The
VVP Mixer IP can perform the blend provided that 1 ISP pipeline has a per pixel
Alpha Channel.

The Alpha Channel non-QPDS IP (supplied with the source project) is an IP that
adds an alpha blend channel from a LUT to every pixel on a given video line.
The LUT only contains a single line of values and so every video line will have
the same alpha blend channel values as the last. The Software Application
programs the LUT with the alpha blend channel values to determine the opacity
of the pixel. Values range from 0 - fully transparent, to max value - fully
opaque. The bit size of the alpha channel is configured to be the same size as
the pixel color channel depth. The LUT is generally limited in size and so
typically does not contain a unique alpha channel value for every pixel on a
line. Instead, the LUT supports a default start and stop alpha channel value,
as well as a nominal sequential sequence of values in between. The Software
Application effectively programs a start and stop pixel number such that pixels
received up until the start start pixel number, will get assigned the same
default start alpha channel value as located at the LUT start location. From
then on, and up to the stop pixel number, the alpha channel value comes from
sequential LUT locations. At the stop pixel number, the LUT no longer
increments and the same default stop alpha channel value is applied to the
remaining pixels. At the start of the next line, the LUT is reset back to its
first location and the process repeats.

A VVP Mixer IP can be used to position the left image with the alpha
channel over the top of the right image with a small overlap. A VVP TPG IP
provides the solid color base layer for the stitch output. The width of the
overlap should be the width of the total alpha blend function. Typically the
Software Application would use a ramp or non linear function to generate the
Alpha Channel LUT in order to create a smooth blend affect.

To facilitate an effective blend, the VVP Warp IP is used prior to the stitch
to correct for lens distortion and project the source image onto a stitch
canvas (typically cylindrical).

> **Related Information** <br/>
> [Alpha Channel IP] <br/>
> [Test Pattern Generator IP] <br/>
> [Mixer IP]

<br/>




## Output Processing

<br/>
<center markdown="1">

![output_processing.](../camera_4k_stitch/images/ISP/output_processing.png)

**Output Processing**
</center>
<br/>

This section summarizes notable output processing functions and IPs used in the
Camera Solutions:

* [Video Mixer](#video-mixer)
* [1D LUT](#1d-lut)
  
* [DP Egress](#dp-egress)


### Video Mixer

The Video Mixer is used to combine the different input images into a single
output image. It uses a VVP Test Pattern Generator IP, a VVP Mixer IP, VVP
Frame Reader IP, a VVP Scaler IP, and a VVP Pixel Adapter IP.

The TPG is the base layer for the Mixer IP and is configured to match the
required output resolution. By default it produces a solid black image which
also serves as the screensaver function. In addition, the TPG also supports
color bars which can be used to test the DP output.

The ISP output layer is mixed over the base layer. It can also be positioned
anywhere over the base layer when the ISP image is a lower resolution than the
output.

The Frame Reader IP reads an HPS generated ARGB888 (Alpha+RGB 8-bit color
channels) overlay image from the HPS DDR SDRAM (via the HPS F2SDRAM interface).
The Scaler can be used to upscale the overlay image if required. Since the ISP
image is 10-bit color, a Pixel Adapter (VVP Bits per Color Sample Adapter IP)
is used to convert to ARGB10101010 format. Limiting the overlay image in size
and to 8-bit values in the HPS domain helps achieve higher overlay FPS. The
final overlay image is mixed over the base and ISP image. The opacity of the
overlay image is controlled by the Alpha channel itself, which can also be
changed at runtime by the SW App. The Mixer can also position the overlay image
anywhere over the mixed base and ISP image therefore supporting any size
overlay images.


> **Related Information** <br/>
> [Test Pattern Generator IP] <br/>
> [Mixer IP] <br/>
> [Video Frame Reader IP] <br/>
> [Scaler IP] <br/>
> [Bits per Color Sample Adapter IP]

<br/>


### 1D LUT

The 1D LUT IP uses a runtime configurable LUT to apply an input output transfer
function to the image. You may use it to implement OOTF, OETF, and EOTF
transfer functions defined for video standards and legacy gamma compression or
decompression. You may also change the LUT content arbitrarily for other
transfer functions or to apply an artistic effect to the image.


<br/>
<center markdown="1">

![1DLUT_diagram.](../camera_4k_resources/images/ISP/1DLUT_diagram.png)

**1D LUT Block Diagram**
</center>

<br/>

The 1D LUT IP calculates LUT addresses from the input pixels. It interpolates
fractional differences between LUT values to generate output pixel values. The
IP uses an independent LUT for each color plane. The SW App uses the Avalon®
memory-mapped interface to configure the LUTs.




The 1D LUT is used for traditional Gamma, High Dynamic Range Perceptual
Quantizer (HDR PQ) and Hybrid Log-Gamma (HDR HLG) correction.



> **Related Information** <br/>
> [1D LUT IP]

<br/>



### ISP Egress

The ISP Egress is used to interface the final ISP 4K output to the multi-rate
DP IP. The following output resolutions and color bit depths are supported by
the Camera Solution System Example Design:


* 4Kp60 @ 8-bit RGB Color

* 4Kp30 @ 8/10-bit RGB color
* 1080p60 @ 8/10-bit RGB color
* 720p60 @ 8/10-bit RGB color

* 5120*1440p34 @ 8-bit RGB Color (non-standard monitor support)
* 3840*1080p60 @ 8-bit RGB Color (non-standard monitor support)

Note that the input image is resized such that the stitched output fills the
horizontal resolution of the connected monitor therefore maintaining the
correct aspect ratio. So for a 16:9 monitor, the design will display a
letterbox image, while a 32:9 monitor will display a slightly cropped image but
will generally fill the entire display.



Since the DP IP does not support the VVP AXI4-S Lite protocol, the output is
passed through a VVP Protocol Converter IP.




> **Related Information** <br/>
> [Protocol Converter IP]



<br/>


## DP Egress

The DP Tx function is provided by the Altera® DisplayPort connectivity IP.

It is configured to support DisplayPort 1.4 (x4 lanes of 8.1 Gbps, sufficient for
4Kp60 8-bit RGB and 4Kp30 8/10-bit RGB). The DP IP also supports
the VVP AXI4-S Full protocol interface.



<br/>


## Hard Processor System 

Hard Processor System (HPS) runs the Software Application that configures the
external optical sensor module and the internal IPs, and provides the AI
functionality and various camera control loops such as AWB and AE. In addition,
it runs a web server that allows you to interact with the design demonstration
via an Ethernet connection. The HPS has its own external DDR4 SDRAM that is
used exclusively by the software stack.

In addition, the HPS has access to the Modular Scatter-Gather Direct
Memory Access IP (mSGDMA). This IP can be programmed by the HPS to offload
memory copy functions between HPS and FPGA external DDR4 SDRAM/s.



Memory copy functions include the Warp IP processing functions.


> **Related Information** <br/>
> [mSGDMA IP]

<br/>


## Additional Reference Information
* [Video and Vision Processing Suite Altera® FPGA IP User Guide]
* [Altera® FPGA Streaming Video Protocol Specification]
* [AMBA 4 AXI4-Stream Protocol Specification]
* [Avalon® Interface Specifications – Avalon® Streaming Interfaces]

<br>
<br>

***

<center markdown="1">

[BACK](../camera_4k_stitch/camera_4k_stitch.md#documentation)
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


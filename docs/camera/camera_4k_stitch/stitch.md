# A Guide to Lens correction and Projection

## Lens Types

### Perspective projection F-Tan Theta lens

Also called Rectilinear or Orthoscopic projection lenses. Used for narrow field
of view.

![](./images/stitch/FTanTheta.png)

### Equidistant projection F-Theta lens

Used for wide-angle, fish-eye lenses.

![](./images/stitch/FTheta.png)


## Lens distortion F-Theta lens

An ideal F-Theta lens will have a displacement equation of:

    d(θ) = F.θ

    where:
        θ = angle of incidence from optical axis in radians
        F = Focal length of lens

Real world lenses can come very close, but smaller lenses with wide field of
view can have some distortion. Displacement equation can be written as a
polynomial of degree four.

    d(θ) = a.θ + b.θ² + c.θ³ + d.θ⁴

Typically, F-Theta lenses exhibit distortion in the fourth order of theta.

An approximation can be written as:

    a = F
    b = 0
    c = 0
    d = F x (FOV/2) x (MaxDistortion%/100) x (1/(FOV/2)^4)

    where:
        MaxDistortion% = Maximum percentage distortion from ideal
        FOV = Field of view in radians
        FOV/2 = maximum value of theta, which is half the field of view

### Framos 110 (FLP-AM-040-02-V-00)

This lens has the following characteristics:

    F = 4 mm
    Max distortion [%] = -0.177
    FOV (diag.) = 127° = 2.2166 radians
    HFOV = 110°
    VFOV = 62°

Approximate displacement coefficients d(θ) in mm:

    a = 4
    b = 0
    c = 0
    d = 4 x (2.2166/2) x (-0.177/100) x (1/(2.2166/2)⁴) = -0.0052

![](./images/stitch/lens_distortion_framos110.png)

Ideal displacement equation (if the lens had no distortion from F-Theta ideal):

    d(θ) = 4 x θ

Real world approximation:

    d(θ) = 4 x θ + 0 x θ² + 0 x θ³ + -0.0052 x θ⁴
    simplifies to:
    d(θ) = 4 x θ - 0.0052 x θ⁴

### Sunnex 190 (DSL315B-650-F2.3)

This lens has the following characteristics:

    F = 2.67 mm
    Max distortion [%] = -18
    FOV (diag.) = 190° = 3.3161 radians
    Image circle = 7.2mm
    HFOV (Framos IMX678C) = 190°
    VFOV (Framos IMX678C) = approx 114°

Approximate displacement coefficients d(θ) in mm:

    a = 2.67
    b = 0
    c = 0
    d = 2.67 x (3.3161/2) x (-18/100) x (1/(3.3161/2)⁴) = -0.1054

![](./images/stitch/lens_distortion_sunex190.png)

Ideal displacement equation (if the lens had no distortion from F-Theta ideal):

    d(θ) = 2.67 x θ

Real world approximation:

    d(θ) = 2.67 x θ + 0 x θ² + 0 x θ³ + -0.1054 x θ⁴
    simplifies to:
    d(θ) = 2.67 x θ - 0.1054 x θ⁴


## Sensor pixel position to polar coordinates

As described above, for F-Theta wide-angle lenses, displacement onto a sensor
is a function of theta θ, where theta θ is angle between the incoming ray and
the optical axis (z axis). The displacement from the centre of the sensor will
be given by d(θ). The other important angle for polar coordinates is psi ᴪ.
This describes the angle to the x-axis the incoming ray projects onto the x-y
plane. The displacement d(θ) and psi ᴪ can both be determined from the sensor
pixel position (sx, sy):

    d(θ) = √(sx² + sy²)
    ᴪ = atan2(sy, sx)

Note: [atan2](https://en.wikipedia.org/wiki/Atan2) is the two argument arctangent.

Once d(θ) is known, θ can be found. If you ignore distortion and just use the
theoretical equation, θ can be found simply by θ = d(θ)/F. Taking distortion
into account involves solving the fourth order polynomial. For the larger order
polynomials, an approximation to the inverse function d-1(θ) can be obtained
from curve fitting, or by using a binary search of θ using d(θ). Using
numerical methods such as Newton-Raphson, is also possible.

### Polar coordinates to cartesian coordinates

![](./images/stitch/polar_to_cartesian.png)

Line L in polar coordinates is defined as:

    L(A) = (A, θ, ᴪ)

    where:
        A is a scalar

Line L can also be defined in cartesian coordinates as:

    L(A)=(x, y, z) = (A.sin(θ).cos(ψ), A.sin(θ).sin(ψ), A.cos(θ))

    where:
        A is a scalar


## Cylindrical projection

In order to stitch two cameras together onto a single output image, the camera
images must be projected onto the output image. To preserve horizontal lines
across the stitch point and prevent unnatural artifacts, the normal of the two
camera projection at the stitch point, must be equal. This can be achieved using
a cylindrical projection.

Imagine the camera sensor has no magically turned into a powerful projector.
The sensor pixel (sx,sy) now projects a light ray along line L. Now if we
placed a cylindrical projection screen around the camera with its centre at the
lens focal point, we could calculate the point of intersection between the line
L and the screen.

![](./images/stitch/cylinder_intersection.png)

Cylinder C can be defined in cartesian coordinates as:

    x² + z² = cylinder_radius²

Substituting x, and z from the cartesian expression of L gives:

    (A.sin(θ).cos(ψ))² + (A.cos(θ))² = cylinder_radius²

We can now solve for A

    A = √(cylinder_radius² / (sin²(θ).cos²(ψ) + cos²(θ)))

Next convert the image projected onto the cylinder into a two-dimensional
output. Imagine taking a section of the cylinder and flattening it. The section
of the cylinder of interest is the combined field of view of the camera being
stitched together.

Example:
Two Framos IMX678C sensors fitted with Framos 110 (FLP-AM-040-02-V-00) lenses
are mounted in a holder at 90° to each other. Note that it is not physically
possible to place the two focal points at the same point. But the camera mount
places them as close as possible.

![](./images/stitch/camera_mount_front.png) ![](./images/stitch/camera_mount_top.png)

This mount places the focal point at about 40 mm from the mount centre. The
consequence of this offset will be described later.

This example configuration has two 110° lenses mounted at 90° to each other.
This results in a 20° overlap and a 200° field of view. If 0° is considered the
centre of the output image, the left edge of the image should be at -100° and
the right edge 100°. The length of the arc segment is calculated as:

    arc_length = cylinder_radius x arc_angle_radians

    output_width = cylinder_radius x 200° x π / 180°

We can now define the cylinder_radius in terms of pixels:

    cylinder_radius_in_pixels = output_width_in_pixels x 180° / (200° x π)

Now we know the cylinder radius, so we can determine the cartesian coordinates
of the intersection of line L and cylinder C:

    A = √(cylinder_radius_in_pixels / (sin²(θ).cos²(ψ) + cos²(θ)))
    cylinder_cartesian_x = A.sin(θ).cos(ψ);
    cylinder_cartesian_y = A.sin(θ).sin(ψ);
    cylinder_cartesian_z = A.cos(θ);

The cylinder angle (angle projection of L into the x-z plane) can be calculated as:

    cylinder_angle = atan2(cylinder_cartesian_x, cylinder_cartesian_z) + sensor_angle

    where:
        sensor_angle is the angle between the sensor optical axis and the
        centre of the cylindrical arc.
        In the above example sensor 1 will be at -π/4 (-45°) and sensor 2 will
        be at +π/4 (+45°).

note: [atan2](https://en.wikipedia.org/wiki/Atan2) is the two argument arctangent.

To complete the projection onto the final image, the y coordinate will be the
same as the cylinder_cartesian_y above and the x coordinate will be
proportional to cylinder_angle:

    cylindrical_projection_x = cylinder_radius_in_pixels x cylinder_angle
    cylindrical_projection_y = cylinder_cartesian_y

We can now calculate the output projection coordinate
(cylindrical_projection_x, cylindrical_projection_y) for any coordinate on a
sensor (sx,sy) for both sensors. This allows the generation of a warp mesh per
camera.

![](./images/stitch/warp_mesh.png)


# Parallax Error

As mentioned above, it is not physically possible to place the focal point of
two cameras at the projected cylinder centre. The example mount shown above has
an offset (d) of approximately 40 mm. This produces a small error in the
projection.

![](./images/stitch/parallax_error.png) ![](./images/stitch/parallax_error_triangle.png)

    where:
        a is the angle calculated to the point of interest by the projection.
        b is the actual angle to the point of interest from the cylinder centre (mount centre point).
        d is the offset of lens focal point from cylinder centre.
        R is the distance from the cylinder centre (mount centre point) to the point of interest.

    defining:
        p as the angle opposite R.
        q as the angle opposite d.

Using the sine rule, it is possible to calculate the parallax error for a given
point:

    p = π - a

    q can be found since all angles around a triangle must add up to π.
    Subtracting the other two angles (b and p):
        q = π - b - p
        q = π - b - (π - a)
        q = a - b

    so the sine rule gives:

    d/sin(a-b) = R/sin(π - a)

    as sin(π - a) = sin(a):

        d/sin(a-b) = R/sin(a)

    solving for b:

        b = a - asin(d.sin(a)/R)

    parallax error in b:

        error = a - b
        error = a - (a - asin(d.sin(a)/R))
        error = asin(d.sin(a)/R)

The parallax error in b is shown below for a point at the edge of the camera
field of view. This is using the Framos 110 (FLP-AM-040-02-V-00) and the mount
shown in the example above.

![](./images/stitch/parallax_error_angle.png)

Although the error is at its largest very close to the camera, noticeable
errors exist for sizable distances. Assuming the projected image were displayed
on a UHD monitor, where 200° equates to 3840 pixels, each ° of error equates to
an error of 19.2 pixels (px).

![](./images/stitch/parallax_error_pixels.png)

The error shown is for one sensor. Since we are stitching two sensors together,
the total error is effectively doubled.

Minimizing d, the offset of lens focal point from the mount centre, is very
important to reduce the parallax errors to a minimum.

It would be possible to correct for parallax errors if the distance were known
for each sensor pixel. This could be obtained from a depth sensor, such as
a time-of-flight sensor or a LIDAR sensor.

<br>

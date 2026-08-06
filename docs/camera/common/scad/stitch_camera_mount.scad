// Hollow Cube Generator
// Creates a hollow cube with configurable size and wall thickness

// Parameters - adjust these values as needed
cube_size = 36;        // Size of the cube (all dimensions equal)
wall_thickness = 3;    // Thickness of the walls
hole_diameter = 1.60;     // Diameter of the mounting holes
hole_spacing = 22;     // Distance between adjacent holes along each edge
base_height = cube_size / 4; // Height of the bottom cuboid

landscape_window_width = cube_size - 2 * (wall_thickness + 1);
landscape_window_height = hole_spacing - 4;
portrait_window_width = 18;             // To fit in ribbon cable connector
portrait_window_height = cube_size - 2 * wall_thickness;

bevel_size = 7;

tripod_hole_diameter = 5.1;     // Diameter of the tripod mounting hole

// Module to create 4 holes at corners of a square face
// Holes are positioned so that adjacent holes are exactly hole_spacing apart
module corner_holes(face_size, hole_dia, spacing, depth) {
    // Calculate distance from edge based on desired spacing between holes
    distance = (face_size - spacing) / 2;
    
    positions = [
        [distance, distance],
        [face_size - distance, distance],
        [face_size - distance, face_size - distance],
        [distance, face_size - distance]
    ];
    
    for (pos = positions) {
        translate([pos[0] - face_size/2, pos[1] - face_size/2, 0])
            cylinder(h = depth + 4, d = hole_dia, center = true, $fn = 32);
    }
}


module face_cutout(lw, lh, pw, ph, thickness) {
    // Landscape cutout
    cube([lw, lh, thickness], center = true);

    // Portrait cutout
    // Move 1mm down for easier connector fitting
    translate([0, -1, 0])
        cube([pw, ph, thickness], center = true);    
}

// Module to create the hollow cube
module mount_main(size, thickness) {
    difference() {
        // Outer cube
        cube([size, size, size], center = true);
        
        // Inner cube (hollowed out part)
        inner_size = size - 2 * thickness;
        
        cube([inner_size, inner_size, inner_size], center = true);
        
        // Holes on front face (positive X)
        translate([size/2, 0, 0])
            rotate([0, 90, 0]) {
                corner_holes(size, hole_diameter, hole_spacing, thickness + 1);
            }

        translate([size/2, 0, 0])
            rotate([90, 0, 90])
                face_cutout(landscape_window_width, landscape_window_height, portrait_window_width, portrait_window_height, thickness + 4);
                
        // Holes on back face (negative X)
        translate([-size/2, 0, 0])
            rotate([0, 90, 0])
                corner_holes(size, hole_diameter, hole_spacing, thickness + 1);

        translate([-size/2, 0, 0])
            rotate([90, 0, 90])
                face_cutout(landscape_window_width, landscape_window_height, portrait_window_width, portrait_window_height, thickness + 4);
        
        // Holes on right face (positive Y)
        translate([0, size/2, 0])
            rotate([90, 0, 0])
                corner_holes(size, hole_diameter, hole_spacing, thickness + 1);

        translate([0, size/2, 0])
            rotate([90, 0, 0])
                face_cutout(landscape_window_width, landscape_window_height, portrait_window_width, portrait_window_height, thickness + 4);
        
        // Holes on left face (negative Y)
        translate([0, -size/2, 0])
            rotate([90, 0, 0])
                corner_holes(size, hole_diameter, hole_spacing, thickness + 1);

        translate([0, -size/2, 0])
            rotate([90, 0, 0])
                face_cutout(landscape_window_width, landscape_window_height, portrait_window_width, portrait_window_height, thickness + 4);
        
        // Holes on top face
        translate([0, 0, size/2])
            corner_holes(size, hole_diameter, hole_spacing, thickness + 1);
        
        // Cross-shaped hole in center of top face (fits between round holes)
        translate([0, 0, size/2]){
            
            // Horizontal bar of the cross
            cube([landscape_window_width, landscape_window_height, thickness + 4], center = true);
            // Vertical bar of the cross
            cube([portrait_window_width, portrait_window_height, thickness + 4], center = true);

            // Chamfers
            translate([7, 7, 0])
                rotate([0, 0, 45])
                    cube([bevel_size, bevel_size, thickness + 4], center = true);

            translate([7, -7, 0])
                rotate([0, 0, 45])
                    cube([bevel_size, bevel_size, thickness + 4], center = true);

            translate([-7, -7, 0])
                rotate([0, 0, 45])
                    cube([bevel_size, bevel_size, thickness + 4], center = true);

            translate([-7, 7, 0])
                rotate([0, 0, 45])
                    cube([bevel_size, bevel_size, thickness + 4], center = true);                                                        
        }

        // Cutout at the bottom of the main cube
        translate([0, 0, -size/2])
            cube([size - 2 * thickness, size - 2 * thickness, thickness + 4], center = true);

        // Chamfers front, back left and right
        translate([0, 7, 7])
            rotate([45, 0, 0])
                cube([size + 2, bevel_size, bevel_size], center = true);

        translate([0, -7, 7])
            rotate([45, 0, 0])
                cube([size + 2, bevel_size, bevel_size], center = true);

        translate([0, -7, -7])
            rotate([45, 0, 0])
                cube([size + 2, bevel_size, bevel_size], center = true);

        translate([0, 7, -7])
            rotate([45, 0, 0])
                cube([size + 2, bevel_size, bevel_size], center = true);

        translate([7, 0, 7])
            rotate([0, 45, 0])
                cube([ bevel_size, size + 2, bevel_size], center = true);

        translate([7, 0, -7])
            rotate([0, 45, 0])
                cube([ bevel_size, size + 2, bevel_size], center = true);

        translate([-7, 0, 7])
            rotate([0, 45, 0])
                cube([ bevel_size, size + 2, bevel_size], center = true);

        translate([-7, 0, -7])
            rotate([0, 45, 0])
                cube([ bevel_size, size + 2, bevel_size], center = true);
    }
}

// Module to create the base with the tripod mount hole
module mount_base(length, width, height, thickness) {
    difference() {
        // Outer cuboid
        cube([length, width, height], center = true);

        base_bottom_thickness = thickness + 1;
        
        // Inner cuboid (hollowed out part)
        // Same height as the outer
        // Thicker bottom wall created by translating
        // it upwards by the base_bottom_thickness
        inner_length = length - 2 * thickness;
        inner_width = width - 2 * thickness;
        inner_height = height;
        
        translate([0, 0, base_bottom_thickness])
            cube([inner_length, inner_width, inner_height], center = true);

        cylinder(h = height + 2, d = tripod_hole_diameter, center = true, $fn = 32);

        translate([0, 0, -(base_height / 2) - 0.5])
            cylinder(h = tripod_hole_diameter - 1, d1 = tripod_hole_diameter + 1, d2 = 0, center = false, $fn = 32);
    }
}

union() {
// Generate the hollow cube
mount_main(cube_size, wall_thickness);

// Add hollow cuboid at the bottom
translate([0, 0, -(cube_size + base_height) / 2])
    mount_base(cube_size, cube_size, base_height, wall_thickness);
}

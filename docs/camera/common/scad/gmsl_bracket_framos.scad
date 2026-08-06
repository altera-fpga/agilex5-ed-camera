// Bracket for mounting Framos GMSL deserializers onto Agilex 5 MDK carrier board

hole_distance = 91.925;
mount_width = 10;
mount_height = 13;

width = hole_distance + mount_width;
height = 46;
depth = 4;

board_clearance = 11;
hole_dia = 3.2;

support_depth = depth - 2;
support_size = sqrt((board_clearance * board_clearance) / 2);

logo_diameter = 12;
logo_x_offset = 0;
logo_y_offset = 6;
logo_z_offset = depth / 2;

 
module gmsl_module_cutout(depth) {
    module_size = 26.5;
    hole_diameter = 2.20;     // Diameter of the mounting holes
    pad_diameter = 4.3;
    hole_spacing = 22;
    bottom_clearance = 0.5;

    offset = hole_spacing / 2;

    translate([-offset, -offset, 0])
        cylinder(h = depth + 4, d = hole_diameter, center = true, $fn = 32);

    translate([offset, -offset, 0])
        cylinder(h = depth + 4, d = hole_diameter, center = true, $fn = 32);

    translate([-offset, offset, 0])
        cylinder(h = depth + 4, d = hole_diameter, center = true, $fn = 32);

    translate([offset, offset, 0])
        cylinder(h = depth + 4, d = hole_diameter, center = true, $fn = 32);

    translate ([0,bottom_clearance/2,0])
        cube([module_size - 2 * pad_diameter, module_size+bottom_clearance, depth + 4], center = true);
    
    cube([module_size, module_size - 2 * pad_diameter, depth + 4], center = true);

    // Chamfers
    chamfer_size = 4;

    translate([-8,-8,0])
        rotate([0,0,45])
            cube([chamfer_size, chamfer_size, depth + 4], center = true);

    translate([-8,8,0])
        rotate([0,0,45])
            cube([chamfer_size, chamfer_size, depth + 4], center = true);

    translate([8,-8,0])
        rotate([0,0,45])
            cube([chamfer_size, chamfer_size, depth + 4], center = true);

    translate([8,8,0])
        rotate([0,0,45])
            cube([chamfer_size, chamfer_size, depth + 4], center = true);
}


module board_mount(depth) {
    difference() {
        cube([mount_width, mount_height, depth], center = true);
        translate([0, 1.5, 0])
            cylinder(h = depth + 4, d = hole_dia, center = true, $fn = 32);

        // Chamfers
        chamfer_size = 3;

        translate([-mount_width / 2, mount_height / 2, 0])
            rotate([0,0,45])
                cube([chamfer_size, chamfer_size, depth + 4], center = true);

        translate([mount_width / 2, mount_height / 2, 0])
            rotate([0,0,45])
                cube([chamfer_size, chamfer_size, depth + 4], center = true);                
    }
}


module board_mount_support(){
        difference() {
            rotate([0,0,0]){
                cube([support_size, support_size, support_depth], center = true);
            }

            translate([-support_size + 2, -support_size + 2, 0])
                rotate([0,0,45])
                    cube([2 * support_size, 2 * support_size, support_depth + 2], center = true);
        }
}

module logo(diameter, height){
    union(){
        diameter_inner = diameter * 0.55;
        width = (diameter - diameter_inner) / 2;

        difference(){
            union(){
                
                difference(){
                    cylinder(h = height, d = diameter, center = true, $fn = 32);
                    cylinder(h = height + 1, d = diameter_inner, center = true, $fn = 32);
                }

                translate([(diameter_inner + width) / 2, 0, 0])
                    cube([width, diameter * 0.94, height], center = true);
            }

            offset = (diameter * 1.26) / 2;
            translate([offset, offset, 0])
                rotate([0,0,45])
                    cube([diameter, diameter, height + 1], center = true);
        }

        translate([(diameter_inner + width) / 2, (diameter_inner + width) / 2, 0])
            cube([width, width, height], center = true);
    }
}

difference() {
    union() {
        // Main body
        cube([width, height, depth], center = true);

        // Board mount
        //translate([(width - mount_width) / 2, (height - depth) / 2, (mount_height - depth) / 2])
        translate([hole_distance / 2, (height - depth) / 2, (mount_height - depth) / 2])
            rotate([90,0,0])
                board_mount(depth);

        // Board mount
        //translate([-(width - mount_width) / 2, (height - depth) / 2, (mount_height - depth) / 2])
        translate([-hole_distance / 2, (height - depth) / 2, (mount_height - depth) / 2])
            rotate([90,0,0])
                board_mount(depth);

        // Board mount supports
        translate([-(width + support_depth)/2 + mount_width, (height - support_size)/2, support_size / 2])
            rotate([0,90,0])
                board_mount_support();

        translate([-(width - support_depth)/2, (height - support_size)/2, support_size / 2])
            rotate([0,90,0])
                board_mount_support();

        translate([(width + support_depth)/2 - mount_width, (height - support_size)/2, support_size / 2])
            rotate([0,90,0])
                board_mount_support();

        translate([(width - support_depth)/2, (height - support_size)/2, support_size / 2])
            rotate([0,90,0])
                board_mount_support();

        translate([logo_x_offset, -logo_y_offset, logo_z_offset])
            rotate([0,0,180])
                logo(logo_diameter, 0.8);                
    }

    // Cutout for the board connectors
    translate([0, (height - board_clearance) / 2, 0])
    cube([width - 2 * mount_width, board_clearance + 1, depth + 2], center = true);

    gmsl_offset_x = 24;
    gmsl_offset_y = 6;

    translate([-gmsl_offset_x, -gmsl_offset_y, 0])
        gmsl_module_cutout(depth);

    translate([gmsl_offset_x, -gmsl_offset_y ,0])
        gmsl_module_cutout(depth);

    // Chamfers
    chamfer_size = 3;

    translate([-width / 2, -height / 2, 0])
        rotate([0,0,45])
            cube([chamfer_size, chamfer_size, depth + 4], center = true);

    translate([width / 2, -height / 2, 0])
        rotate([0,0,45])
            cube([chamfer_size, chamfer_size, depth + 4], center = true);

    translate([logo_x_offset, -logo_y_offset, -logo_z_offset])
        rotate([180,0,0])
            logo(logo_diameter, 0.4);
}
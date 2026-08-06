width = 36;
height = 36;
depth = 4;

base_height = 16;
base_depth = 6;

tripod_hole_diameter = 5.1;     // Diameter of the tripod mounting hole

module camera_module_cutout(depth) {
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


module chamfer(radius, depth) {
    cylinder(h = depth, d = 2 * radius, center = true, $fn = 32);
}


module base() {
    chamfer_radius = 3;

    union(){
        difference(){
            cube([width, base_height, base_depth], center = true);

            translate([0, depth / 2, 0]){
                cylinder(h = height + 2, d = tripod_hole_diameter, center = true, $fn = 32);

                translate([0, 0, -(base_depth / 2) - 0.5])
                    cylinder(h = tripod_hole_diameter - 1, d1 = tripod_hole_diameter + 1, d2 = 0, center = false, $fn = 32);
            }

            // Chamfer cut-outs
            translate([0, base_height / 2, 0]){
                translate([-width / 2, 0,0])
                    chamfer(chamfer_radius, base_depth + 1);
                translate([width / 2, 0,0])
                    chamfer(chamfer_radius, base_depth + 1);                
            }
        }

        // Chamfers
        translate([0, base_height / 2 - chamfer_radius, 0]){
            translate([-width / 2 + chamfer_radius, 0,0])
                chamfer(chamfer_radius, base_depth);
            translate([width / 2 - chamfer_radius, 0,0])
                chamfer(chamfer_radius, base_depth);
        }
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


module main(){
    union(){
        chamfer_radius = 3;

        difference(){
            cube([width, height, depth], center = true);
            camera_module_cutout(depth);

            // Chamfer cut-outs
            translate([0, -width / 2, 0]){
                translate([-width / 2, 0,0])
                    chamfer(chamfer_radius, depth + 1);
                translate([width / 2, 0,0])
                    chamfer(chamfer_radius, depth + 1);                
            }        
        }

        spacer_height = 4;

        translate([0, (height + spacer_height) / 2, 0])
            cube([width, spacer_height, depth], center = true);

        translate([0, (height + base_depth + spacer_height) / 2, (base_height - depth) / 2])
            rotate([90,0,0])
                base();

        // Chamfers
        translate([0, -width / 2 + chamfer_radius, 0]){
            translate([-width / 2 + chamfer_radius, 0,0])
                chamfer(chamfer_radius, depth);
            translate([width / 2 - chamfer_radius, 0,0])
                chamfer(chamfer_radius, depth);
        }

        logo_diameter = 7;
        logo_x_offset = (width - logo_diameter) / 2;
        logo_y_offset = (height + spacer_height) / 2;
        logo_z_offset = (base_height - depth) / 2 + depth / 2;

        translate([logo_x_offset - 3, logo_y_offset, logo_z_offset])
            rotate([90,180,00])
                logo(logo_diameter, 1.2);
    }
}

main();

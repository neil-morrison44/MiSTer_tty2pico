$viewable_radius = 32.6 / 2;
$screen_radius = 36 / 2;
$pcb_radius = 38 / 2;
$pcb_chin_width = 17;
$pcb_height = 1.62;
$screen_height = 1.86;
$case_height = 6.6;

include <../shared.scad>

module pcb() {
    $fa = 0.1;
    hull() {
        translate([ 0, 0, $screen_height ]) {
            cylinder(h = $pcb_height + $case_height, r = $pcb_radius,
                     center = false);
            translate([ 0, 0, (($pcb_height + $case_height) / 2) ]) cube(
                [
                    $pcb_chin_width, $pcb__max_radius * 2, $pcb_height +
                    $case_height
                ],
                true);
        }
    }
}

module screen() {
    difference() {
        translate([ 0, 0, -0.01 ]) {
            rotate([ 90, 0, 180 ]) {
                scale([ 26.3, 28, 26.3 ]) {
                    import("../../cad_models/GC9A01.stl", convexity = 3);
                }
            }
        }

        // slight angle on the display's chin missing from the stl
        translate([ 0, $viewable_radius + 2, -0.35 ]) {
            rotate([ 5, 0, 0 ]) { cube([ 20, 6, 1 ], true); }
        }
    }
}

module device() {
    hull() {
        screen();
        translate([ 0, 0, 2 ]) screen();
    }
    pcb();
    $fa = 0.1;
    cylinder(r = $viewable_radius, h = 10, center = true);
}

module case () {
    $fa = 0.1;
    cylinder(h = $case_height, r = $pcb__max_radius - 2, center = false);
}

module screwHole() {
    $fs = 0.1;
    translate([ 0, 0, -1 ]) cylinder(h = $case_height + 2, r = 1.5);
}

module cableCutout() {
    scale([1.5,1,1])
    translate([ 0, -20, 8.75 ]) rotate([ 90, 0, 0 ])
        cylinder(h = 25, r = 7.5, center = true);
}

difference() {
  case();
  translate([0,0,1]){
#device();

  }

  for ( x = [-($screw_distance), ($screw_distance)] ){
    translate([x, 0, 0])    screwHole();}

    cableCutout();
    }

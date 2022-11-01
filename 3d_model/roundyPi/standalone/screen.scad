$viewable_radius = 32.4 / 2;
$pcb_radius = 36.6 / 2;
$pcb__max_radius = 24.1;
$pcb_chin_width = 17;
$pcb_height = 1.62;
$screen_height = 2;

module pcb() {
    $fa = 0.1;
    hull() {
        translate([ 0, 0, $screen_height ]) {
            cylinder(h = $pcb_height, r = $pcb_radius, center = true);
            cube([ $pcb_chin_width, $pcb__max_radius * 2, $pcb_height ], true);
        }
    }
}

module screen() {
    difference() {
        translate([ 0, 0, -0.01 ]) {
            rotate([ 90, 0, 180 ]) {
                scale([ 25, 25, 25 ]) {
                    import("../../cad_models/GC9A01.stl", convexity = 3);
                }
            }
        }

        // slight angle on the display's chin missing from the stl
        translate([ 0, $viewable_radius + 2, -0.35 ]) {
            rotate([ 3, 0, 0 ]) { cube([ 20, 6, 1 ], true); }
        }
    }
}

module device() {
    hull() {
        screen();
        pcb();
    }
}

module case () {
  $fa = 0.1;
  cylinder(h = 2.8, r = $pcb__max_radius + 2, center = false);
}

module screwHole(){
  $fs = 0.1;
  cylinder(h = 10, r = 1.5, center = true);
}

difference() {
  case();
  device();

  for ( x = [-($pcb__max_radius - 2), ($pcb__max_radius - 2)] ){
    translate([x, 0, 0])
    screwHole();
}
}

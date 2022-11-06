$fn = 360;

// use <../standalone/screen.scad>;

$pcb__max_radius = 26;
$screw_distance = $pcb__max_radius - 5;

$screw_radius = 2.5 / 2;

module cover() {
    translate([ 119.75, 195, 14.25 ])
        import("../../cad_models/dust_cover_fixed.stl", convexity = 5);
}

module cableHole() {
    $length = 5;
    translate([ 0, 10, 0 ]) hull() {
        for (x = [ -$length, $length ]) {
            translate([ x, 0, 0 ]) cylinder(r = 5, h = 100, center = true);
        }
    }
}

module arm(sd_holder) {
    $width = 8;
    $height = 25;
    cube([ $width, $height - ($width), 5 ], true);
    translate([ 0, -($height / 2 - $width / 2), -7.5 ]) difference() {
        hull() {
            cylinder(r = $width / 2, h = 5 + 5, center = false);

            if (sd_holder) {
                translate([ -20, 0, 0 ])
                    cylinder(r = $width / 1.5, h = 5, center = false);
            }
        }

        cylinder(r = $screw_radius, h = 15, center = true);
    }
}

module arms() {
    for (x = [ -$screw_distance, $screw_distance ]) {
        rotate([ -18, 0, 0 ]) translate([ x, -17.5, 3 ]) arm(x > 0);
    }
}

module cover_with_arms() {
    difference() {
        cover();
        cableHole();
    }

    arms();
}

cover_with_arms();

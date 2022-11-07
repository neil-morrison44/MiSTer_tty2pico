$fn = 360;

include <../shared.scad>;

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

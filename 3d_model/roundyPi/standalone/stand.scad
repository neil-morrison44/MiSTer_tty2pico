$fn = 360;

$pcb__max_radius = 26;
$screw_distance = $pcb__max_radius - 5;

include <../shared.scad>;

module arms() {
    translate(
        [ 0, -2.338, 5.5 ]) for (x = [ -$screw_distance, $screw_distance ]) {
        rotate([ -108, 0, 0 ]) translate([ x, -17.5, 0 ]) arm(x > 0);
    }
}

arms();

hull() {
    translate([ -($screw_distance + 4), -1.88, 0 ])

        cube([ ($screw_distance * 2) + 8, 5, 15 ]);

    translate([ -5, -8, 0 ]) cube([ 10, 30, 1 ]);
}

$fn = 360;

$pcb__max_radius = 26;
$screw_distance = $pcb__max_radius - 5;

include <../shared.scad>;

module arms() {
    translate(
        [ 0, -2.338, 10.5 ]) for (x = [ -$screw_distance, $screw_distance ]) {
        rotate([ -108, 0, 0 ]) translate([ x, -17.5, 0 ]) arm(x > 0);
    }
}

arms();

hull() {
    translate([ -($screw_distance + 4), -1.88, 0 ])

        cube([ ($screw_distance * 2) + 8, 5, 20 ]);

    translate([ -10, -12, 0 ]) cube([ 20, 40, 1 ]);
}

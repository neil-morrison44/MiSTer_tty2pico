$fn = 360;

$pcb__max_radius = 26;
$screw_distance = $pcb__max_radius - 5;

$screw_radius = 2.5 / 2;

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

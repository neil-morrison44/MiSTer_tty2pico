$pcb__max_radius = 26;
$screw_distance = $pcb__max_radius - 5;
$screw_radius = 2.5 / 2;

module arm(sd_holder) {
    $width = 8;
    $height = 25;
    cube([ $width, $height - ($width), 5 ], true);
    translate([ 0, -($height / 2 - $width / 2), -7.5 ]) difference() {
        union() {
            hull() {
                cylinder(r = $width / 2, h = 5 + 5, center = false);

                if (sd_holder) {
                    translate([ -17.5, 0, 0 ])
                        cylinder(r = $width / 1.5, h = 2, center = false);
                }
            }
            if (sd_holder) {
                translate([ -17.5, 0, -0.5 ])
                    cylinder(r = $width / 1.5, h = 2, center = false);
            }
        }

        cylinder(r = $screw_radius, h = 15, center = true);
    }
}

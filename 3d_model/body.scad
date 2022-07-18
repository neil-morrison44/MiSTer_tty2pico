$pico_width = 21;
$dislay_radius = 32.512 / 2;

$face_radius = $dislay_radius + 5;
$front_thickness = 5;

$wiggle_room_scale = 1.03;

$screw_radius = 3 / 2;

$pico_screw_radius = 1.8 / 2;

$lean = 70;

module pico() {
    translate([ $pico_width / 2, -2, 0 ]) rotate([ 0, 180, 0 ])
        rotate([ 90, 0, 0 ]) import("./cad_models/pico.stl", convexity = 3);
}

module lcd() {
    translate([ 0, 0, 35 ]) rotate([ $lean, 0, 0 ])
        scale([ $wiggle_room_scale, $wiggle_room_scale, $wiggle_room_scale ])
            import("./cad_models/lcd.stl", convexity = 3);
}

module screwHole() {
    $fn = 12;
    cylinder(h = 100, r = $screw_radius * 2);
    cylinder(h = 100, r = $screw_radius, center = true);
}

module front() {
    difference() {
        hull() {
            cylinder(h = $front_thickness, r1 = $face_radius,
                     r2 = $face_radius);

            translate([ 0, -20, $front_thickness / 2 ]) {
                cube([ $face_radius * 2, $face_radius * 2, $front_thickness ],
                     true);
            }
        }
        translate([ 0, 0, $front_thickness ]) {
            $fn = 360;
            $fs = 0.1;
            cylinder(h = $front_thickness * 2, r = $dislay_radius + 1,
                     center = true);
        }

        $screwX = $dislay_radius * 0.666;
        for (x = [ -$screwX, $screwX ]) {
            translate([ x, -30, $front_thickness / 2 ]) screwHole();
        }
    }
}

module base(){
    difference(){
        union(){
    translate([0,20,-1]){
        difference(){ 
    cube([$face_radius * 2,60,10], true);
        translate([0,0,1])
        cube([$pico_width + 0.5, 65, 2], true);
        translate([0,0,10])
        cube([$pico_width - 0.25, 65, 21.5], true);
        cube([9, 65, 5], true);
            
            $screwX = (11.4 / 2);
            for (x = [-$screwX, $screwX]){
            $fn = 12;
            translate([x , (48.26 /2) + 3, 0])
              #cylinder(100, r=$pico_screw_radius, center=true);
            }
    }
    
}

hull(){
$fn = 36;
        $screwX = $dislay_radius * 0.666;
        for (x = [ -$screwX, $screwX ]) {
            translate([ x, -5.5, 5])
            rotate([$lean,0,0])
            translate([0,0,-2]) 
            cylinder(7, r1=$screw_radius * 2, r2=$screw_radius * 4);
        }
        
        translate([0,-6,-5]) 
        cube([$face_radius * 2, 6, 1], true);

}
}
        $screwX = $dislay_radius * 0.666;
        for (x = [ -$screwX, $screwX ]) {
            $fn = 36;
            translate([ x, -5.5, 5])
            rotate([$lean,0,0])
            translate([0,0,5]) 
            cylinder(15, r=$screw_radius - 0.25, center = true);
        }



}
}


base();

difference() {
    translate([ 0, 0, 35 ]) rotate([ $lean, 0, 0 ]) front();
    hull() { lcd(); }
}

pico();

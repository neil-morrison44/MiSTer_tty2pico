$fn = 360;

// use <../standalone/screen.scad>;

$pcb__max_radius = 26;
$screw_distance = $pcb__max_radius - 5;

module cover() {
    translate([ 119.75, 195, 14.25 ])
        import("../../cad_models/dust_cover.stl", convexity = 3);
}

module cableHole() {
    $length = 5;
    hull() {
        for (x = [ -$length, $length]) {
            translate([ x, 0, 0 ]) cylinder(r = 2.5, h = 100, center = true);
        }
    }
}

module arm(){
  $width = 10;
  $height = 25;
  cube([$width,$height,5], true);
  translate([0, -($height/2 - $width/2), -5])
  difference(){
  cylinder(r=$width/2, h=5, center=true);
  cylinder(r=1.25, h=15, center=true);
  }
}

module arms(){


  for (x=[-$screw_distance, $screw_distance]){
    rotate([-18,0,0])
    translate([x, -20,3])
    arm();
  }

}

difference() {
    cover();
    cableHole();
}

arms();

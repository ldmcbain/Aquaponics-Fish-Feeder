$fa=1;
$fs=1;

in=25.4;
th=5;
angle=-125;

%rotate([0,0,-18])
translate([0,0,th])
import("entire feeder.stl",convexity=4);
bracket();


module bracket()
{
    inner=115;
    difference()
    {
        cylinder(d=inner+th,h=0.5*in);
        translate([0,0,th])
        cylinder(d=inner,h=1*in);
        
        *translate([10,-inner/4,-th])
        cube([inner,inner/4,1*in]);
        
        rotate([0,0,-9])
        translate([0,0,-20])
        hull()
        {
            translate([10,0,0])
            cube([100,0.1,100]);
            rotate([0,0,-54])
            translate([10,0,0])
            cube([100,0.1,100]);
        }
        
        rotate([0,0,angle+90])
        translate([0,-50-inner/2-th/3,0])
        cube([100,100,600],center=true);
        
    
    translate([15,-10,th])
    cube([100,100,100]);
    }
    
    rotate([0,0,-18])
    translate([-inner/2-0.5,-30/2,0])
    cube([th/2,30,0.5*in]);
    
    rotate([0,0,angle])
    translate([0,-15,0])
    difference()
    {
        translate([th/3,-(1*in+th*3)/2,-4.99*in])
        cube([(inner)/2,1*in+th*3,5*in]);
        cube([(inner)*2,1*in,50*in],center=true);
    }
}
$fa=1;
$fs=1;
in=25.4;


difference()
{
    union()
    {//making small on purpose to reduce friction along edges
    cylinder(d=94,h=15);
    cylinder(d=20,h=35);
    }
    
    for(i=[0:10])
    {
        rotate([0,0,i*360/10])
        translate([34,0,25])
        {
        //scale([1,1,2])
        //sphere(d=24);
            
        cylinder(d1=0.375*in,d2=33,h=100,center=true);
        }
    }
    
    translate([0,0,-0.01])
    cylinder(d=9.5,h=30);
    
    intersection()
    {
        cylinder(d=5.5,h=100);
        cube([10,3.5,100],center=true);
    }
    
}
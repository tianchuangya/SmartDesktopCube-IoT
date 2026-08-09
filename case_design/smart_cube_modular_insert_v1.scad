
// Smart Desktop Cube modular insert v1
// Units: mm. Axis: X=depth, Y=width, Z=height.
// Original mzh51.stl bounds measured approximately 70.4 x 87.5 x 86.7 mm.
// This insert is intentionally smaller: about 72 x 85 x 90 including tabs, trim parameters after test print.

$fn = 32;
module box(c, s) { translate(c) cube(s, center=true); }
module tube_z(c, ro, ri, h) { translate(c) difference(){ cylinder(h=h, r=ro, center=true); cylinder(h=h+0.2, r=ri, center=true); } }

// Main dimensions
base_depth = 64;
base_width = 80;
base_thickness = 2.5;

// Recommended module assumptions:
// ESP32-S3 DevKitM-1: 55 x 28 mm, M2/M2.3 pilot holes or zip ties.
// ILI9341 2.4 touch display board: about 77 x 42 mm, mounted on front Y-Z plane.
// AHT21 / ENS160 / BH1750: small breakout boards in top rear slide slots.
// LD2410B: front/top bay, keep antenna side exposed, avoid metal screws in front.

union(){
  box([0,0,1.25], [64,80,2.5]);
  box([0,-40.5,5], [62,2.4,7.5]);
  box([0,40.5,5], [62,2.4,7.5]);
  box([31,0,16], [2.4,78,28]);
  box([31,0,49], [2.4,78,4]);
  // Add/edit detailed rails in STL generator script for now.
}

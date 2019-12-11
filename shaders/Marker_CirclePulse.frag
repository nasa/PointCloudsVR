// FRAGMENT SHADER
// Draws a pulsing circle (not filled in)
// Pulsing happens at 1Hz, using the equation
//   radius = rad_min + (rad_max - rad_min)*|sin(PI*time)|

// Use GLSL 1.20 (OpenGL 2.1)
#version 120

uniform float osg_FrameTime;
vec2 v;
float dist2;
float rad2;
float thickness = 0.09; 
const vec2 rad2_range = vec2(0.0625, 0.25);
const float PI = 3.1415926536;

void main(void)
{
  // Move origin to point center
  v = gl_PointCoord - vec2(0.5);

  // Compute square of fragment distance from center
  // We use square to avoid sqrt computation
  dist2 = dot(v,v);

  // Compute current radius
  rad2 = rad2_range.x + (rad2_range.y - rad2_range.x)*abs(sin(PI*osg_FrameTime));

  // Discard fragments not on the circle border
  if((dist2 > rad2) || (dist2 < (rad2 - thickness)))
  {
    discard;
  }

  gl_FragColor = gl_Color;
}

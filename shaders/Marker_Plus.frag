// FRAGMENT SHADER
// Draws a plus centered on the origin

// Use GLSL 1.20 (OpenGL 2.1)
#version 120

vec2 dist;

void main(void)
{
  // Compute distance to x/y axes
  dist = abs(gl_PointCoord - vec2(0.5));

  // Discard fragments too far from axes
  if((dist.x > 0.1) && (dist.y > 0.1))
  {
    discard;
  }

  gl_FragColor = gl_Color;
}

// FRAGMENT SHADER
// Draws a square (not filled in)

// Use GLSL 1.20 (OpenGL 2.1)
#version 120

void main(void)
{
  // Discard fragments not on the square border
  if((gl_PointCoord.x > 0.1) && (gl_PointCoord.x < 0.9)
     && (gl_PointCoord.y > 0.1) && (gl_PointCoord.y < 0.9))
  {
    discard;
  }

  gl_FragColor = gl_Color;
}

#version 120

void main(void)
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	//gl_PointSize = 10;
	gl_Color = vec4(1, 0, 0, 1);
}

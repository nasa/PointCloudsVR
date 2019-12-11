#version 330

uniform mat4 vm;

in vec3 norm;
in vec4 pos;

out vec4 fragcolor;

void main()
{
  vec3 n = normalize(mat3(vm) * norm);			// convert normal to view space, vm (view matrix), is a rigid body transform.
  vec3 p = vec3(vm * pos);						// position in view space
  vec3 v = normalize(-p);							// eye vector
  float vdn = 1.0 - max(dot(v, n), 0.0);		// the rim contribution

  fragcolor.a = 1.0;
  //fragcolor.rgb = vec3(0.15, 0.35, 0.8) * vec3(smoothstep(0.6, 1.0, vdn));
  fragcolor.rgb = pos.xyz;
}

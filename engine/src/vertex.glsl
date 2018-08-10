#version 300 es

// Input vertex data, different for all executions of this shader
in vec2 a_position;
in vec2 a_texCoord;

// Output data; will be interpolated for each fragment
out vec2 v_texCoord;
out vec4 v_position;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

void main()
{
  gl_Position = MVP * vec4(a_position, 0, 1);
  v_texCoord = a_texCoord;
  v_position = gl_Position;
}
// vim: syntax=glsl

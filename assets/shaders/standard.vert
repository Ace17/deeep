#version 300 es

// Attributes
in vec2 vertexPos_model;
in vec2 vertexUV;

// Output data; will be interpolated for each fragment
out vec4 vertexPos_world;
out vec2 UV;

void main()
{
  gl_Position = vec4(vertexPos_model, 0, 1);
  UV = vertexUV;
  vertexPos_world = gl_Position;
}
// vim: syntax=glsl

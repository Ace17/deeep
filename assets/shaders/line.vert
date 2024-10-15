#version 300 es

// Attributes
in vec2 attr_pos;
in vec2 attr_uv;
in vec4 attr_color;

// Output data; will be interpolated for each fragment
out vec2 vertexUV;
out vec4 vertexColor;

void main()
{
  gl_Position = vec4(attr_pos, 0, 1);
  vertexColor = attr_color;
  vertexUV = attr_uv;
}
// vim: syntax=glsl

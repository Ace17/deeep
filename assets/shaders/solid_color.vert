#version 300 es

in vec2 attr_pos;
in vec2 attr_uv;
in vec4 attr_color;

out vec2 UV;
out vec4 vertexColor;

void main()
{
  gl_Position = vec4(attr_pos, 0, 1);
  vertexColor = attr_color;
  UV = attr_uv;
}

// vim: syntax=glsl

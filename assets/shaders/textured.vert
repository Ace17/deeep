#version 300 es

in vec2 attr_pos;
in vec2 attr_uv;

out vec2 UV;

void main()
{
  gl_Position = vec4(attr_pos, 0, 1);
  UV = attr_uv;
}

// vim: syntax=glsl

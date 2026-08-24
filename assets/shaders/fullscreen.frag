#version 300 es

precision mediump float;

uniform sampler2D DiffuseTextureSampler;
in vec2 UV;
out vec4 color;

void main()
{
  color = texture(DiffuseTextureSampler, UV);
}

// vim: syntax=glsl

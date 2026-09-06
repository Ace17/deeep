#version 300 es

precision mediump float;

uniform sampler2D DiffuseTextureSampler;

// Interpolated values from the vertex shader
in vec2 UV;
in vec4 vertexColor;

// Ouput data
out vec4 color;

void main()
{
  color = texture(DiffuseTextureSampler, UV) + vertexColor;
}

// vim: syntax=glsl

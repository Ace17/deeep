#version 300 es

precision mediump float;

// Uniforms
uniform vec4 fragOffset;
uniform sampler2D DiffuseTextureSampler;

// Interpolated values from the vertex shader
in vec4 vertexPos_world;
in vec2 UV;

// Ouput data
out vec4 color;

void main()
{
  color = texture(DiffuseTextureSampler, UV) + fragOffset;
}

// vim: syntax=glsl

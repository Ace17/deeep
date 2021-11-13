#version 310 es

precision mediump float;

// Uniforms
layout(location = 0) uniform vec4 fragOffset;
layout(location = 1) uniform sampler2D DiffuseTextureSampler;

// Interpolated values from the vertex shader
layout(location = 0) in vec4 vertexPos_world;
layout(location = 1) in vec2 UV;

// Ouput data
layout(location = 0) out vec4 color;

void main()
{
  color = texture(DiffuseTextureSampler, UV) + fragOffset;
  color += vec4(vertexPos_world.y, vertexPos_world.y, vertexPos_world.y, 0) * 0.2;
}

// vim: syntax=glsl

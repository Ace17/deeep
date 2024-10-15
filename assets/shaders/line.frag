#version 300 es

precision mediump float;

// Uniforms
layout(std140) uniform MyUniformBlock
{
  vec4 fragOffset;
};

// Interpolated values from the vertex shader
in vec2 vertexUV;
in vec4 vertexColor;

// Ouput data
out vec4 color;

float sqr(float v)
{
  return v * v;
}

void main()
{
  color = vertexColor;
  color.a *= min(1.0, sqr(sqr(1.0-abs(vertexUV.x))));
}

// vim: syntax=glsl

#version 300 es

precision mediump float;

// Interpolated values from the vertex shader
in vec2 v_texCoord;
in vec4 v_position;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh
uniform vec4 v_color;
uniform sampler2D s_baseMap;

void main()
{
  color = texture2D(s_baseMap, v_texCoord) + v_color;
  color += vec4(v_position.y, v_position.y, v_position.y, 0) * 0.2;
}

// vim: syntax=glsl

#version 100
precision mediump float;
varying vec2 v_texCoord;
uniform vec4 v_color;
uniform sampler2D s_baseMap;

void main()
{
  gl_FragColor = texture2D(s_baseMap, v_texCoord) + v_color;
}

// vim: syntax=glsl

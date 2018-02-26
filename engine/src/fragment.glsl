#version 100
precision mediump float;
varying vec2 v_texCoord;
varying vec4 v_position;
uniform vec4 v_color;
uniform sampler2D s_baseMap;

void main()
{
  gl_FragColor = texture2D(s_baseMap, v_texCoord) + v_color;
  gl_FragColor += vec4(v_position.y, v_position.y, v_position.y, 0) * 0.2;
}

// vim: syntax=glsl

#version 100

attribute vec4 a_position;
attribute vec2 a_texCoord;
varying vec2 v_texCoord;
varying vec4 v_position;
uniform mat4 MVP;

void main()
{
  gl_Position = MVP * a_position;
  v_texCoord = a_texCoord;
  v_position = gl_Position;
}
// vim: syntax=glsl

#version 100

attribute vec4 a_position;
attribute vec2 a_texCoord;
varying vec2 v_texCoord;
uniform mat4 MVP;
uniform mat4 Scale;

void main()
{
  gl_Position = Scale * MVP * a_position;
  v_texCoord = a_texCoord;
}
// vim: syntax=glsl

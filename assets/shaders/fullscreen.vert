#version 300 es

out vec2 UV;

void main()
{
  // Generates a triangle covering (-1, -1) to (3, 3)
  float x = -1.0 + float((gl_VertexID & 1) << 2);
  float y = -1.0 + float((gl_VertexID & 2) << 1);

  gl_Position = vec4(x, y, 0.0, 1.0);

  UV = vec2((x + 1.0) * 0.5, (y + 1.0) * 0.5); 
}
// vim: syntax=glsl

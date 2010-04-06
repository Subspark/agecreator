uniform mat4 plasma_matrix;

void main(void)
{
  gl_Position = gl_ModelViewProjectionMatrix * (plasma_matrix * gl_Vertex);
  gl_FrontColor = gl_Color;
}
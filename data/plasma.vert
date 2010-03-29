uniform mat4 plasma_matrix;
uniform vec4 layer_color;

void main(void)
{
  gl_Position = gl_ModelViewProjectionMatrix * (plasma_matrix * gl_Vertex);
  gl_FrontColor = gl_Color.bgra * layer_color; // Swizzle from DX order to GL order
  gl_TexCoord[0] = gl_MultiTexCoord0;
}
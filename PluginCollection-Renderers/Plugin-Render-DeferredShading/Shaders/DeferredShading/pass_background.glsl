#version 150

uniform vec4 bkColor;

in vec2 vTexCoord;
out vec4 outColor;

void main() 
{
  outColor = bkColor;
  
  gl_FragDepth = 1.0;
}

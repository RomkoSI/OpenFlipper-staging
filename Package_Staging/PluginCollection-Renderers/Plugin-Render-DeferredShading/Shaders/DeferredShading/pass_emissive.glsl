#version 150

#include "GBufferAccess.glsl"
#include "depthbuffer.glsl"

uniform sampler2D samplerPos;


uniform samplerBuffer samplerMaterial;

uniform vec4 projParams;
uniform vec2 clipPlanes;

in vec2 vTexCoord;
out vec4 outColor;



void main() 
{
  // read G-buffer
  vec3 cEmissive;
  
  int matID = int(floor(texture2D(samplerPos, vTexCoord).y));
  cEmissive = texelFetch(samplerMaterial, matID * 5).xyz;

  
  outColor = vec4(cEmissive, 0);
  
//  outColor = vec4(float(matID) / 100, float(matID) / 100, float(matID) / 100, 1.0);
  
  // compute depth buffer value
  gl_FragDepth = 1.0;
}

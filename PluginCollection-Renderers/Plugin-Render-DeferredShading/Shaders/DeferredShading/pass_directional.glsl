#version 150

#include "GBufferAccess.glsl"
#include "depthbuffer.glsl"

uniform sampler2D samplerPos;
uniform sampler2D samplerNormal;


uniform samplerBuffer samplerMaterial;

uniform vec3 lightDir;

uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

uniform vec4 projParams;
uniform vec2 clipPlanes;

in vec2 vTexCoord;
out vec4 outColor;



void main() 
{
  // read G-buffer
  vec3 cEmissive;
  vec3 cAmbient;
  vec3 cDiffuse;
  vec4 cSpecular;
  
  float shininess;
  
  int matID = int(floor(texture2D(samplerPos, vTexCoord).y));
  cEmissive = texelFetch(samplerMaterial, matID * 5).xyz;
  cAmbient = texelFetch(samplerMaterial, matID * 5 + 1).xyz * lightAmbient;
  cDiffuse = texelFetch(samplerMaterial, matID * 5 + 2).xyz * lightDiffuse;
  cSpecular.xyz = texelFetch(samplerMaterial, matID * 5 + 3).xyz * lightSpecular;

  shininess = texelFetch(samplerMaterial, matID * 5 + 4).y;
  

  vec3 fragPos = ReadPositionVS(samplerPos, vTexCoord, projParams.x, projParams.y);
  vec3 fragNormal = ReadNormalVS(samplerNormal, vTexCoord);

  
//  vec3 fragColor = cEmissive + cAmbient;
  vec3 fragColor = cAmbient;
  
  // lighting
 
  // diffuse
  float ldotn = clamp(dot(lightDir, fragNormal), 0, 1);
  
  fragColor += ldotn * cDiffuse;
  
  // specular
  vec3 h = normalize(lightDir - vec3(0,0,-1));
  float hdotn = max(dot(h, fragNormal), 0);
  
//  hdotn = max(0, dot( reflect(lightDir, fragNormal), vec3(0,0,-1) ) );
  
  fragColor += pow(hdotn, shininess) * cSpecular.xyz;
  
  // saturate
  fragColor = clamp(fragColor, 0, 1);
  
  outColor = vec4(fragColor, 1);
  
//  outColor = vec4(float(matID) / 100, float(matID) / 100, float(matID) / 100, 1.0);

  
  // compute depth buffer value
  gl_FragDepth = ViewspaceToProjectedDepth(fragPos.z, projParams.z, projParams.w);
}

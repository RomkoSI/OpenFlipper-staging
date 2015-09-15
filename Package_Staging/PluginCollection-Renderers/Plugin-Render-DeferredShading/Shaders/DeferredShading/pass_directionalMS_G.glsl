// version with multisampling of G-buffer (no lighting MSAA)

#version 150

#include "GBufferAccess.glsl"
#include "multisampling.glsl"
#include "depthbuffer.glsl"

uniform sampler2DMS samplerPos;
uniform sampler2DMS samplerNormal;

uniform samplerBuffer samplerMaterial;

uniform vec3 lightDir;

uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

uniform vec4 projParams;

uniform int numSamples;
uniform samplerBuffer samplerFilterWeights;

in vec2 vTexCoord;
out vec4 outColor;



void main() 
{

  ivec2 texSize = textureSize(samplerPos);
  
  // absolute pixel index
  ivec2 absIndex = ivec2(floor( texSize * vTexCoord ));

  // read G-buffer
  vec3 cEmissive;
  vec3 cAmbient;
  vec3 cDiffuse;
  vec4 cSpecular;
  
  float shininess;
  
  int matID = int(floor(texelFetch(samplerPos, absIndex, 0).y));
  cEmissive = texelFetch(samplerMaterial, matID * 5).xyz;
  cAmbient = texelFetch(samplerMaterial, matID * 5 + 1).xyz * lightAmbient;
  cDiffuse = texelFetch(samplerMaterial, matID * 5 + 2).xyz * lightDiffuse;
  cSpecular.xyz = texelFetch(samplerMaterial, matID * 5 + 3).xyz * lightSpecular;

  shininess = texelFetch(samplerMaterial, matID * 5 + 4).y;
  
  float depthVS = ReadMultisampledTexture(samplerPos, absIndex, numSamples, samplerFilterWeights).x;
  vec3 fragPos = ComputePosVS(depthVS, vTexCoord, projParams.x, projParams.y);
  vec3 fragNormal = ReadMultisampledTexture(samplerNormal, absIndex, numSamples, samplerFilterWeights).xyz;

  fragNormal = normalize( fragNormal * 2 - vec3(1,1,1) );
  
  vec3 fragColor = cEmissive + cAmbient;
  
  // lighting
 
  // diffuse
  float ldotn = clamp(dot(lightDir, fragNormal), 0, 1);
  
  fragColor += ldotn * cDiffuse;
  
  // specular
  vec3 h = normalize(lightDir - vec3(0,0,-1));
  float hdotn = max(dot(h, fragNormal), 0);
  
  fragColor += pow(hdotn, shininess) * cSpecular.xyz;
  
  // saturate
  fragColor = clamp(fragColor, 0, 1);
  
  outColor = vec4(fragColor, 1);
  
  gl_FragDepth = ViewspaceToProjectedDepth(fragPos.z, projParams.z, projParams.w);
}

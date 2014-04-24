#version 150

#include "GBufferAccess.glsl"
#include "depthbuffer.glsl"

uniform sampler2D samplerPos;
uniform sampler2D samplerNormal;


uniform samplerBuffer samplerMaterial;

uniform vec3 lightPos;
uniform vec3 lightAtten;

uniform vec3 lightSpotDir;
uniform vec2 lightSpotParam; // angle, falloff exp

uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;


uniform vec4 projParams;

in vec2 vTexCoord;
out vec4 outColor;



void main() 
{
  // read material buffer
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
  
  // read G-buffer
  vec3 fragPos = ReadPositionVS(samplerPos, vTexCoord, projParams.x, projParams.y);
  vec3 fragNormal = ReadNormalVS(samplerNormal, vTexCoord);

  
  vec3 fragColor = cEmissive + cAmbient;
  
  // lighting 
 
  vec3 cLight;
 
  vec3 lightDir = lightPos - fragPos; 
  float lightDist2 = dot(lightDir, lightDir);
  float lightDist = sqrt(lightDist2);
  
  lightDir = lightDir / lightDist;
 
  // diffuse
  float ldotn = clamp(dot(lightDir, fragNormal), 0, 1);
  
  cLight = ldotn * cDiffuse;
  
  // specular
  vec3 h = normalize(lightDir - vec3(0,0,-1));
  float hdotn = max(dot(h, fragNormal), 0);
  
  hdotn = max(0, dot( reflect(lightDir, fragNormal), vec3(0,0,-1) ) );
  
  cLight += pow(hdotn, shininess) * cSpecular.xyz;
  
  // attenuate
  float att = lightAtten.x + lightAtten.y * lightDist + lightAtten.z * lightDist2;
  
  // spot factor
  float spot = -dot(lightDir, lightSpotDir);
  spot *= step(lightSpotParam.x, spot);
  spot *= pow(spot, lightSpotParam.y);
  
  cLight *= spot/att;
  
  // saturate
  cLight = clamp(cLight, 0, 1);
  fragColor += cLight;
  
  fragColor = clamp(fragColor, 0, 1);
  
  
  // output
  outColor = vec4(fragColor, 1);
  
  // todo: compute projected depth
  gl_FragDepth = ViewspaceToProjectedDepth(fragPos.z, projParams.z, projParams.w);
}

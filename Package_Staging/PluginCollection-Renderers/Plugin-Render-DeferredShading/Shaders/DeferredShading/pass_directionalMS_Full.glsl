// version with multisampling

#version 150

#include "GBufferAccess.glsl"
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
  
  
  vec3 fragColorMS = vec3(0,0,0);
  vec3 fragPosMS = vec3(0,0,0);
  
  for (int i = 0; i < numSamples; ++i)
  {
  
    // read G-buffer
    vec3 cEmissive;
    vec3 cAmbient;
    vec3 cDiffuse;
    vec4 cSpecular;
  
    float shininess;
  
    vec2 depthMatID = texelFetch(samplerPos, absIndex, i).xy;
  
    int matID = int(floor(depthMatID.y));
    cEmissive = texelFetch(samplerMaterial, matID * 5).xyz;
    cAmbient = texelFetch(samplerMaterial, matID * 5 + 1).xyz * lightAmbient;
    cDiffuse = texelFetch(samplerMaterial, matID * 5 + 2).xyz * lightDiffuse;
    cSpecular.xyz = texelFetch(samplerMaterial, matID * 5 + 3).xyz * lightSpecular;

    shininess = texelFetch(samplerMaterial, matID * 5 + 4).y;
  
    float depthVS = depthMatID.x;
    vec3 fragPos = ComputePosVS(depthVS, vTexCoord, projParams.x, projParams.y);
    vec3 fragNormal = texelFetch(samplerNormal, absIndex, i).xyz;

    fragNormal = normalize( fragNormal * 2 - vec3(1,1,1) );
  
    vec3 sampleColor = cEmissive + cAmbient;
  
    // lighting
 
    // diffuse
    float ldotn = clamp(dot(lightDir, fragNormal), 0, 1);
  
    sampleColor += ldotn * cDiffuse;
  
    // specular
    vec3 h = normalize(lightDir - vec3(0,0,-1));
    float hdotn = max(dot(h, fragNormal), 0);
  
    sampleColor += pow(hdotn, shininess) * cSpecular.xyz;
  
    // saturate
	sampleColor = clamp(sampleColor, 0, 1);
	
	
	// sample weighting
	float weight = texelFetch(samplerFilterWeights, i).x;
	
    fragColorMS += weight * sampleColor;
	fragPosMS += weight * fragPos;
  }

  
  
  // saturate
  fragColorMS = clamp(fragColorMS, 0, 1);
  
  outColor = vec4(fragColorMS, 1);
  
  gl_FragDepth = ViewspaceToProjectedDepth(fragPosMS.z, projParams.z, projParams.w);
}

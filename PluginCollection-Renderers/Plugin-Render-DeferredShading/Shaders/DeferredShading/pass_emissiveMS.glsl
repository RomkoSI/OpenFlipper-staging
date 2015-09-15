#version 150

#include "GBufferAccess.glsl"
#include "depthbuffer.glsl"

uniform sampler2DMS samplerPos;


uniform samplerBuffer samplerMaterial;

uniform vec4 projParams;
uniform vec2 clipPlanes;

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
  
  for (int i = 0; i < numSamples; ++i)
  {
  
    // read G-buffer
    vec3 cEmissive;

    vec2 depthMatID = texelFetch(samplerPos, absIndex, i).xy;
  
    int matID = int(floor(depthMatID.y));
    cEmissive = texelFetch(samplerMaterial, matID * 5).xyz;

    vec3 sampleColor = cEmissive;
  
	sampleColor = clamp(sampleColor, 0, 1);
	
	// sample weighting
	float weight = texelFetch(samplerFilterWeights, i).x;
	
    fragColorMS += weight * sampleColor;
  }

  
  outColor = vec4(fragColorMS, 0);
  
  gl_FragDepth = 1.0;
}

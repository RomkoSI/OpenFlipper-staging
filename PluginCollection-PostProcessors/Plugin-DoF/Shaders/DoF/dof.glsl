#version 430

/*
ref: http://http.developer.nvidia.com/GPUGems/gpugems_ch23.html
*/



#include "../Common/depthbuffer.glsl"

in vec2 vTexCoord;
out vec4 outColor;

layout(binding = 0) uniform sampler2D g_SAT;
layout(binding = 1) uniform sampler2D g_DepthTex;
layout(binding = 2) uniform sampler2D g_SceneTex;


uniform mat4 g_P;
uniform vec2 g_ClipPlanes;

vec4 textureBoxFilter(in sampler2D tex, in vec2 pos, in vec2 size)
{
  ivec2 iDim = ivec2(textureSize(tex, 0));

  ivec2 iPos = ivec2(iDim * pos);
  
  ivec2 iSize = ivec2(size);
  iSize = max(iSize, ivec2(1,1));
  
  ivec2 iPosMax = iPos + iSize;
  ivec2 iPosMin = iPos - iSize;
  
  // fix filtering at border
  ivec2 borderShift = max(iPosMax - iDim + ivec2(1,1), ivec2(0,0));
  iPosMax -= borderShift;
  iPosMin -= borderShift;

  // SAT lookup
  vec4 v = texelFetch(tex, iPosMax, 0);
  v -= texelFetch(tex, ivec2(iPosMin.x, iPosMax.y), 0);
  v -= texelFetch(tex, ivec2(iPosMax.x, iPosMin.y), 0);
  v += texelFetch(tex, iPosMin, 0);
  
  // div by area
  v /= float(iSize.x * iSize.y * 4);  // fast enough, but could be in float alu
  
  return v;
}

// input: depth in view space  (positive depth,  so  -posVS.z)
float ComputeCOC(float depthVS)
{
  // "Improved Depth-of-Field Rendering" Scheuermann and Tatarchuk (ShaderX 3)
  float f;
  
  float focusZ = mix(g_ClipPlanes.x, g_ClipPlanes.y, 0.4);
  
  if (depthVS < focusZ)
  {
    f = (depthVS - focusZ) / (focusZ - g_ClipPlanes.x);
  }
  else
  {
    f = (depthVS - focusZ) / (g_ClipPlanes.y - focusZ);
	f = clamp(f, 0, 1);
  }
  
//  return f * 0.5 + 0.5;


  // not based on a real model, but does not require param tweaking
  return pow(abs(depthVS - focusZ), 1.2) * 0.21;
}

void main()
{
  vec2 texSize = vec2(textureSize(g_SAT, 0));
  
  float depth = texture(g_DepthTex, vTexCoord).x;
  float zVS = ProjectedToViewspaceDepth(depth, g_P[2][2], g_P[3][2]);
 

  float coc = ComputeCOC(-zVS);
  
  vec4 col = vec4(0,0,0,0);
  
  if (coc <= 1.0)
    col = texture(g_SceneTex, vTexCoord);
  else
    col = textureBoxFilter(g_SAT, vTexCoord, vec2(coc, coc));
  
  
  
  
  outColor = col;
//  outColor = textureLod(g_SceneTex, vTexCoord, 0);
//  outColor = vec4(1,1,1,1) * fract(zVS);
//  outColor = vec4(1,1,1,1) * (-zVS);
}

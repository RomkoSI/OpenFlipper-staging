#version 130

// http://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf



in vec2 vTexCoord;
out vec4 outColor;


uniform sampler2D textureSampler;

// fxaa constants
uniform vec2 fxaaQualityRcpFrame;
uniform vec4 fxaaConsoleRcpFrameOpt;
uniform vec4 fxaaConsoleRcpFrameOpt2;
uniform vec4 fxaaConsole360RcpFrameOpt2;
uniform float fxaaQualitySubpix = 0.75;
uniform float fxaaQualityEdgeThreshold = 0.116;
uniform float fxaaQualityEdgeThresholdMin = 0.0833;
uniform float fxaaConsoleEdgeSharpness = 8.0;
uniform float fxaaConsoleEdgeThreshold = 0.125;
uniform float fxaaConsoleEdgeThresholdMin = 0.05;
uniform vec4 fxaaConsole360ConstDir = vec4(0,0,0,0);


// configure fxaa
#define FXAA_PC 1
#define FXAA_GLSL_130 1
#define FXAA_QUALITY__PRESET 39


#ifdef GL_ARB_gpu_shader5
  #extension GL_ARB_gpu_shader5 : enable
  #define FXAA_GATHER4_ALPHA 1
#else
  #define FXAA_GATHER4_ALPHA 0
#endif



#include "Fxaa3_11.h"

void main() 
{
  vec4 fxaaConsolePosPos = vec4(vTexCoord - 0.5 * fxaaQualityRcpFrame, vTexCoord + 0.5 * fxaaQualityRcpFrame);
  
  outColor = FxaaPixelShader(
    vTexCoord, 
    fxaaConsolePosPos,
    textureSampler,
    textureSampler,
    textureSampler,
    fxaaQualityRcpFrame,
    fxaaConsoleRcpFrameOpt,
    fxaaConsoleRcpFrameOpt2,
    fxaaConsole360RcpFrameOpt2,
    fxaaQualitySubpix,
    fxaaQualityEdgeThreshold,
    fxaaQualityEdgeThresholdMin,
    fxaaConsoleEdgeSharpness,
    fxaaConsoleEdgeThreshold,
    fxaaConsoleEdgeThresholdMin,
    fxaaConsole360ConstDir);
			
//  outColor = texture(textureSampler, vTexCoord).wwww;
//  outColor = vec4(1,1,1,1);
}

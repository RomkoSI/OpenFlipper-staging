#version 440

#ifndef SAT_BLOCKSIZE
#define SAT_BLOCKSIZE 32
#endif


#include "macros.glsl"



// IMPORTANT: has to be adjusted for prefixsum of blocksums pass, 
//  local_size_x = inputsize / 64
layout (local_size_x = SAT_BLOCKSIZE, local_size_y = SAT_BLOCKSIZE) in;


layout(binding = 0, SAT_INTERNALFMT) uniform SAT_IMAGETYPE g_InputTex;
layout(binding = 1, SAT_INTERNALFMT) uniform SAT_IMAGETYPE g_OutputTex;


//#define SAT_OOBCHECK


void main()
{
  ivec2 iPixelPos = ivec2(gl_GlobalInvocationID.xy);


#ifdef SAT_OOBCHECK
  SAT_DATAVEC pixelData = SAT_DATANULL;
  ivec2 imgSize = imageSize(g_InputTex).xy;
  if (iPixelPos.x < imgSize.x || iPixelPos.y < imgSize.y)
    pixelData = SAT_imageLoad(g_InputTex, iPixelPos);
#else
  SAT_DATAVEC pixelData = SAT_imageLoad(g_InputTex, iPixelPos);
#endif


  SAT_imageStore(g_OutputTex, iPixelPos, pixelData);
}

#version 440


#include "macros.glsl"

layout (local_size_x = SAT_BLOCKSIZE) in;


layout(binding = 0, SAT_INTERNALFMT) uniform SAT_IMAGETYPE g_InOutTex;
layout(binding = 1, SAT_INTERNALFMT) uniform SAT_IMAGETYPE g_InputBlockTex; // block sums

void main()
{
  int localThreadID = int(gl_LocalInvocationID.x);

  int blockOffsetRd1D = int(gl_WorkGroupID.x * gl_WorkGroupSize.x);
  int blockOffsetWr1D = int((gl_WorkGroupID.x + 1) * gl_WorkGroupSize.x);
  blockOffsetRd1D = blockOffsetWr1D / int(gl_WorkGroupSize.x);
  
#ifdef SAT_2D
  ivec2 blockOffsetRd = ivec2(blockOffsetRd1D, gl_WorkGroupID.y);
  ivec2 blockOffsetWr = ivec2(blockOffsetWr1D, gl_WorkGroupID.y);
#else
  int blockOffsetRd = blockOffsetRd1D;
  int blockOffsetWr = blockOffsetWr1D;
#endif
  
  SAT_DATAVEC blockSum = SAT_imageLoad(g_InputBlockTex, blockOffsetRd);
  
  
#ifdef SAT_2D
  ivec2 iPixelPos = blockOffsetWr + ivec2(localThreadID, 0);
#else 
  int iPixelPos = blockOffsetWr + localThreadID;
#endif

  SAT_DATAVEC vCurSum = SAT_imageLoad(g_InOutTex, iPixelPos);
  vCurSum += blockSum;
  SAT_imageStore(g_InOutTex, iPixelPos, vCurSum);
}

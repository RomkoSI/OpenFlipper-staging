#version 440

/*
summed area table  compute shader

Computes the cumulative sum of elements in upper left area of each pixel.

exclusive scan: the current pixel is not included in the sum.
not in-place!
requires multiple passes depending on block-size and 1D/2D setting

reference
"Parallel Prefix Sum (Scans) with CUDA" by M. Harris et. al.,  GPU Gems 3
https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch39.html
*/


// performance got slighty worse with this on ati,
//  NUM_BANKS probably has to be adjusted depending on gpu
//#define SAT_AVOID_BANKCONFLICTS

#ifdef SAT_AVOID_BANKCONFLICTS
#define NUM_BANKS 16
#define LOG_NUM_BANKS 4
#define CONFLICT_FREE_OFFSET(n) ((n) >> NUM_BANKS + (n) >> (2 * LOG_NUM_BANKS))
#endif

#include "macros.glsl"


/*
Each work-group computes the prefix-sum of a block of size 2*gl_WorkGroupSize.
If the input array consists of more than one block, additional merge passes are required.
defines:
SAT_DATATYPE   default: SAT_FLOAT4
SAT_2D
SAT_BLOCKSCANOUT
SAT_AVOID_BANKCONFLICTS
*/


// each thread processes two elements
layout (local_size_x = SAT_BLOCKSIZE/2, local_size_y = 1) in;


layout(binding = 0, SAT_INTERNALFMT) uniform SAT_IMAGETYPE g_InputTex;
layout(binding = 1, SAT_INTERNALFMT) uniform SAT_IMAGETYPE g_OutputTex;  // if transposed: dimension = size(g_InputTex).yx

#ifdef SAT_BLOCKSCANOUT
layout(binding = 2, SAT_INTERNALFMT) uniform SAT_IMAGETYPE g_OutputBlockSumsTex;
#endif


//#define SAT_OOBCHECK

shared SAT_DATAVEC g_SharedTree[SAT_BLOCKSIZE];


void main()
{
  int n = SAT_BLOCKSIZE;
  
  int localThreadID = int(gl_LocalInvocationID.x);
  
#ifdef SAT_AVOID_BANKCONFLICTS
  int ai = localThreadID;
  int bi = localThreadID + (n/2);
#else
  int ai = localThreadID*2;
  int bi = ai+1;
#endif
  
  int blockOffset = int(gl_WorkGroupID.x * SAT_BLOCKSIZE);
  int iPixelPos1D = ai + blockOffset;
  int iNeighborPixelPos1D = bi + blockOffset;
  
#ifdef SAT_AVOID_BANKCONFLICTS
  ai += CONFLICT_FREE_OFFSET(ai);
  bi += CONFLICT_FREE_OFFSET(bi);
#endif

#ifdef SAT_OOBCHECK
  int imgSize = imageSize(g_InputTex).x;
  if (iPixelPos1D >= imgSize) return;
  if (iNeighborPixelPos1D >= imgSize) return;
#endif


#ifdef SAT_2D
  ivec2 iPixelPos = ivec2(iPixelPos1D, gl_WorkGroupID.y);
  ivec2 iNeighborPixelPos = ivec2(iNeighborPixelPos1D, gl_WorkGroupID.y);
#else
  int iPixelPos = iPixelPos1D;
  int iNeighborPixelPos = iNeighborPixelPos1D;
#endif

  SAT_DATAVEC pixelData = SAT_imageLoad(g_InputTex, iPixelPos);
  SAT_DATAVEC neighborPixelData = SAT_imageLoad(g_InputTex, iNeighborPixelPos);
    

  g_SharedTree[ai] = pixelData;
  g_SharedTree[bi] = neighborPixelData;
  
  // up-sweep
  int offset = 1;
  for (int d = n>>1; d > 0; d >>= 1)
  {
    barrier();
	memoryBarrierShared();
	if (localThreadID < d)
	{
	  int ai = offset * (localThreadID*2 + 1)-1;
	  int bi = offset * (localThreadID*2 + 2)-1;
#ifdef SAT_AVOID_BANKCONFLICTS
	  ai += CONFLICT_FREE_OFFSET(ai);
	  bi += CONFLICT_FREE_OFFSET(bi);
#endif

#ifdef SAT_OOBCHECK
      if (ai >= n) continue;
      if (ai >= n) continue;
#endif
	  
	  g_SharedTree[bi] += g_SharedTree[ai];
	}
	offset *= 2;
  }
  
  // down-sweep
  if (localThreadID == 0)
  {
    int lastElement = n-1;
#ifdef SAT_AVOID_BANKCONFLICTS
    lastElement += CONFLICT_FREE_OFFSET(lastElement);
#endif

#ifdef SAT_BLOCKSCANOUT
#ifdef SAT_2D
    ivec2 iBlockSumPos = ivec2(gl_WorkGroupID.x, gl_WorkGroupID.y);
#else
    int iBlockSumPos = int(gl_WorkGroupID.x);
#endif // SAT_2D
    SAT_imageStore(g_OutputBlockSumsTex, iBlockSumPos, g_SharedTree[lastElement]);
#endif // SAT_BLOCKSCANOUT
    g_SharedTree[lastElement] = SAT_DATANULL;
  }
	
  for (int d = 1; d < n; d *= 2)
  {
    offset >>= 1;
	
	barrier();
	memoryBarrierShared();
	
	if (localThreadID < d)
	{
	  int ai = offset * (localThreadID*2 + 1)-1;
	  int bi = offset * (localThreadID*2 + 2)-1;
	  
#ifdef SAT_AVOID_BANKCONFLICTS
	  ai += CONFLICT_FREE_OFFSET(ai);
	  bi += CONFLICT_FREE_OFFSET(bi);
#endif

#ifdef SAT_OOBCHECK
      if (ai >= n) continue;
      if (ai >= n) continue;
#endif
	  
	  SAT_DATAVEC t = g_SharedTree[ai];
	  g_SharedTree[ai] = g_SharedTree[bi];
	  g_SharedTree[bi] += t;
	}
  }
  
//  g_SharedTree[2*localThreadID] = pixelData;
//  g_SharedTree[2*localThreadID+1] = neighborPixelData;
  
  barrier();
  memoryBarrierShared();

  SAT_imageStore(g_OutputTex, iPixelPos, g_SharedTree[ai]);
  SAT_imageStore(g_OutputTex, iNeighborPixelPos, g_SharedTree[bi]);
  
   // debugging
  #ifdef SAT_2D
//  SAT_imageStore(g_OutputTex, iPixelPos, SAT_DATAVEC(gl_WorkGroupID.xxxx) );
//  SAT_imageStore(g_OutputTex, iNeighborPixelPos, SAT_DATAVEC(gl_WorkGroupID.xxxx));
//  SAT_imageStore(g_OutputTex, iPixelPos, SAT_DATAVEC(gl_GlobalInvocationID.xxxx) );
//  SAT_imageStore(g_OutputTex, iNeighborPixelPos, SAT_DATAVEC(gl_GlobalInvocationID.xxxx));
//  SAT_imageStore(g_OutputTex, iPixelPos, SAT_DATAVEC(vec4(1,2,0,0)) );
//  SAT_imageStore(g_OutputTex, iNeighborPixelPos, SAT_DATAVEC(vec4(3,4,0,0)) );
//  SAT_imageStore(g_OutputTex, iPixelPos, pixelData );
//  SAT_imageStore(g_OutputTex, iNeighborPixelPos, neighborPixelData );
  #endif
}

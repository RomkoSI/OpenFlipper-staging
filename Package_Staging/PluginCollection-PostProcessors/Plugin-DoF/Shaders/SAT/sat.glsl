#version 430

layout (local_size_x = 1024) in;


shared vec4 shared_data[gl_WorkGroupSize.x * 2];


layout (binding = 0, r32f) readonly uniform image2D g_InputTex;
layout (binding = 1, r32f) writeonly uniform image2D g_OutputTex;


void main()
{
  uint uTexelID = gl_LocalInvocationID.x;
  
  ivec2 iPixelPos = ivec2(uTexelID * 2, gl_WorkGroupID.x);
  
  shared_data[uTexelID * 2] = imageLoad(g_InputTex, iPixelPos);
  shared_data[uTexelID * 2 + 1] = imageLoad(g_InputTex, iPixelPos + ivec2(1,0));

  int iNumSteps = int(log2(gl_WorkGroupSize.x)) + 1;
  
  barrier();
  memoryBarrierShared();
  
  for (int i = 0; i < iNumSteps; ++i)
  {
    uint mask = (1 << i) - 1;
	uint readID = ((uTexelID >> i) << (i + 1)) + mask;
	uint writeID = readID + 1 + (uTexelID & mask);
	
	shared_data[writeID] += shared_data[readID];
	
	barrier();
	memoryBarrierShared();
  }
  
  imageStore(g_OutputTex, iPixelPos.yx, shared_data[uTexelID * 2]);
  imageStore(g_OutputTex, iPixelPos.yx + ivec2(0,1), shared_data[uTexelID * 2 + 1]);
}
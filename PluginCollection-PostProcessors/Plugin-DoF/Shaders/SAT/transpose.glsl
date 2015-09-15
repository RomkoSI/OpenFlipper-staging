#version 440

#include "macros.glsl"


layout(local_size_x = SAT_BLOCKSIZE, local_size_y = SAT_BLOCKSIZE) in;


layout(binding = 0, SAT_INTERNALFMT) uniform SAT_IMAGETYPE g_InputTex;
layout(binding = 1, SAT_INTERNALFMT) uniform SAT_IMAGETYPE g_OutputTex;

void main()
{
  ivec2 iPos = ivec2(gl_GlobalInvocationID.xy);
  SAT_DATAVEC val = SAT_imageLoad(g_InputTex, iPos);
  SAT_imageStore(g_OutputTex, iPos.yx, val);
}
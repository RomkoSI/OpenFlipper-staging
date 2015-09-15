#version 420

// reset offset to 0xffffffff for all pixels

layout(pixel_center_integer) in vec4 gl_FragCoord;


layout(binding = 0, r32ui)  uniform uimageBuffer g_StartOffsetBuffer;

uniform uvec2 g_ScreenSize;

void main()
{
  uvec2 absFragCoord = uvec2(gl_FragCoord.xy);
  uint uPixelID = absFragCoord.x + g_ScreenSize.x * absFragCoord.y;
  
  imageStore(g_StartOffsetBuffer, int(uPixelID), uvec4(0xffffffff,0,0,0));
}
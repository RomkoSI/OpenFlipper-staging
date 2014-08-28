#version 420

// use OITLL_MSAA macro to enable coverage multisampling

#define MAXLISTSIZE 16

// default: pixel center at absIndex + vec2(0.5, 0.5)
//  force pixel ceter at absIndex
layout(pixel_center_integer) in vec4 gl_FragCoord;

in vec2 vTexCoord;

out vec4 outColor;

layout(binding = 0, r32ui)  uniform uimageBuffer g_StartOffsetBuffer;
layout(binding = 1, rgba32ui)  uniform uimageBuffer g_ABuffer;

uniform uvec2 g_ScreenSize;


struct FragListEntry 
{
  uint color;
  float depth;
};


void main()
{
  uvec2 absFragCoord = uvec2(gl_FragCoord.xy); // * g_ScreenSize);
  uint uiPixelID = absFragCoord.x + g_ScreenSize.x * absFragCoord.y;
  
  // sorted list of the current pixel list
  FragListEntry list[MAXLISTSIZE];
  
  // get offset to head of the list
  uint uiOffset = imageLoad(g_StartOffsetBuffer, int(uiPixelID)).r;
  uint startOffset = uiOffset;
  
  // keep track of list size
  int listSize = 0;
  
  vec4 dbgColor = vec4(1,1,1,1);
  
  // read linked list from ABuffer, starting at offset of list head
  while (uiOffset != 0xffffffff && listSize < MAXLISTSIZE)
  {
	uvec4 fragEntry = imageLoad(g_ABuffer, int(uiOffset));
	// fragEntry :  (color, depth, next, coverage)
	
#ifdef  OITLL_MSAA
	// check if sample is covered by rasterized triangle
	if ((fragEntry.w & (1 << gl_SampleID)) != 0)
#endif
	{
	
	list[listSize].color = fragEntry.x;
	list[listSize].depth = uintBitsToFloat(fragEntry.y);
	
	
	// insertion sort
	int j = listSize;
	int prev = max(j-1, 0);
	
	// insertion sort: front to back
	while( (j > 0) && (list[prev].depth > list[j].depth) )
	{
	  // swap
	  FragListEntry tmp = list[j];
	  list[j] = list[prev];
	  list[prev] = tmp;
	  
	  --j;
	  prev = max(j-1, 0);
	}
	
	listSize = min(listSize+1, MAXLISTSIZE);

	}

	// goto next entry
	uiOffset = fragEntry.z;
  }
  
  vec4 color = vec4(0,0,0,1);
  
  // blend back to front
  
  for (int i = listSize-1; i >= 0; --i)
  {
    vec4 fragColor = unpackUnorm4x8(list[i].color);
	
	color.xyz = mix(color.xyz, fragColor.xyz, fragColor.w);
  }

  outColor = color; // * gl_NumSamples;
 
 
  // weird bug, for some reason color != (0,0,0,1) for unshaded pixels
  if (startOffset == 0xffffffff)
    outColor = vec4(0,0,0,1);
 
}

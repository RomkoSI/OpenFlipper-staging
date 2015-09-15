#version 430

in vec2 vTexCoord;
out vec4 outColor;

layout(binding = 0) uniform sampler2D g_Input;

layout(binding = 0, rgba32f) uniform image2D g_Output;

void main()
{
  ivec2 pos = ivec2(gl_FragCoord.xy);
  
  vec4 v = texture(g_Input, vTexCoord);
  imageStore(g_Output, pos, v);
  
  discard;
  
  outColor = v;
}

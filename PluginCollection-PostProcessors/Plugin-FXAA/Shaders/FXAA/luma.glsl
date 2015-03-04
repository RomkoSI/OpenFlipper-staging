#version 130

in vec2 vTexCoord;
out vec4 outColor;

uniform sampler2D textureSampler;


void main()
{
  vec3 color = texture(textureSampler, vTexCoord).xyz;
  float lum = dot(color, vec3(0.299, 0.587, 0.114));
  outColor = vec4(color, lum);
}
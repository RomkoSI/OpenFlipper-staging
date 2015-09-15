// G-buffer access functions

// compute view space position of a pixel
vec3 ComputePosVS(in float zPosVS, in vec2 vTexCoord, in float mProj_00, in float mProj_11)
{
  /*
  Reconstructing the position of a pixel based on its z-coord in viewspace
  
  1. Compute the clip-space position of the pixel:
    - extent texcoord from range [0,1] to [-1,1]
  
  2. Undo perspective projection of xy-coordinate:
  
    perspective projection matrix:  http://www.opengl.org/sdk/docs/man2/xhtml/gluPerspective.xml
	
	f/aspect  0  0  0
	0  f  0  0
	0  0   *  *
	0  0  -1  0
	
	(where f = cotangent(fovy/2),   aspect = width/height)
	
	projection of viewspace position:
	xPS = f/aspect * xVS
	yPS = f * yVS
	zPS = ..
	wPS = -zVS
	
	post perspective homogenization in clip-space:
	
	xCS = xPS / wPS = xPS / -zVS
	yCS = yPS / wPS = yPS / -zVS
	

	Revert perspective projection to get xVS and yVS:
	
	xVS = xCS * (-zVS) / (f/aspect)
	yVS = yCS * (-zVS) / f
  */


  // compute xy-position in clip space (position on near clipping plane)
  vec2 vPosCS = vTexCoord * 2 - vec2(1,1);

  // revert perspective projection
  vec3 vPos;
  vPos.xy = (vPosCS * (-zPosVS)) / vec2(mProj_00, mProj_11);
  vPos.z = zPosVS;
  
  return vPos;
}

// read position in view space
vec3 ReadPositionVS(in sampler2D samplerDepth, in vec2 vTexCoord, in float mProj_00, in float mProj_11)
{
  float depthVS = texture2D(samplerDepth, vTexCoord).x;
  return ComputePosVS(depthVS, vTexCoord, mProj_00, mProj_11);
}

// read normal in view space
vec3 ReadNormalVS(in sampler2D samplerNormal, in vec2 vTexCoord)
{
  vec3 n = texture2D(samplerNormal, vTexCoord).xyz;
  n = n * 2 - vec3(1,1,1);
  return normalize(n);
}

vec4 WritePositionVS(in vec4 vPosVS, in float materialID)
{
//  return vPosVS.zzzz;
  return vec4(vPosVS.z, materialID, 0, 1);
}

vec3 WriteNormalVS(in vec3 vNormalVS)
{
  return vNormalVS * 0.5 + vec3(0.5, 0.5, 0.5);
}
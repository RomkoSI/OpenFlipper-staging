/*
Access of depth buffer values:
Convert between projected (non-linear) and linear depth values:

- compute depth value (gl_FragDepth) of positions in clip space
- fast depth value computation given a view space position
- inverse functions mapping from gl_FragDepth to clip space depth or view space z-coordinate

P22 and P23 are the entries in the projection matrix (in zero based row-major)

see http://www.opengl.org/sdk/docs/man2/xhtml/gluPerspective.xml
*/

// linear map from (non-linear) clip space depth in [-1,1] range to gl_DepthRange
float ClipSpaceToDepthValue(in float d);

// compute depth value from a position in clip space, taking the correct depth-range into account
//  example:
//  - in vertex shader: posCS = WorldViewProj * inPosition;
//  - in fragment shader (after clipping): gl_FragDepth = ClipSpaceToDepthValue(posCS);
// can be used for depth buffer manipulation, for instance:
//  - rendering imposters with correct depth
//  - custom depth test
//  - soft particles

float ClipSpaceToDepthValue(in vec4 posCS)
{
  // divide by w in clip space to get the non-linear depth in range [-1,1]:
  
  // posCS.z is a the linearly modified view space z coordinate:  P22 * posVS.z + P23
  // posCS.w is the negative view space z-coordinate: w flips the sign if the viewer direction is along the -z axis 
  
  // division posCS.z / posCS.w yields a non-linear depth -(P22 + P23 / posVS.z)
  //  P22, P23 are chosen such that the non-linear depth is in range [-1,1] for all positions between the near and far clipping planes
  float d = posCS.z / posCS.w; // [-1,1] (ndc depth)
  
  // map to depth-range
  return ClipSpaceToDepthValue(d);
}

float ClipSpaceToDepthValue(in float d)
{
  // linear mapping from [-1,1] to [gl_DepthRange.near, gl_DepthRange.far] (which should be [0,1] in most cases)
  // gl_DepthRange is set by calling glDepthRange()
  // -> http://www.opengl.org/sdk/docs/man/html/glDepthRange.xhtml
  
  return (d * gl_DepthRange.diff + gl_DepthRange.near + gl_DepthRange.far) * 0.5;
  
  // test:
  // -1 is mapped to gl_DepthRange.near: (-1 * (f-n) + n + f) / 2 = (n-f + n +f) / 2 = n
  //  1 is mapped to gl_DepthRange.far: (1 * (f-n) + n + f) / 2 = (f-n + n + f) / 2 = f
}

// convert depth value in gl_DepthRange to clip-space factor (z / w) in range [-1,1]
float DepthValueToClipSpace(in float depth)
{
  return ( depth * 2.0 - (gl_DepthRange.near + gl_DepthRange.far) ) / gl_DepthRange.diff;
}


// convert depth buffer value to viewspace z-coord
float ProjectedToViewspaceDepth(in float depth, in float P22, in float P23)
{
  // convert from [0,1] to [-1,1]
//  float d = depth * 2.0 - 1.0;  // special case gl_DepthRange = [0,1]
  float d = DepthValueToClipSpace(depth); // arbitrary gl_DepthRange

  return -P23 / (d + P22);
}

// convert view space z-coordinate to the depth value written into the depth buffer
float ViewspaceToProjectedDepth(in float zVS, in float P22, in float P23)
{
  // convert to non-linear depth in [-1,1]
  float d = -(P22 + P23 / zVS);
  // convert to [0,1]
//  return d * 0.5 + 0.5;  // special case gl_DepthRange = [0,1]
  return ClipSpaceToDepthValue(d); // arbitrary gl_DepthRange
}

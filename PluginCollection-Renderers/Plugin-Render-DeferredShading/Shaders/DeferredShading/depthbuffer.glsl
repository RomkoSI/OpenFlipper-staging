/*
Access of depth buffer values:
Convert between projected and linear depth values.

P22 and P23 are the entries in the projection matrix

see http://www.opengl.org/sdk/docs/man2/xhtml/gluPerspective.xml
*/


// convert depth buffer value to viewspace z-coord
float ProjectedToViewspaceDepth(in float depth, in float P22, in float P23)
{
	// convert to [-1,1]
	float d = depth * 2.0 - 1.0;

	return -P23 / (d + P22);
}

// convert view space z-coordinate to the depth value written into the depth buffer
float ViewspaceToProjectedDepth(in float zVS, in float P22, in float P23)
{
	// convert to non-linear depth in [-1,1]
	float d = -(P22 + P23 / zVS);
	// convert to [0,1]
	return d * 0.5 + 0.5;
}


#ifdef CVF_OPENGL_ES 
uniform mediump int u_clipPlaneCount;
#else
uniform int u_clipPlaneCount;
#endif

uniform vec4 u_ecClipPlanes[6];

// The dimensioning should probably be via a define to be consistent between vs and fs
varying float v_clipDist[6];

#define CVF_CALC_CLIP_DISTANCES_IMPL

//--------------------------------------------------------------------------------------------------
/// Calculate clip distances and set the array of varyings
//--------------------------------------------------------------------------------------------------
void calcClipDistances(vec4 ecPosition)
{
	// Using eye space clipping plane directly
	int i;
	for (i = 0; i < u_clipPlaneCount; i++)
	{
		v_clipDist[i] = dot(u_ecClipPlanes[i], ecPosition);
	}

	// Using clipping plane specified in world coords would be something like this
	//vec4 ecPlane = u_wcClipPlane*cvfu_modelViewMatrixInverse;
	//clipDist = dot(ecPlane, ecPosition);

	// The proper OpenGL way uing built-in varying gl_ClipDistance 
	// Have not been able to get this to work on ATI yet. Latest driver tested is: Catalyst 11.6
	//gl_ClipDistance[0] = clipDist;
}


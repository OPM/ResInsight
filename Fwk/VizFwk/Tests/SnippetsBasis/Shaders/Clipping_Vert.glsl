#version 130

uniform mat4 cvfu_modelViewProjectionMatrix;
uniform mat4 cvfu_modelViewMatrix;
uniform mat3 cvfu_normalMatrix;

attribute vec4 cvfa_vertex;
attribute vec3 cvfa_normal;
attribute vec2 cvfa_texCoord;

varying vec3 v_ecPosition;
varying vec3 v_ecNormal;
varying vec2 v_texCoord;



//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

uniform int u_clipPlaneCount;
uniform vec4 u_ecClipPlanes[6];

// The dimensioning should probably be via a define to be consistent between vs and fs
varying float v_clipDist[6];

//--------------------------------------------------------------------------------------------------
/// Calculate clip distances and set the varyings
//--------------------------------------------------------------------------------------------------
void calcClipDistances()
{
	vec4 ecPos = cvfu_modelViewMatrix * cvfa_vertex;

	// Using eye space clipping plane directly
	int i;
	for (i = 0; i < u_clipPlaneCount; i++)
	{
		v_clipDist[i] = dot(u_ecClipPlanes[i], ecPos);
	}

	// Using clipping plane specified in world coords would be something like this
	//vec4 ecPlane = u_wcClipPlane*cvfu_modelViewMatrixInverse;
	//clipDist = dot(ecPlane, ecPos);

	// The proper OpenGL way uing built-in varying gl_ClipDistance 
	// Have not been able to get this to work on ATI yet. Latest driver tested is: Catalyst 11.6
	//gl_ClipDistance[0] = clipDist;
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------------
/// Vertex Shader - Clipping plane experiment
//--------------------------------------------------------------------------------------------------
void main()
{
	calcClipDistances();

	// Transforms vertex position and normal vector to eye space
	v_ecPosition = (cvfu_modelViewMatrix * cvfa_vertex).xyz;
	v_ecNormal = cvfu_normalMatrix * cvfa_normal;
	v_texCoord = cvfa_texCoord;

	gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;
}

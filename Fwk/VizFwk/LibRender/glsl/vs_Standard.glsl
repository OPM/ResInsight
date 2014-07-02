
uniform mat4 cvfu_modelViewProjectionMatrix;
uniform mat4 cvfu_modelViewMatrix;
uniform mat3 cvfu_normalMatrix;

attribute vec4 cvfa_vertex;
attribute vec3 cvfa_normal;
attribute vec2 cvfa_texCoord;
attribute vec4 cvfa_color;

varying vec3 v_ecPosition;
varying vec3 v_ecNormal;
varying vec2 v_texCoord;
varying vec4 v_color;

//--------------------------------------------------------------------------------------------------
/// Vertex Shader - Standard
//--------------------------------------------------------------------------------------------------
void main ()
{
#ifdef CVF_CALC_SHADOW_COORD_IMPL
	calcShadowCoord();
#endif

	// Transforms vertex position and normal vector to eye space
	v_ecPosition = (cvfu_modelViewMatrix * cvfa_vertex).xyz;
	v_ecNormal = cvfu_normalMatrix * cvfa_normal;
	v_texCoord = cvfa_texCoord;
    v_color = cvfa_color;

#ifdef CVF_CALC_CLIP_DISTANCES_IMPL
	calcClipDistances(vec4(v_ecPosition, 1));
#endif

	gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;
}

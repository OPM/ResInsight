
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
/// Vertex Shader - Environment mapping
/// Source: http://www.ozone3d.net/tutorials/glsl_texturing_p04.php
//--------------------------------------------------------------------------------------------------
void main ()
{
#ifdef CVF_CALC_SHADOW_COORD_IMPL
	calcShadowCoord();
#endif

	// Transforms vertex position and normal vector to eye space
	v_ecPosition = (cvfu_modelViewMatrix * cvfa_vertex).xyz;
	v_ecNormal = cvfu_normalMatrix * cvfa_normal;

#ifdef CVF_CALC_CLIP_DISTANCES_IMPL
	calcClipDistances(vec4(v_ecPosition, 1));
#endif

	gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;

	// Compute the texture coordinate for the environment map texture lookup
	vec3 u = normalize(v_ecPosition);
	vec3 n = normalize(v_ecNormal);
	vec3 r = reflect(u, n);
	float m = 2.0 * sqrt(r.x*r.x + r.y*r.y + (r.z+1.0)*(r.z+1.0));
	v_texCoord.s = r.x/m + 0.5;
	v_texCoord.t = r.y/m + 0.5;
}


uniform mat4 cvfu_modelViewProjectionMatrix;
uniform mat4 cvfu_modelViewMatrix;
uniform mat3 cvfu_normalMatrix;
uniform mat4 u_transformationMatrix;

attribute vec4 cvfa_vertex;
attribute vec3 cvfa_normal;

varying float v_diffuse;

//--------------------------------------------------------------------------------------------------
/// Vertex Shader - Vector Drawer
//--------------------------------------------------------------------------------------------------
void main ()
{
#ifdef CVF_CALC_CLIP_DISTANCES_IMPL
	vec4 ecPosition = cvfu_modelViewMatrix*u_transformationMatrix*cvfa_vertex;
	calcClipDistances(ecPosition);
#endif

	// Transforms vertex position and normal vector to eye space
	mat3 mat3_transMatr = mat3(u_transformationMatrix);

	gl_Position = cvfu_modelViewProjectionMatrix*u_transformationMatrix*cvfa_vertex;
	v_diffuse = abs(normalize(cvfu_normalMatrix*mat3_transMatr*cvfa_normal).z);
}


uniform mat4 cvfu_modelViewProjectionMatrix;
uniform mat4 cvfu_modelViewMatrix;
uniform mat4 cvfu_modelMatrix;
uniform mat4 cvfu_modelMatrixInverseTranspose;
uniform mat3 cvfu_normalMatrix;

attribute vec4 cvfa_vertex;
attribute vec3 cvfa_normal;

varying vec3 v_ecPosition;
varying vec3 v_ecNormal;
//varying vec3 v_wcPosition;
//varying vec3 v_wcNormal;


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void main ()
{
	// Position and normal in eye coordinates
	v_ecPosition = (cvfu_modelViewMatrix * cvfa_vertex).xyz;
	v_ecNormal = cvfu_normalMatrix * cvfa_normal;

	// Transform to world coordinates
	// Could get away with just transforming normal with modelMatrix as well if we assume that model matrix doesn't contain any non-uniform scaling
	//v_wcPosition = (cvfu_modelMatrix*cvfa_vertex).xyz;
	//v_wcNormal = (cvfu_modelMatrixInverseTranspose*vec4(cvfa_normal, 0)).xyz;

	gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;
}

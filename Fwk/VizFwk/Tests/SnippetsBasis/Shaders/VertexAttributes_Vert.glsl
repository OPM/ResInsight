
uniform mat4 cvfu_modelViewProjectionMatrix;
uniform mat4 cvfu_modelViewMatrix;
uniform mat3 cvfu_normalMatrix;

attribute vec4 cvfa_vertex;
attribute vec3 cvfa_normal;
attribute float a_yDisplacement;
attribute ivec3 a_color;

varying vec3 v_ecPosition;
varying vec3 v_ecNormal;
varying vec3 v_color;

//--------------------------------------------------------------------------------------------------
/// Vertex Shader - snipVertexAttributes
//--------------------------------------------------------------------------------------------------
void main ()
{
	vec4 vertex = cvfa_vertex;
	vertex.y += a_yDisplacement;

	// Transforms vertex position and normal vector to eye space
	v_ecPosition = (cvfu_modelViewMatrix * vertex).xyz;
	v_ecNormal = cvfu_normalMatrix * cvfa_normal;

	// Since color is given as unsigned integer type (ubytes) we must scale to [0,1] range
	v_color = vec3(a_color)/256;

	gl_Position = cvfu_modelViewProjectionMatrix*vertex;
}

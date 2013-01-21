uniform mat4 cvfu_modelViewProjectionMatrix;

#ifdef CVF_CALC_CLIP_DISTANCES_IMPL
uniform mat4 cvfu_modelViewMatrix;
#endif

attribute vec2 cvfa_texCoord;
attribute vec4 cvfa_vertex;

varying vec2 v_texCoord;

//--------------------------------------------------------------------------------------------------
/// Vertex Shader - Text
//--------------------------------------------------------------------------------------------------
void main ()
{
	v_texCoord = cvfa_texCoord;
	gl_Position = cvfu_modelViewProjectionMatrix * cvfa_vertex;

#ifdef CVF_CALC_CLIP_DISTANCES_IMPL
	vec3 ecPosition = (cvfu_modelViewMatrix * cvfa_vertex).xyz;
	calcClipDistances(vec4(ecPosition, 1));
#endif

#ifdef CVF_OPENGL_ES
    gl_PointSize = 1.0;
#endif
}

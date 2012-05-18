uniform mat4 cvfu_modelViewProjectionMatrix;

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

#ifdef CVF_OPENGL_ES
    gl_PointSize = 1.0;
#endif
}

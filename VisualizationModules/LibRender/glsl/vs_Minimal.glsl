
uniform mat4 cvfu_modelViewProjectionMatrix;

attribute vec4 cvfa_vertex;

//--------------------------------------------------------------------------------------------------
/// Vertex Shader - Minimal
//--------------------------------------------------------------------------------------------------
void main ()
{
	gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;

#ifdef CVF_OPENGL_ES
    gl_PointSize = 1.0;
#endif
}

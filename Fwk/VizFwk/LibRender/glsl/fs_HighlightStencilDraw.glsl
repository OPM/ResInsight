
uniform vec4 u_color;

//--------------------------------------------------------------------------------------------------
/// Initial draw pass for highlighting
//--------------------------------------------------------------------------------------------------
void main()
{
	gl_FragData[0] = u_color;

#ifdef CVF_LOG_DEPTH_IMPL
	applyLogDepth();
#endif
}


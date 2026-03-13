
//--------------------------------------------------------------------------------------------------
/// Fragment Shader - Fixed color magenta (for debugging)
//--------------------------------------------------------------------------------------------------
void main()
{
	gl_FragColor = vec4(1,0,1,1);

#ifdef CVF_LOG_DEPTH_IMPL
	applyLogDepth();
#endif
}

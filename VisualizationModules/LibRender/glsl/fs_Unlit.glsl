
vec4 srcFragment();

//--------------------------------------------------------------------------------------------------
/// Fragment Shader - Unlit
//--------------------------------------------------------------------------------------------------
void main()
{
#ifdef CVF_CHECK_DISCARD_FRAGMENT_IMPL
	checkDiscardFragment();
#endif

	vec4 color = srcFragment();

	gl_FragColor = color;
}

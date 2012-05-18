
// Prototypes not needed when we concatinate files, and these breaks the compile on VMWare Fusion 4.0.4
// vec4 srcFragment();
// vec4 lightFragment(vec4 srcFragColor, float shadowFactor);

//--------------------------------------------------------------------------------------------------
/// Fragment Shader - Standard
//--------------------------------------------------------------------------------------------------
void main()
{
#ifdef CVF_CHECK_DISCARD_FRAGMENT_IMPL
	checkDiscardFragment();
#endif

	vec4 color = srcFragment();
	color = lightFragment(color, 1.0);

	gl_FragColor = color;
}

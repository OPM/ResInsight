
varying float v_diffuse;
uniform vec3 u_color;

//--------------------------------------------------------------------------------------------------
/// Fragment Shader - Unlit
//--------------------------------------------------------------------------------------------------
void main()
{
#ifdef CVF_CHECK_DISCARD_FRAGMENT_IMPL
	checkDiscardFragment();
#endif

	gl_FragColor = vec4(u_color*v_diffuse, 1.0);
}

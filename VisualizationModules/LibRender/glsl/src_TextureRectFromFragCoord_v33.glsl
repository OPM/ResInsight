#version 330

uniform sampler2DRect u_texture2DRect;

//--------------------------------------------------------------------------------------------------
/// Texture 2D source fragment
//--------------------------------------------------------------------------------------------------
vec4 srcFragment()
{
	return texture(u_texture2DRect, gl_FragCoord.xy);
}

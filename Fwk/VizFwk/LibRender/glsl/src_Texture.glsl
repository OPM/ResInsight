
uniform sampler2D u_texture2D;

varying vec2 v_texCoord;

//--------------------------------------------------------------------------------------------------
/// Texture 2D source fragment
//--------------------------------------------------------------------------------------------------
vec4 srcFragment()
{
	return texture2D(u_texture2D, v_texCoord);
}
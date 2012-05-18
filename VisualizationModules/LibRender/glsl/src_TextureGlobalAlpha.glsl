
uniform sampler2D u_texture2D;
uniform float	  u_alpha;

varying vec2 v_texCoord;

//--------------------------------------------------------------------------------------------------
/// Texture 2D source fragment
//--------------------------------------------------------------------------------------------------
vec4 srcFragment()
{
	return vec4(texture2D(u_texture2D, v_texCoord).rgb, u_alpha);
}

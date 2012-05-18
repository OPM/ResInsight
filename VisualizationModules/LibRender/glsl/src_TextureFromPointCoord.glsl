
uniform sampler2D u_texture2D;

//--------------------------------------------------------------------------------------------------
/// Texture 2D source fragment
//--------------------------------------------------------------------------------------------------
vec4 srcFragment()
{
    return texture2D(u_texture2D, gl_PointCoord.xy);
}
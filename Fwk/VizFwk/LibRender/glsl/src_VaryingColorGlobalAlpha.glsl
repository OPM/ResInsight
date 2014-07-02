
uniform float  u_alpha;

varying vec4   v_color;

//--------------------------------------------------------------------------------------------------
/// RGB color from varying, alpha from uniform
//--------------------------------------------------------------------------------------------------
vec4 srcFragment()
{
    return vec4(v_color.rgb, u_alpha);
}
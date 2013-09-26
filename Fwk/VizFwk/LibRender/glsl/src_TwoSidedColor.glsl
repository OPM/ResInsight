
uniform vec4 u_color;
uniform vec4 u_backColor;

//--------------------------------------------------------------------------------------------------
/// srcFragment() - Single color by uniform
//--------------------------------------------------------------------------------------------------
vec4 srcFragment()
{
	if (gl_FrontFacing)
	{
		return u_color;
	}
	else
	{
		return u_backColor;
	}
}

#extension GL_ARB_texture_rectangle : enable            

uniform sampler2DRect u_texture2DRect;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void main()
{
	vec4 t = texture2DRect(u_texture2DRect, gl_FragCoord.xy);
	if (t.a > 0.0)
	{
		gl_FragColor = t;
	}
	else
	{
		discard;
	}
}

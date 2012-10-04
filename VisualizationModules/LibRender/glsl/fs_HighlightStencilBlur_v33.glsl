#extension GL_ARB_texture_rectangle : enable            

uniform sampler2DRect u_texture2DRect;


vec4 selectBrightest()
{
	const int siz = 5;
	const int halfSize = 2;
	vec4 maxSample = vec4(0, 0, 0, 0);
	for (int y = 0; y < siz; y++)
	{
		for (int x = 0; x < siz; x++)
		{
			vec4 t = texture2DRect(u_texture2DRect, gl_FragCoord.xy + vec2(x - halfSize, y - halfSize));
			if (t.a > maxSample.a)
			{
				maxSample = t;
			}
		}
	}

	return maxSample;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void main()
{
	gl_FragData[0] = selectBrightest();
}


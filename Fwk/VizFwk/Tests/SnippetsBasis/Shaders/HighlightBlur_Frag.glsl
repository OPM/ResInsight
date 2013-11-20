#version 330

uniform sampler2DRect u_texture2DRect;
uniform sampler2DRect u_depthTexture2DRect;

// -----------------------------------------------------------
vec4 gaussianBlur()
{
	vec4 texSample[25];
	for (int y = 0; y < 5; y++)
	{
		for (int x = 0; x < 5; x++)
		{
			vec4 t = texture(u_texture2DRect, gl_FragCoord.xy + vec2(x - 2, y -2));
			//vec4 t = texture(u_texture2DRect, gl_FragCoord.xy + 1.5*vec2(x - 2, y -2));
			texSample[y*5 + x] = t;
		}
	}

	// 1  4  7  4 1
	// 4 16 26 16 4
	// 7 26 41 26 7 / 273
	// 4 16 26 16 4
	// 1  4  7  4 1

    vec4 clr = ( (1.0  * (texSample[0] + texSample[4]  + texSample[20] + texSample[24])) +
                 (4.0  * (texSample[1] + texSample[3]  + texSample[5]  + texSample[9] + texSample[15] + texSample[19] + texSample[21] + texSample[23])) +
                 (7.0  * (texSample[2] + texSample[10] + texSample[14] + texSample[22])) +
                 (16.0 * (texSample[6] + texSample[8]  + texSample[16] + texSample[18])) +
                 (26.0 * (texSample[7] + texSample[11] + texSample[13] + texSample[17])) +
                 (41.0 * texSample[12])
               ) / 273.0;

	/*
    vec4 clr = ( (1.0  * (texSample[0] + texSample[4]  + texSample[20] + texSample[24])) +
                 (1.0  * (texSample[1] + texSample[3]  + texSample[5]  + texSample[9] + texSample[15] + texSample[19] + texSample[21] + texSample[23])) +
                 (1.0  * (texSample[2] + texSample[10] + texSample[14] + texSample[22])) +
                 (1.0 * (texSample[6] + texSample[8]  + texSample[16] + texSample[18])) +
                 (1.0 * (texSample[7] + texSample[11] + texSample[13] + texSample[17])) +
                 (1.0 * texSample[12])
               ) / 25.0;
	*/

	return clr;
}


// -----------------------------------------------------------
vec4 selectBrightest(out float depthVal)
{
	const int siz = 9;
	vec4 maxSample = vec4(0, 0, 0, 0);
	float maxDepth = 0;
	for (int y = 0; y < siz; y++)
	{
		for (int x = 0; x < siz; x++)
		{
			vec4 t = texture(u_texture2DRect, gl_FragCoord.xy + vec2(x - siz/2, y - siz/2));
			float d = texture(u_depthTexture2DRect, gl_FragCoord.xy + vec2(x - siz/2, y - siz/2));
			
			if (length(t.rgb) > length(maxSample.rgb))
			{
				maxSample = t;
			}

			if (d > maxDepth)
			{
				maxDepth = d;
			}
		}
	}

	//depthVal = maxDepth;

	return maxSample;
}

void main(void)
{
	float depthVal = 99;
	
	gl_FragData[0] = selectBrightest(depthVal);
	gl_FragData[1] = depthVal;
}

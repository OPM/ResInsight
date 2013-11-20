#version 330

uniform sampler2DRect u_texture2DRect;
uniform sampler2DRect u_depthTexture2DRect;

void main(void)
{
	vec4 t = texture(u_texture2DRect, gl_FragCoord.xy);
	if (t.a > 0.0)
	{
		//float d = texture(u_depthTexture2DRect, gl_FragCoord.xy);
		//gl_FragDepth = d;
		gl_FragColor = t;
	}
	else
	{
		//gl_FragColor = t;
		//gl_FragColor = vec4(0, 1, 0, 1);
		discard;
	}
}

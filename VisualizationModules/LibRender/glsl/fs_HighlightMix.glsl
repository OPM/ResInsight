//#extension GL_ARB_texture_rectangle : enable            

uniform vec3 u_highlightColor;

//uniform sampler2DRect u_texture2DRect;
uniform sampler2D u_texture2D;

varying vec2 v_texCoord;


//--------------------------------------------------------------------------------------------------
/// Mix fragment shader for use with part highlighter
//--------------------------------------------------------------------------------------------------
void main()
{
	const float interiorAlphaScaleFactor = 4.0;
	const float exteriorAlphaScaleFactor = 2.0;

	//float alpha = texture2DRect(u_texture2DRect, gl_FragCoord.xy).a;
	float alpha = texture2D(u_texture2D, v_texCoord).a;

	/*
	// Naive version
	if (alpha > 0.5) 
	{
		alpha = interiorAlphaScaleFactor*(1.0 - alpha);
	}
	else
	{
		alpha *= exteriorAlphaScaleFactor;
	}
	*/

	// Better
	//alpha = 0.5 - abs(alpha - 0.5);

	// Fra Jakob, doesn't compile, but rewrite with step function
	//alpha = (alpha < 0.5)*alpha + (alpha >= 0.5)*(1 - alpha);

	float s = step(alpha, 0.5);
	alpha = (s*alpha)*exteriorAlphaScaleFactor + ((1 - s)*(1 - alpha))*interiorAlphaScaleFactor ;
	
	
	gl_FragColor = vec4(u_highlightColor, alpha);
}

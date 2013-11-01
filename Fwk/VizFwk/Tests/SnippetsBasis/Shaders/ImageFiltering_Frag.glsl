#version 330

//uniform sampler2DRect u_texture2DRect;

// The texture containing the results of the first pass 
uniform sampler2D u_texture2D;

uniform float EdgeThreshold;	// The squared threshold value 
uniform int cvfu_viewportWidth;		
uniform int cvfu_viewportHeight;	

varying vec2 v_texCoord;



// Approximates the brightness of a RGB value. 
float luma(vec3 color) 
{ 
	return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void main(void)
{
	// From cookbook
	float dx = 1.0 / float(cvfu_viewportWidth); 
	float dy = 1.0 / float(cvfu_viewportHeight); 
	float s00 = luma(texture( u_texture2D, v_texCoord + vec2(-dx,  dy)  ).rgb); 
	float s10 = luma(texture( u_texture2D, v_texCoord + vec2(-dx,  0.0) ).rgb); 
	float s20 = luma(texture( u_texture2D, v_texCoord + vec2(-dx, -dy)  ).rgb); 
	float s01 = luma(texture( u_texture2D, v_texCoord + vec2(0.0,  dy)  ).rgb); 
	float s21 = luma(texture( u_texture2D, v_texCoord + vec2(0.0, -dy)  ).rgb); 
	float s02 = luma(texture( u_texture2D, v_texCoord + vec2(dx,   dy)  ).rgb); 
	float s12 = luma(texture( u_texture2D, v_texCoord + vec2(dx,   0.0) ).rgb); 
	float s22 = luma(texture( u_texture2D, v_texCoord + vec2(dx,  -dy)  ).rgb); 
	
	float sx = s00 + 2*s10 + s20 - (s02 + 2*s12 + s22); 
	float sy = s00 + 2*s01 + s02 - (s20 + 2*s21 + s22); 
	float dist = sx*sx + sy*sy; 
	
	vec4 clr;
	if (dist > EdgeThreshold) 
	{
		clr = vec4(1.0); 
		//float c = dist/10;//sqrt(dist);
		//clr = vec4(c, c, c, 1.0);
	}
	else 
	{
		//clr = vec4(0.0,0.0,0.0,1.0); 
		clr = texture(u_texture2D, v_texCoord); 
	}

	gl_FragColor = clr;
}



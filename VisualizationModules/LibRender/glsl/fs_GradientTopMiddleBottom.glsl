
uniform vec4 u_topColor;                                                         
uniform vec4 u_middleColor;                                                      
uniform vec4 u_bottomColor;                                                      
                                                                                 
varying vec2 v_texCoord;                                                         
                                                                                 
//--------------------------------------------------------------------------------------------------
/// Fragment Shader - Top -> Bottom gradient
//--------------------------------------------------------------------------------------------------
void main()                                                                      
{                                                                                
	if (v_texCoord.y > 0.5)
	{
	    gl_FragColor = u_middleColor+ (u_topColor - u_middleColor)*(2.0*(v_texCoord.y - 0.5));    
	}
	else
	{
	    gl_FragColor = u_bottomColor + (u_middleColor - u_bottomColor)*v_texCoord.y*2.0;    
	}
}                                                                                


uniform vec4 u_topColor;                                                         
uniform vec4 u_bottomColor;                                                      
                                                                                 
varying vec2 v_texCoord;                                                         
                                                                                 
//--------------------------------------------------------------------------------------------------
/// Fragment Shader - Top -> Bottom gradient
//--------------------------------------------------------------------------------------------------
void main()                                                                      
{                                                                                
    gl_FragColor = u_bottomColor + (u_topColor - u_bottomColor)*v_texCoord.y;    
}                                                                                

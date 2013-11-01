//##################################################################################################
// Depth Peeling Snippet shader
//
// Final step fragment shader
// 
//##################################################################################################

#extension GL_ARB_texture_rectangle : enable                        
                                                                    
uniform sampler2DRect FrontBlenderTex;                              
uniform sampler2DRect BackBlenderTex;                               
                                                                    
void main ()                                                        
{                                                                   
    vec4 frontColor = texture(FrontBlenderTex, gl_FragCoord.xy);    
    vec3 backColor = texture(BackBlenderTex, gl_FragCoord.xy).rgb;  
    float alphaMultiplier = 1.0 - frontColor.w;                     
                                                                    
    // front + back                                                 
    gl_FragColor.rgb = frontColor.rgb + backColor * alphaMultiplier;
}                                                                   

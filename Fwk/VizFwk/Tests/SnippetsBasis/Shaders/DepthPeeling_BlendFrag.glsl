//##################################################################################################
// Depth Peeling Snippet shader
//
// Blend step fragment shader
// 
//##################################################################################################

#extension GL_ARB_texture_rectangle : enable            

uniform sampler2DRect TempTex;
                                                        
void main ()                                            
{                                                       
    gl_FragColor = texture(TempTex, gl_FragCoord.xy);   
    // for occlusion query                              
    if (gl_FragColor.a == 0.0) discard;                 
}                                                       

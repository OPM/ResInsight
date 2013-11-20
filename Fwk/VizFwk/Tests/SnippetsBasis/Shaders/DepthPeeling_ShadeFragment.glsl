//##################################################################################################
// Depth Peeling Snippet shader
//
// Implements the ShadeFragment() function
// 
//##################################################################################################

uniform float Alpha;                                                       
//uniform vec4 u_color;                                                        
uniform vec4 u_color;                                                        
varying float diffuse;                                                     
                                                                           
vec4 ShadeFragment()                                                       
{                                                                          
    vec4 col;       
    col.rgb = u_color.rgb;   
    col.a = Alpha;                                                          
    col.rgb *= diffuse;                                                     

    return col;                                                             
}

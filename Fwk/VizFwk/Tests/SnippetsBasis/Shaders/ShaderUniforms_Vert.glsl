
uniform mat4 cvfu_modelViewProjectionMatrix;                              
uniform mat4 cvfu_modelViewMatrix;                                        
uniform mat3 cvfu_normalMatrix;             

attribute vec4 cvfa_vertex;                                               
attribute vec3 cvfa_normal;                                               

varying vec3 v_ecPosition;                                               
varying vec3 v_ecNormal;                                               
                                                                         
void main ()                                                    
{                                          
    v_ecPosition = vec3(cvfu_modelViewMatrix * cvfa_vertex);
    v_ecNormal = normalize(cvfu_normalMatrix * cvfa_normal);

    gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;     
}                                                               


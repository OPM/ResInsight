#version 330

uniform mat4 cvfu_modelViewProjectionMatrix;                              
uniform mat4 cvfu_modelViewMatrix;                                        
uniform mat3 cvfu_normalMatrix;             

in vec4 cvfa_vertex;                                               
in vec3 cvfa_normal;                                               

out vec3 v_ecPosition;                                               
out vec3 v_ecNormal;                                               
                                                                         
void main ()                                                    
{                                          
    v_ecPosition = vec3(cvfu_modelViewMatrix * cvfa_vertex);
    v_ecNormal = normalize(cvfu_normalMatrix * cvfa_normal);

    gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;     
}                                                               


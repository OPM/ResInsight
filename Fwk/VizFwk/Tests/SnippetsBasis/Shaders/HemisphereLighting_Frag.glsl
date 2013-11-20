#version 330

uniform mat4 cvfu_modelViewMatrix;
uniform vec3 u_wcLightPosition;

in vec3 v_ecPosition;                                               
in vec3 v_ecNormal;
                                               
out vec4 o_fragColor;
                                                                         
void main ()                                                    
{     
	const vec3 skyColor = vec3(1,1,1);
	const vec3 groundColor = vec3(0.1,0.1,0.1);

	vec3 ecLightPosition = vec3(cvfu_modelViewMatrix*vec4(u_wcLightPosition, 1));

    vec3 lightVec   = normalize(ecLightPosition - v_ecPosition);
    float costheta  = dot(normalize(v_ecNormal), lightVec);
    float a         = 0.5 + 0.5 * costheta;

    vec3 diffuseColor    = mix(groundColor, skyColor, a);
               
	o_fragColor = vec4(diffuseColor, 1.0);
} 



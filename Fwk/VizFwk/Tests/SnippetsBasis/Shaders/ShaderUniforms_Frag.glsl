
uniform vec3  u_color0;
uniform vec3  u_color1;
uniform float u_colorMixFactor;

void main ()                                                    
{     
	vec3 clr = mix(u_color0, u_color1, u_colorMixFactor);
	gl_FragColor = lightFragment(vec4(clr, 1), 0);
} 




varying vec3 v_ecPosition;
varying vec3 v_ecNormal; 
varying vec3 v_color;

//--------------------------------------------------------------------------------------------------
/// Fragment Shader - snipVertexAttributes
//--------------------------------------------------------------------------------------------------
void main()
{
	const vec3  ecLightPosition = vec3(0.5, 5.0, 7.0);
	const float ambientIntensity = 0.2;

	vec3 L = normalize(ecLightPosition - v_ecPosition);
	vec3 N = normalize(v_ecNormal);

	vec3 ambient = v_color*ambientIntensity;
	vec3 diffuse = v_color*(1.0 - ambientIntensity)*abs(dot(N, L));

	gl_FragColor = vec4(ambient + diffuse, 0.9);
}

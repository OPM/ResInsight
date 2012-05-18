
varying vec3 v_ecPosition;
varying vec3 v_ecNormal; 

//--------------------------------------------------------------------------------------------------
/// lightFragment() - Simple Headlight
/// 
/// A fixed, two sided headlight 
//--------------------------------------------------------------------------------------------------
vec4 lightFragment(vec4 srcFragColor, float not_in_use_shadowFactor)
{
	const vec3  ecLightPosition = vec3(0.5, 5.0, 7.0);
	const float ambientIntensity = 0.2;
	const float specularIntensity = 0.5;

	// Light vector (from point to light source)
	vec3 L = normalize(ecLightPosition - v_ecPosition);

	// Viewing vector (from point to eye)
	// Since we are in eye space, the eye pos is at (0, 0, 0)
	vec3 V = normalize(-v_ecPosition);

	vec3 N = normalize(v_ecNormal);
	vec3 R = normalize(reflect(-L, N));

	vec3 ambient = srcFragColor.rgb*ambientIntensity;
	vec3 diffuse = srcFragColor.rgb*(1.0 - ambientIntensity)*abs(dot(N, L));
	vec3 specular = vec3(specularIntensity*pow(max(dot(R, V), 0.0), 8.0));

	return vec4(ambient + diffuse + specular, srcFragColor.a);
}

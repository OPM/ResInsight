uniform vec3 u_ecLightPosition;
uniform float u_ambientIntensity;

varying vec3 v_ecPosition;
varying vec3 v_ecNormal; 

//--------------------------------------------------------------------------------------------------
/// lightFragment() - inShadowFactor: 0..1 (1 not in shadow, 0 totally in shadow)
//--------------------------------------------------------------------------------------------------
vec4 lightFragment(vec4 srcFragColor, float shadowFactor)
{
	const float specularIntensity = 0.5;

	// Light vector (from point to light source)
	vec3 L = normalize(u_ecLightPosition - v_ecPosition);

	// Viewing vector (from point to eye)
	// Since we are in eye space, the eye pos is at (0, 0, 0)
	vec3 V = normalize(-v_ecPosition);

	vec3 N = normalize(v_ecNormal);
	vec3 R = normalize(reflect(-L, N));

	vec3 ambient = srcFragColor.rgb*u_ambientIntensity;
	vec3 diffuse = (0.25 + shadowFactor*0.75)*srcFragColor.rgb*(1.0 - u_ambientIntensity)*max(dot(N, L), 0.0);  // mix(step...)
	vec3 specular = shadowFactor*vec3(specularIntensity*pow(max(dot(R, V), 0.0), 8.0));

	return vec4(ambient + diffuse + specular, srcFragColor.a);
}

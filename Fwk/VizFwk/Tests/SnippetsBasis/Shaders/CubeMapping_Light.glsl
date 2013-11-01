
uniform mat4 cvfu_viewMatrixInverse;

uniform vec3 u_ecLightPosition;
uniform samplerCube u_cubeMap;
uniform float u_ambientIntensity;
uniform float u_reflectivity;

varying vec3 v_ecPosition;
varying vec3 v_ecNormal;


//--------------------------------------------------------------------------------------------------
/// Assumes nN, nL and nV are normalized
//--------------------------------------------------------------------------------------------------
void phongShadingWhiteLight(vec3 nN, vec3 nL, vec3 nV, float specularExponent, out float diffuseContrib, out float specularContrib)
{
	vec3 R = normalize(reflect(-nL, nN));
	diffuseContrib = max(dot(nN, nL), 0.0);
	specularContrib = pow(max(dot(R, nV), 0.0), specularExponent);
}


//--------------------------------------------------------------------------------------------------
/// lightFragment() - currently using fixed position headlight
//--------------------------------------------------------------------------------------------------
vec4 lightFragment(vec4 srcFragColor, float not_in_use_shadowFactor)
{
	//const vec3 ecLightPosition = vec3(0.5, 5.0, 7.0);
	const float specularIntensity = 0.5;
	const float specularExponent = 55.0;

	vec3 diffuseColor = srcFragColor.rgb;
	float diffuseIntensity = (1 - u_ambientIntensity);

	// Light vector and viewing vector from point to light/eye
	vec3 L = normalize(u_ecLightPosition - v_ecPosition);
	vec3 V = normalize(-v_ecPosition);
	vec3 N = normalize(v_ecNormal);

	float diffuseContrib = 0;
	float specularContrib = 0;
	phongShadingWhiteLight(N, L, V, specularExponent, diffuseContrib, specularContrib);

	vec3 clr = u_ambientIntensity*diffuseColor;
	clr += diffuseIntensity*diffuseContrib*diffuseColor;
	clr += specularIntensity*specularContrib*vec3(1, 1, 1);

	// Transform the reflection vector to world for cube map lookup
	vec3 R = reflect(-V, N);
	vec3 wR = (cvfu_viewMatrixInverse*vec4(R, 0)).xyz;
	vec3 clrReflect = u_reflectivity*textureCube(u_cubeMap, wR).rgb;
	clr += clrReflect*diffuseColor;

	return vec4(clr, srcFragColor.a);
}


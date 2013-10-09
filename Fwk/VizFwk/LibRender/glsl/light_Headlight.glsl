
varying vec3 v_ecPosition;
varying vec3 v_ecNormal; 

uniform float u_specularIntensity;
uniform float u_ambientIntensity;
uniform vec3  u_ecLightPosition;
uniform vec3  u_emissiveColor;

//--------------------------------------------------------------------------------------------------
/// lightFragment() - Headlight with all params exposed as uniforms
/// 
/// A fixed, two sided headlight 
//--------------------------------------------------------------------------------------------------
vec4 lightFragment(vec4 srcFragColor, float not_in_use_shadowFactor)
{
    // Light vector (from point to light source)
    vec3 L = normalize(u_ecLightPosition - v_ecPosition);

    // Viewing vector (from point to eye)
    // Since we are in eye space, the eye pos is at (0, 0, 0)
    vec3 V = normalize(-v_ecPosition);

    vec3 N = normalize(v_ecNormal);
    vec3 R = normalize(reflect(-L, N));

    vec3 ambient = srcFragColor.rgb*u_ambientIntensity;
    vec3 diffuse = srcFragColor.rgb*(1.0 - u_ambientIntensity)*abs(dot(N, L));
    vec3 specular = vec3(u_specularIntensity*pow(max(dot(R, V), 0.0), 8.0));

    return vec4(ambient + diffuse + specular + u_emissiveColor, srcFragColor.a);
}

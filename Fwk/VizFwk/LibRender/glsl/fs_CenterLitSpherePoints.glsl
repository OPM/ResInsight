
varying vec3 v_ecPosition;

vec4 srcFragment();
vec4 lightFragment(vec4 srcFragColor, float shadowFactor);

//--------------------------------------------------------------------------------------------------
/// Fragment Shader - Points rendered as spheres with diffuse lighting 
//--------------------------------------------------------------------------------------------------
void main()
{
    //const vec3  ecLightPosition = vec3(0.5, 5.0, 7.0);
    //vec3 lightDir = normalize(ecLightPosition - v_ecPosition);
	const vec3 lightDir = vec3(0.0, 0.0, 1.0);

    // Calculate normal from texture coordinates
    vec3 N;
    N.xy = gl_PointCoord.st*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
	float mag = dot(N.xy, N.xy);
	
	// Kill pixels outside circle
    if (mag > 1.0) discard;   
    N.z = sqrt(1.0 - mag);

    // Calculate diffuse lighting
    float diffuse = max(0.0, dot(lightDir, N));

    vec4 color = srcFragment();
    gl_FragColor = vec4(color.rgb*diffuse, color.a);

    //gl_FragDepth = gl_FragCoord.z - 15.0*(1.0-mag);
}






varying vec2 v_circleFactors;
varying float v_alpha;

vec4 srcFragment();

//--------------------------------------------------------------------------------------------------
/// Fragment Shader - Particle trace comets
//--------------------------------------------------------------------------------------------------
void main()
{
    const vec3 lightDir = vec3(0, 0, 1);

    // Calculate normal from the circle factors
    vec3 N;
    N.xy = vec2(v_circleFactors.x, v_circleFactors.y);
    float mag = dot(N.xy, N.xy);
    if (mag > 1.0) discard;   
    N.z = sqrt(1.0 - mag);

    float diffuse = max(0.0, dot(lightDir, N));

	vec3 color = srcFragment().rgb;
    gl_FragColor = vec4(color*diffuse, v_alpha);
}


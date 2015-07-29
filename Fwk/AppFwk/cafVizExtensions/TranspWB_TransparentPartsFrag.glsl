//--------------------------------------------------------------------------------------
// Weighted Blended Order-Independent Transparency
// Morgan McGuire and Louis Bavoil NVIDIA
// Journal of Computer Graphics Techniques Vol. 2, No. 2, 2013

//--------------------------------------------------------------------------------------

#extension GL_ARB_draw_buffers : require

vec4 srcFragment();
vec4 lightFragment(vec4 srcFragColor, float shadowFactor);

uniform float cameraNear;
uniform float cameraFar;

float realZDepth(float fragCoordZ) {

    return (cameraFar * cameraNear) / (cameraFar - fragCoordZ * (cameraFar - cameraNear)) ;
}

float depthWeight(float fragCoordZ, float alpha)
{
    //return 1.0; // For testing
    //return alpha * max(1e-2, 3e3 * pow((1 - fragCoordZ), 3));  // Proposed by paper

    float zDepth = realZDepth(fragCoordZ);
    return alpha * max(1e-2, min(3e3, 10 / (1e5 + pow(zDepth/10, 3) + pow(zDepth/200, 6) ))); // Proposed by paper
    //return zDepth; 
}

void main(void)
{
    vec4 color = srcFragment();
    color = lightFragment(color, 1.0);

    vec3 preMultipliedRgb = color.a*color.rgb;

    gl_FragData[0] = vec4(preMultipliedRgb * depthWeight(gl_FragCoord.z, color.a), color.a);
    gl_FragData[1].r = color.a * depthWeight(gl_FragCoord.z, color.a);
}

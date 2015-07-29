//--------------------------------------------------------------------------------------
// Weighted Blended Order-Independent Transparency
// Morgan McGuire and Louis Bavoil NVIDIA
// Journal of Computer Graphics Techniques Vol. 2, No. 2, 2013

//--------------------------------------------------------------------------------------

#extension GL_ARB_draw_buffers : require

vec4 srcFragment();
vec4 lightFragment(vec4 srcFragColor, float shadowFactor);

uniform int isOpaquePass;

float depthWeight(float depth, float alpha)
{
    //return 1.0; // For testing
    return alpha * max(1e-2, 3e3 * pow((1 - gl_FragCoord.z), 3));  // Proposed by paper
    //return alpha * max(1e-1, 3e4 * pow((1 - gl_FragCoord.z), 4));  // JJS
}

void main(void)
{
    vec4 color = srcFragment();
    color = lightFragment(color, 1.0);
    if (isOpaquePass == 1)
    {
        if (color.a < 1.0)
        {
            discard;
        }
        else
        {
            gl_FragData[0] = color;
        }
    }
    else
    {
        if (color.a == 1.0)
        {
            discard;
        }
        else
        {
            vec3 preMultipliedRgb = color.a*color.rgb;

            gl_FragData[0] = vec4(preMultipliedRgb * depthWeight(0, color.a), color.a);
            gl_FragData[1].r = color.a * depthWeight(0, color.a);
        }
    }
}

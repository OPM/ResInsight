//--------------------------------------------------------------------------------------
// Weighted Blended Order-Independent Transparency
// Morgan McGuire and Louis Bavoil NVIDIA
// Journal of Computer Graphics Techniques Vol. 2, No. 2, 2013
// Adapted by Jacob Støren to fit with a pure combination pass with no blending
//--------------------------------------------------------------------------------------

#extension GL_ARB_texture_rectangle : enable    

uniform sampler2DRect sumWeightedRgbAndProductOneMinusAlphaTexture;
uniform sampler2DRect sumWeightedAlphaTexture;
uniform sampler2DRect opaceColorTexture;

void main(void)
{
    // Reference codeFrom the paper. Not correct because we do no blending in the last pass
    // gl_FragColor = vec4(sumWeightedColor/clamp(sumWeightedAlpha, 1e-4, 5e4), productOneMinusAlpha);

    // Get the different components from the textures 
    vec3  sumWeightedColor = texture2DRect(sumWeightedRgbAndProductOneMinusAlphaTexture, gl_FragCoord.xy).rgb;
    float sumWeightedAlpha = texture2DRect(sumWeightedAlphaTexture, gl_FragCoord.xy).r;
    float productOneMinusAlpha = texture2DRect(sumWeightedRgbAndProductOneMinusAlphaTexture, gl_FragCoord.xy).a;

    vec4 opaceColor = texture2DRect(opaceColorTexture, gl_FragCoord.xy);

    //vec3 sumWColorPrSumWAlpha = sumWeightedColor/sumWeightedAlpha;
    vec3 sumWColorPrSumWAlpha = sumWeightedColor/clamp(sumWeightedAlpha, 1.0e-7, 5.0e3 );


    // Helpers to visualize different parts of the equation
    vec4 resultColor; 

    //resultColor = opaceColor;
    //resultColor = opaceColor * productOneMinusAlpha;
    //resultColor = vec4(productOneMinusAlpha * opaceColor.rgb, 1.0);
    //resultColor = vec4( sumWColorPrSumWAlpha, 1.0);
    //resultColor = vec4(sumWColorPrSumWAlpha  + productOneMinusAlpha * opaceColor.rgb, 1.0);
    //resultColor = vec4(productOneMinusAlpha* sumWColorPrSumWAlpha, 1.0);

    // The final correct equation
    resultColor = vec4(sumWColorPrSumWAlpha - productOneMinusAlpha * sumWColorPrSumWAlpha + productOneMinusAlpha * opaceColor.rgb, 1.0);

    gl_FragColor =  resultColor;
}

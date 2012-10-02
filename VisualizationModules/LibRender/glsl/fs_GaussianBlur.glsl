
#if defined(USE_TEXTURE_RECT)
#extension GL_ARB_texture_rectangle : enable            
#endif


// Shader program based on source code in article:
//   http://callumhay.blogspot.no/2010/09/gaussian-blur-shader-glsl.html
// Which in turn is based on GPU Gems 3; Chapter 40 ('Incremental Computation of the Gaussian' by Ken Turkowski).
//   http://http.developer.nvidia.com/GPUGems3/gpugems3_ch40.html


// The sigma value is the standard deviation for the Gaussian distribution
// A higher sigma value means more blur, but the sigma value should be tuned to the kernel size.

// Defensive rule of thumb when choosing sigma values vs kernel size (see first link below):
// The gaussian distribution will have the vast majority of values in the interval [-3*sigma, 3*sigma], 
// so a kernel width of 6*sigma should suffice. Just make sure to round it up to the closest odd number, 
// so your kernel will be symmetric.
// http://www.gamedev.net/topic/526122-gaussian-blur-calculating-kernel-weight/
// http://en.wikipedia.org/wiki/Gaussian_blur
//
// See also for a discussion of sigma vs kernel size:
// http://theinstructionlimit.com/gaussian-blur-revisited-part-two
// His suggestions for sigma values based on kernel size
//   17-tap : 3.66 – 4.95
//   15-tap : 2.85 – 3.34
//   13-tap : 2.49 – 2.95
//   11-tap : 2.18 – 2.54
//    9-tap : 1.8 – 2.12
//    7-tap : 1.55 – 1.78
//    5-tap : 1.35 – 1.54

uniform float u_sigma;   // The sigma value for the gaussian function: higher value means more blur
                         // A good value for 9x9 is around 3 to 5
                         // A good value for 7x7 is around 2.5 to 4
                         // A good value for 5x5 is around 2 to 3.5
                         // ... play around with this based on what you need :)


// Texture that will be blurred by this shader
#if defined(USE_TEXTURE_RECT)
uniform sampler2DRect u_blurSampler;
#else
uniform sampler2D u_blurSampler;

uniform float u_blurSize;  // This should usually be equal to
                         // 1.0f / texture_pixel_width for a horizontal blur, and
                         // 1.0f / texture_pixel_height for a vertical blur.
varying vec2 v_texCoord;
#endif



// The following are all mutually exclusive macros for various seperable blurs of varying kernel size
// The original code had these as constants and using defines to choose one. 
// Currently we do more or less the same, but we should investigate using uniforms instead

// Vertical or horizontal pass?
#if defined(VERTICAL_BLUR)
const vec2  blurMultiplyVec      = vec2(0.0f, 1.0f);
#elif defined(HORIZONTAL_BLUR)
const vec2  blurMultiplyVec      = vec2(1.0f, 0.0f);
#endif

#if defined(KERNEL_SIZE)
const float numBlurPixelsPerSide = (KERNEL_SIZE - 1.0f)/2.0f;
#endif

const float pi = 3.14159265f;



//--------------------------------------------------------------------------------------------------
/// Separable Gaussian blur shader 
//--------------------------------------------------------------------------------------------------
void main() 
{
    // Incremental Gaussian Coefficent Calculation (See GPU Gems 3 pp. 877 - 889)
    vec3 incrementalGaussian;
    incrementalGaussian.x = 1.0f / (sqrt(2.0f * pi) * u_sigma);
    incrementalGaussian.y = exp(-0.5f / (u_sigma * u_sigma));
    incrementalGaussian.z = incrementalGaussian.y * incrementalGaussian.y;

    vec4 avgValue = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    float coefficientSum = 0.0f;

    // Take the central sample first...
#if defined(USE_TEXTURE_RECT)
	avgValue += texture2DRect(u_blurSampler, gl_FragCoord.xy) * incrementalGaussian.x;
#else
	avgValue += texture2D(u_blurSampler, v_texCoord) * incrementalGaussian.x;
#endif
    coefficientSum += incrementalGaussian.x;
    incrementalGaussian.xy *= incrementalGaussian.yz;

    // Go through the remaining 8 vertical samples (4 on each side of the center)
    for (float i = 1.0f; i <= numBlurPixelsPerSide; i++) 
    { 
#if defined(USE_TEXTURE_RECT)
        avgValue += texture2DRect(u_blurSampler, gl_FragCoord.xy - i*blurMultiplyVec) * incrementalGaussian.x;         
        avgValue += texture2DRect(u_blurSampler, gl_FragCoord.xy + i*blurMultiplyVec) * incrementalGaussian.x;         
#else
        avgValue += texture2D(u_blurSampler, v_texCoord - i*u_blurSize*blurMultiplyVec) * incrementalGaussian.x;         
        avgValue += texture2D(u_blurSampler, v_texCoord + i*u_blurSize*blurMultiplyVec) * incrementalGaussian.x;         
#endif
        coefficientSum += 2*incrementalGaussian.x;
        incrementalGaussian.xy *= incrementalGaussian.yz;
    }

    gl_FragColor = avgValue / coefficientSum;
}


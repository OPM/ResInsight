
varying float v_logz;

uniform float u_logDepthFC;   // = 2.0 / log2(farPlane + 1.0), updated each frame

#define CVF_LOG_DEPTH_IMPL

//--------------------------------------------------------------------------------------------------
/// Fragment component - logarithmic depth buffer
/// Writes a log-distributed depth to gl_FragDepth, replacing the default linear depth.
/// Requires v_logz to be set by calcLogDepth() in the vertex shader.
/// Reference: http://outerra.blogspot.com/2013/07/logarithmic-depth-buffer-optimizations.html
//--------------------------------------------------------------------------------------------------
void applyLogDepth()
{
    gl_FragDepth = log2(max(1.0e-6, v_logz)) * u_logDepthFC * 0.5;
}


varying float v_logz;

uniform float u_logDepthFC;           // = 2.0 / log2(farPlane + 1.0), updated each frame
uniform float u_useLogDepth;          // 1.0 = perspective (log depth), 0.0 = orthographic (linear depth)
uniform float u_polygonOffsetFactor;  // mirrors glPolygonOffset factor (default 0)
uniform float u_polygonOffsetUnits;   // mirrors glPolygonOffset units  (default 0)

#define CVF_LOG_DEPTH_IMPL

//--------------------------------------------------------------------------------------------------
/// Fragment component - logarithmic depth buffer
/// Writes a log-distributed depth to gl_FragDepth, replacing the default linear depth.
/// Requires v_logz to be set by calcLogDepth() in the vertex shader.
///
/// For orthographic projection (u_useLogDepth == 0), falls back to the standard linear
/// gl_FragCoord.z, since w_clip == 1 in orthographic and log depth gives no benefit.
///
/// Also implements polygon offset manually, because glPolygonOffset has no effect when
/// gl_FragDepth is written explicitly. Set u_polygonOffsetFactor/Units to mirror the
/// glPolygonOffset parameters used on the effect.
///
/// Reference: http://outerra.blogspot.com/2013/07/logarithmic-depth-buffer-optimizations.html
//--------------------------------------------------------------------------------------------------
void applyLogDepth()
{
    float depth;
    if (u_useLogDepth > 0.5)
    {
        depth = log2(max(1.0e-6, v_logz)) * u_logDepthFC * 0.5;
    }
    else
    {
        depth = gl_FragCoord.z;
    }

    // Manual polygon offset: factor * max_slope + units * depth_resolution
    // Resolution constant = 1/2^24 for a 24-bit depth buffer.
    float slope = max(abs(dFdx(depth)), abs(dFdy(depth)));
    depth += u_polygonOffsetFactor * slope + u_polygonOffsetUnits * (1.0 / 16777216.0);

    gl_FragDepth = depth;
}

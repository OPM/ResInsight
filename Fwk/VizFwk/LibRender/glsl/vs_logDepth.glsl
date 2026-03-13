
varying float v_logz;

#define CVF_LOG_DEPTH_IMPL

//--------------------------------------------------------------------------------------------------
/// Vertex component - logarithmic depth buffer (Outerra method)
/// Store (1 + w_clip) in v_logz for use in the fragment shader.
/// Call calcLogDepth(gl_Position) at the end of the vertex main() to activate.
//--------------------------------------------------------------------------------------------------
void calcLogDepth(vec4 clipPos)
{
    v_logz = 1.0 + clipPos.w;
}

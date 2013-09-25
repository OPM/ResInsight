
uniform mat4 cvfu_modelViewMatrix;
uniform mat4 u_lightViewProjectionMatrix;

attribute vec4 cvfa_vertex;

varying vec4 v_shadowCoord;

#define CVF_CALC_SHADOW_COORD_IMPL

//--------------------------------------------------------------------------------------------------
/// Calculate shadow coord and set the v_shadowCoord varying
//--------------------------------------------------------------------------------------------------
void calcShadowCoord()
{
    // Also including cvfu_modelViewMatrix (with inverse camera in  lightViewProjectionMatrix)  to include
    // the part transformations. If not, we need to pass the Model Matrix as well
    v_shadowCoord = u_lightViewProjectionMatrix*cvfu_modelViewMatrix*cvfa_vertex;
}


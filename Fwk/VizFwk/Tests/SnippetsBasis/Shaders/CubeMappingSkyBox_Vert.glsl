                  
uniform mat4 cvfu_projectionMatrix;                                                 
uniform mat4 cvfu_modelViewMatrix;                                          
                                                                           
attribute vec4 cvfa_vertex;                                                 

varying vec3 v_cubeMapTextureCoord;

                                                                           
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void main(void) 
{
    // Pass on the texture coordinates 
    v_cubeMapTextureCoord = normalize(cvfa_vertex.xyz);

	// Only rotation part of modelview here!
	gl_Position = cvfu_projectionMatrix*vec4((cvfu_modelViewMatrix*vec4(cvfa_vertex.xyz, 0)).xyz, 1);
}

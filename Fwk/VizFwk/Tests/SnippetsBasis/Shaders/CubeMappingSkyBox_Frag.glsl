
uniform samplerCube u_cubeMap;                                               

varying vec3 v_cubeMapTextureCoord; 
                                                                           

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void main ()                                                               
{                                                                          
    vec4 clrCubeMap = textureCube(u_cubeMap, v_cubeMapTextureCoord);                             

    gl_FragColor = clrCubeMap;                                             
}


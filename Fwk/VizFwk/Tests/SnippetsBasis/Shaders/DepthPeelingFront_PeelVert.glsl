//--------------------------------------------------------------------------------------
// Order Independent Transparency with Depth Peeling
//
// Author: Louis Bavoil
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


uniform mat4 cvfu_modelViewProjectionMatrix;                     
uniform mat3 cvfu_normalMatrix;                                  
                                                                
attribute vec4 cvfa_vertex;                                      
attribute vec3 cvfa_normal;                                      
                                                                
varying float diffuse;      

void main(void)
{
   gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;      
   diffuse = abs(normalize(cvfu_normalMatrix * cvfa_normal).z);   
}

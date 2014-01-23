//--------------------------------------------------------------------------------------
// Order Independent Transparency with Depth Peeling
//
// Author: Louis Bavoil
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#extension GL_ARB_texture_rectangle : enable    
uniform sampler2DRect TempTex;

void main(void)
{
	gl_FragColor = texture(TempTex, gl_FragCoord.xy);
}

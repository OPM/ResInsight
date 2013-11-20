//--------------------------------------------------------------------------------------
// Order Independent Transparency with Depth Peeling
//
// Author: Louis Bavoil
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#extension GL_ARB_texture_rectangle : enable  

uniform sampler2DRect DepthTex;
uniform sampler2DRect SolidDepthTex;

vec4 ShadeFragment();

void main(void)
{
	float solidDepth = texture(SolidDepthTex, gl_FragCoord.xy).r;

    if (gl_FragCoord.z > solidDepth) 
    {
		discard;
	}

	// Bit-exact comparison between FP32 z-buffer and fragment depth
	float frontDepth = texture(DepthTex, gl_FragCoord.xy).r;
	
    if (gl_FragCoord.z <= frontDepth) 
    {
		discard;
	}

	// Shade all the fragments behind the z-buffer
	vec4 color = ShadeFragment();
	gl_FragColor = vec4(color.rgb * color.a, color.a);
}

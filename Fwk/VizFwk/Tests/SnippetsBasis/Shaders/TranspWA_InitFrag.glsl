//--------------------------------------------------------------------------------------
// Order Independent Transparency with Average Color
//
// Author: Louis Bavoil
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#extension GL_ARB_draw_buffers : require

vec4 srcFragment();
vec4 lightFragment(vec4 srcFragColor, float shadowFactor);

void main(void)
{
	vec4 color = srcFragment();
	color = lightFragment(color, 1.0);

	gl_FragData[0] = vec4(color.rgb * color.a, color.a);
	gl_FragData[1] = vec4(1.0);
}

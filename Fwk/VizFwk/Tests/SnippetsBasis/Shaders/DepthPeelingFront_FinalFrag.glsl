//--------------------------------------------------------------------------------------
// Order Independent Transparency with Depth Peeling
//
// Author: Louis Bavoil
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

uniform sampler2DRect ColorTex;
uniform sampler2DRect BackgroundAndOpaqueTex;

void main(void)
{
    vec3 backgroundAndOpaqueColor = texture(BackgroundAndOpaqueTex, gl_FragCoord.xy).rgb;

	vec4 frontColor = texture(ColorTex, gl_FragCoord.xy);
	gl_FragColor.rgb = frontColor.rgb + backgroundAndOpaqueColor * frontColor.a;
}

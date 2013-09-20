#version 330

uniform sampler2DShadow u_shadowMap;

in vec4 v_shadowCoord;

out vec4 o_fragColor;

vec4 srcFragment();
vec4 lightFragment(vec4 srcFragColor, float inShadowFactor);

//--------------------------------------------------------------------------------------------------
/// Fragment Shader - Shadow
//--------------------------------------------------------------------------------------------------
void main()
{
	vec4 fragColor = srcFragment();

    // Offset to avoid self-shadowing
    vec4 offShadowCoord = v_shadowCoord;
	offShadowCoord.z -= 0.0005;

    // 3x3 sampling and averaging
    float sum;
    sum  = textureProjOffset(u_shadowMap, offShadowCoord, ivec2(-1, -1));
    sum += textureProjOffset(u_shadowMap, offShadowCoord, ivec2(1, -1));
    sum += textureProjOffset(u_shadowMap, offShadowCoord, ivec2(-1, 1));
    sum += textureProjOffset(u_shadowMap, offShadowCoord, ivec2(1, 1));
    sum += textureProjOffset(u_shadowMap, offShadowCoord, ivec2(0, 0));
    sum += textureProjOffset(u_shadowMap, offShadowCoord, ivec2(1, 0));
    sum += textureProjOffset(u_shadowMap, offShadowCoord, ivec2(0, 1));
    sum += textureProjOffset(u_shadowMap, offShadowCoord, ivec2(-1, 0));
    sum += textureProjOffset(u_shadowMap, offShadowCoord, ivec2(0, -1));

	// shadowFactor 0..1 (0 not in shadow, 1 toally in shadow)
    float shadowFactor = sum*0.111111;

	vec3 coordPos  = offShadowCoord.xyz / offShadowCoord.w;
	if (coordPos.x < 0.0 || coordPos.y < 0.0 || coordPos.x > 1.0 || coordPos.y > 1.0)
   	{
		shadowFactor = 1.0;
	}

	o_fragColor = lightFragment(fragColor, shadowFactor);	
}

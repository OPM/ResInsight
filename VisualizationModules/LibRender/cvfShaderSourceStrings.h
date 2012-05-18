
// -----------------------------------------
// THIS IS A GENERATED FILE!!  DO NOT MODIFY
// -----------------------------------------

//#############################################################################################################################
//#############################################################################################################################
static const char calcClipDistances_inl[] =
"                                                                                                      \n"
"uniform int u_clipPlaneCount;                                                                         \n"
"uniform vec4 u_ecClipPlanes[6];                                                                       \n"
"                                                                                                      \n"
"// The dimensioning should probably be via a define to be consistent between vs and fs                \n"
"varying float v_clipDist[6];                                                                          \n"
"                                                                                                      \n"
"#define CVF_CALC_CLIP_DISTANCES_IMPL                                                                  \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Calculate clip distances and set the array of varyings                                            \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void calcClipDistances(vec4 ecPosition)                                                               \n"
"{                                                                                                     \n"
"    // Using eye space clipping plane directly                                                        \n"
"    int i;                                                                                            \n"
"    for (i = 0; i < u_clipPlaneCount; i++)                                                            \n"
"    {                                                                                                 \n"
"        v_clipDist[i] = dot(u_ecClipPlanes[i], ecPosition);                                           \n"
"    }                                                                                                 \n"
"                                                                                                      \n"
"    // Using clipping plane specified in world coords would be something like this                    \n"
"    //vec4 ecPlane = u_wcClipPlane*cvfu_modelViewMatrixInverse;                                       \n"
"    //clipDist = dot(ecPlane, ecPosition);                                                            \n"
"                                                                                                      \n"
"    // The proper OpenGL way uing built-in varying gl_ClipDistance                                    \n"
"    // Have not been able to get this to work on ATI yet. Latest driver tested is: Catalyst 11.6      \n"
"    //gl_ClipDistance[0] = clipDist;                                                                  \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char calcShadowCoord_inl[] =
"                                                                                                            \n"
"uniform mat4 cvfu_modelViewMatrix;                                                                          \n"
"uniform mat4 u_lightViewProjectionMatrix;                                                                   \n"
"                                                                                                            \n"
"attribute vec4 cvfa_vertex;                                                                                 \n"
"                                                                                                            \n"
"varying vec4 v_shadowCoord;                                                                                 \n"
"                                                                                                            \n"
"#define CVF_CALC_SHADOW_COORD_IMPL                                                                          \n"
"                                                                                                            \n"
"//--------------------------------------------------------------------------------------------------        \n"
"/// Calculate shadow coord and set the v_shadowCoord varying                                                \n"
"//--------------------------------------------------------------------------------------------------        \n"
"void calcShadowCoord()                                                                                      \n"
"{                                                                                                           \n"
"    // Also including cvfu_modelViewMatrix (with inverse camera in  lightViewProjectionMatrix)  to include  \n"
"    // the part transformations. If not, we need to pass the Model Matrix as well                           \n"
"    v_shadowCoord = u_lightViewProjectionMatrix*cvfu_modelViewMatrix*cvfa_vertex;                           \n"
"}                                                                                                           \n";



//#############################################################################################################################
//#############################################################################################################################
static const char checkDiscard_ClipDistances_inl[] =
"                                                                                                      \n"
"uniform int u_clipPlaneCount;                                                                         \n"
"                                                                                                      \n"
"varying float v_clipDist[6];                                                                          \n"
"                                                                                                      \n"
"#define CVF_CHECK_DISCARD_FRAGMENT_IMPL                                                               \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Check if fragment should be discarded based on clip distances and discard if needed               \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void checkDiscardFragment()                                                                           \n"
"{                                                                                                     \n"
"    int i;                                                                                            \n"
"    for (i = 0; i < u_clipPlaneCount; i++)                                                            \n"
"    {                                                                                                 \n"
"        if (v_clipDist[i] < 0) discard;                                                               \n"
"    }                                                                                                 \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char fs_CenterLitSpherePoints_inl[] =
"                                                                                                      \n"
"varying vec3 v_ecPosition;                                                                            \n"
"                                                                                                      \n"
"vec4 srcFragment();                                                                                   \n"
"vec4 lightFragment(vec4 srcFragColor, float shadowFactor);                                            \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Fragment Shader - Points rendered as spheres with diffuse lighting                                \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main()                                                                                           \n"
"{                                                                                                     \n"
"    //const vec3  ecLightPosition = vec3(0.5, 5.0, 7.0);                                              \n"
"    //vec3 lightDir = normalize(ecLightPosition - v_ecPosition);                                      \n"
"    const vec3 lightDir = vec3(0, 0, 1);                                                              \n"
"                                                                                                      \n"
"    // Calculate normal from texture coordinates                                                      \n"
"    vec3 N;                                                                                           \n"
"    N.xy = gl_PointCoord.st*vec2(2.0, -2.0) + vec2(-1.0, 1.0);                                        \n"
"    float mag = dot(N.xy, N.xy);                                                                      \n"
"                                                                                                      \n"
"    // Kill pixels outside circle                                                                     \n"
"    if (mag > 1) discard;                                                                             \n"
"    N.z = sqrt(1 - mag);                                                                              \n"
"                                                                                                      \n"
"    // Calculate diffuse lighting                                                                     \n"
"    float diffuse = max(0.0, dot(lightDir, N));                                                       \n"
"                                                                                                      \n"
"    vec4 color = srcFragment();                                                                       \n"
"    gl_FragColor = vec4(color.rgb*diffuse, color.a);                                                  \n"
"                                                                                                      \n"
"    //gl_FragDepth = gl_FragCoord.z - 15.0*(1-mag);                                                   \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char fs_FixedColorMagenta_inl[] =
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Fragment Shader - Fixed color magenta (for debugging)                                             \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main()                                                                                           \n"
"{                                                                                                     \n"
"    gl_FragColor = vec4(1,0,1,1);                                                                     \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char fs_GradientTopBottom_inl[] =
"                                                                                                      \n"
"uniform vec4 u_topColor;                                                                              \n"
"uniform vec4 u_bottomColor;                                                                           \n"
"                                                                                                      \n"
"varying vec2 v_texCoord;                                                                              \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Fragment Shader - Top -> Bottom gradient                                                          \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main()                                                                                           \n"
"{                                                                                                     \n"
"    gl_FragColor = u_bottomColor + (u_topColor - u_bottomColor)*v_texCoord.y;                         \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char fs_GradientTopMiddleBottom_inl[] =
"                                                                                                      \n"
"uniform vec4 u_topColor;                                                                              \n"
"uniform vec4 u_middleColor;                                                                           \n"
"uniform vec4 u_bottomColor;                                                                           \n"
"                                                                                                      \n"
"varying vec2 v_texCoord;                                                                              \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Fragment Shader - Top -> Bottom gradient                                                          \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main()                                                                                           \n"
"{                                                                                                     \n"
"    if (v_texCoord.y > 0.5)                                                                           \n"
"    {                                                                                                 \n"
"        gl_FragColor = u_middleColor+ (u_topColor - u_middleColor)*(2.0*(v_texCoord.y - 0.5));        \n"
"    }                                                                                                 \n"
"    else                                                                                              \n"
"    {                                                                                                 \n"
"        gl_FragColor = u_bottomColor + (u_middleColor - u_bottomColor)*v_texCoord.y*2.0;              \n"
"    }                                                                                                 \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char fs_ParticleTraceComets_inl[] =
"                                                                                                      \n"
"varying vec2 v_circleFactors;                                                                         \n"
"varying float v_alpha;                                                                                \n"
"                                                                                                      \n"
"vec4 srcFragment();                                                                                   \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Fragment Shader - Particle trace comets                                                           \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main()                                                                                           \n"
"{                                                                                                     \n"
"    const vec3 lightDir = vec3(0, 0, 1);                                                              \n"
"                                                                                                      \n"
"    // Calculate normal from the circle factors                                                       \n"
"    vec3 N;                                                                                           \n"
"    N.xy = vec2(v_circleFactors.x, v_circleFactors.y);                                                \n"
"    float mag = dot(N.xy, N.xy);                                                                      \n"
"    if (mag > 1) discard;                                                                             \n"
"    N.z = sqrt(1 - mag);                                                                              \n"
"                                                                                                      \n"
"    float diffuse = max(0.0, dot(lightDir, N));                                                       \n"
"                                                                                                      \n"
"    vec3 color = srcFragment().rgb;                                                                   \n"
"    gl_FragColor = vec4(color*diffuse, v_alpha);                                                      \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char fs_Shadow_v33_inl[] =
"#version 330                                                                                          \n"
"                                                                                                      \n"
"uniform sampler2DShadow u_shadowMap;                                                                  \n"
"                                                                                                      \n"
"in vec4 v_shadowCoord;                                                                                \n"
"                                                                                                      \n"
"out vec4 o_fragColor;                                                                                 \n"
"                                                                                                      \n"
"vec4 srcFragment();                                                                                   \n"
"vec4 lightFragment(vec4 srcFragColor, float inShadowFactor);                                          \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Fragment Shader - Shadow                                                                          \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main()                                                                                           \n"
"{                                                                                                     \n"
"    vec4 fragColor = srcFragment();                                                                   \n"
"                                                                                                      \n"
"    // Offset to avoid self-shadowing                                                                 \n"
"    vec4 offShadowCoord = v_shadowCoord;                                                              \n"
"    offShadowCoord.z -= 0.0005;                                                                       \n"
"                                                                                                      \n"
"    // 3x3 sampling and averaging                                                                     \n"
"    float sum;                                                                                        \n"
"    sum  = textureProjOffset(u_shadowMap, offShadowCoord, ivec2(-1, -1));                             \n"
"    sum += textureProjOffset(u_shadowMap, offShadowCoord, ivec2(1, -1));                              \n"
"    sum += textureProjOffset(u_shadowMap, offShadowCoord, ivec2(-1, 1));                              \n"
"    sum += textureProjOffset(u_shadowMap, offShadowCoord, ivec2(1, 1));                               \n"
"    sum += textureProjOffset(u_shadowMap, offShadowCoord, ivec2(0, 0));                               \n"
"    sum += textureProjOffset(u_shadowMap, offShadowCoord, ivec2(1, 0));                               \n"
"    sum += textureProjOffset(u_shadowMap, offShadowCoord, ivec2(0, 1));                               \n"
"    sum += textureProjOffset(u_shadowMap, offShadowCoord, ivec2(-1, 0));                              \n"
"    sum += textureProjOffset(u_shadowMap, offShadowCoord, ivec2(0, -1));                              \n"
"                                                                                                      \n"
"    // shadowFactor 0..1 (0 not in shadow, 1 toally in shadow)                                        \n"
"    float shadowFactor = sum*0.111111;                                                                \n"
"                                                                                                      \n"
"    vec3 coordPos  = offShadowCoord.xyz / offShadowCoord.w;                                           \n"
"    if (coordPos.x < 0.0 || coordPos.y < 0.0 || coordPos.x > 1.0 || coordPos.y > 1.0)                 \n"
"       {                                                                                              \n"
"        shadowFactor = 1.0;                                                                           \n"
"    }                                                                                                 \n"
"                                                                                                      \n"
"    o_fragColor = lightFragment(fragColor, shadowFactor);                                             \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char fs_Standard_inl[] =
"                                                                                                         \n"
"// Prototypes not needed when we concatinate files, and these breaks the compile on VMWare Fusion 4.0.4  \n"
"// vec4 srcFragment();                                                                                   \n"
"// vec4 lightFragment(vec4 srcFragColor, float shadowFactor);                                            \n"
"                                                                                                         \n"
"//--------------------------------------------------------------------------------------------------     \n"
"/// Fragment Shader - Standard                                                                           \n"
"//--------------------------------------------------------------------------------------------------     \n"
"void main()                                                                                              \n"
"{                                                                                                        \n"
"#ifdef CVF_CHECK_DISCARD_FRAGMENT_IMPL                                                                   \n"
"    checkDiscardFragment();                                                                              \n"
"#endif                                                                                                   \n"
"                                                                                                         \n"
"    vec4 color = srcFragment();                                                                          \n"
"    color = lightFragment(color, 1.0);                                                                   \n"
"                                                                                                         \n"
"    gl_FragColor = color;                                                                                \n"
"}                                                                                                        \n";



//#############################################################################################################################
//#############################################################################################################################
static const char fs_Text_inl[] =
"                                                                                                      \n"
"uniform sampler2D u_texture2D;                                                                        \n"
"uniform vec3 u_color;                                                                                 \n"
"                                                                                                      \n"
"varying vec2 v_texCoord;                                                                              \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Fragment Shader - Text                                                                            \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main()                                                                                           \n"
"{                                                                                                     \n"
"    float alpha = texture2D(u_texture2D, v_texCoord).a;                                               \n"
"    gl_FragColor = vec4(u_color, alpha);                                                              \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char fs_Unlit_inl[] =
"                                                                                                      \n"
"vec4 srcFragment();                                                                                   \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Fragment Shader - Unlit                                                                           \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main()                                                                                           \n"
"{                                                                                                     \n"
"#ifdef CVF_CHECK_DISCARD_FRAGMENT_IMPL                                                                \n"
"    checkDiscardFragment();                                                                           \n"
"#endif                                                                                                \n"
"                                                                                                      \n"
"    vec4 color = srcFragment();                                                                       \n"
"                                                                                                      \n"
"    gl_FragColor = color;                                                                             \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char fs_VectorDrawer_inl[] =
"                                                                                                      \n"
"varying float v_diffuse;                                                                              \n"
"uniform vec3 u_color;                                                                                 \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Fragment Shader - Unlit                                                                           \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main()                                                                                           \n"
"{                                                                                                     \n"
"#ifdef CVF_CHECK_DISCARD_FRAGMENT_IMPL                                                                \n"
"    checkDiscardFragment();                                                                           \n"
"#endif                                                                                                \n"
"                                                                                                      \n"
"    gl_FragColor = vec4(u_color*v_diffuse, 1.0);                                                      \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char fs_Void_inl[] =
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Fragment Shader - void                                                                            \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main()                                                                                           \n"
"{                                                                                                     \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char gs_PassThroughTriangle_v33_inl[] =
"#version 330                                                                                          \n"
"                                                                                                      \n"
"layout(triangles) in;                                                                                 \n"
"layout(triangle_strip, max_vertices = 3) out;                                                         \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Geometry Shader - Pass through                                                                    \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main()                                                                                           \n"
"{                                                                                                     \n"
"    // Init to avoid warning on nVidia ('gl_Position might be used before being initialized')         \n"
"    // Seems buggy since geom shader shouldn't be invoked if length() is 0                            \n"
"    gl_Position = vec4(0, 0, 0, 0);                                                                   \n"
"                                                                                                      \n"
"    for (int i = 0; i < gl_in.length(); i++)                                                          \n"
"    {                                                                                                 \n"
"        gl_Position = gl_in[i].gl_Position;                                                           \n"
"        EmitVertex();                                                                                 \n"
"    }                                                                                                 \n"
"                                                                                                      \n"
"    EndPrimitive();                                                                                   \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char light_AmbientDiffuse_inl[] =
"                                                                                                      \n"
"varying vec3 v_ecPosition;                                                                            \n"
"varying vec3 v_ecNormal;                                                                              \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// lightFragment() - Simple Headlight                                                                \n"
"///                                                                                                   \n"
"/// A fixed, two sided headlight                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"vec4 lightFragment(vec4 srcFragColor, float not_in_use_shadowFactor)                                  \n"
"{                                                                                                     \n"
"    const vec3  ecLightPosition = vec3(0.5, 5.0, 7.0);                                                \n"
"    const float ambientIntensity = 0.2;                                                               \n"
"                                                                                                      \n"
"    // Light vector (from point to light source)                                                      \n"
"    vec3 L = normalize(ecLightPosition - v_ecPosition);                                               \n"
"                                                                                                      \n"
"    // Viewing vector (from point to eye)                                                             \n"
"    // Since we are in eye space, the eye pos is at (0, 0, 0)                                         \n"
"    vec3 V = normalize(-v_ecPosition);                                                                \n"
"                                                                                                      \n"
"    vec3 N = normalize(v_ecNormal);                                                                   \n"
"    vec3 R = normalize(reflect(-L, N));                                                               \n"
"                                                                                                      \n"
"    vec3 ambient = srcFragColor.rgb*ambientIntensity;                                                 \n"
"    vec3 diffuse = srcFragColor.rgb*(1.0 - ambientIntensity)*abs(dot(N, L));                          \n"
"                                                                                                      \n"
"    return vec4(ambient + diffuse, srcFragColor.a);                                                   \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char light_Phong_inl[] =
"uniform vec3 u_ecLightPosition;                                                                                                  \n"
"uniform float u_ambientIntensity;                                                                                                \n"
"                                                                                                                                 \n"
"varying vec3 v_ecPosition;                                                                                                       \n"
"varying vec3 v_ecNormal;                                                                                                         \n"
"                                                                                                                                 \n"
"//--------------------------------------------------------------------------------------------------                             \n"
"/// lightFragment() - inShadowFactor: 0..1 (1 not in shadow, 0 totally in shadow)                                                \n"
"//--------------------------------------------------------------------------------------------------                             \n"
"vec4 lightFragment(vec4 srcFragColor, float shadowFactor)                                                                        \n"
"{                                                                                                                                \n"
"    const float specularIntensity = 0.5;                                                                                         \n"
"                                                                                                                                 \n"
"    // Light vector (from point to light source)                                                                                 \n"
"    vec3 L = normalize(u_ecLightPosition - v_ecPosition);                                                                        \n"
"                                                                                                                                 \n"
"    // Viewing vector (from point to eye)                                                                                        \n"
"    // Since we are in eye space, the eye pos is at (0, 0, 0)                                                                    \n"
"    vec3 V = normalize(-v_ecPosition);                                                                                           \n"
"                                                                                                                                 \n"
"    vec3 N = normalize(v_ecNormal);                                                                                              \n"
"    vec3 R = normalize(reflect(-L, N));                                                                                          \n"
"                                                                                                                                 \n"
"    vec3 ambient = srcFragColor.rgb*u_ambientIntensity;                                                                          \n"
"    vec3 diffuse = (0.25 + shadowFactor*0.75)*srcFragColor.rgb*(1.0 - u_ambientIntensity)*max(dot(N, L), 0.0);  // mix(step...)  \n"
"    vec3 specular = shadowFactor*vec3(specularIntensity*pow(max(dot(R, V), 0.0), 8.0));                                          \n"
"                                                                                                                                 \n"
"    return vec4(ambient + diffuse + specular, srcFragColor.a);                                                                   \n"
"}                                                                                                                                \n";



//#############################################################################################################################
//#############################################################################################################################
static const char light_PhongDual_inl[] =
"                                                                                                                     \n"
"uniform vec3 u_ecLightPosition;                                                                                      \n"
"uniform vec3 u_ecLightPosition2;                                                                                     \n"
"uniform float u_ambientIntensity;                                                                                    \n"
"                                                                                                                     \n"
"varying vec3 v_ecPosition;                                                                                           \n"
"varying vec3 v_ecNormal;                                                                                             \n"
"                                                                                                                     \n"
"//--------------------------------------------------------------------------------------------------                 \n"
"/// lightFragment() - inShadowFactor: 0..1 (1 not in shadow, 0 totally in shadow)                                    \n"
"//--------------------------------------------------------------------------------------------------                 \n"
"vec4 lightFragment(vec4 srcFragColor, float shadowFactor)                                                            \n"
"{                                                                                                                    \n"
"    const float specularIntensity = 0.5;                                                                             \n"
"                                                                                                                     \n"
"    // Viewing vector (from point to eye)                                                                            \n"
"    // Since we are in eye space, the eye pos is at (0, 0, 0)                                                        \n"
"    vec3 V = normalize(-v_ecPosition);                                                                               \n"
"    vec3 N = normalize(v_ecNormal);                                                                                  \n"
"                                                                                                                     \n"
"    // Light vector (from point to light source)                                                                     \n"
"    vec3 L1 = normalize(u_ecLightPosition - v_ecPosition);                                                           \n"
"    vec3 L2 = normalize(u_ecLightPosition2 - v_ecPosition);                                                          \n"
"                                                                                                                     \n"
"    vec3 R1 = normalize(reflect(-L1, N));                                                                            \n"
"    vec3 R2 = normalize(reflect(-L2, N));                                                                            \n"
"                                                                                                                     \n"
"    vec3 ambient = srcFragColor.rgb*u_ambientIntensity;                                                              \n"
"                                                                                                                     \n"
"    vec3 diffuse = (0.25 + shadowFactor*0.75)*0.8*srcFragColor.rgb*(1.0 - u_ambientIntensity)*max(dot(N, L1), 0.0);  \n"
"    diffuse += (0.25 + shadowFactor*0.75)*0.2*srcFragColor.rgb*(1.0 - u_ambientIntensity)*max(dot(N, L2), 0.0);      \n"
"                                                                                                                     \n"
"    //vec3 specular = vec3(0,0,0);                                                                                   \n"
"    vec3 specular = shadowFactor*0.8*vec3(specularIntensity*pow(max(dot(R1, V), 0.0), 20.0));                        \n"
"    specular += shadowFactor*0.2*vec3(specularIntensity*pow(max(dot(R2, V), 0.0), 20.0));                            \n"
"                                                                                                                     \n"
"    return vec4(ambient + diffuse + specular, srcFragColor.a);                                                       \n"
"}                                                                                                                    \n";



//#############################################################################################################################
//#############################################################################################################################
static const char light_SimpleHeadlight_inl[] =
"                                                                                                      \n"
"varying vec3 v_ecPosition;                                                                            \n"
"varying vec3 v_ecNormal;                                                                              \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// lightFragment() - Simple Headlight                                                                \n"
"///                                                                                                   \n"
"/// A fixed, two sided headlight                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"vec4 lightFragment(vec4 srcFragColor, float not_in_use_shadowFactor)                                  \n"
"{                                                                                                     \n"
"    const vec3  ecLightPosition = vec3(0.5, 5.0, 7.0);                                                \n"
"    const float ambientIntensity = 0.2;                                                               \n"
"    const float specularIntensity = 0.5;                                                              \n"
"                                                                                                      \n"
"    // Light vector (from point to light source)                                                      \n"
"    vec3 L = normalize(ecLightPosition - v_ecPosition);                                               \n"
"                                                                                                      \n"
"    // Viewing vector (from point to eye)                                                             \n"
"    // Since we are in eye space, the eye pos is at (0, 0, 0)                                         \n"
"    vec3 V = normalize(-v_ecPosition);                                                                \n"
"                                                                                                      \n"
"    vec3 N = normalize(v_ecNormal);                                                                   \n"
"    vec3 R = normalize(reflect(-L, N));                                                               \n"
"                                                                                                      \n"
"    vec3 ambient = srcFragColor.rgb*ambientIntensity;                                                 \n"
"    vec3 diffuse = srcFragColor.rgb*(1.0 - ambientIntensity)*abs(dot(N, L));                          \n"
"    vec3 specular = vec3(specularIntensity*pow(max(dot(R, V), 0.0), 8.0));                            \n"
"                                                                                                      \n"
"    return vec4(ambient + diffuse + specular, srcFragColor.a);                                        \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char light_SimpleHeadlight_spec_uniform_inl[] =
"                                                                                                      \n"
"varying vec3 v_ecPosition;                                                                            \n"
"varying vec3 v_ecNormal;                                                                              \n"
"                                                                                                      \n"
"uniform float u_specularIntensity;                                                                    \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// lightFragment() - Simple Headlight with specular uniform                                          \n"
"///                                                                                                   \n"
"/// A fixed, two sided headlight                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"vec4 lightFragment(vec4 srcFragColor, float not_in_use_shadowFactor)                                  \n"
"{                                                                                                     \n"
"    const vec3  ecLightPosition = vec3(0.5, 5.0, 7.0);                                                \n"
"    const float ambientIntensity = 0.2;                                                               \n"
"                                                                                                      \n"
"    // Light vector (from point to light source)                                                      \n"
"    vec3 L = normalize(ecLightPosition - v_ecPosition);                                               \n"
"                                                                                                      \n"
"    // Viewing vector (from point to eye)                                                             \n"
"    // Since we are in eye space, the eye pos is at (0, 0, 0)                                         \n"
"    vec3 V = normalize(-v_ecPosition);                                                                \n"
"                                                                                                      \n"
"    vec3 N = normalize(v_ecNormal);                                                                   \n"
"    vec3 R = normalize(reflect(-L, N));                                                               \n"
"                                                                                                      \n"
"    vec3 ambient = srcFragColor.rgb*ambientIntensity;                                                 \n"
"    vec3 diffuse = srcFragColor.rgb*(1.0 - ambientIntensity)*abs(dot(N, L));                          \n"
"    vec3 specular = vec3(u_specularIntensity*pow(max(dot(R, V), 0.0), 8.0));                          \n"
"                                                                                                      \n"
"    return vec4(ambient + diffuse + specular, srcFragColor.a);                                        \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char src_Color_inl[] =
"                                                                                                      \n"
"uniform vec4 u_color;                                                                                 \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// srcFragment() - Single color by uniform                                                           \n"
"//--------------------------------------------------------------------------------------------------  \n"
"vec4 srcFragment()                                                                                    \n"
"{                                                                                                     \n"
"    return u_color;                                                                                   \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char src_Texture_inl[] =
"                                                                                                      \n"
"uniform sampler2D u_texture2D;                                                                        \n"
"                                                                                                      \n"
"varying vec2 v_texCoord;                                                                              \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Texture 2D source fragment                                                                        \n"
"//--------------------------------------------------------------------------------------------------  \n"
"vec4 srcFragment()                                                                                    \n"
"{                                                                                                     \n"
"    return texture2D(u_texture2D, v_texCoord);                                                        \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char src_TextureFromPointCoord_inl[] =
"                                                                                                      \n"
"uniform sampler2D u_texture2D;                                                                        \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Texture 2D source fragment                                                                        \n"
"//--------------------------------------------------------------------------------------------------  \n"
"vec4 srcFragment()                                                                                    \n"
"{                                                                                                     \n"
"    return texture2D(u_texture2D, gl_PointCoord.xy);                                                  \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char src_TextureGlobalAlpha_inl[] =
"                                                                                                      \n"
"uniform sampler2D u_texture2D;                                                                        \n"
"uniform float      u_alpha;                                                                           \n"
"                                                                                                      \n"
"varying vec2 v_texCoord;                                                                              \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Texture 2D source fragment                                                                        \n"
"//--------------------------------------------------------------------------------------------------  \n"
"vec4 srcFragment()                                                                                    \n"
"{                                                                                                     \n"
"    return vec4(texture2D(u_texture2D, v_texCoord).rgb, u_alpha);                                     \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char src_TextureRectFromFragCoord_v33_inl[] =
"#version 330                                                                                          \n"
"                                                                                                      \n"
"uniform sampler2DRect u_texture2DRect;                                                                \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Texture 2D source fragment                                                                        \n"
"//--------------------------------------------------------------------------------------------------  \n"
"vec4 srcFragment()                                                                                    \n"
"{                                                                                                     \n"
"    return texture(u_texture2DRect, gl_FragCoord.xy);                                                 \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char vs_DistanceScaledPoints_inl[] =
"                                                                                                      \n"
"uniform mat4  cvfu_modelViewProjectionMatrix;                                                         \n"
"uniform mat4  cvfu_modelViewMatrix;                                                                   \n"
"uniform float cvfu_pixelHeightAtUnitDistance;                                                         \n"
"                                                                                                      \n"
"// Point radius in world space                                                                        \n"
"uniform float u_pointRadius;                                                                          \n"
"                                                                                                      \n"
"attribute vec4 cvfa_vertex;                                                                           \n"
"attribute vec2 cvfa_texCoord;                                                                         \n"
"                                                                                                      \n"
"varying vec3 v_ecPosition;                                                                            \n"
"varying vec2 v_texCoord;                                                                              \n"
"                                                                                                      \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Vertex Shader for point rendering. Scales point size based on distance from eye                   \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main ()                                                                                          \n"
"{                                                                                                     \n"
"    // Do standard stuff                                                                              \n"
"    v_ecPosition = (cvfu_modelViewMatrix * cvfa_vertex).xyz;                                          \n"
"    v_texCoord = cvfa_texCoord;                                                                       \n"
"                                                                                                      \n"
"    gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;                                         \n"
"                                                                                                      \n"
"    // Compute the point diameter in window coords (pixels)                                           \n"
"    // Scale with distance for perspective correction of the size                                     \n"
"    float dist = length(v_ecPosition);                                                                \n"
"    gl_PointSize = 2*u_pointRadius/(cvfu_pixelHeightAtUnitDistance*dist);                             \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char vs_EnvironmentMapping_inl[] =
"                                                                                                      \n"
"uniform mat4 cvfu_modelViewProjectionMatrix;                                                          \n"
"uniform mat4 cvfu_modelViewMatrix;                                                                    \n"
"uniform mat3 cvfu_normalMatrix;                                                                       \n"
"                                                                                                      \n"
"attribute vec4 cvfa_vertex;                                                                           \n"
"attribute vec3 cvfa_normal;                                                                           \n"
"attribute vec2 cvfa_texCoord;                                                                         \n"
"                                                                                                      \n"
"varying vec3 v_ecPosition;                                                                            \n"
"varying vec3 v_ecNormal;                                                                              \n"
"varying vec2 v_texCoord;                                                                              \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Vertex Shader - Environment mapping                                                               \n"
"/// Source: http://www.ozone3d.net/tutorials/glsl_texturing_p04.php                                   \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main ()                                                                                          \n"
"{                                                                                                     \n"
"    // Transforms vertex position and normal vector to eye space                                      \n"
"    v_ecPosition = (cvfu_modelViewMatrix * cvfa_vertex).xyz;                                          \n"
"    v_ecNormal = cvfu_normalMatrix * cvfa_normal;                                                     \n"
"                                                                                                      \n"
"    gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;                                         \n"
"                                                                                                      \n"
"    // Compute the texture coordinate for the environment map texture lookup                          \n"
"    vec3 u = normalize(v_ecPosition);                                                                 \n"
"    vec3 n = normalize(v_ecNormal);                                                                   \n"
"    vec3 r = reflect(u, n);                                                                           \n"
"    float m = 2.0 * sqrt(r.x*r.x + r.y*r.y + (r.z+1.0)*(r.z+1.0));                                    \n"
"    v_texCoord.s = r.x/m + 0.5;                                                                       \n"
"    v_texCoord.t = r.y/m + 0.5;                                                                       \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char vs_FullScreenQuad_inl[] =
"                                                                                                      \n"
"attribute vec4 cvfa_vertex;                                                                           \n"
"                                                                                                      \n"
"varying vec2 v_texCoord;                                                                              \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Vertex Shader - Full Screen Quad                                                                  \n"
"/// Assumes cvfa_vertex to be in (0,1) space                                                          \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main ()                                                                                          \n"
"{                                                                                                     \n"
"    // Tex coord = vertex                                                                             \n"
"    v_texCoord =  cvfa_vertex.xy;                                                                     \n"
"                                                                                                      \n"
"    // Transform from <0,1> to <-1, 1>                                                                \n"
"    vec4 vert = cvfa_vertex;                                                                          \n"
"    vert.x = 2.0*cvfa_vertex.x - 1.0;                                                                 \n"
"    vert.y = 2.0*cvfa_vertex.y - 1.0;                                                                 \n"
"                                                                                                      \n"
"    gl_Position = vert;                                                                               \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char vs_Minimal_inl[] =
"                                                                                                      \n"
"uniform mat4 cvfu_modelViewProjectionMatrix;                                                          \n"
"                                                                                                      \n"
"attribute vec4 cvfa_vertex;                                                                           \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Vertex Shader - Minimal                                                                           \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main ()                                                                                          \n"
"{                                                                                                     \n"
"    gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;                                         \n"
"                                                                                                      \n"
"#ifdef CVF_OPENGL_ES                                                                                  \n"
"    gl_PointSize = 1.0;                                                                               \n"
"#endif                                                                                                \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char vs_MinimalTexture_inl[] =
"uniform mat4 cvfu_modelViewProjectionMatrix;                                                          \n"
"                                                                                                      \n"
"attribute vec2 cvfa_texCoord;                                                                         \n"
"attribute vec4 cvfa_vertex;                                                                           \n"
"                                                                                                      \n"
"varying vec2 v_texCoord;                                                                              \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Vertex Shader - Text                                                                              \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main ()                                                                                          \n"
"{                                                                                                     \n"
"    v_texCoord = cvfa_texCoord;                                                                       \n"
"    gl_Position = cvfu_modelViewProjectionMatrix * cvfa_vertex;                                       \n"
"                                                                                                      \n"
"#ifdef CVF_OPENGL_ES                                                                                  \n"
"    gl_PointSize = 1.0;                                                                               \n"
"#endif                                                                                                \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char vs_ParticleTraceComets_inl[] =
"                                                                                                      \n"
"uniform mat4 cvfu_modelViewProjectionMatrix;                                                          \n"
"uniform mat4 cvfu_modelViewMatrix;                                                                    \n"
"uniform mat3 cvfu_normalMatrix;                                                                       \n"
"uniform mat4 cvfu_projectionMatrix;                                                                   \n"
"                                                                                                      \n"
"// Line (cylinder) radius in world space                                                              \n"
"uniform float u_lineRadius;                                                                           \n"
"                                                                                                      \n"
"attribute vec4  cvfa_vertex;                                                                          \n"
"attribute vec2  cvfa_texCoord;                                                                        \n"
"attribute vec3  a_fwdVector;                                                                          \n"
"attribute vec2  a_circleFactors;                                                                      \n"
"attribute float a_alpha;                                                                              \n"
"                                                                                                      \n"
"varying vec3  v_ecPosition;                                                                           \n"
"varying vec2  v_texCoord;                                                                             \n"
"varying vec2  v_circleFactors;                                                                        \n"
"varying float v_alpha;                                                                                \n"
"                                                                                                      \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Vertex Shader - Particle trace comets                                                             \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main()                                                                                           \n"
"{                                                                                                     \n"
"    vec4 ecVertex = cvfu_modelViewMatrix * cvfa_vertex;                                               \n"
"    vec3 ecFwd = cvfu_normalMatrix * a_fwdVector;                                                     \n"
"    vec3 ecSide = normalize(cross(ecFwd, ecVertex.xyz));                                              \n"
"                                                                                                      \n"
"    // Scale is the same in world and eye space so use specified radius directly                      \n"
"    ecVertex.xyz += u_lineRadius*(a_circleFactors.x*ecSide + a_circleFactors.y*ecFwd);                \n"
"                                                                                                      \n"
"    v_ecPosition = ecVertex.xyz;                                                                      \n"
"    v_texCoord = cvfa_texCoord;                                                                       \n"
"    v_circleFactors = a_circleFactors;                                                                \n"
"    v_alpha = a_alpha;                                                                                \n"
"                                                                                                      \n"
"    gl_Position = cvfu_projectionMatrix * ecVertex;                                                   \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char vs_Standard_inl[] =
"                                                                                                      \n"
"uniform mat4 cvfu_modelViewProjectionMatrix;                                                          \n"
"uniform mat4 cvfu_modelViewMatrix;                                                                    \n"
"uniform mat3 cvfu_normalMatrix;                                                                       \n"
"                                                                                                      \n"
"attribute vec4 cvfa_vertex;                                                                           \n"
"attribute vec3 cvfa_normal;                                                                           \n"
"attribute vec2 cvfa_texCoord;                                                                         \n"
"                                                                                                      \n"
"varying vec3 v_ecPosition;                                                                            \n"
"varying vec3 v_ecNormal;                                                                              \n"
"varying vec2 v_texCoord;                                                                              \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Vertex Shader - Standard                                                                          \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main ()                                                                                          \n"
"{                                                                                                     \n"
"#ifdef CVF_CALC_SHADOW_COORD_IMPL                                                                     \n"
"    calcShadowCoord();                                                                                \n"
"#endif                                                                                                \n"
"                                                                                                      \n"
"    // Transforms vertex position and normal vector to eye space                                      \n"
"    v_ecPosition = (cvfu_modelViewMatrix * cvfa_vertex).xyz;                                          \n"
"    v_ecNormal = cvfu_normalMatrix * cvfa_normal;                                                     \n"
"    v_texCoord = cvfa_texCoord;                                                                       \n"
"                                                                                                      \n"
"#ifdef CVF_CALC_CLIP_DISTANCES_IMPL                                                                   \n"
"    calcClipDistances(vec4(v_ecPosition, 1));                                                         \n"
"#endif                                                                                                \n"
"                                                                                                      \n"
"    gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;                                         \n"
"}                                                                                                     \n";



//#############################################################################################################################
//#############################################################################################################################
static const char vs_VectorDrawer_inl[] =
"                                                                                                      \n"
"uniform mat4 cvfu_modelViewProjectionMatrix;                                                          \n"
"uniform mat4 cvfu_modelViewMatrix;                                                                    \n"
"uniform mat3 cvfu_normalMatrix;                                                                       \n"
"uniform mat4 u_transformationMatrix;                                                                  \n"
"                                                                                                      \n"
"attribute vec4 cvfa_vertex;                                                                           \n"
"attribute vec3 cvfa_normal;                                                                           \n"
"                                                                                                      \n"
"varying float v_diffuse;                                                                              \n"
"                                                                                                      \n"
"//--------------------------------------------------------------------------------------------------  \n"
"/// Vertex Shader - Vector Drawer                                                                     \n"
"//--------------------------------------------------------------------------------------------------  \n"
"void main ()                                                                                          \n"
"{                                                                                                     \n"
"#ifdef CVF_CALC_CLIP_DISTANCES_IMPL                                                                   \n"
"    vec4 ecPosition = cvfu_modelViewMatrix*u_transformationMatrix*cvfa_vertex;                        \n"
"    calcClipDistances(ecPosition);                                                                    \n"
"#endif                                                                                                \n"
"                                                                                                      \n"
"    // Transforms vertex position and normal vector to eye space                                      \n"
"    mat3 mat3_transMatr = mat3(u_transformationMatrix);                                               \n"
"                                                                                                      \n"
"    gl_Position = cvfu_modelViewProjectionMatrix*u_transformationMatrix*cvfa_vertex;                  \n"
"    v_diffuse = abs(normalize(cvfu_normalMatrix*mat3_transMatr*cvfa_normal).z);                       \n"
"}                                                                                                     \n";



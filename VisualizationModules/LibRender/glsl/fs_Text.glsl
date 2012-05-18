
uniform sampler2D u_texture2D;
uniform vec3 u_color;

varying vec2 v_texCoord;

//--------------------------------------------------------------------------------------------------
/// Fragment Shader - Text
//--------------------------------------------------------------------------------------------------
void main()
{
	float alpha = texture2D(u_texture2D, v_texCoord).a;
    gl_FragColor = vec4(u_color, alpha);
}

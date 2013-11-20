
uniform vec4 u_color;

void main(void)
{
	gl_FragData[0] = u_color;
	gl_FragData[1] = gl_FragCoord.z;
}

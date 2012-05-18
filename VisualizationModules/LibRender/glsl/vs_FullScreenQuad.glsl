
attribute vec4 cvfa_vertex;

varying vec2 v_texCoord;

//--------------------------------------------------------------------------------------------------
/// Vertex Shader - Full Screen Quad
/// Assumes cvfa_vertex to be in (0,1) space
//--------------------------------------------------------------------------------------------------
void main ()
{
	// Tex coord = vertex
	v_texCoord =  cvfa_vertex.xy;

	// Transform from <0,1> to <-1, 1>
	vec4 vert = cvfa_vertex;
	vert.x = 2.0*cvfa_vertex.x - 1.0;
	vert.y = 2.0*cvfa_vertex.y - 1.0;

	gl_Position = vert;
}

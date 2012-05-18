#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

//--------------------------------------------------------------------------------------------------
/// Geometry Shader - Pass through
//--------------------------------------------------------------------------------------------------
void main()
{
    // Init to avoid warning on nVidia ('gl_Position might be used before being initialized')
    // Seems buggy since geom shader shouldn't be invoked if length() is 0
	gl_Position = vec4(0, 0, 0, 0);

	for (int i = 0; i < gl_in.length(); i++)
	{
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}

	EndPrimitive();
}

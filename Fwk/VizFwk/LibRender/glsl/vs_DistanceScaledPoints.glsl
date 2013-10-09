
uniform mat4  cvfu_modelViewProjectionMatrix;
uniform mat4  cvfu_modelViewMatrix;
uniform float cvfu_pixelHeightAtUnitDistance;

// Point radius in world space
uniform float u_pointRadius;  

attribute vec4 cvfa_vertex;
attribute vec2 cvfa_texCoord;

varying vec3 v_ecPosition;
varying vec2 v_texCoord;


//--------------------------------------------------------------------------------------------------
/// Vertex Shader for point rendering. Scales point size based on distance from eye
//--------------------------------------------------------------------------------------------------
void main ()
{
	// Do standard stuff
	v_ecPosition = (cvfu_modelViewMatrix * cvfa_vertex).xyz;
	v_texCoord = cvfa_texCoord;

	gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;

	// Compute the point diameter in window coords (pixels)
	// Scale with distance for perspective correction of the size
    float dist = length(v_ecPosition);
    gl_PointSize = 2.0*u_pointRadius/(cvfu_pixelHeightAtUnitDistance*dist);
}

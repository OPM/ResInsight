
uniform mat4 cvfu_modelViewProjectionMatrix;
uniform mat4 cvfu_modelViewMatrix;
uniform mat3 cvfu_normalMatrix;
uniform mat4 cvfu_projectionMatrix;

// Line (cylinder) radius in world space
uniform float u_lineRadius;  

attribute vec4  cvfa_vertex;
attribute vec2  cvfa_texCoord;
attribute vec3  a_fwdVector;
attribute vec2  a_circleFactors;
attribute float a_alpha;

varying vec3  v_ecPosition;
varying vec2  v_texCoord;
varying vec2  v_circleFactors;
varying float v_alpha;


//--------------------------------------------------------------------------------------------------
/// Vertex Shader - Particle trace comets
//--------------------------------------------------------------------------------------------------
void main() 
{ 
    vec4 ecVertex = cvfu_modelViewMatrix * cvfa_vertex;
    vec3 ecFwd = cvfu_normalMatrix * a_fwdVector;
    vec3 ecSide = normalize(cross(ecFwd, ecVertex.xyz));

	// Scale is the same in world and eye space so use specified radius directly
	ecVertex.xyz += u_lineRadius*(a_circleFactors.x*ecSide + a_circleFactors.y*ecFwd);

    v_ecPosition = ecVertex.xyz;
	v_texCoord = cvfa_texCoord;
	v_circleFactors = a_circleFactors;
	v_alpha = a_alpha;

    gl_Position = cvfu_projectionMatrix * ecVertex;
}

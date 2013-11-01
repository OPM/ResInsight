
#if !defined WORLD_RADIUS && !defined PIXEL_WIDTH
#define WORLD_RADIUS
#endif

uniform mat4 cvfu_modelViewProjectionMatrix;
uniform mat4 cvfu_modelViewMatrix;
uniform mat3 cvfu_normalMatrix;
uniform mat4 cvfu_projectionMatrix;

#ifdef WORLD_RADIUS
// Line (cylinder) radius in world space
uniform float u_lineRadius;  
#endif

#ifdef PIXEL_WIDTH
uniform float cvfu_pixelHeightAtUnitDistance;
uniform float u_lineWidthPixels;  
#endif

attribute vec4  cvfa_vertex;
attribute vec2  cvfa_texCoord;
attribute vec3  a_fwdVector;
attribute vec2  a_circleFactors;
attribute float a_alpha;

varying vec3 v_ecPosition;
varying vec2 v_texCoord;
varying vec2 v_circleFactors;
varying float v_alpha;


//--------------------------------------------------------------------------------------------------
/// Vertex Shader - snipLineDrawing
//--------------------------------------------------------------------------------------------------
void main() 
{ 
    vec4 ecVertex = cvfu_modelViewMatrix * cvfa_vertex;
    vec3 ecFwd = cvfu_normalMatrix * a_fwdVector;
    vec3 ecSide = normalize(cross(ecFwd, ecVertex.xyz));

#ifdef WORLD_RADIUS
	// Scale is the same in world and eye space
	float ecRadius = u_lineRadius;
#endif

#ifdef PIXEL_WIDTH
	float dist = length(ecVertex.xyz);
	float ecRadius = 0.5*u_lineWidthPixels*cvfu_pixelHeightAtUnitDistance*dist;
#endif

    //ecVertex.xyz += 0.1*(cvfa_texCoord.x*ecSide + cvfa_texCoord.y*ecFwd);
	ecVertex.xyz += ecRadius*(a_circleFactors.x*ecSide + a_circleFactors.y*ecFwd);

    v_ecPosition = ecVertex.xyz;
	v_texCoord = cvfa_texCoord;
	v_circleFactors = a_circleFactors;
	v_alpha = a_alpha;

    gl_Position = cvfu_projectionMatrix * ecVertex;
}


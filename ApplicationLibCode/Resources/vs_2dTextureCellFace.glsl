uniform sampler2D u_edgeTexture2D;
uniform sampler2D u_cellTexture2D;

attribute vec2 a_localCoord;
attribute float a_face;

attribute vec2 a_cellTextureCoord;
attribute float a_colorPosI;
attribute float a_colorNegI;
attribute float a_colorPosJ;
attribute float a_colorNegJ;
attribute float a_colorPosK;
attribute float a_colorNegK;

varying vec2 v_localCoord;
varying vec4 v_cellColor;
varying vec4 v_bottomColor;
varying vec4 v_rightColor;
varying vec4 v_topColor;
varying vec4 v_leftColor;

// Native visualization lib stuff
uniform mat4 cvfu_modelViewProjectionMatrix;
uniform mat4 cvfu_modelViewMatrix;
uniform mat3 cvfu_normalMatrix;

attribute vec4 cvfa_vertex;
attribute vec3 cvfa_normal;

varying vec3 v_ecPosition;
varying vec3 v_ecNormal;
// End native vz stuff

#define POS_I 0.0
#define NEG_I 1.0
#define POS_J 2.0
#define NEG_J 3.0
#define POS_K 4.0
#define NEG_K 5.0

//
//     7---------6               
//    /|        /|     |k        
//   / |       / |     | /j      
//  4---------5  |     |/        
//  |  3------|--2     *---i     
//  | /       | /                
//  |/        |/                 
//  0---------1                     
//
// Face layout expected
// POS_I   1, 2, 6, 5  
// NEG_I   0, 4, 7, 3
// POS_J   3, 7, 6, 2
// NEG_J   0, 1, 5, 4
// POS_K   4, 5, 6, 7
// NEG_K   0, 3, 2, 1
//

vec4 getColorFromTextureCoord(float textureCoord, vec4 cellColor)
{
    if (textureCoord < 0.0)
        return cellColor;
    else
        return texture2D(u_edgeTexture2D, vec2(textureCoord, 0.5f ));
}

void main()
{	
    v_localCoord = a_localCoord;
    
	v_cellColor = texture2D(u_cellTexture2D, a_cellTextureCoord);

    if (a_face == POS_I)
    {
        v_bottomColor	= getColorFromTextureCoord(a_colorNegK, v_cellColor);
        v_rightColor	= getColorFromTextureCoord(a_colorPosJ, v_cellColor);
        v_topColor	= getColorFromTextureCoord(a_colorPosK, v_cellColor);
        v_leftColor	= getColorFromTextureCoord(a_colorNegJ, v_cellColor);
    }
    else if (a_face == NEG_I)
    {
        v_bottomColor	= getColorFromTextureCoord(a_colorNegJ, v_cellColor);
        v_rightColor	= getColorFromTextureCoord(a_colorPosK, v_cellColor);
        v_topColor	= getColorFromTextureCoord(a_colorPosJ, v_cellColor);
        v_leftColor	= getColorFromTextureCoord(a_colorNegK, v_cellColor);
    }
    else if (a_face == POS_J )
    {
        v_bottomColor	= getColorFromTextureCoord(a_colorNegI, v_cellColor);
        v_rightColor	= getColorFromTextureCoord(a_colorPosK, v_cellColor);
        v_topColor	= getColorFromTextureCoord(a_colorPosI, v_cellColor);
        v_leftColor	= getColorFromTextureCoord(a_colorNegK, v_cellColor);
    }
    else if (a_face == NEG_J)
    {
        v_bottomColor	= getColorFromTextureCoord(a_colorNegK, v_cellColor);
        v_rightColor	= getColorFromTextureCoord(a_colorPosI, v_cellColor);
        v_topColor	= getColorFromTextureCoord(a_colorPosK, v_cellColor);
        v_leftColor	= getColorFromTextureCoord(a_colorNegI, v_cellColor);
    }
    else if (a_face == POS_K )
    {
        v_bottomColor	= getColorFromTextureCoord(a_colorNegJ, v_cellColor);
        v_rightColor	= getColorFromTextureCoord(a_colorPosI, v_cellColor);
        v_topColor	= getColorFromTextureCoord(a_colorPosJ, v_cellColor);
        v_leftColor	= getColorFromTextureCoord(a_colorNegI, v_cellColor);
    }
    else if (a_face == NEG_K)
    {
        v_bottomColor	= getColorFromTextureCoord(a_colorNegI, v_cellColor);
        v_rightColor	= getColorFromTextureCoord(a_colorPosJ, v_cellColor);
        v_topColor	= getColorFromTextureCoord(a_colorPosI, v_cellColor);
        v_leftColor	= getColorFromTextureCoord(a_colorNegJ, v_cellColor);
    }
    else
    {
        v_bottomColor 	= v_cellColor;
        v_rightColor	= v_cellColor;
        v_topColor		= v_cellColor;
        v_leftColor		= v_cellColor;
    }

    // Transforms vertex position and normal vector to eye space
    v_ecPosition = (cvfu_modelViewMatrix * cvfa_vertex).xyz;
    v_ecNormal = cvfu_normalMatrix * cvfa_normal;

    gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;
}


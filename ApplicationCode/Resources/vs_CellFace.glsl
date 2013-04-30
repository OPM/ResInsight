uniform sampler2D u_edgeTexture2D;
uniform sampler2D u_cellTexture2D;

attribute vec2 a_localCoord;
attribute float a_face;

attribute float a_colorCell;
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

    // Alternative to ifs
    // return (textureCoord < 0)*cellColor + (textureCoord >= 0)*texture2D(u_edgeTexture2D, vec2(textureCoord, 0.5f ));
}

void main()
{	
    v_localCoord = a_localCoord;
    
	if ( a_colorCell < 0.0)
		v_cellColor = vec4(0.75, 0.75, 0.75, 1); // Light grayish
	else if ( a_colorCell >= 2.0)
		v_cellColor = texture2D(u_cellTexture2D, vec2( a_colorCell-2.0f, 0.0f)); // Opaque, because the y=0 texcoord points to the opaque part of an modified texture
    else
		v_cellColor = texture2D(u_cellTexture2D, vec2( a_colorCell, 0.5f)); // Default, transparent if the texture is modified

    /*
        // Performance test code
        v_bottomColor	= 0.5*getColorFromTextureCoord(a_colorNegK, v_cellColor)+ getColorFromTextureCoord(a_colorNegI, v_cellColor);
        v_rightColor	= getColorFromTextureCoord(a_colorPosJ, v_cellColor);
        v_topColor	= getColorFromTextureCoord(a_colorPosK, v_cellColor);
        v_leftColor	= 0.5*getColorFromTextureCoord(a_colorNegJ, v_cellColor)+ getColorFromTextureCoord(a_colorPosI, v_cellColor);
    */

    /*
      // Alternative to ifs
 v_bottomColor =
      (a_face == POS_I) * getColorFromTextureCoord(a_colorNegK, v_cellColor)
    + (a_face == NEG_I) * getColorFromTextureCoord(a_colorNegJ, v_cellColor)
    + (a_face == POS_J) * getColorFromTextureCoord(a_colorNegI, v_cellColor)
    + (a_face == NEG_J) * getColorFromTextureCoord(a_colorNegK, v_cellColor)
    + (a_face == POS_K) * getColorFromTextureCoord(a_colorNegJ, v_cellColor)
    + (a_face == NEG_K) * getColorFromTextureCoord(a_colorNegI, v_cellColor);
 v_rightColor =
      (a_face == POS_I) * getColorFromTextureCoord(a_colorPosJ, v_cellColor)
    + (a_face == NEG_I) * getColorFromTextureCoord(a_colorPosK, v_cellColor)
    + (a_face == POS_J) * getColorFromTextureCoord(a_colorPosK, v_cellColor)
    + (a_face == NEG_J) * getColorFromTextureCoord(a_colorPosI, v_cellColor)
    + (a_face == POS_K) * getColorFromTextureCoord(a_colorPosI, v_cellColor)
    + (a_face == NEG_K) * getColorFromTextureCoord(a_colorPosJ, v_cellColor);
 v_topColor  =
      (a_face == POS_I) * getColorFromTextureCoord(a_colorPosK, v_cellColor)
    + (a_face == NEG_I) * getColorFromTextureCoord(a_colorPosJ, v_cellColor)
    + (a_face == POS_J) * getColorFromTextureCoord(a_colorPosI, v_cellColor)
    + (a_face == NEG_J) * getColorFromTextureCoord(a_colorPosK, v_cellColor)
    + (a_face == POS_K) * getColorFromTextureCoord(a_colorPosJ, v_cellColor)
    + (a_face == NEG_K) * getColorFromTextureCoord(a_colorPosI, v_cellColor);
 v_leftColor =
      (a_face == POS_I) * getColorFromTextureCoord(a_colorNegJ, v_cellColor)
    + (a_face == NEG_I) * getColorFromTextureCoord(a_colorNegK, v_cellColor)
    + (a_face == POS_J) * getColorFromTextureCoord(a_colorNegK, v_cellColor)
    + (a_face == NEG_J) * getColorFromTextureCoord(a_colorNegI, v_cellColor)
    + (a_face == POS_K) * getColorFromTextureCoord(a_colorNegI, v_cellColor)
    + (a_face == NEG_K) * getColorFromTextureCoord(a_colorNegJ, v_cellColor);
*/

    ///*
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
        v_bottomColor	= getColorFromTextureCoord(a_colorCell, v_cellColor);
        v_rightColor	= getColorFromTextureCoord(a_colorCell, v_cellColor);
        v_topColor	= getColorFromTextureCoord(a_colorCell, v_cellColor);
        v_leftColor	= getColorFromTextureCoord(a_colorCell, v_cellColor);
    }
    //*/

    // Transforms vertex position and normal vector to eye space
    v_ecPosition = (cvfu_modelViewMatrix * cvfa_vertex).xyz;
    v_ecNormal = cvfu_normalMatrix * cvfa_normal;

    gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;
}



varying vec2 v_localCoord;		// Normalized coordinates along quad

varying vec4 v_cellColor;		// cell face color

varying vec4 v_bottomColor;		// color along edge v0-v1
varying vec4 v_rightColor;		// color along edge v1-v2
varying vec4 v_topColor;		// color along edge v2-v3
varying vec4 v_leftColor;		// color along edge v3-v0


void colormapCorners(in vec2 coord, inout vec4 color)
{
    if (coord.x < 0.2 && coord.y < 0.2)			// lower, left corner
    {
        if (v_bottomColor == color)	color = v_leftColor;
        else if (v_leftColor == color)	color = v_bottomColor;
        else if (coord.x > coord.y)	color = v_bottomColor;
        else				color = v_leftColor;
    }
    else if (coord.x > 0.8 && coord.y < 0.2)		// lower, right corner
    {
        float x = 1.0 - coord.x;
        if (v_rightColor == color)	 color = v_bottomColor;
        else if (v_bottomColor == color) color = v_rightColor;
        else if (x < coord.y)		 color = v_rightColor;
        else				 color = v_bottomColor;
    }
    else if (coord.x > 0.8 && coord.y > 0.8)		// upper, right corner
    {
        if (v_rightColor == color)	color = v_topColor;
        else if (v_topColor == color)	color = v_rightColor;
        else if (coord.x > coord.y)	color = v_rightColor;
        else				color = v_topColor;
    }
    else if (coord.x < 0.2 && coord.y > 0.8)		// upper, left corner
    {
        float y = 1.0 - coord.y;
	
        if (v_topColor == color)	color = v_leftColor;
        else if (v_leftColor == color)	color = v_topColor;
        else if (coord.x > y)		color = v_topColor;
        else				color = v_leftColor;
    }
}


void colormapEdgeRegions(in vec2 coord, inout vec4 color)
{
    if (coord.y < 0.2 && coord.x > 0.2 && coord.x < 0.8)
    {
        color = v_bottomColor;
    }
    else if (coord.x > 0.8 && coord.y > 0.2 && coord.y < 0.8)
    {
        color = v_rightColor;
    }
    else if (coord.y > 0.8 && coord.x > 0.2 && coord.x < 0.8)
    {
        color = v_topColor;
    }
    else if (coord.x < 0.2 && coord.y > 0.2 && coord.y < 0.8)
    {
        color = v_leftColor;
    }
}


//--------------------------------------------------------------------------------------------------
/// Texture 2D source fragment
//--------------------------------------------------------------------------------------------------
vec4 srcFragment()
{
    vec4 color = v_cellColor;		// cell color to be used for areas that are not along edges

    // Performance test code
    // color = 0.2 * (v_cellColor + v_bottomColor + v_rightColor + v_topColor + v_leftColor);

    colormapEdgeRegions(v_localCoord, color);
    colormapCorners(v_localCoord, color);

    return color;
}

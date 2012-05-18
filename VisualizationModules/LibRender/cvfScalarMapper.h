//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
//   This library is free software: you can redistribute it and/or modify 
//   it under the terms of the GNU General Public License as published by 
//   the Free Software Foundation, either version 3 of the License, or 
//   (at your option) any later version. 
//    
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY 
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
//   FITNESS FOR A PARTICULAR PURPOSE.   
//    
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>> 
//   for more details. 
//
//##################################################################################################

#pragma once

#include "cvfObject.h"
#include "cvfArray.h"

namespace cvf {

class TextureImage;


//==================================================================================================
//
// Abstract base class for mapping scalar values to texture coordinates/colors
//
//==================================================================================================
class ScalarMapper : public Object
{
public:
    enum ColorTable
    {
        NORMAL,
        BLACK_WHITE,
        BLUE_RED,
        BLUE_GREEN,
        YELLOW_RED,
        GREEN_YELLOW_RED,
        RED_YELLOW,
        THERMAL_1,
        THERMAL_2,
        THERMAL_3,
        METAL_CASTING
    };

public:
    virtual Vec2f       mapToTextureCoord(double scalarValue) const = 0;
    virtual Color3ub    mapToColor(double scalarValue) const = 0;

    virtual bool        updateTexture(TextureImage* image) const = 0;

protected:
    static ref<Color3ubArray> colorTableArray(ColorTable colorTable);
    static ref<Color3ubArray> normalColorTableArray(uint colorCount);
    static ref<Color3ubArray> interpolateColorArray(const Color3ubArray& colorArray, uint targetColorCount);
};


}

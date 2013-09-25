//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
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
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#pragma once

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfArray.h"
#include <vector>

namespace cvf {

class TextureImage;


//==================================================================================================
//
// Abstract base class for mapping scalar values to texture coordinates/colors
// It also provides an interface that OverlayScalarMapperLegend's use to draw consistent colors 
// and labels/ticks
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
    //////
    // Interface for mapping of scalar values to color and texture

    /// Calculate texture coords into an image produced by updateTexture, from the scalarValue
    virtual Vec2f               mapToTextureCoord(double scalarValue) const = 0;
    /// Update the supplied TextureImage to be addressable by the texture coords delivered by mapToTextureCoord
    virtual bool                updateTexture(TextureImage* image) const = 0;

    /// Calculate a color from the scalar value
    virtual Color3ub            mapToColor(double scalarValue) const = 0;

    //////
    // Interface used by OverlayScalarMapperLegend:
    
    /// Return a the set of domain values representing sensible major tickmarks
    virtual void                majorTickValues(std::vector<double>* domainValues) const = 0;  
    /// Return the normalized (0.0, 1.0) representation of the domainValue
    virtual double              normalizedValue(double domainValue) const = 0;
    /// Return the domain value from a normalized val
    virtual double              domainValue(double normalizedValue) const = 0;

protected:

    // Static utility methods that can be used when creating real ScalarMapper's

    static ref<Color3ubArray>   colorTableArray(ColorTable colorTable);
    static ref<Color3ubArray>   normalColorTableArray(uint colorCount);
    static ref<Color3ubArray>   interpolateColorArray(const Color3ubArray& colorArray, uint targetColorCount);
};


}

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


#include "cvfBase.h"
#include "cvfScalarMapper.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::ScalarMapper
/// \ingroup Render
///
/// Abstract base class for mapping scalar values to texture coordinates/colors
///
//==================================================================================================




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Color3ubArray> ScalarMapper::colorTableArray(ColorTable colorTable)
{
    ref<Color3ubArray> colors = new Color3ubArray;
    switch (colorTable)
    {
        case BLACK_WHITE:
        {
            colors->reserve(2);
            colors->add(Color3ub(  0,   0,   0));
            colors->add(Color3ub(255, 255, 255));
            break;
        }
        case BLUE_RED:
        {
            colors->reserve(2);
            colors->add(Color3ub(  0,   0, 255));
            colors->add(Color3ub(255,   0,   0));
            break;
        }
        case BLUE_GREEN:
        {
            colors->reserve(2);
            colors->add(Color3ub(  0,   0, 255));
            colors->add(Color3ub(  0, 255,   0));
            break;
        }
        case YELLOW_RED:
        {
            colors->reserve(2);
            colors->add(Color3ub(255, 255,   0));
            colors->add(Color3ub(255,   0,   0));
            break;
        }
        case GREEN_YELLOW_RED:
        {
            colors->reserve(3);
            colors->add(Color3ub(  0, 255,   0));
            colors->add(Color3ub(255, 255,   0));
            colors->add(Color3ub(255,   0,   0));
            break;
        }
        case RED_YELLOW:
        {
            colors->reserve(2);
            colors->add(Color3ub(255,   0,   0));
            colors->add(Color3ub(255, 255,   0));
            break;
        }
        case THERMAL_1:
        {
            colors->reserve(11);
            colors->add(Color3ub(  0,   0,   0));
            colors->add(Color3ub( 16,  80,  79));
            colors->add(Color3ub( 27, 159, 155));
            colors->add(Color3ub( 96, 213,  87));
            colors->add(Color3ub(170, 234,  37));
            colors->add(Color3ub(240, 215,  16));
            colors->add(Color3ub(255, 167,   0));
            colors->add(Color3ub(255,  64,  17));
            colors->add(Color3ub(252, 137, 125));
            colors->add(Color3ub(255, 200, 200));
            colors->add(Color3ub(255, 255, 255));
            break;
        }
        case THERMAL_2:
        {
            colors->reserve(12);
            colors->add(Color3ub(  0,   0,   0));
            colors->add(Color3ub( 18,   0, 130));
            colors->add(Color3ub( 95,   0, 142));
            colors->add(Color3ub(159,   0, 146));
            colors->add(Color3ub(193,   6, 135));
            colors->add(Color3ub(225,  48,  65));
            colors->add(Color3ub(240,  86,   0));
            colors->add(Color3ub(240, 126,   0));
            colors->add(Color3ub(255, 166,   0));
            colors->add(Color3ub(255, 205,   0));
            colors->add(Color3ub(255, 237,  94));
            colors->add(Color3ub(255, 255, 255));
            break;
        }
        case THERMAL_3:
        {
            colors->reserve(4);
            colors->add(Color3ub(  0,   0,   0));
            colors->add(Color3ub(218,  20,   5));
            colors->add(Color3ub(210, 210,  30));
            colors->add(Color3ub(218, 218, 218));
            break;
        }
        case METAL_CASTING:
        {
            colors->reserve(5);
            colors->add(Color3ub(  0, 255, 255));
            colors->add(Color3ub(  0,   0, 255));
            colors->add(Color3ub(255,   0,   0));
            colors->add(Color3ub(255, 255,   0));
            colors->add(Color3ub(255, 255, 255));
            break;
        }

        case NORMAL:
        {
            // Which number of levels should we choose here?
            colors = normalColorTableArray(10);
            break;
        }
    }

    CVF_ASSERT(colors->size() > 0);

    return colors;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Color3ubArray> ScalarMapper::normalColorTableArray(uint colorCount)
{
    CVF_ASSERT(colorCount > 0 && colorCount <= 13);

    ref<Color3ubArray> colors = new Color3ubArray;

    switch (colorCount)
    {
        case 1:
        {
            colors->reserve(1);
            colors->add(Color3ub(  0,   0, 255));
            break;
        }
        
        case 2:
        {
            colors->reserve(2);
            colors->add(Color3ub(  0,   0, 255));
            colors->add(Color3ub(255,   0, 255));
            break;
        }
        
        case 3:
        {
            colors->reserve(3);
            colors->add(Color3ub(  0,   0, 255));
            colors->add(Color3ub(  0, 255,   0));
            colors->add(Color3ub(255,   0,   0));
            break;
        }

        case 4:
        {
            colors->reserve(4);
            colors->add(Color3ub(  0,   0, 255));
            colors->add(Color3ub(  0, 255, 255));
            colors->add(Color3ub(255, 255,   0));
            colors->add(Color3ub(255,   0,   0));
            break;
        }

        case 5:
        {
            colors->reserve(5);
            colors->add(Color3ub(  0,   0, 255));
            colors->add(Color3ub(  0, 255, 255));
            colors->add(Color3ub(  0, 255,   0));
            colors->add(Color3ub(255, 255,   0));
            colors->add(Color3ub(255,   0,   0));
            break;
        }

        case 6:
        {
            colors->reserve(6);
            colors->add(Color3ub(  0,   0, 255));
            colors->add(Color3ub(  0, 185, 255));
            colors->add(Color3ub(  0, 255, 255));
            colors->add(Color3ub(  0, 255,   0));
            colors->add(Color3ub(255, 186,   0));
            colors->add(Color3ub(255,   0,   0));
            break;
        }

        case 7:
        {
            colors->reserve(7);
            colors->add(Color3ub(  0,   0, 255));
            colors->add(Color3ub(  0, 185, 255));
            colors->add(Color3ub(  0, 255, 255));
            colors->add(Color3ub(  0, 255,   0));
            colors->add(Color3ub(255, 255,   0));
            colors->add(Color3ub(255, 186,   0));
            colors->add(Color3ub(255,   0,   0));
            break;
        }

        case 8:
        {
            colors->reserve(8);
            colors->add(Color3ub(  0,   0, 255));
            colors->add(Color3ub(  0, 120, 255));
            colors->add(Color3ub(  0, 200, 255));
            colors->add(Color3ub(  0, 255, 255));
            colors->add(Color3ub(  0, 255,   0));
            colors->add(Color3ub(255, 255,   0));
            colors->add(Color3ub(255, 185,   0));
            colors->add(Color3ub(255,   0,   0));
            break;
        }

        case 9:
        {
            colors->reserve(9);
            colors->add(Color3ub(  0,   0, 255));
            colors->add(Color3ub(  0, 120, 255));
            colors->add(Color3ub(  0, 200, 255));
            colors->add(Color3ub(  0, 255, 255));
            colors->add(Color3ub(  0, 255,   0));
            colors->add(Color3ub(255, 255,   0));
            colors->add(Color3ub(255, 200,   0));
            colors->add(Color3ub(255, 120,   0));
            colors->add(Color3ub(255,   0,   0));
            break;
        }

        case 10:
        {
            colors->reserve(10);
            colors->add(Color3ub(  0,   0, 255));
            colors->add(Color3ub(  0, 100, 255));
            colors->add(Color3ub(  0, 160, 255));
            colors->add(Color3ub(  0, 210, 255));
            colors->add(Color3ub(  0, 255, 255));
            colors->add(Color3ub(  0, 255,   0));
            colors->add(Color3ub(255, 255,   0));
            colors->add(Color3ub(255, 200,   0));
            colors->add(Color3ub(255, 120,   0));
            colors->add(Color3ub(255,   0,   0));
            break;
        }

        case 11:
        {
            colors->reserve(11);
            colors->add(Color3ub(  0,   0, 255));
            colors->add(Color3ub(  0, 100, 255));
            colors->add(Color3ub(  0, 160, 255));
            colors->add(Color3ub(  0, 210, 255));
            colors->add(Color3ub(  0, 255, 255));
            colors->add(Color3ub(  0, 255,   0));
            colors->add(Color3ub(255, 255,   0));
            colors->add(Color3ub(255, 210,   0));
            colors->add(Color3ub(255, 160,   0));
            colors->add(Color3ub(255, 100,   0));
            colors->add(Color3ub(255,   0,   0));
            break;
        }

        case 12:
        {
            colors->reserve(12);
            colors->add(Color3ub(  0,   0, 255));
            colors->add(Color3ub(  0,  90, 255));
            colors->add(Color3ub(  0, 140, 255));
            colors->add(Color3ub(  0, 180, 255));
            colors->add(Color3ub(  0, 220, 255));
            colors->add(Color3ub(  0, 255, 255));
            colors->add(Color3ub(  0, 255,   0));
            colors->add(Color3ub(255, 255,   0));
            colors->add(Color3ub(255, 210,   0));
            colors->add(Color3ub(255, 160,   0));
            colors->add(Color3ub(255, 100,   0));
            colors->add(Color3ub(255,   0,   0));
            break;
        }

        case 13:
        {
            colors->reserve(13);
            colors->add(Color3ub(  0,   0, 255));
            colors->add(Color3ub(  0,  90, 255));
            colors->add(Color3ub(  0, 140, 255));
            colors->add(Color3ub(  0, 180, 255));
            colors->add(Color3ub(  0, 220, 255));
            colors->add(Color3ub(  0, 255, 255));
            colors->add(Color3ub(  0, 255,   0));
            colors->add(Color3ub(255, 255,   0));
            colors->add(Color3ub(255, 220,   0));
            colors->add(Color3ub(255, 180,   0));
            colors->add(Color3ub(255, 140,   0));
            colors->add(Color3ub(255,  90,   0));
            colors->add(Color3ub(255,   0,   0));
            break;
        }

        default:
        {
            CVF_FAIL_MSG("Invalid input color count");
            break;
        }
    }

    CVF_ASSERT(colors->size() >= 1);

    return colors;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Color3ubArray> ScalarMapper::interpolateColorArray(const Color3ubArray& colorArray, uint targetColorCount)
{
    uint inputColorCount = static_cast<uint>(colorArray.size());
    CVF_ASSERT(inputColorCount > 1);
    CVF_ASSERT(targetColorCount > 1);

    ref<Color3ubArray> colors = new Color3ubArray;
    colors->reserve(targetColorCount);

    const uint inputColorsMaxIdx = inputColorCount - 1;
    const uint outputColorsMaxIdx = targetColorCount - 1;

    uint outputLevelIdx;
    for (outputLevelIdx = 0; outputLevelIdx < outputColorsMaxIdx; outputLevelIdx++)
    {
        double dblInputLevelIndex = inputColorsMaxIdx * (outputLevelIdx / static_cast<double>(outputColorsMaxIdx));

        const uint inputLevelIndex = static_cast<uint>(dblInputLevelIndex);
        CVF_ASSERT(inputLevelIndex < inputColorsMaxIdx);

        double t = dblInputLevelIndex - inputLevelIndex;
        CVF_ASSERT(t >= 0 && t <= 1.0);

        Color3ub c1 = colorArray[inputLevelIndex];
        Color3ub c2 = colorArray[inputLevelIndex + 1];

        int r = static_cast<int>(c1.r() + t*(c2.r() - c1.r()) + 0.5);
        int g = static_cast<int>(c1.g() + t*(c2.g() - c1.g()) + 0.5);
        int b = static_cast<int>(c1.b() + t*(c2.b() - c1.b()) + 0.5);

        r = Math::clamp(r, 0, 255);
        g = Math::clamp(g, 0, 255);
        b = Math::clamp(b, 0, 255);

        Color3ub col((ubyte)r, (ubyte)g, (ubyte)b);
        colors->add(col);
    }

    colors->add(colorArray[colorArray.size() - 1]);

    return colors;
}


} // namespace cvf


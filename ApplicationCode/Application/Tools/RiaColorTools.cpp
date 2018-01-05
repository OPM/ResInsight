/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiaColorTools.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaColorTools::isBrightnessAboveThreshold(cvf::Color3f backgroundColor)
{
    if (backgroundColor.r() + backgroundColor.g() + backgroundColor.b() > 1.5f)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiaColorTools::darkContrastColor()
{
    return cvf::Color3f::fromByteColor(10, 10, 10);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiaColorTools::brightContrastColor()
{
    return cvf::Color3f::WHITE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiaColorTools::constrastColor(cvf::Color3f backgroundColor)
{
    if (isBrightnessAboveThreshold(backgroundColor))
    {
        return darkContrastColor();
    }
 
    return brightContrastColor();
}

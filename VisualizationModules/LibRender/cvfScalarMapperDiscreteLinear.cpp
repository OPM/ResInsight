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

#include "cvfScalarMapperDiscreteLinear.h"
#include "cvfMath.h"
#include "cvfTextureImage.h"
#include <cmath>

namespace cvf {
//==================================================================================================
///
/// \class cvf::ScalarMapperDiscreteLinear
/// \ingroup Render
///
/// Maps scalar values to texture coordinates/colors using discrete linear mapping
//==================================================================================================

ScalarMapperDiscreteLinear::ScalarMapperDiscreteLinear()
{
 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec2f ScalarMapperDiscreteLinear::mapToTextureCoord(double scalarValue) const
{
    double discVal = discretize(scalarValue);
    return ScalarMapperRangeBased::mapToTextureCoord(discVal);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3ub ScalarMapperDiscreteLinear::mapToColor(double scalarValue) const
{
    double discVal = discretize(scalarValue);
    return ScalarMapperRangeBased::mapToColor(discVal);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double ScalarMapperDiscreteLinear::discretize(double scalarValue) const
{
    std::set<double>::iterator it;

    it = m_sortedLevels.upper_bound(scalarValue);
    if (it == m_sortedLevels.begin()) return (*it);
    if (it == m_sortedLevels.end()) return (*m_sortedLevels.rbegin());
    double upperValue = *it;
    it--;
    double lowerValue = *it;
    return 0.5 * (upperValue + lowerValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double ScalarMapperDiscreteLinear::normalizedValue(double domainScalarValue) const
{
    double range = m_rangeMax - m_rangeMin;
    if (range != 0) return cvf::Math::clamp((domainScalarValue - m_rangeMin)/range, 0.0, 1.0);
    else            return 0;
} 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double ScalarMapperDiscreteLinear::domainValue(double normalizedPosition) const
{
    double range = m_rangeMax - m_rangeMin;

    return (normalizedPosition * range + m_rangeMin);

}

} // namespace cvf

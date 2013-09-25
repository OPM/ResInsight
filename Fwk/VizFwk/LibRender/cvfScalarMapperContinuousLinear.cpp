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


#include "cvfScalarMapperContinuousLinear.h"

#include "cvfMath.h"

namespace cvf {

//==================================================================================================
///
/// \class cvf::ScalarMapperContinuousLinear
/// \ingroup Render
///
/// Maps scalar values to texture coordinates/colors using continuous linear mapping
/// Configured by specifying a number of level colors and a min/max range. 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double ScalarMapperContinuousLinear::normalizedValue(double domainValue) const
{
    double range = m_rangeMax - m_rangeMin;
    if (range != 0) return cvf::Math::clamp((domainValue - m_rangeMin)/range, 0.0, 1.0);
    else            return 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double ScalarMapperContinuousLinear::domainValue(double normalizedPosition) const
{
    double range = m_rangeMax - m_rangeMin;

    return (normalizedPosition * range + m_rangeMin);
}

} // namespace cvf

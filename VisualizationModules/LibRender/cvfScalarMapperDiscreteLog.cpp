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

#include "cvfScalarMapperDiscreteLog.h"
#include <cmath>
#include "cvfMath.h"

namespace cvf {

//==================================================================================================
///
/// \class cvf::ScalarMapperDiscreteLog
/// \ingroup Render
///
/// Maps scalar values to texture coordinates/colors using discrete logarithmic mapping
//==================================================================================================


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double ScalarMapperDiscreteLog::normalizedLevelPosition(double domainScalarValue) const
{
    double logRangeMax = log10(m_rangeMax);
    double logRangeMin = log10(m_rangeMin);
    double logRange = logRangeMax - logRangeMin; 
    double logValue;

    if (domainScalarValue <= 0) logValue = logRangeMin;
    else                        logValue = log10(domainScalarValue);

    if (logRange != 0)
    {
        return cvf::Math::clamp((logValue - logRangeMin)/logRange, 0.0, 1.0);
    }
    else
    {
        return 0;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double ScalarMapperDiscreteLog::domainValue(double normalizedPosition) const
{
    double logRangeMax = log10(m_rangeMax);
    double logRangeMin = log10(m_rangeMin);
    double logRange = logRangeMax - logRangeMin; 

    double logValue = normalizedPosition*logRange + logRangeMin;

    return pow(10, logValue);
}

} // namespace cvf

//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2018-     Ceetron Solutions AS
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


#include <vector>
#include <cmath>
#include "cafAssert.h"

namespace caf
{
class TickMarkGenerator
{
public:
    TickMarkGenerator(double min, double max, double minAllowedStepSize)
    {
        if (minAllowedStepSize < 0.0) minAllowedStepSize = -minAllowedStepSize;

        double step = roundUpToLog_1_2_5_10(minAllowedStepSize);

        double startStepCount = ceil(min / step);
        
        if ( startStepCount*step < (min + 0.5*minAllowedStepSize) )
        {
            ++startStepCount;
        }

        double tick = startStepCount*step;
        double currentStepCount = startStepCount;
        while ( tick < (max - 0.5*minAllowedStepSize) )
        {
            m_tickMarkValues.push_back(tick);
            ++currentStepCount;
            tick = currentStepCount*step;
        } 
    }

    const std::vector<double>& tickMarkValues() {  return m_tickMarkValues; }

    static double roundUpToLog_1_2_5_10(double val)
    {
        CAF_ASSERT(val >= 0.0);

        const static double logOf5   = log10(5.0);
        const static double logOf2   = log10(2.0);
        const static double logOf0_5 = log10(0.5);
        const static double logOf0_2 = log10(0.2);

        double logValue = log10(val);
        double intPart = 0.0;
        double fraction = modf(logValue, &intPart);

        double factor = 1.0;

        if (fraction == 0.0)
        {
            factor = 1.0;
        }
        else if (fraction > 0.0)
        {
            if ( fraction > logOf5 )
            {
                factor = 10.0;
            }
            else if ( fraction > logOf2 )
            {
                factor = 5.0;
            }
            else
            {
                factor = 2.0;
            }
        }
        else
        {
            if (fraction > logOf0_5)
            {
                factor = 1;
            }
            else if ( fraction > logOf0_2 )
            {
                factor = 0.5;
            }
            else
            {
                factor = 0.2;
            }
        }
        
        double roundedValue = pow(10.0, intPart) * factor;

        return roundedValue;
    }

private:
    std::vector<double> m_tickMarkValues;
};

}



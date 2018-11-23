/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#pragma once

#include <QString>

#include <cmath>

//==================================================================================================
//
// 
//
//==================================================================================================
class RiaStatisticsTools
{
public:
    static const QString replacePercentileByPValueText(const QString& percentile);


    template<class NumberType> static bool isInvalidNumber(NumberType value)
    {
        return !isValidNumber<NumberType>(value);
    }

    template<class NumberType> static bool isValidNumber(NumberType value)
    {
        if (std::isinf(value)) return false;
        if (std::isnan(value)) return false;

        return true;
    }
};


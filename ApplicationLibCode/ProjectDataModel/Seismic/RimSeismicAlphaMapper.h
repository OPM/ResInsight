/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "cvfBase.h"

#include <vector>

class RimSeismicAlphaMapper
{
public:
    RimSeismicAlphaMapper();
    ~RimSeismicAlphaMapper();

    void setDataRangeAndAlphas( double minVal, double maxVal, std::vector<double> alphas );

    cvf::ubyte alphaValue( double dataValue ) const;

private:
    std::vector<double> m_alphavalues;
    double              m_maxValue;
    double              m_minValue;
    double              m_dataRange;
    double              m_scaleFactor;
};

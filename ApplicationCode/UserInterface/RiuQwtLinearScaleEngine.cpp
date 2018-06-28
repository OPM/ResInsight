/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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


#include "RiuQwtLinearScaleEngine.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtScaleDiv RiuQwtLinearScaleEngine::divideScale(double x1, double x2, double majorStepInterval, double minorStepInterval)
{
    QwtInterval interval(x1, x2);
    QwtInterval roundedInterval = this->align(interval, majorStepInterval);
    QList<double> majorTicks = this->buildMajorTicks(roundedInterval, majorStepInterval);
    QList<double> minorTicks = this->buildMajorTicks(roundedInterval, minorStepInterval);

    return QwtScaleDiv(x1, x2, minorTicks, minorTicks, majorTicks);
}

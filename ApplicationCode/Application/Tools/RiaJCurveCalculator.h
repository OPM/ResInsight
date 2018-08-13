/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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
#include "cvfVector3.h"

class RiaJCurveCalculator
{
public:
    RiaJCurveCalculator(cvf::Vec3d p1,  double azi1, double inc1, double r1,
                        cvf::Vec3d p2);

    bool       isOk()             const { return m_isCalculationOK;}
                                  
    cvf::Vec3d firstArcEndpoint() const { return m_firstArcEndpoint; }
    cvf::Vec3d firstCenter()      const { return m_c1; }
    cvf::Vec3d firstNormal()      const { return m_n1; }

private:
    bool m_isCalculationOK;

    cvf::Vec3d m_firstArcEndpoint;
    cvf::Vec3d m_c1;
    cvf::Vec3d m_n1;
};



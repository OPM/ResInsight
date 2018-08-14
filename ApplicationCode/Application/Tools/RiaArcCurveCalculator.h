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
#include "cvfMatrix4.h"

//--------------------------------------------------------------------------------------------------
///                + p1 
///           t1 // 
///              |      + C   
///               \
///                + p2
//--------------------------------------------------------------------------------------------------
class RiaArcCurveCalculator
{
public:
    RiaArcCurveCalculator(cvf::Vec3d p1, cvf::Vec3d t1,  cvf::Vec3d p2);
    RiaArcCurveCalculator(cvf::Vec3d p1, double azi1, double inc1, cvf::Vec3d p2);

    bool       isOk()   const { return m_isCalculationOK;}

    cvf::Mat4d arcCS()  const { return m_arcCS; }
    double     radius() const { return m_radius;}
    cvf::Vec3d center() const { return m_arcCS.translation();}
    cvf::Vec3d normal() const { return cvf::Vec3d(m_arcCS.col(2));}

    double     endAzimuth()       const  { return m_endAzi; }
    double     endInclination()   const  { return m_endInc; }

private:
    bool m_isCalculationOK;

    double m_radius;
    cvf::Mat4d m_arcCS;

    double     m_endAzi;
    double     m_endInc;

};



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

//--------------------------------------------------------------------------------------------------
///                + p1 
///           t1 //   
///              |  r1  + C   
///               \
///                + firstArcEndpoint
///                 \ 
///                  \
///                   + p2
//--------------------------------------------------------------------------------------------------
class RiaJCurveCalculator
{
public:
    RiaJCurveCalculator(cvf::Vec3d p1,  double azi1, double inc1, double r1,
                        cvf::Vec3d p2);
    enum CurveStatus
    {
        OK,
        OK_STRAIGHT_LINE,
        FAILED_INPUT_OVERLAP,
        FAILED_RADIUS_TOO_LARGE
    };

    CurveStatus curveStatus()      const { return m_curveStatus;}

    cvf::Vec3d  firstArcEndpoint() const { return m_firstArcEndpoint; }

    double      radius()           const  { return m_radius; }
    cvf::Vec3d  firstCenter()      const { return m_c1; }
    cvf::Vec3d  firstNormal()      const { return m_n1; }
                
    double      endAzimuth()       const  { return m_endAzi; }
    double      endInclination()   const  { return m_endInc; }
private:        
    CurveStatus m_curveStatus;

    cvf::Vec3d  m_firstArcEndpoint;
    
    double      m_radius;
    cvf::Vec3d  m_c1;
    cvf::Vec3d  m_n1;

    double      m_endAzi;
    double      m_endInc;
};



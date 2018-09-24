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
#include "RiaArcCurveCalculator.h"

#include "RiaOffshoreSphericalCoords.h"

//--------------------------------------------------------------------------------------------------
///                + p1 
///           t1 // 
///              |      + C   
///               \
///                + p2
//--------------------------------------------------------------------------------------------------
RiaArcCurveCalculator::RiaArcCurveCalculator(cvf::Vec3d p1, cvf::Vec3d t1, cvf::Vec3d p2)
    : m_radius(std::numeric_limits<double>::infinity())
    , m_arcCS(cvf::Mat4d::ZERO)
    , m_endAzi(0)
    , m_endInc(0)
    , m_curveStatus(OK)
{
    bool isOk = t1.normalize();
    if (!isOk)
    {
        // No tangent. Bail out
        m_curveStatus = FAILED_INPUT_OVERLAP;
        return;
    }

    cvf::Vec3d p1p2 = p2 - p1;
    cvf::Vec3d t12 = p1p2.getNormalized(&isOk);
    if (!isOk)
    {
        // p1 and p2 in the same place.
        m_curveStatus = FAILED_INPUT_OVERLAP;

        return;
    }

    cvf::Vec3d N = (t1 ^ t12).getNormalized(&isOk);
    if (!isOk)
    {
        // P2 is on the p1 + k*t1 line. We have a straight line
        m_curveStatus = OK_STRAIGHT_LINE;

        RiaOffshoreSphericalCoords endTangent(t1);
        m_endAzi = endTangent.azi();
        m_endInc = endTangent.inc();
        m_radius = std::numeric_limits<double>::infinity();
        return;
    }

    cvf::Vec3d tr1 = (N ^ t1).getNormalized();

    m_radius = 0.5 * p1p2.length() / (tr1.dot(t12));
    
    cvf::Vec3d C = p1 + m_radius * tr1;

    cvf::Vec3d nTr1 = -tr1;
    m_arcCS = cvf::Mat4d::fromCoordSystemAxes( &nTr1, &t1, &N );

    m_arcCS.setTranslation(C);

    cvf::Vec3d t2 = N ^ (p2 - C).getNormalized();

    RiaOffshoreSphericalCoords endTangent(t2);
    m_endAzi = endTangent.azi();
    m_endInc = endTangent.inc();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaArcCurveCalculator::RiaArcCurveCalculator(cvf::Vec3d p1, double azi1, double inc1, cvf::Vec3d p2)
{
    cvf::Vec3d t1( RiaOffshoreSphericalCoords::unitVectorFromAziInc(azi1,inc1));

    (*this) = RiaArcCurveCalculator(p1, t1, p2);
}



/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018- Statoil ASA
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

#include "RiaWellPlanCalculator.h"
#include "cvfGeometryTools.h"
#include "cvfMatrix4.h"
#include "RiaArcCurveCalculator.h"
#include "RiaOffshoreSphericalCoords.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaWellPlanCalculator::RiaWellPlanCalculator(const cvf::Vec3d& startTangent, 
                                             const std::vector<cvf::Vec3d>& lineArcEndPoints)
    : m_startTangent(startTangent)
    , m_lineArcEndPoints(lineArcEndPoints)
{
    if (m_lineArcEndPoints.size() < 2) return ;

    WellPlanSegment segment = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

    RiaOffshoreSphericalCoords startAziIncRad(m_startTangent);
    segment.inc = cvf::Math::toDegrees(startAziIncRad.inc());
    segment.azi = cvf::Math::toDegrees(startAziIncRad.azi());

    segment.TVD = -lineArcEndPoints[0].z();
    segment.NS = lineArcEndPoints[0].y();
    segment.EW = lineArcEndPoints[0].x();

    m_wpResult.push_back(segment);

    cvf::Vec3d p1 = m_lineArcEndPoints[0];
    cvf::Vec3d p2 = m_lineArcEndPoints[1];

    cvf::Vec3d t2 = m_startTangent; 

    for (size_t pIdx = 0; pIdx < m_lineArcEndPoints.size() - 1 ; ++pIdx)
    {
        addSegment(t2, m_lineArcEndPoints[pIdx], m_lineArcEndPoints[pIdx + 1] , &t2);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaWellPlanCalculator::addSegment(cvf::Vec3d t1, cvf::Vec3d p1, cvf::Vec3d p2, cvf::Vec3d* endTangent)
{
    cvf::Vec3d p1p2 = p2 - p1;

    CVF_ASSERT (p1p2.lengthSquared() > 1e-20);

    if (cvf::GeometryTools::getAngle(t1, p1p2) < 1e-5)
    {
        addLineSegment(p1, p2, endTangent);
    } 
    else // resample arc
    {
        addArcSegment(t1, p1, p2, endTangent);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaWellPlanCalculator::addLineSegment(cvf::Vec3d p1, cvf::Vec3d p2, cvf::Vec3d* endTangent)
{
    WellPlanSegment segment = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

    cvf::Vec3d p1p2 = p2 - p1;
    double length = p1p2.length();

    segment.CL = length;
    segment.MD = m_wpResult.back().MD + length;

    cvf::Vec3d tangent = p1p2 / length;

    RiaOffshoreSphericalCoords aziIncRad(p1p2);
    segment.inc = cvf::Math::toDegrees(aziIncRad.inc());
    segment.azi = cvf::Math::toDegrees(aziIncRad.azi());
    
    segment.TVD = -p2.z();
    segment.NS = p2.y();
    segment.EW = p2.x();
    segment.dogleg = 0.0;
    segment.build = 0.0;
    segment.turn = 0.0;

    m_wpResult.push_back(segment);
    *endTangent = tangent;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaWellPlanCalculator::addArcSegment(cvf::Vec3d t1, cvf::Vec3d p1, cvf::Vec3d p2, cvf::Vec3d* endTangent)
{
    WellPlanSegment segment = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

    RiaArcCurveCalculator arcCalc(p1, t1, p2);

    segment.CL = arcCalc.arcLength();
    segment.MD = m_wpResult.back().MD + segment.CL;
    segment.inc = cvf::Math::toDegrees(arcCalc.endInclination());
    segment.azi = cvf::Math::toDegrees(arcCalc.endAzimuth());
    segment.TVD = -p2.z();
    segment.NS = p2.y();
    segment.EW = p2.x();
    segment.dogleg = cvf::Math::toDegrees(30.0/arcCalc.radius());
    RiaOffshoreSphericalCoords startAziIncRad(t1);
    double buildInRadsPrLength = (arcCalc.endInclination() - startAziIncRad.inc())/arcCalc.arcLength();
    double turnInRadsPrLength = (arcCalc.endAzimuth() - startAziIncRad.azi())/arcCalc.arcLength();
    segment.build = 30*cvf::Math::toDegrees(buildInRadsPrLength) ;
    segment.turn = 30*cvf::Math::toDegrees(turnInRadsPrLength) ;

    m_wpResult.push_back(segment);

    (*endTangent) = arcCalc.endTangent();
}

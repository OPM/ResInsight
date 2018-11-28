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

#include "RiaPolyArcLineSampler.h"
#include "RiaArcCurveCalculator.h"

#include "cvfGeometryTools.h"
#include "cvfMatrix4.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPolyArcLineSampler::RiaPolyArcLineSampler(const cvf::Vec3d& startTangent, const std::vector<cvf::Vec3d>& lineArcEndPoints)
    : m_startTangent(startTangent)
    , m_lineArcEndPoints(lineArcEndPoints)
    , m_samplingsInterval(0.15)
    , m_isResamplingLines(true)
    , m_totalMD(0.0)
    , m_points(nullptr)
    , m_meshDs(nullptr)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

void RiaPolyArcLineSampler::sampledPointsAndMDs(double                   sampleInterval,
                                                bool                     isResamplingLines,
                                                std::vector<cvf::Vec3d>* points,
                                                std::vector<double>*     mds)
{
    CVF_ASSERT(sampleInterval > 0.0);

    m_samplingsInterval = sampleInterval;
    m_isResamplingLines = isResamplingLines;

    double startMD = 0.0;
    points->clear();
    mds->clear();

    std::vector<cvf::Vec3d> pointsNoDuplicates = RiaPolyArcLineSampler::pointsWithoutDuplicates(m_lineArcEndPoints);

    if (pointsNoDuplicates.size() < 2) return;

    m_points  = points;
    m_meshDs  = mds;
    m_totalMD = startMD;

    cvf::Vec3d p1 = pointsNoDuplicates[0];
    cvf::Vec3d p2 = pointsNoDuplicates[1];

    m_points->push_back(p1);
    m_meshDs->push_back(m_totalMD);

    cvf::Vec3d t2 = m_startTangent;

    for (size_t pIdx = 0; pIdx < pointsNoDuplicates.size() - 1; ++pIdx)
    {
        sampleSegment(t2, pointsNoDuplicates[pIdx], pointsNoDuplicates[pIdx + 1], &t2);
    }

    return;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPolyArcLineSampler::sampleSegment(cvf::Vec3d t1, cvf::Vec3d p1, cvf::Vec3d p2, cvf::Vec3d* endTangent)
{
    cvf::Vec3d p1p2 = p2 - p1;

    CVF_ASSERT(p1p2.lengthSquared() > 1e-20);

    if (cvf::GeometryTools::getAngle(t1, p1p2) < 1e-5)
    {
        sampleLine(p1, p2, endTangent);
    }
    else // resample arc
    {
        sampleArc(t1, p1, p2, endTangent);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RiaPolyArcLineSampler::pointsWithoutDuplicates(const std::vector<cvf::Vec3d>& points)
{
    std::vector<cvf::Vec3d> outputPoints;

    cvf::Vec3d   previousPoint = cvf::Vec3d::UNDEFINED;
    const double threshold     = 1e-6;
    for (const auto& p : points)
    {
        if (previousPoint.isUndefined() || ((previousPoint - p).lengthSquared()) > threshold)
        {
            outputPoints.push_back(p);
            previousPoint = p;
        }
    }

    return outputPoints;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPolyArcLineSampler::sampleLine(cvf::Vec3d p1, cvf::Vec3d p2, cvf::Vec3d* endTangent)
{
    cvf::Vec3d p1p2 = p2 - p1;

    double p1p2Length = p1p2.length();
    if (p1p2Length > m_samplingsInterval && m_isResamplingLines)
    {
        cvf::Vec3d tp1p2 = p1p2 / p1p2Length;
        double     mdInc = m_samplingsInterval;
        while (mdInc < p1p2Length)
        {
            cvf::Vec3d ps = p1 + mdInc * tp1p2;
            m_points->push_back(ps);
            m_meshDs->push_back(m_totalMD + mdInc);
            mdInc += m_samplingsInterval;
        }
    }
    m_totalMD += p1p2Length;
    m_points->push_back(p2);
    m_meshDs->push_back(m_totalMD);

    (*endTangent) = p1p2.getNormalized();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPolyArcLineSampler::sampleArc(cvf::Vec3d t1, cvf::Vec3d p1, cvf::Vec3d p2, cvf::Vec3d* endTangent)
{
    // Find arc CS
    RiaArcCurveCalculator CS_rad(p1, t1, p2);

    double     radius = CS_rad.radius();
    cvf::Mat4d arcCS  = CS_rad.arcCS();

    double angleInc = m_samplingsInterval / radius;

    cvf::Vec3d C = CS_rad.center();
    cvf::Vec3d N = CS_rad.normal();

    // Sample arc by
    // Rotate vector an increment, and transform to arc CS

    double arcAngle = cvf::GeometryTools::getAngle(N, p1 - C, p2 - C);
    if (arcAngle / angleInc > 5000)
    {
        angleInc = arcAngle / 5000;
    }

    for (double angle = angleInc; angle < arcAngle; angle += angleInc)
    {
        cvf::Vec3d C_to_incP = cvf::Vec3d::X_AXIS;
        C_to_incP *= radius;
        C_to_incP.transformVector(cvf::Mat3d::fromRotation(cvf::Vec3d::Z_AXIS, angle));

        C_to_incP.transformPoint(arcCS);

        m_points->push_back(C_to_incP);
        m_meshDs->push_back(m_totalMD + angle * radius);
    }
    m_totalMD += arcAngle * radius;
    m_points->push_back(p2);
    m_meshDs->push_back(m_totalMD);

    (*endTangent) = CS_rad.endTangent();
}

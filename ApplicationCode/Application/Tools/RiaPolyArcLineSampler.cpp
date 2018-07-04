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
#include "cvfGeometryTools.h"
#include "cvfMatrix4.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaPolyArcLineSampler::RiaPolyArcLineSampler(const cvf::Vec3d& startTangent, 
                                             const std::vector<cvf::Vec3d>& lineArcEndPoints)
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

void RiaPolyArcLineSampler::sampledPointsAndMDs(double sampleInterval,
                                                bool isResamplingLines,
                                                std::vector<cvf::Vec3d>* points,
                                                std::vector<double>* mds)
{
    CVF_ASSERT(sampleInterval > 0.0);

    m_samplingsInterval = sampleInterval;
    m_isResamplingLines = isResamplingLines;

    double startMD = 0.0;
    points->clear();
    mds->clear();

    if (m_lineArcEndPoints.size() < 2) return ;

    m_points = points;
    m_meshDs = mds;
    m_totalMD = startMD;

    cvf::Vec3d p1 = m_lineArcEndPoints[0];
    cvf::Vec3d p2 = m_lineArcEndPoints[1];

    m_points->push_back(p1);
    m_meshDs->push_back(m_totalMD);

    cvf::Vec3d t2 = m_startTangent; 

    for (size_t pIdx = 0; pIdx < m_lineArcEndPoints.size() - 1 ; ++pIdx)
    {
        sampleSegment(t2, m_lineArcEndPoints[pIdx], m_lineArcEndPoints[pIdx + 1] , &t2);
    }

    return ;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaPolyArcLineSampler::sampleSegment(cvf::Vec3d t1, cvf::Vec3d p1, cvf::Vec3d p2, cvf::Vec3d* endTangent)
{
    cvf::Vec3d p1p2 = p2 - p1;
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
void RiaPolyArcLineSampler::sampleLine(cvf::Vec3d p1, cvf::Vec3d p2, cvf::Vec3d* endTangent )
{
    cvf::Vec3d p1p2 = p2 - p1;

    double p1p2Length = p1p2.length();
    if ( p1p2Length > m_samplingsInterval && m_isResamplingLines )
    {
        cvf::Vec3d tp1p2 = p1p2 / p1p2Length;
        double mdInc = m_samplingsInterval;
        while ( mdInc < p1p2Length )
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

std::pair<cvf::Mat4d, double> calculateArcCSAndRadius(cvf::Vec3d t1, cvf::Vec3d p1, cvf::Vec3d p2);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaPolyArcLineSampler::sampleArc(cvf::Vec3d t1, cvf::Vec3d p1, cvf::Vec3d p2, cvf::Vec3d* endTangent)
{
    // Find arc CS
    auto CS_rad = calculateArcCSAndRadius(t1, p1, p2);

    double radius = CS_rad.second;
    
    //if (radius > 1e)
    // Find sampleLength angle

    double angleInc = m_samplingsInterval/ CS_rad.second;

    cvf::Vec3d C = CS_rad.first.translation();
    cvf::Vec3d N(CS_rad.first.col(2));
    cvf::Vec3d tr2 = (C - p2).getNormalized();
    cvf::Vec3d t2 = tr2 ^ N;

    // Sample arc by 
    // Rotate vector an increment, and transform to arc CS

    double arcAngle = cvf::GeometryTools::getAngle(N, p1-C, p2-C);
    if (arcAngle/angleInc > 5000)
    {
        angleInc = arcAngle/5000;
    }

    for ( double angle = angleInc; angle < arcAngle; angle += angleInc )
    {
        cvf::Vec3d C_to_incP = cvf::Vec3d::X_AXIS;
        C_to_incP *= CS_rad.second;
        C_to_incP.transformVector(cvf::Mat3d::fromRotation(cvf::Vec3d::Z_AXIS, angle));

        C_to_incP.transformPoint(CS_rad.first);

        m_points->push_back(C_to_incP);
        m_meshDs->push_back(m_totalMD + angle * CS_rad.second);

    }
    m_totalMD += arcAngle*CS_rad.second;
    m_points->push_back(p2);
    m_meshDs->push_back(m_totalMD);

    (*endTangent) = t2;
}

//--------------------------------------------------------------------------------------------------
///                + p1 
///           t1 // 
///              |      + C   
///               \
///                + p2
//--------------------------------------------------------------------------------------------------
std::pair<cvf::Mat4d, double> calculateArcCSAndRadius(cvf::Vec3d t1, cvf::Vec3d p1, cvf::Vec3d p2)
{
    t1.normalize();
    cvf::Vec3d p1p2 = p2 - p1;
    cvf::Vec3d t12 = p1p2.getNormalized();
    cvf::Vec3d N = (t1 ^ t12).getNormalized();
    cvf::Vec3d tr1 = (N ^ t1).getNormalized();
    double radius = 0.5*p1p2.length()/(tr1.dot(t12));
    cvf::Vec3d C = p1 + radius * tr1;

    cvf::Vec3d nTr1 = -tr1;
    cvf::Mat4d CS = cvf::Mat4d::fromCoordSystemAxes(&nTr1, &t1, &N);
    CS.setTranslation(C);

    return std::make_pair(CS, radius);
}



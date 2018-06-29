/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RigWellPath.h"

#include "cvfGeometryTools.h"
#include "cvfBoundingBox.h"
#include "cvfPlane.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellPath::RigWellPath()
    : m_hasDatumElevation(false),
    m_datumElevation(0.0)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellPath::setDatumElevation(double value)
{
    m_hasDatumElevation = true;
    m_datumElevation = value;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigWellPath::hasDatumElevation() const
{
    return m_hasDatumElevation;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigWellPath::datumElevation() const
{
    return m_datumElevation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellPath::rkbDiff() const
{
    if (hasDatumElevation())
    {
        return datumElevation();
    }

    // If measured depth is zero, use the z-value of the well path points
    if (m_wellPathPoints.size() > 0 && m_measuredDepths.size() > 0)
    {
        double epsilon = 1e-3;

        if (cvf::Math::abs(m_measuredDepths[0]) < epsilon)
        {
            double diff = m_measuredDepths[0] - (-wellPathPoints()[0].z());

            return diff;
        }
    }
    return HUGE_VAL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigWellPath::interpolatedVectorAlongWellPath(const std::vector<cvf::Vec3d>& vectors,
                                                        double measuredDepth,
                                                        double * horizontalLengthAlongWellToStartClipPoint /*= nullptr*/) const
{
    CVF_ASSERT(vectors.size() == m_wellPathPoints.size());
    cvf::Vec3d interpolatedVector = cvf::Vec3d::ZERO;

    if (horizontalLengthAlongWellToStartClipPoint) *horizontalLengthAlongWellToStartClipPoint = 0.0;
    
    size_t vxIdx = 0;
    while ( vxIdx < m_measuredDepths.size() && m_measuredDepths.at(vxIdx) < measuredDepth )
    {
        if ( vxIdx > 0 && horizontalLengthAlongWellToStartClipPoint)
        {
            cvf::Vec3d segment = m_wellPathPoints[vxIdx] - m_wellPathPoints[vxIdx-1];
            segment[2] = 0.0;
            *horizontalLengthAlongWellToStartClipPoint += segment.length();
        }
        vxIdx++;
    }

    if ( m_measuredDepths.size() > vxIdx )
    {
        if ( vxIdx == 0 )
        {
            //For measuredDepth same or lower than first point, use this first point
            interpolatedVector = vectors.at(0);
        }
        else
        {
            //Do interpolation
            double segmentFraction = (measuredDepth - m_measuredDepths.at(vxIdx-1)) /
                                     (m_measuredDepths.at(vxIdx) - m_measuredDepths.at(vxIdx - 1));
            cvf::Vec3d segment = m_wellPathPoints[vxIdx] - m_wellPathPoints[vxIdx - 1];
            interpolatedVector = (1.0 - segmentFraction) * vectors[vxIdx - 1] + segmentFraction * vectors[vxIdx];

            if ( horizontalLengthAlongWellToStartClipPoint )
            {
                segment[2] = 0.0;
                *horizontalLengthAlongWellToStartClipPoint += segment.length();
            }
        }
    }
    else
    {
        // Use endpoint if measuredDepth same or higher than last point
        interpolatedVector = vectors.at(vxIdx-1);
    }


    return interpolatedVector;
}

cvf::Vec3d RigWellPath::interpolatedPointAlongWellPath(double measuredDepth, double * horizontalLengthAlongWellToStartClipPoint /*= nullptr*/) const
{
    return interpolatedVectorAlongWellPath(m_wellPathPoints, measuredDepth, horizontalLengthAlongWellToStartClipPoint);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigWellPath::wellPathAzimuthAngle(const cvf::Vec3d& position) const
{
    size_t closestIndex = cvf::UNDEFINED_SIZE_T;
    double closestDistance = cvf::UNDEFINED_DOUBLE;

    for ( size_t i = 1; i < m_wellPathPoints.size(); i++ )
    {
        cvf::Vec3d p1 = m_wellPathPoints[i - 1];
        cvf::Vec3d p2 = m_wellPathPoints[i - 0];

        double candidateDistance = cvf::GeometryTools::linePointSquareDist(p1, p2, position);
        if ( candidateDistance < closestDistance )
        {
            closestDistance = candidateDistance;
            closestIndex = i;
        }
    }

    //For vertical well (x-component of direction = 0) returned angle will be 90. 
    double azimuthAngleDegrees = 90.0;

    if ( closestIndex != cvf::UNDEFINED_DOUBLE )
    {
        cvf::Vec3d p1;
        cvf::Vec3d p2;

        if ( closestIndex > 0 )
        {
            p1 = m_wellPathPoints[closestIndex - 1];
            p2 = m_wellPathPoints[closestIndex - 0];
        }
        else
        {
            p1 = m_wellPathPoints[closestIndex + 1];
            p2 = m_wellPathPoints[closestIndex + 0];
        }

        cvf::Vec3d direction = p2 - p1;

        if ( fabs(direction.y()) > 1e-5 )
        {
            double atanValue = direction.x() / direction.y();
            double azimuthRadians = atan(atanValue);
            azimuthAngleDegrees = cvf::Math::toDegrees(azimuthRadians);
        }
    }

    return azimuthAngleDegrees;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellPath::twoClosestPoints(const cvf::Vec3d& position, cvf::Vec3d* p1, cvf::Vec3d* p2) const
{
    CVF_ASSERT(p1 && p2);

    size_t closestIndex = cvf::UNDEFINED_SIZE_T;
    double closestDistance = cvf::UNDEFINED_DOUBLE;

    for ( size_t i = 1; i < m_wellPathPoints.size(); i++ )
    {
        cvf::Vec3d p1 = m_wellPathPoints[i - 1];
        cvf::Vec3d p2 = m_wellPathPoints[i - 0];

        double candidateDistance = cvf::GeometryTools::linePointSquareDist(p1, p2, position);
        if ( candidateDistance < closestDistance )
        {
            closestDistance = candidateDistance;
            closestIndex = i;
        }
    }

    if (closestIndex != cvf::UNDEFINED_SIZE_T)
    {
        if ( closestIndex > 0 )
        {
            *p1 = m_wellPathPoints[closestIndex - 1];
            *p2 = m_wellPathPoints[closestIndex - 0];
        }
        else
        {
            *p1 = m_wellPathPoints[closestIndex + 1];
            *p2 = m_wellPathPoints[closestIndex + 0];
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<cvf::Vec3d>, std::vector<double> > RigWellPath::clippedPointSubset(double startMD, 
                                                                                         double endMD,
                                                                                         double * horizontalLengthAlongWellToStartClipPoint) const
{
    std::pair<std::vector<cvf::Vec3d>, std::vector<double> >  pointsAndMDs;
    if ( m_measuredDepths.empty() ) return pointsAndMDs;
    if ( startMD > endMD ) return pointsAndMDs;

    pointsAndMDs.first.push_back(interpolatedPointAlongWellPath(startMD, horizontalLengthAlongWellToStartClipPoint));
    pointsAndMDs.second.push_back(startMD);

    for ( size_t i = 0; i < m_measuredDepths.size(); ++i )
    {
        double measuredDepth = m_measuredDepths[i];
        if ( measuredDepth > startMD && measuredDepth < endMD )
        {
            pointsAndMDs.first.push_back(m_wellPathPoints[i]);
            pointsAndMDs.second.push_back(measuredDepth);
        }
    }
    pointsAndMDs.first.push_back(interpolatedPointAlongWellPath(endMD));
    pointsAndMDs.second.push_back(endMD);


    return pointsAndMDs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigWellPath::wellPathPointsIncludingInterpolatedIntersectionPoint(double intersectionMeasuredDepth) const
{
    std::vector<cvf::Vec3d> points;
    if ( m_measuredDepths.empty() ) return points;

    cvf::Vec3d interpolatedWellPathPoint = interpolatedPointAlongWellPath(intersectionMeasuredDepth);

    for ( size_t i = 0; i < m_measuredDepths.size() - 1; i++ )
    {
        if ( m_measuredDepths[i] == intersectionMeasuredDepth )
        {
            points.push_back(m_wellPathPoints[i]);
        }
        else if ( m_measuredDepths[i] < intersectionMeasuredDepth )
        {
            points.push_back(m_wellPathPoints[i]);
            if ( m_measuredDepths[i + 1] > intersectionMeasuredDepth )
            {
                points.push_back(interpolatedWellPathPoint);
            }
        }
        else if ( m_measuredDepths[i] > intersectionMeasuredDepth )
        {
            if ( i == 0 )
            {
                points.push_back(interpolatedWellPathPoint);
            }
            else
            {
                points.push_back(m_wellPathPoints[i]);
            }
        }
    }
    points.push_back(m_wellPathPoints.back());

    return points;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigWellPath::isAnyPointInsideBoundingBox(const std::vector<cvf::Vec3d>& points, const cvf::BoundingBox& boundingBox)
{
    for (const cvf::Vec3d& point : points)
    {
        if (boundingBox.contains(point)) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigWellPath::clipPolylineStartAboveZ(const std::vector<cvf::Vec3d>& polyLine,
                                                             double maxZ,
                                                             double * horizontalLengthAlongWellToClipPoint,
                                                             size_t * indexToFirstVisibleSegment)
{
    CVF_ASSERT(horizontalLengthAlongWellToClipPoint);
    CVF_ASSERT(indexToFirstVisibleSegment);

    // Find first visible point, and accumulate distance along wellpath
    
    *horizontalLengthAlongWellToClipPoint = 0.0;
    *indexToFirstVisibleSegment = cvf::UNDEFINED_SIZE_T;

    size_t firstVisiblePointIndex = cvf::UNDEFINED_SIZE_T;

    for ( size_t vxIdx = 0 ; vxIdx < polyLine.size(); ++vxIdx )
    {
        if ( polyLine[vxIdx].z() > maxZ )
        {
            if ( vxIdx > 0 )
            {
                cvf::Vec3d segment = polyLine[vxIdx] - polyLine[vxIdx-1];
                segment[2] = 0.0;
                *horizontalLengthAlongWellToClipPoint += segment.length();
            }
        }
        else
        {
            firstVisiblePointIndex = vxIdx; 
            break;
        }
    }

    // Clip line, and add vx to the start of the clipped result
    
    std::vector<cvf::Vec3d> clippedPolyLine;

    if ( firstVisiblePointIndex ==  cvf::UNDEFINED_SIZE_T )
    {
        return clippedPolyLine;
    }

    if ( firstVisiblePointIndex > 0 )
    {

        cvf::Plane topPlane;
        topPlane.setFromPointAndNormal({ 0.0, 0.0, maxZ }, cvf::Vec3d::Z_AXIS);
        cvf::Vec3d intersection;

        if ( topPlane.intersect(polyLine[firstVisiblePointIndex-1],
                                polyLine[firstVisiblePointIndex],
                                &intersection) )
        {
            cvf::Vec3d segment = intersection - polyLine[firstVisiblePointIndex-1];
            segment[2] = 0.0;
            *horizontalLengthAlongWellToClipPoint += segment.length();

            clippedPolyLine.push_back(intersection);
        }

        *indexToFirstVisibleSegment = firstVisiblePointIndex - 1;
    }
    else
    {
        *indexToFirstVisibleSegment = 0;
    }
    
    // Add the rest of the polyline

    for (  size_t vxIdx = firstVisiblePointIndex; vxIdx < polyLine.size(); ++vxIdx )
    {
        clippedPolyLine.push_back(polyLine[vxIdx]);
    }

    return clippedPolyLine;
}

const std::vector<cvf::Vec3d>& RigWellPath::wellPathPoints() const
{
    return m_wellPathPoints;
}

const std::vector<double>& RigWellPath::measureDepths() const
{
    return m_measuredDepths;
}


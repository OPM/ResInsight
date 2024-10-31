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

#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"
#include "cvfPlane.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellPath::RigWellPath()
    : cvf::Object()
    , m_hasDatumElevation( false )
    , m_datumElevation( 0.0 )
    , m_uniqueStartIndex( 0u )
    , m_uniqueEndIndex( std::numeric_limits<size_t>::max() )
    , objectBeingDeleted( this )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellPath::RigWellPath( const RigWellPath& rhs )
    : cvf::Object()
    , m_wellPathPoints( rhs.m_wellPathPoints )
    , m_measuredDepths( rhs.m_measuredDepths )
    , m_hasDatumElevation( rhs.m_hasDatumElevation )
    , m_datumElevation( rhs.m_datumElevation )
    , m_uniqueStartIndex( rhs.m_uniqueStartIndex )
    , m_uniqueEndIndex( rhs.m_uniqueEndIndex )
    , objectBeingDeleted( this )
{
    CVF_ASSERT( m_wellPathPoints.size() == m_measuredDepths.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellPath::RigWellPath( const std::vector<cvf::Vec3d>& wellPathPoints, const std::vector<double>& measuredDepths )
    : cvf::Object()
    , m_wellPathPoints( wellPathPoints )
    , m_measuredDepths( measuredDepths )
    , m_hasDatumElevation( false )
    , m_datumElevation( 0.0 )
    , m_uniqueStartIndex( 0u )
    , m_uniqueEndIndex( std::numeric_limits<size_t>::max() )
    , objectBeingDeleted( this )
{
    CVF_ASSERT( m_wellPathPoints.size() == m_measuredDepths.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellPath& RigWellPath::operator=( const RigWellPath& rhs )
{
    m_wellPathPoints = rhs.m_wellPathPoints;
    m_measuredDepths = rhs.m_measuredDepths;
    CVF_ASSERT( m_wellPathPoints.size() == m_measuredDepths.size() );
    m_hasDatumElevation = rhs.m_hasDatumElevation;
    m_datumElevation    = rhs.m_datumElevation;
    m_uniqueStartIndex  = rhs.m_uniqueStartIndex;
    m_uniqueEndIndex    = rhs.m_uniqueEndIndex;
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellPath::~RigWellPath()
{
    objectBeingDeleted.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3d>& RigWellPath::wellPathPoints() const
{
    return m_wellPathPoints;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigWellPath::measuredDepths() const
{
    return m_measuredDepths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellPath::trueVerticalDepths() const
{
    std::vector<double> tvds;
    for ( const cvf::Vec3d& point : m_wellPathPoints )
    {
        tvds.push_back( std::fabs( point.z() ) );
    }
    return tvds;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPath::setWellPathPoints( const std::vector<cvf::Vec3d>& wellPathPoints )
{
    m_wellPathPoints = wellPathPoints;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPath::setMeasuredDepths( const std::vector<double>& measuredDepths )
{
    m_measuredDepths = measuredDepths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPath::addWellPathPoint( const cvf::Vec3d& wellPathPoint )
{
    m_wellPathPoints.push_back( wellPathPoint );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPath::addMeasuredDepth( double measuredDepth )
{
    m_measuredDepths.push_back( measuredDepth );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPath::setDatumElevation( double value )
{
    m_hasDatumElevation = true;
    m_datumElevation    = value;
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
    if ( hasDatumElevation() )
    {
        return datumElevation();
    }

    // If measured depth is zero, use the z-value of the well path points
    if ( !m_wellPathPoints.empty() && !m_measuredDepths.empty() )
    {
        double epsilon = 1e-3;

        if ( cvf::Math::abs( m_measuredDepths[0] ) < epsilon )
        {
            double diff = m_measuredDepths[0] - ( -wellPathPoints()[0].z() );

            return diff;
        }

        if ( cvf::Math::abs( m_wellPathPoints[0].z() ) < epsilon )
        {
            return m_measuredDepths[0]; // Assume a vertical drop before the first md point.
        }
    }
    return HUGE_VAL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigWellPath::interpolatedVectorValuesAlongWellPath( const std::vector<cvf::Vec3d>& vectorValuesAlongWellPath,
                                                               double                         measuredDepth,
                                                               double* horizontalLengthAlongWellToStartClipPoint /*= nullptr*/ ) const
{
    CVF_ASSERT( vectorValuesAlongWellPath.size() == m_wellPathPoints.size() );
    cvf::Vec3d interpolatedVector = cvf::Vec3d::ZERO;

    if ( horizontalLengthAlongWellToStartClipPoint ) *horizontalLengthAlongWellToStartClipPoint = 0.0;

    size_t vxIdx = 0;
    while ( vxIdx < m_measuredDepths.size() && m_measuredDepths.at( vxIdx ) < measuredDepth )
    {
        if ( vxIdx > 0 && horizontalLengthAlongWellToStartClipPoint )
        {
            cvf::Vec3d segment = m_wellPathPoints[vxIdx] - m_wellPathPoints[vxIdx - 1];
            segment[2]         = 0.0;
            *horizontalLengthAlongWellToStartClipPoint += segment.length();
        }
        vxIdx++;
    }

    if ( m_measuredDepths.size() > vxIdx )
    {
        if ( vxIdx == 0 )
        {
            // For measuredDepth same or lower than first point, use this first point
            interpolatedVector = vectorValuesAlongWellPath.at( 0 );
        }
        else
        {
            // Do interpolation
            double segmentFraction =
                ( measuredDepth - m_measuredDepths.at( vxIdx - 1 ) ) / ( m_measuredDepths.at( vxIdx ) - m_measuredDepths.at( vxIdx - 1 ) );
            cvf::Vec3d segment = m_wellPathPoints[vxIdx] - m_wellPathPoints[vxIdx - 1];
            interpolatedVector = ( 1.0 - segmentFraction ) * vectorValuesAlongWellPath[vxIdx - 1] +
                                 segmentFraction * vectorValuesAlongWellPath[vxIdx];

            if ( horizontalLengthAlongWellToStartClipPoint )
            {
                segment[2] = 0.0;
                *horizontalLengthAlongWellToStartClipPoint += segment.length() * segmentFraction;
            }
        }
    }
    else
    {
        // Use endpoint if measuredDepth same or higher than last point
        interpolatedVector = vectorValuesAlongWellPath.at( vxIdx - 1 );
    }

    return interpolatedVector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigWellPath::interpolatedPointAlongWellPath( double  measuredDepth,
                                                        double* horizontalLengthAlongWellToStartClipPoint /*= nullptr*/ ) const
{
    return interpolatedVectorValuesAlongWellPath( m_wellPathPoints, measuredDepth, horizontalLengthAlongWellToStartClipPoint );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigWellPath::tangentAlongWellPath( double measuredDepth ) const
{
    if ( m_measuredDepths.size() < 2u ) return cvf::Vec3d::UNDEFINED;

    if ( measuredDepth <= m_measuredDepths.front() )
    {
        return ( m_wellPathPoints[1] - m_wellPathPoints[0] ).getNormalized();
    }

    if ( measuredDepth >= m_measuredDepths.back() )
    {
        auto N = m_measuredDepths.size();
        return ( m_wellPathPoints[N - 1] - m_wellPathPoints[N - 2] ).getNormalized();
    }

    for ( size_t i = 1; i < m_measuredDepths.size(); i++ )
    {
        cvf::Vec3d point1 = m_wellPathPoints[i - 1];
        cvf::Vec3d point2 = m_wellPathPoints[i - 0];

        double md1 = m_measuredDepths[i - 1];
        double md2 = m_measuredDepths[i];

        if ( measuredDepth >= md1 && measuredDepth < md2 )
        {
            return ( point2 - point1 ).getNormalized();
        }
    }

    return cvf::Vec3d::UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellPath::wellPathAzimuthAngle( const cvf::Vec3d& position ) const
{
    // For vertical well (x-component of direction = 0) returned angle will be 90.
    double azimuthAngleDegrees = 90.0;

    cvf::Vec3d p1 = cvf::Vec3d::UNDEFINED;
    cvf::Vec3d p2 = cvf::Vec3d::UNDEFINED;
    twoClosestPoints( position, &p1, &p2 );

    if ( !p1.isUndefined() )
    {
        cvf::Vec3d direction = p2 - p1;

        if ( fabs( direction.y() ) > 1e-5 )
        {
            double atanValue      = direction.x() / direction.y();
            double azimuthRadians = atan( atanValue );
            azimuthAngleDegrees   = cvf::Math::toDegrees( azimuthRadians );
        }
    }

    return azimuthAngleDegrees;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPath::twoClosestPoints( const cvf::Vec3d& position, cvf::Vec3d* p1, cvf::Vec3d* p2 ) const
{
    CVF_ASSERT( p1 && p2 );

    auto closeIndices = closestIndices( position );
    if ( closeIndices.first != cvf::UNDEFINED_SIZE_T )
    {
        *p1 = m_wellPathPoints[closeIndices.first];
        *p2 = m_wellPathPoints[closeIndices.second];
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellPath::identicalTubeLength( const RigWellPath& other ) const
{
    const double eps = 1.0e-8;

    size_t minimumVertexCount = std::min( m_wellPathPoints.size(), other.wellPathPoints().size() );
    if ( minimumVertexCount < 2u ) return 0.0;

    double identicalLength = 0.0;
    if ( ( m_wellPathPoints.front() - other.wellPathPoints().front() ).length() < eps )
    {
        for ( size_t vIndex = 1; vIndex < minimumVertexCount; ++vIndex )
        {
            if ( ( m_wellPathPoints[vIndex] - other.wellPathPoints()[vIndex] ).length() < eps )
            {
                identicalLength = m_measuredDepths[vIndex];
            }
            else
            {
                break;
            }
        }
    }
    return identicalLength;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellPath::closestMeasuredDepth( const cvf::Vec3d& position ) const
{
    auto [firstIndex, secondIndex] = closestIndices( position );
    if ( firstIndex != cvf::UNDEFINED_SIZE_T )
    {
        cvf::Vec3d p1 = m_wellPathPoints[firstIndex];
        cvf::Vec3d p2 = m_wellPathPoints[secondIndex];

        double diffP1 = ( p1 - position ).length();
        double diffP2 = ( p2 - position ).length();

        double weigth1 = diffP2 / ( diffP1 + diffP2 );

        double measureDepth1 = m_measuredDepths[firstIndex];
        double measureDepth2 = m_measuredDepths[secondIndex];

        double interpolatedValue = measureDepth1 * weigth1 + measureDepth2 * ( 1.0 - weigth1 );
        return interpolatedValue;
    }

    return -1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigWellPath> RigWellPath::commonGeometry( const std::vector<const RigWellPath*>& allGeometries )
{
    const double eps = 1.0e-3;

    if ( allGeometries.empty() ) return nullptr;

    if ( allGeometries.size() == 1u ) return cvf::ref<RigWellPath>( new RigWellPath( *allGeometries.front() ) );

    const RigWellPath* firstGeometry = allGeometries.front();

    std::vector<cvf::Vec3d> commonWellPathPoints;
    std::vector<double>     commonMDs;

    for ( size_t vIndex = 0u; vIndex < firstGeometry->wellPathPoints().size(); ++vIndex )
    {
        const cvf::Vec3d& firstGeometryVertex = firstGeometry->wellPathPoints()[vIndex];

        bool allMatches = std::all_of( allGeometries.begin() + 1,
                                       allGeometries.end(),
                                       [=]( const RigWellPath* geometry )
                                       {
                                           if ( geometry->wellPathPoints().size() > vIndex )
                                           {
                                               return ( firstGeometryVertex - geometry->wellPathPoints()[vIndex] ).length() < eps;
                                           }
                                           return false;
                                       } );

        if ( allMatches )
        {
            commonWellPathPoints.push_back( firstGeometryVertex );
            commonMDs.push_back( firstGeometry->measuredDepths()[vIndex] );
        }
        else
        {
            break;
        }
    }
    return cvf::ref<RigWellPath>( new RigWellPath( commonWellPathPoints, commonMDs ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellPath::setUniqueStartAndEndIndex( size_t uniqueStartIndex, size_t uniqueEndIndex )
{
    if ( m_measuredDepths.empty() ) return;

    m_uniqueStartIndex = std::clamp( uniqueStartIndex, (size_t)0u, m_measuredDepths.size() - 1u );
    m_uniqueEndIndex   = std::clamp( uniqueEndIndex, m_uniqueStartIndex, m_measuredDepths.size() - 1u );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigWellPath::uniqueStartIndex() const
{
    return m_uniqueStartIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigWellPath::uniqueEndIndex() const
{
    if ( m_measuredDepths.empty() ) return 0;

    return std::clamp( m_uniqueEndIndex, m_uniqueStartIndex, m_measuredDepths.size() - 1u );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigWellPath::uniqueWellPathPoints() const
{
    if ( m_wellPathPoints.empty() ) return {};

    return std::vector<cvf::Vec3d>( m_wellPathPoints.begin() + uniqueStartIndex(), m_wellPathPoints.begin() + uniqueEndIndex() + 1u );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellPath::uniqueMeasuredDepths() const
{
    if ( m_measuredDepths.empty() ) return {};

    return std::vector<double>( m_measuredDepths.begin() + m_uniqueStartIndex, m_measuredDepths.begin() + uniqueEndIndex() + 1u );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<cvf::Vec3d>, std::vector<double>>
    RigWellPath::clippedPointSubset( double startMD, double endMD, double* horizontalLengthAlongWellToStartClipPoint ) const
{
    if ( m_measuredDepths.empty() ) return {};
    if ( startMD > endMD ) return {};

    std::pair<std::vector<cvf::Vec3d>, std::vector<double>> pointsAndMDs;

    pointsAndMDs.first.push_back( interpolatedPointAlongWellPath( startMD, horizontalLengthAlongWellToStartClipPoint ) );
    pointsAndMDs.second.push_back( startMD );

    for ( size_t i = 0; i < m_measuredDepths.size(); ++i )
    {
        double measuredDepth = m_measuredDepths[i];
        if ( measuredDepth > startMD && measuredDepth <= endMD )
        {
            pointsAndMDs.first.push_back( m_wellPathPoints[i] );
            pointsAndMDs.second.push_back( measuredDepth );
        }
    }
    pointsAndMDs.first.push_back( interpolatedPointAlongWellPath( endMD ) );
    pointsAndMDs.second.push_back( endMD );

    return pointsAndMDs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigWellPath::wellPathPointsIncludingInterpolatedIntersectionPoint( double intersectionMeasuredDepth ) const
{
    std::vector<cvf::Vec3d> points;
    if ( m_measuredDepths.empty() ) return points;

    cvf::Vec3d interpolatedWellPathPoint = interpolatedPointAlongWellPath( intersectionMeasuredDepth );

    for ( size_t i = 0; i < m_measuredDepths.size() - 1; i++ )
    {
        if ( m_measuredDepths[i] == intersectionMeasuredDepth )
        {
            points.push_back( m_wellPathPoints[i] );
        }
        else if ( m_measuredDepths[i] < intersectionMeasuredDepth )
        {
            points.push_back( m_wellPathPoints[i] );
            if ( m_measuredDepths[i + 1] > intersectionMeasuredDepth )
            {
                points.push_back( interpolatedWellPathPoint );
            }
        }
        else if ( m_measuredDepths[i] > intersectionMeasuredDepth )
        {
            if ( i == 0 )
            {
                points.push_back( interpolatedWellPathPoint );
            }
            else
            {
                points.push_back( m_wellPathPoints[i] );
            }
        }
    }
    points.push_back( m_wellPathPoints.back() );

    return points;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWellPath::isAnyPointInsideBoundingBox( const std::vector<cvf::Vec3d>& points, const cvf::BoundingBox& boundingBox )
{
    for ( const cvf::Vec3d& point : points )
    {
        if ( boundingBox.contains( point ) ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigWellPath::clipPolylineStartAboveZ( const std::vector<cvf::Vec3d>& polyLine,
                                                              const double                   maxZ,
                                                              double&                        horizontalLengthAlongWellToClipPoint,
                                                              double&                        measuredDepthAtFirstClipPoint,
                                                              size_t&                        indexToFirstVisibleSegment )
{
    // Find first visible point, and accumulate distance along wellpath

    horizontalLengthAlongWellToClipPoint = 0.0;
    indexToFirstVisibleSegment           = cvf::UNDEFINED_SIZE_T;

    size_t firstVisiblePointIndex = cvf::UNDEFINED_SIZE_T;

    for ( size_t vxIdx = 0; vxIdx < polyLine.size(); ++vxIdx )
    {
        if ( polyLine[vxIdx].z() > maxZ )
        {
            if ( vxIdx > 0 )
            {
                cvf::Vec3d segment = polyLine[vxIdx] - polyLine[vxIdx - 1];
                measuredDepthAtFirstClipPoint += segment.length();

                segment[2] = 0.0;
                horizontalLengthAlongWellToClipPoint += segment.length();
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

    if ( firstVisiblePointIndex == cvf::UNDEFINED_SIZE_T )
    {
        return clippedPolyLine;
    }

    if ( firstVisiblePointIndex > 0 )
    {
        cvf::Plane topPlane;
        topPlane.setFromPointAndNormal( { 0.0, 0.0, maxZ }, cvf::Vec3d::Z_AXIS );
        cvf::Vec3d intersection;

        if ( topPlane.intersect( polyLine[firstVisiblePointIndex - 1], polyLine[firstVisiblePointIndex], &intersection ) )
        {
            cvf::Vec3d segment = intersection - polyLine[firstVisiblePointIndex - 1];
            segment[2]         = 0.0;
            horizontalLengthAlongWellToClipPoint += segment.length();

            clippedPolyLine.push_back( intersection );
        }

        indexToFirstVisibleSegment = firstVisiblePointIndex - 1;
    }
    else
    {
        indexToFirstVisibleSegment = 0;
    }

    // Add the rest of the polyline

    for ( size_t vxIdx = firstVisiblePointIndex; vxIdx < polyLine.size(); ++vxIdx )
    {
        clippedPolyLine.push_back( polyLine[vxIdx] );
    }

    return clippedPolyLine;
}

//--------------------------------------------------------------------------------------------------
// Returns the closes indices with smallest index first
// If not found, cvf::UNDEFINED_SIZE_T is returned for both
//--------------------------------------------------------------------------------------------------
std::pair<size_t, size_t> RigWellPath::closestIndices( const cvf::Vec3d& position ) const
{
    size_t closestIndex    = cvf::UNDEFINED_SIZE_T;
    double closestDistance = cvf::UNDEFINED_DOUBLE;

    for ( size_t i = 1; i < m_wellPathPoints.size(); i++ )
    {
        cvf::Vec3d point1 = m_wellPathPoints[i - 1];
        cvf::Vec3d point2 = m_wellPathPoints[i - 0];

        double candidateDistance = cvf::GeometryTools::linePointSquareDist( point1, point2, position );
        if ( candidateDistance < closestDistance )
        {
            closestDistance = candidateDistance;
            closestIndex    = i;
        }
    }

    if ( closestIndex != cvf::UNDEFINED_SIZE_T )
    {
        if ( closestIndex > 0 )
        {
            return { closestIndex - 1, closestIndex };
        }
        else
        {
            return { closestIndex, closestIndex + 1 };
        }
    }

    return { cvf::UNDEFINED_SIZE_T, cvf::UNDEFINED_SIZE_T };
}

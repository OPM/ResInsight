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

#include "RigWellPathGeometryTools.h"

#include "RigWellPath.h"

#include "cvfMath.h"
#include "cvfMatrix3.h"

#include <algorithm>
#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigWellPathGeometryTools::calculateLineSegmentNormals( const std::vector<cvf::Vec3d>& vertices,
                                                                               double planeAngle )
{
    std::vector<cvf::Vec3d> pointNormals;

    if ( vertices.empty() ) return pointNormals;

    pointNormals.reserve( vertices.size() );

    const cvf::Vec3d up( 0, 0, 1 );
    const cvf::Vec3d rotatedUp =
        up.getTransformedVector( cvf::Mat3d::fromRotation( cvf::Vec3d( 0.0, 1.0, 0.0 ), planeAngle ) );

    const cvf::Vec3d dominantDirection = estimateDominantDirectionInXYPlane( vertices );

    const cvf::Vec3d projectionPlaneNormal = ( up ^ dominantDirection ).getNormalized();
    CVF_ASSERT( projectionPlaneNormal * dominantDirection <= std::numeric_limits<double>::epsilon() );

    double sumDotWithRotatedUp = 0.0;
    for ( size_t i = 0; i < vertices.size() - 1; ++i )
    {
        cvf::Vec3d p1 = vertices[i];
        cvf::Vec3d p2 = vertices[i + 1];

        cvf::Vec3d tangent = ( p2 - p1 ).getNormalized();
        cvf::Vec3d normal( 0, 0, 0 );
        if ( cvf::Math::abs( tangent * projectionPlaneNormal ) < 0.7071 )
        {
            cvf::Vec3d projectedTangent =
                ( tangent - ( tangent * projectionPlaneNormal ) * projectionPlaneNormal ).getNormalized();
            normal = ( projectedTangent ^ projectionPlaneNormal ).getNormalized();
            normal = normal.getTransformedVector( cvf::Mat3d::fromRotation( tangent, planeAngle ) );
        }
        pointNormals.push_back( normal );
        sumDotWithRotatedUp += normal * rotatedUp;
    }

    pointNormals.push_back( pointNormals.back() );

    if ( sumDotWithRotatedUp < 0.0 )
    {
        for ( cvf::Vec3d& normal : pointNormals )
        {
            normal *= -1.0;
        }
    }

    return interpolateUndefinedNormals( up, pointNormals, vertices );
}

//--------------------------------------------------------------------------------------------------
/// Lets you estimate MD values from an existing md/tvd relationship and a new set of TVD-values
/// Requires the points to be ordered from the start/top of the well path to the end/bottom.
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellPathGeometryTools::interpolateMdFromTvd( const std::vector<double>& originalMdValues,
                                                                    const std::vector<double>& originalTvdValues,
                                                                    const std::vector<double>& tvdValuesToInterpolateFrom )
{
    CVF_ASSERT( !originalMdValues.empty() );
    if ( originalMdValues.size() < 2u )
    {
        return {originalMdValues};
    }

    std::vector<double> interpolatedMdValues;
    interpolatedMdValues.reserve( tvdValuesToInterpolateFrom.size() );

    QwtSpline        spline              = createSpline( originalMdValues, originalTvdValues );
    std::vector<int> segmentStartIndices = findSplineSegmentsContainingRoots( spline, tvdValuesToInterpolateFrom );

    for ( size_t i = 0; i < segmentStartIndices.size(); ++i )
    {
        double currentTVDValue = tvdValuesToInterpolateFrom[i];
        double startMD         = spline.points().front().x();
        double endMD           = spline.points().back().y();
        if ( segmentStartIndices[i] != -1 )
        {
            int startIndex = segmentStartIndices[i];
            int endIndex   = startIndex + 1;

            // Search interval for best MD value
            startMD = spline.points()[startIndex].x();
            endMD   = spline.points().back().y();

            if ( endIndex < spline.points().size() )
            {
                if ( !interpolatedMdValues.empty() )
                {
                    double mdDiff = 0.0;
                    if ( interpolatedMdValues.size() > 1 )
                    {
                        mdDiff = interpolatedMdValues[i - 1] - interpolatedMdValues[i - 2];
                    }
                    startMD = std::max( startMD, interpolatedMdValues.back() + 0.1 * mdDiff );
                }
                endMD = spline.points()[endIndex].x();
            }
        }
        double mdValue = solveForX( spline, startMD, endMD, currentTVDValue );
        interpolatedMdValues.push_back( mdValue );
    }
    return interpolatedMdValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int>
    RigWellPathGeometryTools::findSplineSegmentsContainingRoots( const QwtSpline&           spline,
                                                                 const std::vector<double>& tvdValuesToInterpolateFrom )
{
    std::vector<int> segmentStartIndices;
    segmentStartIndices.reserve( tvdValuesToInterpolateFrom.size() );

    int lastSplineStartIndex = 0;
    for ( double tvdValue : tvdValuesToInterpolateFrom )
    {
        int currentSplineStartIndex = lastSplineStartIndex;

        bool foundMatch = false;
        // Increment current_it until we find an interval containing our TVD
        while ( currentSplineStartIndex < spline.points().size() - 2 )
        {
            double diffCurrent = spline.points()[currentSplineStartIndex].y() - tvdValue;
            if ( std::abs( diffCurrent ) < 1.0e-8 ) // Current is matching the point
            {
                foundMatch = true;
                break;
            }

            int nextStartIndex = currentSplineStartIndex + 1;

            double diffNext = spline.points()[nextStartIndex].y() - tvdValue;
            if ( diffCurrent * diffNext < 0.0 ) // One is above, the other is below
            {
                foundMatch = true;
                break;
            }
            currentSplineStartIndex = nextStartIndex;
        }
        if ( foundMatch )
        {
            segmentStartIndices.push_back( currentSplineStartIndex );
            lastSplineStartIndex = currentSplineStartIndex;
        }
        else
        {
            segmentStartIndices.push_back( -1 );
        }
    }

    return segmentStartIndices;
}

std::vector<cvf::Vec3d> RigWellPathGeometryTools::interpolateUndefinedNormals( const cvf::Vec3d& planeNormal,
                                                                               const std::vector<cvf::Vec3d>& normals,
                                                                               const std::vector<cvf::Vec3d>& vertices )
{
    std::vector<cvf::Vec3d> interpolated( normals );
    cvf::Vec3d              lastNormalNonInterpolated( 0, 0, 0 );
    cvf::Vec3d              lastNormalAny( 0, 0, 0 );
    double                  distanceFromLast = 0.0;

    for ( size_t i = 0; i < normals.size(); ++i )
    {
        cvf::Vec3d currentNormal       = normals[i];
        bool       currentInterpolated = false;
        if ( i > 0 )
        {
            distanceFromLast += ( vertices[i] - vertices[i - 1] ).length();
        }

        if ( currentNormal.length() == 0.0 ) // Undefined. Need to estimate from neighbors.
        {
            currentInterpolated = true;
            currentNormal       = planeNormal; // By default use the plane normal

            cvf::Vec3d nextNormal( 0, 0, 0 );
            double     distanceToNext = 0.0;
            for ( size_t j = i + 1; j < normals.size() && nextNormal.length() == 0.0; ++j )
            {
                nextNormal = normals[j];
                distanceToNext += ( vertices[j] - vertices[j - 1] ).length();
            }

            if ( lastNormalNonInterpolated.length() > 0.0 && nextNormal.length() > 0.0 )
            {
                // Both last and next are acceptable, interpolate!
                currentNormal =
                    ( distanceToNext * lastNormalNonInterpolated + distanceFromLast * nextNormal ).getNormalized();
            }
            else if ( lastNormalNonInterpolated.length() > 0.0 )
            {
                currentNormal = lastNormalNonInterpolated;
            }
            else if ( nextNormal.length() > 0.0 )
            {
                currentNormal = nextNormal;
            }
        }
        if ( i > 0 && currentNormal * lastNormalAny < -std::numeric_limits<double>::epsilon() )
        {
            currentNormal *= -1.0;
        }
        if ( !currentInterpolated )
        {
            lastNormalNonInterpolated = currentNormal;
            distanceFromLast          = 0.0; // Reset distance
        }
        lastNormalAny   = currentNormal;
        interpolated[i] = currentNormal;
    }
    return interpolated;
}

cvf::Vec3d RigWellPathGeometryTools::estimateDominantDirectionInXYPlane( const std::vector<cvf::Vec3d>& vertices )
{
    cvf::Vec3d directionSum( 0, 0, 0 );
    for ( size_t i = 1; i < vertices.size(); ++i )
    {
        cvf::Vec3d vec = vertices[i] - vertices[i - 1];
        vec.z()        = 0.0;
        if ( directionSum.length() > 0.0 && ( directionSum * vec ) < 0.0 )
        {
            vec *= -1;
        }
        directionSum += vec;
    }

    if ( directionSum.length() < 1.0e-8 )
    {
        directionSum = cvf::Vec3d( 0, -1, 0 );
    }

    return directionSum.getNormalized();
}

//--------------------------------------------------------------------------------------------------
/// Golden-section minimization: https://en.wikipedia.org/wiki/Golden-section_search
//--------------------------------------------------------------------------------------------------
double RigWellPathGeometryTools::solveForX( const QwtSpline& spline, double minX, double maxX, double y )
{
    const double phi = ( 1.0 + std::sqrt( 5.0 ) ) / 2.0;
    const double tol = 1.0e-8;

    double a = minX, b = maxX;
    double c = b - ( b - a ) / phi;
    double d = a + ( b - a ) / phi;

    double fc = spline.value( c ) - y;
    double fd = spline.value( d ) - y;

    for ( int n = 0; n < 100; ++n )
    {
        if ( std::fabs( c - d ) < tol )
        {
            break;
        }

        if ( std::fabs( fc ) < std::fabs( fd ) )
        {
            b  = d;
            d  = c;
            fd = fc;
            c  = b - ( b - a ) / phi;
            fc = spline.value( c ) - y;
        }
        else
        {
            a  = c;
            c  = d;
            fc = fd;
            d  = a + ( b - a ) / phi;
            fd = spline.value( d ) - y;
        }
    }
    return ( a + b ) / 2.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtSpline RigWellPathGeometryTools::createSpline( const std::vector<double>& originalMdValues,
                                                  const std::vector<double>& originalTvdValues )
{
    QPolygonF polygon;
    for ( size_t i = 0; i < originalMdValues.size(); ++i )
    {
        polygon << QPointF( originalMdValues[i], originalTvdValues[i] );
    }
    QwtSplineCurveFitter curveFitter;
    QPolygonF            splinePoints = curveFitter.fitCurve( polygon );

    // Extend spline from 0.0 to a large value for MD
    // This is to force a specific and known extrapolation.
    // Otherwise we get an undefined and unknown extrapolation.
    {
        double x1 = splinePoints[0].x();
        double x2 = splinePoints[1].x();
        double y1 = splinePoints[0].y();
        double y2 = splinePoints[1].y();
        double M  = ( y2 - y1 ) / ( x2 - x1 );

        QPointF startPoint( 0.0f, M * ( 0.0f - x1 ) + y1 );
        splinePoints.push_front( startPoint );
    }
    {
        int    N    = splinePoints.size() - 1;
        double x1   = splinePoints[N - 1].x();
        double x2   = splinePoints[N].x();
        double y1   = splinePoints[N - 1].y();
        double y2   = splinePoints[N].y();
        double M    = ( y2 - y1 ) / ( x2 - x1 );
        double endX = 2.0 * splinePoints[N].x();

        QPointF endPoint( endX, M * ( endX - x1 ) + y1 );
        splinePoints.push_back( endPoint );
    }

    QwtSpline spline;
    spline.setPoints( splinePoints );

    return spline;
}

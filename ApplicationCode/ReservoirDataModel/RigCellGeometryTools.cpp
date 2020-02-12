/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RigCellGeometryTools.h"
#include "cvfGeometryTools.h"
#include "cvfStructGrid.h"

#include "cafHexGridIntersectionTools/cafHexGridIntersectionTools.h"
#include "cvfBoundingBox.h"
#include "cvfMatrix3.h"

#include "clipper/clipper.hpp"

#include "cvfMath.h"

#include <algorithm>
#include <array>
#include <vector>

//--------------------------------------------------------------------------------------------------
/// Efficient Computation of Volume of Hexahedral Cells
/// Jeffrey Grandy, Lawrence Livermore National Laboratory
/// https://www.osti.gov/servlets/purl/632793/
///
/// Note that in the paper the following vertex numbering is used
///     6---------7
///    /|        /|     |k
///   / |       / |     | /j
///  4---------5  |     |/
///  |  2------|--3     *---i
///  | /       | /
///  |/        |/
///  0---------1
///
/// While in ResInsight, this is the numbering. Thus we need to swap 2<->3, 6<->7 in the equations.
/// Note the negative k! This causes an additional set of 0<->4, 1<->5, etc. index swaps.
///     7---------6
///    /|        /|     |-k
///   / |       / |     | /j
///  4---------5  |     |/
///  |  3------|--2     *---i
///  | /       | /
///  |/        |/
///  0---------1
//--------------------------------------------------------------------------------------------------
double RigCellGeometryTools::calculateCellVolume( const std::array<cvf::Vec3d, 8>& x )
{
    // 6 * 3 flops = 18 flops

    // Perform index swap when retrieving corners but keep indices in variable names matching paper.
    cvf::Vec3d x3mx0 = x[6] - x[4]; // Swap 3->2, then negate z by 2->6 and 0->4
    cvf::Vec3d x5mx0 = x[1] - x[4]; // Negate z by Swap 5->1 and 0->4
    cvf::Vec3d x6mx0 = x[3] - x[4]; // Swap 6->7, then negate z by 7->3 and 0->4
    cvf::Vec3d x7mx1 = x[2] - x[5]; // Swap 7->6, then negate z by 6->2 and 1->5
    cvf::Vec3d x7mx2 = x[2] - x[7]; // Swap 7->6, 2->3, then negate z by 6->2 and 3->7
    cvf::Vec3d x7mx4 = x[2] - x[0]; // Swap 7->6 then negate z by 6->2 and 4->0

    // 3 flops for summation + 5 for dot product + 9 flops for cross product = 17 flops
    double det1 = ( x7mx1 + x6mx0 ) * ( x7mx2 ^ x3mx0 );
    // 3 flops for summation + 5 for dot product + 9 flops for cross product = 17 flops
    double det2 = x6mx0 * ( ( x7mx2 + x5mx0 ) ^ x7mx4 );
    // 3 flops for summation + 5 for dot product + 9 flops for cross product = 17 flops
    double det3 = x7mx1 * ( x5mx0 ^ ( x7mx4 + x3mx0 ) );

    // 2 flops for summation + 1 for division = 3 flops
    double volume = ( det1 + det2 + det3 ) / 12.0;

    // In order for this to work in any rotation of the cell, we need the absolute value. 1 flop.
    return std::abs( volume ); // Altogether 18 + 3*17 + 3 + 1 flops = 73 flops.
}

//--------------------------------------------------------------------------------------------------
/// A reasonable approximation to the overlap volume
//--------------------------------------------------------------------------------------------------
bool RigCellGeometryTools::estimateHexOverlapWithBoundingBox( const std::array<cvf::Vec3d, 8>& hexCorners,
                                                              const cvf::BoundingBox&          boundingBox,
                                                              std::array<cvf::Vec3d, 8>*       overlapElement,
                                                              cvf::BoundingBox*                overlapBoundingBox )
{
    CVF_ASSERT( overlapElement && overlapBoundingBox );
    *overlapBoundingBox = cvf::BoundingBox();

    std::array<cvf::Vec3d, 8> overlapCorners = hexCorners;

    std::vector<cvf::Vec3d> uniqueTopPoints = {hexCorners[0], hexCorners[1], hexCorners[2], hexCorners[3]};
    uniqueTopPoints.erase( std::unique( uniqueTopPoints.begin(), uniqueTopPoints.end() ), uniqueTopPoints.end() );
    if ( uniqueTopPoints.size() < 3 ) return false;

    cvf::Plane topPlane;
    if ( !topPlane.setFromPoints( uniqueTopPoints[0], uniqueTopPoints[1], uniqueTopPoints[2] ) ) return false;

    std::vector<cvf::Vec3d> uniqueBottomPoints = {hexCorners[4], hexCorners[5], hexCorners[6], hexCorners[7]};
    uniqueBottomPoints.erase( std::unique( uniqueBottomPoints.begin(), uniqueBottomPoints.end() ),
                              uniqueBottomPoints.end() );
    if ( uniqueBottomPoints.size() < 3 ) return false;

    cvf::Plane bottomPlane;
    if ( !bottomPlane.setFromPoints( uniqueBottomPoints[0], uniqueBottomPoints[1], uniqueBottomPoints[2] ) )
        return false;

    for ( size_t i = 0; i < 4; ++i )
    {
        cvf::Vec3d& corner    = overlapCorners[i];
        corner.x()            = cvf::Math::clamp( corner.x(), boundingBox.min().x(), boundingBox.max().x() );
        corner.y()            = cvf::Math::clamp( corner.y(), boundingBox.min().y(), boundingBox.max().y() );
        corner.z()            = cvf::Math::clamp( corner.z(), boundingBox.min().z(), boundingBox.max().z() );
        cvf::Vec3d maxZCorner = corner;
        maxZCorner.z()        = boundingBox.max().z();
        cvf::Vec3d minZCorner = corner;
        minZCorner.z()        = boundingBox.min().z();
        if ( topPlane.intersect( minZCorner, maxZCorner, &corner ) ) overlapBoundingBox->add( corner );
    }
    for ( size_t i = 4; i < 8; ++i )
    {
        cvf::Vec3d& corner    = overlapCorners[i];
        corner.x()            = cvf::Math::clamp( corner.x(), boundingBox.min().x(), boundingBox.max().x() );
        corner.y()            = cvf::Math::clamp( corner.y(), boundingBox.min().y(), boundingBox.max().y() );
        corner.z()            = cvf::Math::clamp( corner.z(), boundingBox.min().z(), boundingBox.max().z() );
        cvf::Vec3d maxZCorner = corner;
        maxZCorner.z()        = boundingBox.max().z();
        cvf::Vec3d minZCorner = corner;
        minZCorner.z()        = boundingBox.min().z();
        if ( bottomPlane.intersect( minZCorner, maxZCorner, &corner ) ) overlapBoundingBox->add( corner );
    }

    *overlapElement = overlapCorners;
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCellGeometryTools::createPolygonFromLineSegments( std::list<std::pair<cvf::Vec3d, cvf::Vec3d>>& intersectionLineSegments,
                                                          std::vector<std::vector<cvf::Vec3d>>& polygons,
                                                          double                                tolerance )
{
    bool startNewPolygon = true;
    while ( !intersectionLineSegments.empty() )
    {
        if ( startNewPolygon )
        {
            std::vector<cvf::Vec3d> polygon;
            // Add first line segments to polygon and remove from list
            std::pair<cvf::Vec3d, cvf::Vec3d> linesegment = intersectionLineSegments.front();
            polygon.push_back( linesegment.first );
            polygon.push_back( linesegment.second );
            intersectionLineSegments.remove( linesegment );
            polygons.push_back( polygon );
            startNewPolygon = false;
        }

        std::vector<cvf::Vec3d>& polygon = polygons.back();

        // Search remaining list for next point...

        bool isFound = false;

        for ( std::list<std::pair<cvf::Vec3d, cvf::Vec3d>>::iterator lIt = intersectionLineSegments.begin();
              lIt != intersectionLineSegments.end();
              lIt++ )
        {
            cvf::Vec3d lineSegmentStart = lIt->first;
            cvf::Vec3d lineSegmentEnd   = lIt->second;
            cvf::Vec3d polygonEnd       = polygon.back();

            double lineSegmentLength = ( lineSegmentStart - lineSegmentEnd ).lengthSquared();
            if ( lineSegmentLength < tolerance * tolerance )
            {
                intersectionLineSegments.erase( lIt );
                isFound = true;
                break;
            }

            double lineSegmentStartDiff = ( lineSegmentStart - polygonEnd ).lengthSquared();
            if ( lineSegmentStartDiff < tolerance * tolerance )
            {
                polygon.push_back( lIt->second );
                intersectionLineSegments.erase( lIt );
                isFound = true;
                break;
            }

            double lineSegmentEndDiff = ( lineSegmentEnd - polygonEnd ).lengthSquared();
            if ( lineSegmentEndDiff < tolerance * tolerance )
            {
                polygon.push_back( lIt->first );
                intersectionLineSegments.erase( lIt );
                isFound = true;
                break;
            }
        }

        if ( isFound )
        {
            continue;
        }
        else
        {
            startNewPolygon = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Ramer-Douglas-Peucker simplification algorithm
///
/// https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
//--------------------------------------------------------------------------------------------------
void RigCellGeometryTools::simplifyPolygon( std::vector<cvf::Vec3d>* vertices, double epsilon )
{
    CVF_ASSERT( vertices );
    if ( vertices->size() < 3 ) return;

    std::pair<size_t, double> maxDistPoint( 0u, 0.0 );

    for ( size_t i = 1; i < vertices->size() - 1; ++i )
    {
        cvf::Vec3d v = vertices->at( i );
        double     u;
        cvf::Vec3d v_proj   = cvf::GeometryTools::projectPointOnLine( vertices->front(), vertices->back(), v, &u );
        double     distance = ( v_proj - v ).length();
        if ( distance > maxDistPoint.second )
        {
            maxDistPoint = std::make_pair( i, distance );
        }
    }

    if ( maxDistPoint.second > epsilon )
    {
        std::vector<cvf::Vec3d> newVertices1( vertices->begin(), vertices->begin() + maxDistPoint.first + 1 );
        std::vector<cvf::Vec3d> newVertices2( vertices->begin() + maxDistPoint.first, vertices->end() );

        // Recurse
        simplifyPolygon( &newVertices1, epsilon );
        simplifyPolygon( &newVertices2, epsilon );

        std::vector<cvf::Vec3d> newVertices( newVertices1.begin(), newVertices1.end() - 1 );
        newVertices.insert( newVertices.end(), newVertices2.begin(), newVertices2.end() );
        *vertices = newVertices;
    }
    else
    {
        std::vector<cvf::Vec3d> newVertices = {vertices->front(), vertices->back()};
        *vertices                           = newVertices;
    }
}

//==================================================================================================
///
//==================================================================================================
void RigCellGeometryTools::findCellLocalXYZ( const std::array<cvf::Vec3d, 8>& hexCorners,
                                             cvf::Vec3d&                      localXdirection,
                                             cvf::Vec3d&                      localYdirection,
                                             cvf::Vec3d&                      localZdirection )
{
    cvf::ubyte                         faceVertexIndices[4];
    cvf::StructGridInterface::FaceEnum face;

    face = cvf::StructGridInterface::NEG_I;
    cvf::StructGridInterface::cellFaceVertexIndices( face, faceVertexIndices );
    cvf::Vec3d faceCenterNegI = cvf::GeometryTools::computeFaceCenter( hexCorners[faceVertexIndices[0]],
                                                                       hexCorners[faceVertexIndices[1]],
                                                                       hexCorners[faceVertexIndices[2]],
                                                                       hexCorners[faceVertexIndices[3]] );
    // TODO: Should we use face centroids instead of face centers?

    face = cvf::StructGridInterface::POS_I;
    cvf::StructGridInterface::cellFaceVertexIndices( face, faceVertexIndices );
    cvf::Vec3d faceCenterPosI = cvf::GeometryTools::computeFaceCenter( hexCorners[faceVertexIndices[0]],
                                                                       hexCorners[faceVertexIndices[1]],
                                                                       hexCorners[faceVertexIndices[2]],
                                                                       hexCorners[faceVertexIndices[3]] );

    face = cvf::StructGridInterface::NEG_J;
    cvf::StructGridInterface::cellFaceVertexIndices( face, faceVertexIndices );
    cvf::Vec3d faceCenterNegJ = cvf::GeometryTools::computeFaceCenter( hexCorners[faceVertexIndices[0]],
                                                                       hexCorners[faceVertexIndices[1]],
                                                                       hexCorners[faceVertexIndices[2]],
                                                                       hexCorners[faceVertexIndices[3]] );

    face = cvf::StructGridInterface::POS_J;
    cvf::StructGridInterface::cellFaceVertexIndices( face, faceVertexIndices );
    cvf::Vec3d faceCenterPosJ = cvf::GeometryTools::computeFaceCenter( hexCorners[faceVertexIndices[0]],
                                                                       hexCorners[faceVertexIndices[1]],
                                                                       hexCorners[faceVertexIndices[2]],
                                                                       hexCorners[faceVertexIndices[3]] );

    cvf::Vec3d faceCenterCenterVectorI = faceCenterPosI - faceCenterNegI;
    cvf::Vec3d faceCenterCenterVectorJ = faceCenterPosJ - faceCenterNegJ;

    localZdirection.cross( faceCenterCenterVectorI, faceCenterCenterVectorJ );
    localZdirection.normalize();

    cvf::Vec3d crossPoductJZ;
    crossPoductJZ.cross( faceCenterCenterVectorJ, localZdirection );
    localXdirection = faceCenterCenterVectorI + crossPoductJZ;
    localXdirection.normalize();

    cvf::Vec3d crossPoductIZ;
    crossPoductIZ.cross( faceCenterCenterVectorI, localZdirection );
    localYdirection = faceCenterCenterVectorJ - crossPoductIZ;
    localYdirection.normalize();

    // TODO: Check if we end up with 0-vectors, and handle this case...
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCellGeometryTools::polygonLengthInLocalXdirWeightedByArea( const std::vector<cvf::Vec3d>& polygonToCalcLengthOf )
{
    // Find bounding box
    cvf::BoundingBox polygonBBox;
    for ( cvf::Vec3d nodeCoord : polygonToCalcLengthOf )
        polygonBBox.add( nodeCoord );
    cvf::Vec3d bboxCorners[8];
    polygonBBox.cornerVertices( bboxCorners );

    // Split bounding box in multiple polygons (2D)
    int    resolutionOfLengthCalc = 20;
    double widthOfPolygon         = polygonBBox.extent().y() / resolutionOfLengthCalc;

    std::vector<double> areasOfPolygonContributions;
    std::vector<double> lengthOfPolygonContributions;

    cvf::Vec3d directionOfLength( 1, 0, 0 );

    for ( int i = 0; i < resolutionOfLengthCalc; i++ )
    {
        cvf::Vec3d pointOnLine1( bboxCorners[0].x(), bboxCorners[0].y() + i * widthOfPolygon, 0 );
        cvf::Vec3d pointOnLine2( bboxCorners[0].x(), bboxCorners[0].y() + ( i + 1 ) * widthOfPolygon, 0 );

        std::pair<cvf::Vec3d, cvf::Vec3d> line1 = getLineThroughBoundingBox( directionOfLength, polygonBBox, pointOnLine1 );
        std::pair<cvf::Vec3d, cvf::Vec3d> line2 = getLineThroughBoundingBox( directionOfLength, polygonBBox, pointOnLine2 );

        std::vector<cvf::Vec3d> polygon;
        polygon.push_back( line1.first );
        polygon.push_back( line1.second );
        polygon.push_back( line2.second );
        polygon.push_back( line2.first );

        // Use clipper to find overlap between bbpolygon and fracture
        std::vector<std::vector<cvf::Vec3d>> clippedPolygons = intersectPolygons( polygonToCalcLengthOf, polygon );

        double     area       = 0;
        double     length     = 0;
        cvf::Vec3d areaVector = cvf::Vec3d::ZERO;

        // Calculate length (max-min) and area
        for ( std::vector<cvf::Vec3d> clippedPolygon : clippedPolygons )
        {
            areaVector = cvf::GeometryTools::polygonAreaNormal3D( clippedPolygon );
            area += areaVector.length();
            length += ( getLengthOfPolygonAlongLine( line1, clippedPolygon ) +
                        getLengthOfPolygonAlongLine( line2, clippedPolygon ) ) /
                      2;
        }
        areasOfPolygonContributions.push_back( area );
        lengthOfPolygonContributions.push_back( length );
    }

    // Calculate area-weighted length average.
    double totalArea        = 0.0;
    double totalAreaXlength = 0.0;

    for ( size_t i = 0; i < areasOfPolygonContributions.size(); i++ )
    {
        totalArea += areasOfPolygonContributions[i];
        totalAreaXlength += ( areasOfPolygonContributions[i] * lengthOfPolygonContributions[i] );
    }

    double areaWeightedLength = totalAreaXlength / totalArea;
    return areaWeightedLength;
}

double clipperConversionFactor = 10000; // For transform to clipper int

ClipperLib::IntPoint toClipperPoint( const cvf::Vec3d& cvfPoint )
{
    int xInt = cvfPoint.x() * clipperConversionFactor;
    int yInt = cvfPoint.y() * clipperConversionFactor;
    int zInt = cvfPoint.z() * clipperConversionFactor;
    return ClipperLib::IntPoint( xInt, yInt, zInt );
}

cvf::Vec3d fromClipperPoint( const ClipperLib::IntPoint& clipPoint )
{
    double zDValue;

    if ( clipPoint.Z == std::numeric_limits<int>::max() )
    {
        zDValue = HUGE_VAL;
    }
    else
    {
        zDValue = clipPoint.Z;
    }

    return cvf::Vec3d( clipPoint.X, clipPoint.Y, zDValue ) / clipperConversionFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::Vec3d>> RigCellGeometryTools::intersectPolygons( const std::vector<cvf::Vec3d>& polygon1,
                                                                              const std::vector<cvf::Vec3d>& polygon2 )
{
    std::vector<std::vector<cvf::Vec3d>> clippedPolygons;

    // Convert to int for clipper library and store as clipper "path"
    ClipperLib::Path polygon1path;
    for ( const cvf::Vec3d& v : polygon1 )
    {
        polygon1path.push_back( toClipperPoint( v ) );
    }

    ClipperLib::Path polygon2path;
    for ( const cvf::Vec3d& v : polygon2 )
    {
        polygon2path.push_back( toClipperPoint( v ) );
    }

    ClipperLib::Clipper clpr;
    clpr.AddPath( polygon1path, ClipperLib::ptSubject, true );
    clpr.AddPath( polygon2path, ClipperLib::ptClip, true );

    ClipperLib::Paths solution;
    clpr.Execute( ClipperLib::ctIntersection, solution, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd );

    // Convert back to std::vector<std::vector<cvf::Vec3d> >
    for ( ClipperLib::Path pathInSol : solution )
    {
        std::vector<cvf::Vec3d> clippedPolygon;
        for ( ClipperLib::IntPoint IntPosition : pathInSol )
        {
            clippedPolygon.push_back( fromClipperPoint( IntPosition ) );
        }
        clippedPolygons.push_back( clippedPolygon );
    }

    return clippedPolygons;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::Vec3d>>
    RigCellGeometryTools::subtractPolygons( const std::vector<cvf::Vec3d>&              sourcePolygon,
                                            const std::vector<std::vector<cvf::Vec3d>>& polygonsToSubtract )
{
    ClipperLib::Clipper clpr;

    {
        // Convert to int for clipper library and store as clipper "path"
        ClipperLib::Path polygon1path;
        for ( const auto& v : sourcePolygon )
        {
            polygon1path.push_back( toClipperPoint( v ) );
        }
        clpr.AddPath( polygon1path, ClipperLib::ptSubject, true );
    }

    for ( const auto& path : polygonsToSubtract )
    {
        ClipperLib::Path polygon2path;
        for ( const auto& v : path )
        {
            polygon2path.push_back( toClipperPoint( v ) );
        }

        clpr.AddPath( polygon2path, ClipperLib::ptClip, true );
    }

    ClipperLib::Paths solution;
    clpr.Execute( ClipperLib::ctDifference, solution, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd );

    std::vector<std::vector<cvf::Vec3d>> clippedPolygons;

    // Convert back to std::vector<std::vector<cvf::Vec3d> >
    for ( ClipperLib::Path pathInSol : solution )
    {
        std::vector<cvf::Vec3d> clippedPolygon;
        for ( ClipperLib::IntPoint IntPosition : pathInSol )
        {
            clippedPolygon.push_back( fromClipperPoint( IntPosition ) );
        }

        clippedPolygons.push_back( clippedPolygon );
    }

    return clippedPolygons;
}

//--------------------------------------------------------------------------------------------------
/// Note for cppcheck : First four parameter cannot be const to match the signature of the receiver
//--------------------------------------------------------------------------------------------------
void fillInterpolatedSubjectZ( ClipperLib::IntPoint& e1bot,
                               ClipperLib::IntPoint& e1top,
                               ClipperLib::IntPoint& e2bot,
                               ClipperLib::IntPoint& e2top,
                               ClipperLib::IntPoint& pt )
{
    ClipperLib::IntPoint ePLbot;
    ClipperLib::IntPoint ePLtop;

    if ( e1top.Z == std::numeric_limits<int>::max() )
    {
        ePLtop = e2top;
        ePLbot = e2bot;
    }
    else
    {
        ePLtop = e1top;
        ePLbot = e1bot;
    }

    double ePLXRange = ( ePLtop.X - ePLbot.X );
    double ePLYRange = ( ePLtop.Y - ePLbot.Y );

    double ePLLength = sqrt( ePLXRange * ePLXRange + ePLYRange * ePLYRange );

    if ( ePLLength <= 1 )
    {
        pt.Z = ePLbot.Z;
        return;
    }

    double ePLBotPtXRange = pt.X - ePLbot.X;
    double ePLBotPtYRange = pt.Y - ePLbot.Y;

    double ePLBotPtLength = sqrt( ePLBotPtXRange * ePLBotPtXRange + ePLBotPtYRange * ePLBotPtYRange );

    double fraction = ePLBotPtLength / ePLLength;

    pt.Z = std::nearbyint( ePLbot.Z + fraction * ( ePLtop.Z - ePLbot.Z ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void fillUndefinedZ( ClipperLib::IntPoint& e1bot,
                     ClipperLib::IntPoint& e1top,
                     ClipperLib::IntPoint& e2bot,
                     ClipperLib::IntPoint& e2top,
                     ClipperLib::IntPoint& pt )
{
    pt.Z = std::numeric_limits<int>::max();
}

//--------------------------------------------------------------------------------------------------
/// Assumes x.y plane polygon. Polyline might have a Z, and the returned Z is the polyline Z, interpolated if it is
/// clipped.
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::Vec3d>> RigCellGeometryTools::clipPolylineByPolygon( const std::vector<cvf::Vec3d>& polyLine,
                                                                                  const std::vector<cvf::Vec3d>& polygon,
                                                                                  ZInterpolationType interpolType )
{
    std::vector<std::vector<cvf::Vec3d>> clippedPolyline;

    // Adjusting polygon to avoid clipper issue with interpolating z-values when lines crosses though polygon vertecies
    std::vector<cvf::Vec3d> adjustedPolygon = ajustPolygonToAvoidIntersectionsAtVertex( polyLine, polygon );

    // Convert to int for clipper library and store as clipper "path"
    ClipperLib::Path polyLinePath;
    for ( const cvf::Vec3d& v : polyLine )
    {
        polyLinePath.push_back( toClipperPoint( v ) );
    }

    ClipperLib::Path polygonPath;
    for ( const cvf::Vec3d& v : adjustedPolygon )
    {
        ClipperLib::IntPoint intp = toClipperPoint( v );
        intp.Z                    = std::numeric_limits<int>::max();
        polygonPath.push_back( intp );
    }

    ClipperLib::Clipper clpr;
    clpr.AddPath( polyLinePath, ClipperLib::ptSubject, false );
    clpr.AddPath( polygonPath, ClipperLib::ptClip, true );

    if ( interpolType == INTERPOLATE_LINE_Z )
    {
        clpr.ZFillFunction( &fillInterpolatedSubjectZ );
    }
    else if ( interpolType == USE_HUGEVAL )
    {
        clpr.ZFillFunction( &fillUndefinedZ );
    }

    ClipperLib::PolyTree solution;
    clpr.Execute( ClipperLib::ctIntersection, solution, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd );

    // We only expect open paths from this method (unless the polyline is self intersecting, a condition we do not handle)
    ClipperLib::Paths solutionPaths;
    ClipperLib::OpenPathsFromPolyTree( solution, solutionPaths );

    // Convert back to std::vector<std::vector<cvf::Vec3d> >
    for ( ClipperLib::Path pathInSol : solutionPaths )
    {
        std::vector<cvf::Vec3d> clippedPolygon;
        for ( ClipperLib::IntPoint IntPosition : pathInSol )
        {
            clippedPolygon.push_back( fromClipperPoint( IntPosition ) );
        }
        clippedPolyline.push_back( clippedPolygon );
    }

    return clippedPolyline;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::Vec3d, cvf::Vec3d> RigCellGeometryTools::getLineThroughBoundingBox( const cvf::Vec3d& lineDirection,
                                                                                   const cvf::BoundingBox& polygonBBox,
                                                                                   const cvf::Vec3d&       pointOnLine )
{
    cvf::Vec3d bboxCorners[8];
    polygonBBox.cornerVertices( bboxCorners );

    cvf::Vec3d startPoint = pointOnLine;
    cvf::Vec3d endPoint   = pointOnLine;
    cvf::Vec3d lineDir    = lineDirection;

    // To avoid doing many iterations in loops below linedirection should be quite large.
    lineDir.normalize();
    lineDir = lineDir * polygonBBox.extent().length() / 5;

    // Extend line in positive direction
    while ( polygonBBox.contains( startPoint ) )
    {
        startPoint = startPoint + lineDir;
    }
    // Extend line in negative direction
    while ( polygonBBox.contains( endPoint ) )
    {
        endPoint = endPoint - lineDir;
    }

    std::pair<cvf::Vec3d, cvf::Vec3d> line;
    line = {startPoint, endPoint};
    return line;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCellGeometryTools::getLengthOfPolygonAlongLine( const std::pair<cvf::Vec3d, cvf::Vec3d>& line,
                                                          const std::vector<cvf::Vec3d>&           polygon )
{
    cvf::BoundingBox lineBoundingBox;

    for ( cvf::Vec3d polygonPoint : polygon )
    {
        cvf::Vec3d pointOnLine = cvf::GeometryTools::projectPointOnLine( line.first, line.second, polygonPoint, nullptr );
        lineBoundingBox.add( pointOnLine );
    }

    double length = lineBoundingBox.extent().length();

    return length;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigCellGeometryTools::unionOfPolygons( const std::vector<std::vector<cvf::Vec3d>>& polygons )
{
    // Convert to int for clipper library and store as clipper "path"
    std::vector<ClipperLib::Path> polygonPaths;
    for ( const std::vector<cvf::Vec3d>& polygon : polygons )
    {
        polygonPaths.emplace_back();
        auto& p = polygonPaths.back();
        for ( const cvf::Vec3d& pp : polygon )
        {
            p.push_back( toClipperPoint( pp ) );
        }
    }

    ClipperLib::Clipper clpr;
    clpr.AddPaths( polygonPaths, ClipperLib::ptSubject, true );

    ClipperLib::Paths solution;
    clpr.Execute( ClipperLib::ctUnion, solution, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd );

    // Convert back to std::vector<std::vector<cvf::Vec3d> >
    std::vector<cvf::Vec3d> unionPolygon;
    for ( ClipperLib::Path pathInSol : solution )
    {
        for ( ClipperLib::IntPoint IntPosition : pathInSol )
        {
            unionPolygon.push_back( fromClipperPoint( IntPosition ) );
        }
    }

    return unionPolygon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d>
    RigCellGeometryTools::ajustPolygonToAvoidIntersectionsAtVertex( const std::vector<cvf::Vec3d>& polyLine,
                                                                    const std::vector<cvf::Vec3d>& polygon )
{
    std::vector<cvf::Vec3d> adjustedPolygon;

    double treshold = ( 1.0 / 10000.0 ) * 5; // 5 times polygonScaleFactor for converting to int for clipper

    for ( cvf::Vec3d polygonPoint : polygon )
    {
        for ( size_t i = 0; i < polyLine.size() - 1; i++ )
        {
            cvf::Vec3d linePoint1( polyLine[i].x(), polyLine[i].y(), 0.0 );
            cvf::Vec3d linePoint2( polyLine[i + 1].x(), polyLine[i + 1].y(), 0.0 );

            double pointDistanceFromLine = cvf::GeometryTools::linePointSquareDist( linePoint1, linePoint2, polygonPoint );
            if ( pointDistanceFromLine < treshold )
            {
                // calculate new polygonPoint
                cvf::Vec3d directionOfLineSegment = linePoint2 - linePoint1;
                // finding normal to the direction of the line segment in the XY plane (z=0)
                cvf::Vec3d normalToLine( -directionOfLineSegment.y(), directionOfLineSegment.x(), 0.0 );
                normalToLine.normalize();
                polygonPoint = polygonPoint + normalToLine * 0.005;
            }
        }
        adjustedPolygon.push_back( polygonPoint );
    }

    return adjustedPolygon;
}

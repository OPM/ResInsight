/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RigContourPolygonsTools.h"

#include "RigCellGeometryTools.h"
#include "RigContourMapGrid.h"

#include "cafContourLines.h"

#include "cvfGeometryTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigContourPolygonsTools::ContourPolygons
    RigContourPolygonsTools::createContourPolygonsFromLineSegments( caf::ContourLines::ListOfLineSegments& unorderedLineSegments,
                                                                    double                                 contourValue,
                                                                    double                                 areaThreshold )
{
    ContourPolygons contourPolygons;

    std::vector<std::vector<cvf::Vec3d>> polygons;
    RigCellGeometryTools::createPolygonFromLineSegments( unorderedLineSegments, polygons, 1.0e-8 );
    for ( size_t j = 0; j < polygons.size(); ++j )
    {
        double         signedArea = cvf::GeometryTools::signedAreaPlanarPolygon( cvf::Vec3d::Z_AXIS, polygons[j] );
        ContourPolygon contourPolygon;
        contourPolygon.value = contourValue;
        if ( signedArea < 0.0 )
        {
            contourPolygon.vertices.insert( contourPolygon.vertices.end(), polygons[j].rbegin(), polygons[j].rend() );
        }
        else
        {
            contourPolygon.vertices = polygons[j];
        }

        contourPolygon.area = cvf::GeometryTools::signedAreaPlanarPolygon( cvf::Vec3d::Z_AXIS, contourPolygon.vertices );
        if ( contourPolygon.area > areaThreshold )
        {
            for ( const cvf::Vec3d& vertex : contourPolygon.vertices )
            {
                contourPolygon.bbox.add( vertex );
            }
            contourPolygons.push_back( contourPolygon );
        }
    }
    return contourPolygons;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigContourPolygonsTools::smoothContourPolygons( ContourPolygons& contourPolygons, bool favourExpansion, double sampleSpacing )
{
    for ( RigContourPolygonsTools::ContourPolygon& polygon : contourPolygons )
    {
        for ( size_t n = 0; n < 20; ++n )
        {
            std::vector<cvf::Vec3d> newVertices;
            newVertices.resize( polygon.vertices.size() );
            double maxChange = 0.0;
            for ( size_t j = 0; j < polygon.vertices.size(); ++j )
            {
                cvf::Vec3d vm1 = polygon.vertices.back();
                cvf::Vec3d v   = polygon.vertices[j];
                cvf::Vec3d vp1 = polygon.vertices.front();
                if ( j > 0u )
                {
                    vm1 = polygon.vertices[j - 1];
                }
                if ( j < polygon.vertices.size() - 1 )
                {
                    vp1 = polygon.vertices[j + 1];
                }
                // Only expand.
                cvf::Vec3d modifiedVertex = 0.5 * ( v + 0.5 * ( vm1 + vp1 ) );
                cvf::Vec3d delta          = modifiedVertex - v;
                cvf::Vec3d tangent3d      = vp1 - vm1;
                cvf::Vec2d tangent2d( tangent3d.x(), tangent3d.y() );
                cvf::Vec3d norm3d( tangent2d.getNormalized().perpendicularVector() );
                if ( delta * norm3d > 0 && favourExpansion )
                {
                    // Normal is always inwards facing so a positive dot product means inward movement
                    // Favour expansion rather than contraction by only contracting by a fraction.
                    // The fraction is empirically found to give a decent result.
                    modifiedVertex = v + 0.2 * delta;
                }
                newVertices[j] = modifiedVertex;
                maxChange      = std::max( maxChange, ( modifiedVertex - v ).length() );
            }
            polygon.vertices.swap( newVertices );
            if ( maxChange < sampleSpacing * 1.0e-2 ) break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigContourPolygonsTools::clipContourPolygons( ContourPolygons& contourPolygons, const ContourPolygons& clipBy )
{
    for ( RigContourPolygonsTools::ContourPolygon& polygon : contourPolygons )
    {
        for ( const RigContourPolygonsTools::ContourPolygon& clipPolygon : contourPolygons )
        {
            std::vector<std::vector<cvf::Vec3d>> intersections =
                RigCellGeometryTools::intersectionWithPolygon( polygon.vertices, clipPolygon.vertices );
            if ( !intersections.empty() )
            {
                polygon.vertices = intersections.front();
                polygon.area     = std::abs( cvf::GeometryTools::signedAreaPlanarPolygon( cvf::Vec3d::Z_AXIS, polygon.vertices ) );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourPolygonsTools::sumPolygonArea( const ContourPolygons& contourPolygons )
{
    double sumArea = 0.0;
    for ( const ContourPolygon& polygon : contourPolygons )
    {
        sumArea += polygon.area;
    }
    return sumArea;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigContourPolygonsTools::lineOverlapsWithContourPolygons( const cvf::Vec3d&                               lineCenter,
                                                               const RigContourPolygonsTools::ContourPolygons& contourPolygons,
                                                               double                                          tolerance )
{
    const int64_t jump = 50;
    for ( const RigContourPolygonsTools::ContourPolygon& edgePolygon : contourPolygons )
    {
        std::pair<int64_t, double> closestIndex( 0, std::numeric_limits<double>::infinity() );
        for ( int64_t i = 0; i < (int64_t)edgePolygon.vertices.size(); i += jump )
        {
            const cvf::Vec3d& edgeVertex1 = edgePolygon.vertices[i];
            const cvf::Vec3d& edgeVertex2 = edgePolygon.vertices[( i + 1 ) % edgePolygon.vertices.size()];
            double            dist1       = cvf::GeometryTools::linePointSquareDist( edgeVertex1, edgeVertex2, lineCenter );
            if ( dist1 < tolerance )
            {
                return true;
            }
            if ( dist1 < closestIndex.second )
            {
                closestIndex = std::make_pair( i, dist1 );
            }
        }

        for ( int64_t i = std::max( (int64_t)1, closestIndex.first - jump + 1 );
              i < std::min( (int64_t)edgePolygon.vertices.size(), closestIndex.first + jump );
              ++i )
        {
            const cvf::Vec3d& edgeVertex1 = edgePolygon.vertices[i];
            const cvf::Vec3d& edgeVertex2 = edgePolygon.vertices[( i + 1 ) % edgePolygon.vertices.size()];
            double            dist1       = cvf::GeometryTools::linePointSquareDist( edgeVertex1, edgeVertex2, lineCenter );
            if ( dist1 < tolerance )
            {
                return true;
            }
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RigContourPolygonsTools::generatePickPointPolygon( const cvf::Vec2d& pickPoint, const RigContourMapGrid& contourMapGrid )
{
    std::vector<cvf::Vec3d> points;

    if ( !pickPoint.isUndefined() )
    {
        double sampleSpacing = contourMapGrid.sampleSpacing();
#ifndef NDEBUG
        cvf::Vec2d  cellDiagonal( sampleSpacing * 0.5, sampleSpacing * 0.5 );
        cvf::Vec2ui pickedCell = contourMapGrid.ijFromLocalPos( pickPoint );
        cvf::Vec2d  cellCenter = contourMapGrid.cellCenterPosition( pickedCell.x(), pickedCell.y() );
        cvf::Vec2d  cellCorner = cellCenter - cellDiagonal;
        points.push_back( cvf::Vec3d( cellCorner, 0.0 ) );
        points.push_back( cvf::Vec3d( cellCorner + cvf::Vec2d( sampleSpacing, 0.0 ), 0.0 ) );
        points.push_back( cvf::Vec3d( cellCorner + cvf::Vec2d( sampleSpacing, 0.0 ), 0.0 ) );
        points.push_back( cvf::Vec3d( cellCorner + cvf::Vec2d( sampleSpacing, sampleSpacing ), 0.0 ) );
        points.push_back( cvf::Vec3d( cellCorner + cvf::Vec2d( sampleSpacing, sampleSpacing ), 0.0 ) );
        points.push_back( cvf::Vec3d( cellCorner + cvf::Vec2d( 0.0, sampleSpacing ), 0.0 ) );
        points.push_back( cvf::Vec3d( cellCorner + cvf::Vec2d( 0.0, sampleSpacing ), 0.0 ) );
        points.push_back( cvf::Vec3d( cellCorner, 0.0 ) );
#endif
        points.push_back( cvf::Vec3d( pickPoint - cvf::Vec2d( 0.5 * sampleSpacing, 0.0 ), 0.0 ) );
        points.push_back( cvf::Vec3d( pickPoint + cvf::Vec2d( 0.5 * sampleSpacing, 0.0 ), 0.0 ) );
        points.push_back( cvf::Vec3d( pickPoint - cvf::Vec2d( 0.0, 0.5 * sampleSpacing ), 0.0 ) );
        points.push_back( cvf::Vec3d( pickPoint + cvf::Vec2d( 0.0, 0.5 * sampleSpacing ), 0.0 ) );
    }
    return points;
}

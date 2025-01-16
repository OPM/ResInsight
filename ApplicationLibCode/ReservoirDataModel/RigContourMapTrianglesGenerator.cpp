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

#include "RigContourMapTrianglesGenerator.h"

#include "RiaOpenMPTools.h"

#include "RigCellGeometryTools.h"
#include "RigContourMapGrid.h"
#include "RigContourMapProjection.h"

#include "cvfGeometryUtils.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec4d>
    RigContourMapTrianglesGenerator::generateTrianglesWithVertexValues( const RigContourMapGrid&            contourMapGrid,
                                                                        const RigContourMapProjection&      contourMapProjection,
                                                                        const std::vector<ContourPolygons>& contourPolygons,
                                                                        const std::vector<double>&          contourLevels,
                                                                        const std::vector<double>&          contourLevelCumulativeAreas,
                                                                        bool                                discrete,
                                                                        double                              sampleSpacing )
{
    std::vector<cvf::Vec3d> vertices = contourMapGrid.generateVertices();

    cvf::Vec2ui              patchSize = contourMapGrid.numberOfVerticesIJ();
    cvf::ref<cvf::UIntArray> faceList  = new cvf::UIntArray;
    cvf::GeometryUtils::tesselatePatchAsTriangles( patchSize.x(), patchSize.y(), 0u, true, faceList.p() );

    const double cellArea      = sampleSpacing * sampleSpacing;
    const double areaThreshold = 1.0e-5 * 0.5 * cellArea;

    std::vector<std::vector<std::vector<cvf::Vec3d>>> subtractPolygons;
    if ( !contourPolygons.empty() )
    {
        subtractPolygons.resize( contourPolygons.size() );
        for ( size_t i = 0; i < contourPolygons.size() - 1; ++i )
        {
            for ( size_t j = 0; j < contourPolygons[i + 1].size(); ++j )
            {
                subtractPolygons[i].push_back( contourPolygons[i + 1][j].vertices );
            }
        }
    }

    int numberOfThreads = RiaOpenMPTools::availableThreadCount();

    std::vector<std::vector<std::vector<cvf::Vec4d>>> threadTriangles( numberOfThreads );

    auto aggregatedVertexResults = contourMapProjection.aggregatedVertexResultsFiltered();

#pragma omp parallel
    {
        int myThread = RiaOpenMPTools::currentThreadIndex();
        threadTriangles[myThread].resize( std::max( (size_t)1, contourPolygons.size() ) );

#pragma omp for schedule( dynamic )
        for ( int64_t i = 0; i < (int64_t)faceList->size(); i += 3 )
        {
            std::vector<cvf::Vec3d> triangle( 3 );
            std::vector<cvf::Vec4d> triangleWithValues( 3 );
            bool                    anyValidVertex = false;
            for ( size_t n = 0; n < 3; ++n )
            {
                uint   vn    = ( *faceList )[i + n];
                double value = vn < aggregatedVertexResults.size() ? aggregatedVertexResults[vn] : std::numeric_limits<double>::infinity();
                triangle[n]  = vertices[vn];
                triangleWithValues[n] = cvf::Vec4d( vertices[vn], value );
                if ( value != std::numeric_limits<double>::infinity() )
                {
                    anyValidVertex = true;
                }
            }

            if ( !anyValidVertex )
            {
                continue;
            }

            if ( contourPolygons.empty() )
            {
                threadTriangles[myThread][0].insert( threadTriangles[myThread][0].end(), triangleWithValues.begin(), triangleWithValues.end() );
                continue;
            }

            bool outsideOuterLimit = false;
            for ( size_t c = 0; c < contourPolygons.size() && !outsideOuterLimit; ++c )
            {
                std::vector<std::vector<cvf::Vec3d>> intersectPolygons;
                for ( size_t j = 0; j < contourPolygons[c].size(); ++j )
                {
                    bool containsAtLeastOne = false;
                    for ( size_t t = 0; t < 3; ++t )
                    {
                        if ( contourPolygons[c][j].bbox.contains( triangle[t] ) )
                        {
                            containsAtLeastOne = true;
                        }
                    }
                    if ( containsAtLeastOne )
                    {
                        std::vector<std::vector<cvf::Vec3d>> clippedPolygons =
                            RigCellGeometryTools::intersectionWithPolygon( triangle, contourPolygons[c][j].vertices );
                        intersectPolygons.insert( intersectPolygons.end(), clippedPolygons.begin(), clippedPolygons.end() );
                    }
                }

                if ( intersectPolygons.empty() )
                {
                    outsideOuterLimit = true;
                    continue;
                }

                std::vector<std::vector<cvf::Vec3d>> clippedPolygons;

                if ( !subtractPolygons[c].empty() )
                {
                    for ( const std::vector<cvf::Vec3d>& polygon : intersectPolygons )
                    {
                        std::vector<std::vector<cvf::Vec3d>> fullyClippedPolygons =
                            RigCellGeometryTools::subtractPolygons( polygon, subtractPolygons[c] );
                        clippedPolygons.insert( clippedPolygons.end(), fullyClippedPolygons.begin(), fullyClippedPolygons.end() );
                    }
                }
                else
                {
                    clippedPolygons.swap( intersectPolygons );
                }

                std::vector<cvf::Vec4d> clippedTriangles;
                for ( std::vector<cvf::Vec3d>& clippedPolygon : clippedPolygons )
                {
                    std::vector<std::vector<cvf::Vec3d>> polygonTriangles;
                    if ( clippedPolygon.size() == 3u )
                    {
                        polygonTriangles.push_back( clippedPolygon );
                    }
                    else
                    {
                        cvf::Vec3d baryCenter = cvf::Vec3d::ZERO;
                        for ( size_t v = 0; v < clippedPolygon.size(); ++v )
                        {
                            cvf::Vec3d& clippedVertex = clippedPolygon[v];
                            baryCenter += clippedVertex;
                        }
                        baryCenter /= clippedPolygon.size();
                        for ( size_t v = 0; v < clippedPolygon.size(); ++v )
                        {
                            std::vector<cvf::Vec3d> clippedTriangle;
                            if ( v == clippedPolygon.size() - 1 )
                            {
                                clippedTriangle = { clippedPolygon[v], clippedPolygon[0], baryCenter };
                            }
                            else
                            {
                                clippedTriangle = { clippedPolygon[v], clippedPolygon[v + 1], baryCenter };
                            }
                            polygonTriangles.push_back( clippedTriangle );
                        }
                    }
                    for ( const std::vector<cvf::Vec3d>& polygonTriangle : polygonTriangles )
                    {
                        // Check triangle area
                        double area =
                            0.5 * ( ( polygonTriangle[1] - polygonTriangle[0] ) ^ ( polygonTriangle[2] - polygonTriangle[0] ) ).length();
                        if ( area < areaThreshold ) continue;
                        for ( const cvf::Vec3d& localVertex : polygonTriangle )
                        {
                            double value = std::numeric_limits<double>::infinity();
                            if ( discrete )
                            {
                                value = contourLevels[c] + 0.01 * ( contourLevels.back() - contourLevels.front() ) / contourLevels.size();
                            }
                            else
                            {
                                for ( size_t n = 0; n < 3; ++n )
                                {
                                    if ( ( triangle[n] - localVertex ).length() < sampleSpacing * 0.01 &&
                                         triangleWithValues[n].w() != std::numeric_limits<double>::infinity() )
                                    {
                                        value = triangleWithValues[n].w();
                                        break;
                                    }
                                }
                                if ( value == std::numeric_limits<double>::infinity() )
                                {
                                    value = contourMapProjection.interpolateValue( cvf::Vec2d( localVertex.x(), localVertex.y() ) );
                                    if ( value == std::numeric_limits<double>::infinity() )
                                    {
                                        value = contourLevels[c];
                                    }
                                }
                            }

                            cvf::Vec4d globalVertex( localVertex, value );
                            clippedTriangles.push_back( globalVertex );
                        }
                    }

                    {
                        // Add critical section here due to a weird bug when running in a single thread
                        // Running multi threaded does not require this critical section, as we use a thread local data
                        // structure
#pragma omp critical
                        threadTriangles[myThread][c].insert( threadTriangles[myThread][c].end(),
                                                             clippedTriangles.begin(),
                                                             clippedTriangles.end() );
                    }
                }
            }
        }
    }

    std::vector<std::vector<cvf::Vec4d>> trianglesPerLevel( std::max( (size_t)1, contourPolygons.size() ) );
    for ( size_t c = 0; c < trianglesPerLevel.size(); ++c )
    {
        std::vector<cvf::Vec4d> allTrianglesThisLevel;
        for ( size_t i = 0; i < threadTriangles.size(); ++i )
        {
            allTrianglesThisLevel.insert( allTrianglesThisLevel.end(), threadTriangles[i][c].begin(), threadTriangles[i][c].end() );
        }

        double triangleAreasThisLevel = RigContourMapProjection::sumTriangleAreas( allTrianglesThisLevel );
        if ( c >= contourLevelCumulativeAreas.size() || triangleAreasThisLevel > 1.0e-3 * contourLevelCumulativeAreas[c] )
        {
            trianglesPerLevel[c] = allTrianglesThisLevel;
        }
    }

    std::vector<cvf::Vec4d> finalTriangles;
    for ( size_t i = 0; i < trianglesPerLevel.size(); ++i )
    {
        finalTriangles.insert( finalTriangles.end(), trianglesPerLevel[i].begin(), trianglesPerLevel[i].end() );
    }

    return finalTriangles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<RigContourMapTrianglesGenerator::ContourPolygons>, std::vector<double>>
    RigContourMapTrianglesGenerator::generateContourPolygons( const RigContourMapGrid&       contourMapGrid,
                                                              const RigContourMapProjection& contourMapProjection,
                                                              const std::vector<double>&     initialContourLevels,
                                                              double                         sampleSpacing,
                                                              double                         sampleSpacingFactor,
                                                              bool                           smoothContourLines )
{
    if ( contourMapProjection.minValue() != std::numeric_limits<double>::infinity() &&
         contourMapProjection.maxValue() != -std::numeric_limits<double>::infinity() &&
         std::fabs( contourMapProjection.maxValue() - contourMapProjection.minValue() ) > 1.0e-8 )
    {
        std::vector<double> contourLevels = initialContourLevels;

        int nContourLevels = static_cast<int>( contourLevels.size() );
        if ( nContourLevels > 2 )
        {
            const size_t N = contourLevels.size();
            // Adjust contour levels slightly to avoid weird visual artifacts due to numerical error.
            double fudgeFactor    = 1.0e-3;
            double fudgeAmountMin = fudgeFactor * ( contourLevels[1] - contourLevels[0] );
            double fudgeAmountMax = fudgeFactor * ( contourLevels[N - 1u] - contourLevels[N - 2u] );

            contourLevels.front() += fudgeAmountMin;
            contourLevels.back() -= fudgeAmountMax;

            double simplifyEpsilon = smoothContourLines ? 5.0e-2 * sampleSpacing : 1.0e-3 * sampleSpacing;

            if ( nContourLevels >= 10 )
            {
                simplifyEpsilon *= 2.0;
            }
            if ( contourMapGrid.numberOfCells() > 100000 )
            {
                simplifyEpsilon *= 2.0;
            }
            else if ( contourMapGrid.numberOfCells() > 1000000 )
            {
                simplifyEpsilon *= 4.0;
            }

            auto aggregatedVertexResults = contourMapProjection.aggregatedVertexResultsFiltered();
            if ( aggregatedVertexResults.empty() ) return {};

            std::vector<caf::ContourLines::ListOfLineSegments> unorderedLineSegmentsPerLevel =
                caf::ContourLines::create( aggregatedVertexResults,
                                           contourMapProjection.xVertexPositions(),
                                           contourMapProjection.yVertexPositions(),
                                           contourLevels );

            std::vector<ContourPolygons> contourPolygons = std::vector<ContourPolygons>( unorderedLineSegmentsPerLevel.size() );
            const double areaThreshold = 1.5 * ( sampleSpacing * sampleSpacing ) / ( sampleSpacingFactor * sampleSpacingFactor );

#pragma omp parallel for
            for ( int i = 0; i < (int)unorderedLineSegmentsPerLevel.size(); ++i )
            {
                contourPolygons[i] = RigContourPolygonsTools::createContourPolygonsFromLineSegments( unorderedLineSegmentsPerLevel[i],
                                                                                                     contourLevels[i],
                                                                                                     areaThreshold );

                if ( smoothContourLines )
                {
                    RigContourPolygonsTools::smoothContourPolygons( contourPolygons[i], true, sampleSpacing );
                }

                for ( RigContourPolygonsTools::ContourPolygon& polygon : contourPolygons[i] )
                {
                    RigCellGeometryTools::simplifyPolygon( &polygon.vertices, simplifyEpsilon );
                }
            }

            // The clipping of contour polygons is intended to detect and fix a smoothed contour polygons
            // crossing into an outer contour line. The current implementation has some side effects causing
            // several contour lines to disappear. Disable this clipping for now
            /*
            if ( m_smoothContourLines() )
            {
                for ( size_t i = 1; i < contourPolygons.size(); ++i )
                {
                    RigContourPolygonsTools::clipContourPolygons(&contourPolygons[i], &contourPolygons[i - 1] );
                }
            }
            */

            std::vector<double> contourLevelCumulativeAreas( contourPolygons.size(), 0.0 );
            for ( int64_t i = (int64_t)contourPolygons.size() - 1; i >= 0; --i )
            {
                double levelOuterArea          = RigContourPolygonsTools::sumPolygonArea( contourPolygons[i] );
                contourLevelCumulativeAreas[i] = levelOuterArea;
            }

            return { contourPolygons, contourLevelCumulativeAreas };
        }
    }

    return {};
}

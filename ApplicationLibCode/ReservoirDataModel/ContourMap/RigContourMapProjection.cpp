/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018- Equinor ASA
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

#include "RigContourMapProjection.h"

#include "RiaWeightedMeanCalculator.h"
#include "RigStatisticsTools.h"

#include "RigContourMapCalculator.h"
#include "RigContourMapGrid.h"

#include "cvfArray.h"
#include "cvfGeometryTools.h"

#include <algorithm>
#include <array>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigContourMapProjection::RigContourMapProjection( const RigContourMapGrid& contourMapGrid )
    : m_contourMapGrid( contourMapGrid )
    , m_currentResultTimestep( -1 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<std::pair<size_t, double>>>
    RigContourMapProjection::generateGridMapping( RigContourMapCalculator::ResultAggregationType resultAggregation,
                                                  const std::vector<double>&                     weights,
                                                  const std::set<int>&                           kLayers,
                                                  const std::vector<std::vector<cvf::Vec3d>>&    limitToPolygons )
{
    m_projected3dGridIndices =
        RigContourMapCalculator::generateGridMapping( *this, m_contourMapGrid, resultAggregation, weights, kLayers, limitToPolygons );
    return m_projected3dGridIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapProjection::maxValue() const
{
    return maxValue( m_aggregatedResults );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapProjection::minValue() const
{
    return minValue( m_aggregatedResults );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapProjection::meanValue() const
{
    return sumAllValues() / numberOfValidCells();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapProjection::sumAllValues() const
{
    double sum = 0.0;

    for ( size_t index = 0; index < m_aggregatedResults.size(); ++index )
    {
        if ( m_aggregatedResults[index] != std::numeric_limits<double>::infinity() )
        {
            sum += m_aggregatedResults[index];
        }
    }
    return sum;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RigContourMapProjection::numberOfElementsIJ() const
{
    return m_contourMapGrid.numberOfElementsIJ();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec2ui RigContourMapProjection::numberOfVerticesIJ() const
{
    return m_contourMapGrid.numberOfVerticesIJ();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigContourMapProjection::vertexIndex( unsigned int i, unsigned int j ) const
{
    return m_contourMapGrid.vertexIndexFromIJ( i, j );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapProjection::valueAtVertex( unsigned int i, unsigned int j ) const
{
    size_t index = m_contourMapGrid.vertexIndexFromIJ( i, j );
    if ( index < numberOfVertices() )
    {
        return m_aggregatedVertexResults.at( index );
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
unsigned int RigContourMapProjection::numberOfCells() const
{
    return m_contourMapGrid.numberOfCells();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
unsigned int RigContourMapProjection::numberOfValidCells() const
{
    unsigned int validCount = 0u;
    for ( unsigned int i = 0; i < numberOfCells(); ++i )
    {
        cvf::Vec2ui ij = m_contourMapGrid.ijFromCellIndex( i );
        if ( hasResultInCell( ij.x(), ij.y() ) )
        {
            validCount++;
        }
    }
    return validCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigContourMapProjection::numberOfVertices() const
{
    cvf::Vec2ui gridSize = numberOfVerticesIJ();
    return static_cast<size_t>( gridSize.x() ) * static_cast<size_t>( gridSize.y() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigContourMapProjection::checkForMapIntersection( const cvf::Vec3d& domainPoint3d, cvf::Vec2d* contourMapPoint, double* valueAtPoint ) const
{
    CVF_TIGHT_ASSERT( contourMapPoint );
    CVF_TIGHT_ASSERT( valueAtPoint );

    const cvf::Vec3d& minPoint = m_contourMapGrid.expandedBoundingBox().min();
    cvf::Vec3d        mapPos3d = domainPoint3d - minPoint;
    cvf::Vec2d        mapPos2d( mapPos3d.x(), mapPos3d.y() );
    cvf::Vec2d        gridorigin( minPoint.x(), minPoint.y() );

    double value = interpolateValue( mapPos2d );
    if ( value != std::numeric_limits<double>::infinity() )
    {
        *valueAtPoint    = value;
        *contourMapPoint = mapPos2d + gridorigin;

        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigContourMapProjection::origin3d() const
{
    return m_contourMapGrid.expandedBoundingBox().min();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapProjection::topDepthBoundingBox() const
{
    return m_contourMapGrid.expandedBoundingBox().max().z();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigContourMapProjection::gridResultIndex( size_t globalCellIdx ) const
{
    return globalCellIdx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapProjection::calculateValueInMapCell( unsigned int                                   i,
                                                         unsigned int                                   j,
                                                         const std::vector<double>&                     gridCellValues,
                                                         RigContourMapCalculator::ResultAggregationType resultAggregation ) const
{
    const std::vector<std::pair<size_t, double>>& matchingCells = cellsAtIJ( i, j );
    return RigContourMapCalculator::calculateValueInMapCell( *this, matchingCells, gridCellValues, resultAggregation );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapProjection::maxValue( const std::vector<double>& aggregatedResults )
{
    double maxV = RigStatisticsTools::maximumValue( aggregatedResults );
    return maxV != -std::numeric_limits<double>::max() ? maxV : -std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapProjection::minValue( const std::vector<double>& aggregatedResults )
{
    double minV = RigStatisticsTools::minimumValue( aggregatedResults );
    return minV != std::numeric_limits<double>::max() ? minV : std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigContourMapProjection::setCellVisibility( cvf::ref<cvf::UByteArray> cellVisibility )
{
    m_cellGridIdxVisibility = cellVisibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray> RigContourMapProjection::getCellVisibility() const
{
    return m_cellGridIdxVisibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigContourMapProjection::generateVertexResults()
{
    size_t nCells = numberOfCells();
    if ( nCells != m_aggregatedResults.size() ) return;

    size_t nVertices          = numberOfVertices();
    m_aggregatedVertexResults = std::vector<double>( nVertices, std::numeric_limits<double>::infinity() );
#pragma omp parallel for
    for ( int index = 0; index < static_cast<int>( nVertices ); ++index )
    {
        cvf::Vec2ui ij                   = m_contourMapGrid.ijFromVertexIndex( index );
        m_aggregatedVertexResults[index] = calculateValueAtVertex( ij.x(), ij.y() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigContourMapProjection::setValueFilter( std::optional<std::pair<double, double>> valueFilter )
{
    m_valueFilter = valueFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<std::pair<double, double>> RigContourMapProjection::valueFilter() const
{
    return m_valueFilter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapProjection::sumTriangleAreas( const std::vector<cvf::Vec4d>& triangles )
{
    double sumArea = 0.0;
    for ( size_t i = 0; i < triangles.size(); i += 3 )
    {
        cvf::Vec3d v1( triangles[i].x(), triangles[i].y(), triangles[i].z() );
        cvf::Vec3d v2( triangles[i + 1].x(), triangles[i + 1].y(), triangles[i + 1].z() );
        cvf::Vec3d v3( triangles[i + 2].x(), triangles[i + 2].y(), triangles[i + 2].z() );
        double     area = 0.5 * ( ( v3 - v1 ) ^ ( v2 - v1 ) ).length();
        sumArea += area;
    }
    return sumArea;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapProjection::interpolateValue( const cvf::Vec2d& gridPos2d ) const
{
    cvf::Vec2ui cellContainingPoint = m_contourMapGrid.ijFromLocalPos( gridPos2d );
    cvf::Vec2d  cellCenter          = m_contourMapGrid.cellCenterPosition( cellContainingPoint.x(), cellContainingPoint.y() );

    double                    sampleSpacing = m_contourMapGrid.sampleSpacing();
    std::array<cvf::Vec3d, 4> x;
    x[0] = cvf::Vec3d( cellCenter + cvf::Vec2d( -sampleSpacing * 0.5, -sampleSpacing * 0.5 ), 0.0 );
    x[1] = cvf::Vec3d( cellCenter + cvf::Vec2d( sampleSpacing * 0.5, -sampleSpacing * 0.5 ), 0.0 );
    x[2] = cvf::Vec3d( cellCenter + cvf::Vec2d( sampleSpacing * 0.5, sampleSpacing * 0.5 ), 0.0 );
    x[3] = cvf::Vec3d( cellCenter + cvf::Vec2d( -sampleSpacing * 0.5, sampleSpacing * 0.5 ), 0.0 );

    cvf::Vec4d baryCentricCoords = cvf::GeometryTools::barycentricCoords( x[0], x[1], x[2], x[3], cvf::Vec3d( gridPos2d, 0.0 ) );

    std::array<cvf::Vec2ui, 4> v;
    v[0] = cellContainingPoint;
    v[1] = cvf::Vec2ui( cellContainingPoint.x() + 1u, cellContainingPoint.y() );
    v[2] = cvf::Vec2ui( cellContainingPoint.x() + 1u, cellContainingPoint.y() + 1u );
    v[3] = cvf::Vec2ui( cellContainingPoint.x(), cellContainingPoint.y() + 1u );

    std::array<double, 4> vertexValues;
    double                validBarycentricCoordsSum = 0.0;
    for ( int i = 0; i < 4; ++i )
    {
        double vertexValue = valueAtVertex( v[i].x(), v[i].y() );
        if ( vertexValue == std::numeric_limits<double>::infinity() )
        {
            return std::numeric_limits<double>::infinity();
        }
        else
        {
            vertexValues[i] = vertexValue;
            validBarycentricCoordsSum += baryCentricCoords[i];
        }
    }

    if ( validBarycentricCoordsSum < 1.0e-8 )
    {
        return std::numeric_limits<double>::infinity();
    }

    // Calculate final value
    double value = 0.0;
    for ( int i = 0; i < 4; ++i )
    {
        value += baryCentricCoords[i] / validBarycentricCoordsSum * vertexValues[i];
    }

    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapProjection::valueInCell( unsigned int i, unsigned int j ) const
{
    size_t index = m_contourMapGrid.cellIndexFromIJ( i, j );
    if ( index < numberOfCells() )
    {
        return m_aggregatedResults.at( index );
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigContourMapProjection::hasResultInCell( unsigned int i, unsigned int j ) const
{
    return !cellsAtIJ( i, j ).empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapProjection::calculateValueAtVertex( unsigned int vi, unsigned int vj ) const
{
    std::vector<unsigned int> averageIs;
    std::vector<unsigned int> averageJs;

    if ( vi > 0u ) averageIs.push_back( vi - 1 );
    if ( vj > 0u ) averageJs.push_back( vj - 1 );
    if ( vi < m_contourMapGrid.mapSize().x() ) averageIs.push_back( vi );
    if ( vj < m_contourMapGrid.mapSize().y() ) averageJs.push_back( vj );

    RiaWeightedMeanCalculator<double> calc;
    for ( unsigned int j : averageJs )
    {
        for ( unsigned int i : averageIs )
        {
            if ( hasResultInCell( i, j ) )
            {
                calc.addValueAndWeight( valueInCell( i, j ), 1.0 );
            }
        }
    }
    if ( calc.validAggregatedWeight() )
    {
        return calc.weightedMean();
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigContourMapProjection::contourMapCellContainsOnlyInactiveCells( unsigned int i, unsigned int j ) const
{
    const std::vector<CellIndexAndResult>& matchingCells = cellsAtIJ( i, j );

    if ( matchingCells.empty() )
    {
        return true;
    }

    for ( const auto& cellResult : matchingCells )
    {
        if ( isCellActive( cellResult.first ) )
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<size_t, double>> RigContourMapProjection::cellsAtIJ( unsigned int i, unsigned int j ) const
{
    size_t cellIndex = m_contourMapGrid.cellIndexFromIJ( i, j );
    if ( cellIndex < m_projected3dGridIndices.size() )
    {
        return m_projected3dGridIndices[cellIndex];
    }
    return std::vector<std::pair<size_t, double>>();
}

//--------------------------------------------------------------------------------------------------
/// Vertex positions in local coordinates (add origin2d.x() for UTM x)
//--------------------------------------------------------------------------------------------------
std::vector<double> RigContourMapProjection::xVertexPositions() const
{
    return m_contourMapGrid.xVertexPositions();
}

//--------------------------------------------------------------------------------------------------
/// Vertex positions in local coordinates (add origin2d.y() for UTM y)
//--------------------------------------------------------------------------------------------------
std::vector<double> RigContourMapProjection::yVertexPositions() const
{
    return m_contourMapGrid.yVertexPositions();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigContourMapProjection::aggregatedResults() const
{
    return m_aggregatedResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigContourMapProjection::aggregatedVertexResultsFiltered() const
{
    if ( m_valueFilter )
    {
        std::vector<double> filteredResults = m_aggregatedVertexResults;
        std::transform( filteredResults.begin(),
                        filteredResults.end(),
                        filteredResults.begin(),
                        [this]( double value )
                        {
                            return ( value < m_valueFilter->first || value > m_valueFilter->second ) ? std::numeric_limits<double>::infinity()
                                                                                                     : value;
                        } );
        return filteredResults;
    }

    return m_aggregatedVertexResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<std::pair<size_t, double>>>& RigContourMapProjection::projected3dGridIndices() const
{
    return m_projected3dGridIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigContourMapProjection::clearResults()
{
    m_aggregatedResults.clear();
    m_aggregatedVertexResults.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigContourMapProjection::clearGridMapping()
{
    m_projected3dGridIndices.clear();
}

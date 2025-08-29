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

#pragma once

#include "RigContourMapCalculator.h"
#include "RigContourPolygonsTools.h"

#include "cvfArray.h"
#include "cvfBoundingBox.h"
#include "cvfVector2.h"

#include <optional>
#include <set>

class RigContourMapGrid;
class RimGridView;
class RimRegularLegendConfig;
class RigPolyLinesData;

//==================================================================================================
///
///
//==================================================================================================
class RigContourMapProjection
{
public:
    using CellIndexAndResult = std::pair<size_t, double>;

    RigContourMapProjection( const RigContourMapGrid& );

    void clearResults();
    void clearGridMapping();

    void                                     generateVertexResults();
    void                                     setValueFilter( std::optional<std::pair<double, double>> valueFilter );
    std::optional<std::pair<double, double>> valueFilter() const;

    double maxValue() const;
    double minValue() const;

    double meanValue() const;
    double sumAllValues() const;

    cvf::Vec2ui numberOfElementsIJ() const;
    cvf::Vec2ui numberOfVerticesIJ() const;

    size_t vertexIndex( unsigned int i, unsigned int j ) const;
    double valueAtVertex( unsigned int i, unsigned int j ) const;

    unsigned int numberOfCells() const;
    unsigned int numberOfValidCells() const;
    size_t       numberOfVertices() const;

    bool       checkForMapIntersection( const cvf::Vec3d& domainPoint3d, cvf::Vec2d* contourMapPoint, double* valueAtPoint ) const;
    cvf::Vec3d origin3d() const;
    double     topDepthBoundingBox() const;

    std::vector<double> xVertexPositions() const;
    std::vector<double> yVertexPositions() const;

    // Use this function to get the result index into grid cell results. The index will differ if we have active cells
    virtual size_t gridResultIndex( size_t globalCellIdx ) const;

    virtual size_t              kLayer( size_t globalCellIdx ) const                                               = 0;
    virtual size_t              kLayers() const                                                                    = 0;
    virtual std::vector<size_t> findIntersectingCells( const cvf::BoundingBox& bbox ) const                        = 0;
    virtual double              calculateOverlapVolume( size_t globalCellIdx, const cvf::BoundingBox& bbox ) const = 0;
    virtual double calculateRayLengthInCell( size_t globalCellIdx, const cvf::Vec3d& highestPoint, const cvf::Vec3d& lowestPoint ) const = 0;
    virtual double getParameterWeightForCell( size_t globalCellIdx, const std::vector<double>& parameterWeights ) const = 0;
    virtual std::vector<bool> getMapCellVisibility( int viewStepIndex, RigContourMapCalculator::ResultAggregationType resultAggregation ) = 0;
    virtual bool isCellActive( size_t globalCellIdx ) const = 0;

    void                      setCellVisibility( cvf::ref<cvf::UByteArray> cellVisibility );
    cvf::ref<cvf::UByteArray> getCellVisibility() const;

    virtual std::vector<std::vector<std::pair<size_t, double>>>
        generateGridMapping( RigContourMapCalculator::ResultAggregationType resultAggregation,
                             const std::vector<double>&                     weights,
                             const std::set<int>&                           kLayers,
                             const std::vector<std::vector<cvf::Vec3d>>&    limitToPolygons );

    double interpolateValue( const cvf::Vec2d& gridPosition2d ) const;

    const std::vector<double>&                                 aggregatedResults() const;
    std::vector<double>                                        aggregatedVertexResultsFiltered() const;
    const std::vector<std::vector<std::pair<size_t, double>>>& projected3dGridIndices() const;

    // Cell index and position conversion
    virtual std::vector<CellIndexAndResult> cellsAtIJ( unsigned int i, unsigned int j ) const;

    static double maxValue( const std::vector<double>& aggregatedResults );
    static double minValue( const std::vector<double>& aggregatedResults );
    static double sumTriangleAreas( const std::vector<cvf::Vec4d>& triangles );

protected:
    double calculateValueInMapCell( unsigned int                                   i,
                                    unsigned int                                   j,
                                    const std::vector<double>&                     gridCellValues,
                                    RigContourMapCalculator::ResultAggregationType resultAggregation ) const;

    double valueInCell( unsigned int i, unsigned int j ) const;
    bool   hasResultInCell( unsigned int i, unsigned int j ) const;
    double calculateValueAtVertex( unsigned int i, unsigned int j ) const;
    bool   contourMapCellContainsOnlyInactiveCells( unsigned int i, unsigned int j ) const;

protected:
    cvf::ref<cvf::UByteArray>                           m_cellGridIdxVisibility;
    std::vector<double>                                 m_aggregatedResults;
    std::vector<double>                                 m_aggregatedVertexResults;
    std::vector<std::vector<std::pair<size_t, double>>> m_projected3dGridIndices;

    int               m_currentResultTimestep;
    std::vector<bool> m_mapCellVisibility;

    std::optional<std::pair<double, double>> m_valueFilter;

    const RigContourMapGrid& m_contourMapGrid;
};

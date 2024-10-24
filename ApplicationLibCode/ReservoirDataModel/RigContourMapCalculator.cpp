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

#include "RigContourMapCalculator.h"

#include "RiaWeightedGeometricMeanCalculator.h"
#include "RiaWeightedHarmonicMeanCalculator.h"
#include "RiaWeightedMeanCalculator.h"

#include "RigContourMapGrid.h"

#include "RimCase.h"
#include "RimContourMapProjection.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapCalculator::calculateValueInMapCell( const RimContourMapProjection&                contourMapProjection,
                                                         const std::vector<std::pair<size_t, double>>& matchingCells,
                                                         const std::vector<double>&                    gridCellValues,
                                                         ResultAggregationEnum                         resultAggregation )
{
    if ( matchingCells.empty() ) return std::numeric_limits<double>::infinity();

    switch ( resultAggregation )
    {
        case RESULTS_TOP_VALUE:
            return calculateTopValue( contourMapProjection, matchingCells, gridCellValues );
        case RESULTS_MEAN_VALUE:
            return calculateMeanValue( contourMapProjection, matchingCells, gridCellValues );
        case RESULTS_GEOM_VALUE:
            return calculateGeometricMeanValue( contourMapProjection, matchingCells, gridCellValues );
        case RESULTS_HARM_VALUE:
            return calculateHarmonicMeanValue( contourMapProjection, matchingCells, gridCellValues );
        case RESULTS_MAX_VALUE:
            return calculateMaxValue( contourMapProjection, matchingCells, gridCellValues );
        case RESULTS_MIN_VALUE:
            return calculateMinValue( contourMapProjection, matchingCells, gridCellValues );
        case RESULTS_VOLUME_SUM:
        case RESULTS_SUM:
        case RESULTS_OIL_COLUMN:
        case RESULTS_GAS_COLUMN:
        case RESULTS_HC_COLUMN:
            return calculateSum( contourMapProjection, matchingCells, gridCellValues );
        default:
        {
            CVF_TIGHT_ASSERT( false );
            return std::numeric_limits<double>::infinity();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapCalculator::calculateTopValue( const RimContourMapProjection&                contourMapProjection,
                                                   const std::vector<std::pair<size_t, double>>& matchingCells,
                                                   const std::vector<double>&                    gridCellValues )
{
    for ( auto [cellIdx, weight] : matchingCells )
    {
        double cellValue = gridCellValues[contourMapProjection.gridResultIndex( cellIdx )];
        if ( std::abs( cellValue ) != std::numeric_limits<double>::infinity() )
        {
            return cellValue;
        }
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapCalculator::calculateMeanValue( const RimContourMapProjection&                contourMapProjection,
                                                    const std::vector<std::pair<size_t, double>>& matchingCells,
                                                    const std::vector<double>&                    gridCellValues )
{
    RiaWeightedMeanCalculator<double> calculator;
    for ( auto [cellIdx, weight] : matchingCells )
    {
        double cellValue = gridCellValues[contourMapProjection.gridResultIndex( cellIdx )];
        if ( std::abs( cellValue ) != std::numeric_limits<double>::infinity() )
        {
            calculator.addValueAndWeight( cellValue, weight );
        }
    }
    if ( calculator.validAggregatedWeight() )
    {
        return calculator.weightedMean();
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapCalculator::calculateGeometricMeanValue( const RimContourMapProjection&                contourMapProjection,
                                                             const std::vector<std::pair<size_t, double>>& matchingCells,
                                                             const std::vector<double>&                    gridCellValues )
{
    RiaWeightedGeometricMeanCalculator calculator;
    for ( auto [cellIdx, weight] : matchingCells )
    {
        double cellValue = gridCellValues[contourMapProjection.gridResultIndex( cellIdx )];
        if ( std::abs( cellValue ) != std::numeric_limits<double>::infinity() )
        {
            if ( cellValue < 1.0e-8 )
            {
                return 0.0;
            }
            calculator.addValueAndWeight( cellValue, weight );
        }
    }
    if ( calculator.validAggregatedWeight() )
    {
        return calculator.weightedMean();
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapCalculator::calculateHarmonicMeanValue( const RimContourMapProjection&                contourMapProjection,
                                                            const std::vector<std::pair<size_t, double>>& matchingCells,
                                                            const std::vector<double>&                    gridCellValues )
{
    RiaWeightedHarmonicMeanCalculator calculator;
    for ( auto [cellIdx, weight] : matchingCells )
    {
        double cellValue = gridCellValues[contourMapProjection.gridResultIndex( cellIdx )];
        if ( std::fabs( cellValue ) < 1.0e-8 )
        {
            return 0.0;
        }
        if ( std::abs( cellValue ) != std::numeric_limits<double>::infinity() )
        {
            calculator.addValueAndWeight( cellValue, weight );
        }
    }
    if ( calculator.validAggregatedWeight() )
    {
        return calculator.weightedMean();
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapCalculator::calculateMaxValue( const RimContourMapProjection&                contourMapProjection,
                                                   const std::vector<std::pair<size_t, double>>& matchingCells,
                                                   const std::vector<double>&                    gridCellValues )
{
    double maxValue = -std::numeric_limits<double>::infinity();
    for ( auto [cellIdx, weight] : matchingCells )
    {
        double cellValue = gridCellValues[contourMapProjection.gridResultIndex( cellIdx )];
        if ( std::abs( cellValue ) != std::numeric_limits<double>::infinity() )
        {
            maxValue = std::max( maxValue, cellValue );
        }
    }
    if ( maxValue == -std::numeric_limits<double>::infinity() )
    {
        maxValue = std::numeric_limits<double>::infinity();
    }
    return maxValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigContourMapCalculator::calculateMinValue( const RimContourMapProjection&                contourMapProjection,
                                                   const std::vector<std::pair<size_t, double>>& matchingCells,
                                                   const std::vector<double>&                    gridCellValues )
{
    double minValue = std::numeric_limits<double>::infinity();
    for ( auto [cellIdx, weight] : matchingCells )
    {
        double cellValue = gridCellValues[contourMapProjection.gridResultIndex( cellIdx )];
        minValue         = std::min( minValue, cellValue );
    }
    return minValue;
}

double RigContourMapCalculator::calculateSum( const RimContourMapProjection&                contourMapProjection,
                                              const std::vector<std::pair<size_t, double>>& matchingCells,
                                              const std::vector<double>&                    gridCellValues )
{
    double sum = 0.0;
    for ( auto [cellIdx, weight] : matchingCells )
    {
        double cellValue = gridCellValues[contourMapProjection.gridResultIndex( cellIdx )];
        if ( std::abs( cellValue ) != std::numeric_limits<double>::infinity() )
        {
            sum += cellValue * weight;
        }
    }
    return sum;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<std::pair<size_t, double>>>
    RigContourMapCalculator::generateGridMapping( RimContourMapProjection& contourMapProjection, const RigContourMapGrid& contourMapGrid )
{
    int                                                 nCells = contourMapGrid.numberOfCells();
    std::vector<std::vector<std::pair<size_t, double>>> projected3dGridIndices( nCells );

    std::vector<double> weightingResultValues = contourMapProjection.retrieveParameterWeights();

    if ( contourMapProjection.isStraightSummationResult() )
    {
#pragma omp parallel for
        for ( int index = 0; index < nCells; ++index )
        {
            cvf::Vec2ui ij = contourMapGrid.ijFromCellIndex( index );

            cvf::Vec2d globalPos = contourMapGrid.cellCenterPosition( ij.x(), ij.y() ) + contourMapGrid.origin2d();
            projected3dGridIndices[index] =
                cellRayIntersectionAndResults( contourMapProjection, contourMapGrid, globalPos, weightingResultValues );
        }
    }
    else
    {
#pragma omp parallel for
        for ( int index = 0; index < nCells; ++index )
        {
            cvf::Vec2ui ij = contourMapGrid.ijFromCellIndex( index );

            cvf::Vec2d globalPos = contourMapGrid.cellCenterPosition( ij.x(), ij.y() ) + contourMapGrid.origin2d();
            projected3dGridIndices[index] =
                cellOverlapVolumesAndResults( contourMapProjection, contourMapGrid, globalPos, weightingResultValues );
        }
    }

    return projected3dGridIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigContourMapCalculator::CellIndexAndResult>
    RigContourMapCalculator::cellOverlapVolumesAndResults( const RimContourMapProjection& contourMapProjection,
                                                           const RigContourMapGrid&       contourMapGrid,
                                                           const cvf::Vec2d&              globalPos2d,
                                                           const std::vector<double>&     weightingResultValues )
{
    const cvf::BoundingBox& expandedBoundingBox = contourMapGrid.expandedBoundingBox();
    cvf::Vec3d              top2dElementCentroid( globalPos2d, expandedBoundingBox.max().z() );
    cvf::Vec3d              bottom2dElementCentroid( globalPos2d, expandedBoundingBox.min().z() );
    cvf::Vec3d              planarDiagonalVector( 0.5 * contourMapGrid.sampleSpacing(), 0.5 * contourMapGrid.sampleSpacing(), 0.0 );
    cvf::Vec3d              topNECorner    = top2dElementCentroid + planarDiagonalVector;
    cvf::Vec3d              bottomSWCorner = bottom2dElementCentroid - planarDiagonalVector;

    cvf::BoundingBox bbox2dElement( bottomSWCorner, topNECorner );

    std::vector<std::pair<size_t, double>> matchingVisibleCellsAndWeight;

    // Bounding box has been expanded, so 2d element may be outside actual 3d grid
    if ( !bbox2dElement.intersects( contourMapGrid.originalBoundingBox() ) )
    {
        return matchingVisibleCellsAndWeight;
    }

    std::vector<size_t> allCellIndices = contourMapProjection.findIntersectingCells( bbox2dElement );

    std::vector<std::vector<size_t>> kLayerCellIndexVector;
    kLayerCellIndexVector.resize( contourMapProjection.kLayers() );

    if ( kLayerCellIndexVector.empty() )
    {
        return matchingVisibleCellsAndWeight;
    }

    auto cellGridIdxVisibility = contourMapProjection.getCellVisibility();
    for ( size_t globalCellIdx : allCellIndices )
    {
        if ( ( *cellGridIdxVisibility )[globalCellIdx] )
        {
            kLayerCellIndexVector[contourMapProjection.kLayer( globalCellIdx )].push_back( globalCellIdx );
        }
    }

    for ( const auto& kLayerIndices : kLayerCellIndexVector )
    {
        for ( size_t globalCellIdx : kLayerIndices )
        {
            double overlapVolume = contourMapProjection.calculateOverlapVolume( globalCellIdx, bbox2dElement );
            if ( overlapVolume > 0.0 )
            {
                double weight =
                    overlapVolume * contourMapProjection.getParameterWeightForCell( contourMapProjection.gridResultIndex( globalCellIdx ),
                                                                                    weightingResultValues );
                if ( weight > 0.0 )
                {
                    matchingVisibleCellsAndWeight.push_back( std::make_pair( globalCellIdx, weight ) );
                }
            }
        }
    }

    return matchingVisibleCellsAndWeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigContourMapCalculator::CellIndexAndResult>
    RigContourMapCalculator::cellRayIntersectionAndResults( const RimContourMapProjection& contourMapProjection,
                                                            const RigContourMapGrid&       contourMapGrid,
                                                            const cvf::Vec2d&              globalPos2d,
                                                            const std::vector<double>&     weightingResultValues )
{
    std::vector<std::pair<size_t, double>> matchingVisibleCellsAndWeight;

    const cvf::BoundingBox& expandedBoundingBox = contourMapGrid.expandedBoundingBox();

    cvf::Vec3d highestPoint( globalPos2d, expandedBoundingBox.max().z() );
    cvf::Vec3d lowestPoint( globalPos2d, expandedBoundingBox.min().z() );

    // Bounding box has been expanded, so ray may be outside actual 3d grid
    if ( !contourMapGrid.originalBoundingBox().contains( highestPoint ) )
    {
        return matchingVisibleCellsAndWeight;
    }

    cvf::BoundingBox rayBBox;
    rayBBox.add( highestPoint );
    rayBBox.add( lowestPoint );

    std::vector<size_t> allCellIndices = contourMapProjection.findIntersectingCells( rayBBox );

    std::map<size_t, std::vector<size_t>> kLayerIndexMap;

    auto cellGridIdxVisibility = contourMapProjection.getCellVisibility();
    for ( size_t globalCellIdx : allCellIndices )
    {
        if ( ( *cellGridIdxVisibility )[globalCellIdx] )
        {
            kLayerIndexMap[contourMapProjection.kLayer( globalCellIdx )].push_back( globalCellIdx );
        }
    }

    for ( const auto& kLayerIndexPair : kLayerIndexMap )
    {
        double                                 weightSumThisKLayer = 0.0;
        std::vector<std::pair<size_t, double>> cellsAndWeightsThisLayer;
        for ( size_t globalCellIdx : kLayerIndexPair.second )
        {
            double lengthInCell = contourMapProjection.calculateRayLengthInCell( globalCellIdx, highestPoint, lowestPoint );
            if ( lengthInCell > 0.0 )
            {
                cellsAndWeightsThisLayer.push_back( std::make_pair( globalCellIdx, lengthInCell ) );
                weightSumThisKLayer += lengthInCell;
            }
        }
        for ( auto& cellWeightPair : cellsAndWeightsThisLayer )
        {
            cellWeightPair.second /= weightSumThisKLayer;
            matchingVisibleCellsAndWeight.push_back( cellWeightPair );
        }
    }

    return matchingVisibleCellsAndWeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigContourMapCalculator::isColumnResult( ResultAggregationEnum aggregationType )
{
    return aggregationType == RESULTS_OIL_COLUMN || aggregationType == RESULTS_GAS_COLUMN || aggregationType == RESULTS_HC_COLUMN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigContourMapCalculator::isMeanResult( ResultAggregationEnum aggregationType )
{
    return aggregationType == RESULTS_MEAN_VALUE || aggregationType == RESULTS_HARM_VALUE || aggregationType == RESULTS_GEOM_VALUE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigContourMapCalculator::isStraightSummationResult( ResultAggregationEnum aggregationType )
{
    return aggregationType == RESULTS_OIL_COLUMN || aggregationType == RESULTS_GAS_COLUMN || aggregationType == RESULTS_HC_COLUMN ||
           aggregationType == RESULTS_SUM;
}

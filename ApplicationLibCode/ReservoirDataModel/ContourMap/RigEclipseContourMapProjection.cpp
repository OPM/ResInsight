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

#include "RigEclipseContourMapProjection.h"

#include "RiaResultNames.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigCellGeometryTools.h"
#include "RigContourMapCalculator.h"
#include "RigContourMapGrid.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigHexIntersectionTools.h"
#include "RigHydrocarbonFlowTools.h"
#include "RigMainGrid.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseContourMapProjection::RigEclipseContourMapProjection( const RigContourMapGrid& contourMapGrid,
                                                                RigEclipseCaseData&      eclipseCaseData,
                                                                RigCaseCellResultsData&  resultData )
    : RigContourMapProjection( contourMapGrid )
    , m_eclipseCaseData( eclipseCaseData )
    , m_resultData( resultData )
    , m_kLayers( 0u )
    , m_useActiveCellInfo( true )
{
    m_mainGrid       = m_eclipseCaseData.mainGrid();
    m_activeCellInfo = m_eclipseCaseData.activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    m_kLayers        = m_mainGrid->cellCountK();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseContourMapProjection::~RigEclipseContourMapProjection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigEclipseContourMapProjection::generateAndSaveResults( const RigEclipseResultAddress&                 resultAddress,
                                                             RigContourMapCalculator::ResultAggregationType resultAggregation,
                                                             int                                            timeStep,
                                                             RigFloodingSettings&                           floodingSettings )
{
    std::tie( m_useActiveCellInfo, m_aggregatedResults ) =
        generateResults( *this, m_contourMapGrid, m_resultData, resultAddress, resultAggregation, timeStep, floodingSettings );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigEclipseContourMapProjection::generateResults( const RigEclipseResultAddress&                 resultAddress,
                                                                     RigContourMapCalculator::ResultAggregationType resultAggregation,
                                                                     int                                            timeStep,
                                                                     RigFloodingSettings&                           floodingSettings ) const
{
    std::pair<bool, std::vector<double>> result =
        generateResults( *this, m_contourMapGrid, m_resultData, resultAddress, resultAggregation, timeStep, floodingSettings );
    return result.second;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::vector<double>>
    RigEclipseContourMapProjection::generateResults( const RigEclipseContourMapProjection&          contourMapProjection,
                                                     const RigContourMapGrid&                       contourMapGrid,
                                                     RigCaseCellResultsData&                        resultData,
                                                     const RigEclipseResultAddress&                 resultAddress,
                                                     RigContourMapCalculator::ResultAggregationType resultAggregation,
                                                     int                                            timeStep,
                                                     RigFloodingSettings&                           floodingSettings )
{
    size_t nCells = contourMapProjection.numberOfCells();

    std::vector<double> aggregatedResults = std::vector<double>( nCells, std::numeric_limits<double>::infinity() );

    bool useActiveCellInfo = resultAddress.isValid() && resultData.hasResultEntry( resultAddress ) &&
                             resultData.isUsingGlobalActiveIndex( resultAddress );

    auto isTernaryResult = []( const RigEclipseResultAddress& address ) -> bool
    {
        return address.resultCatType() == RiaDefines::ResultCatType::DYNAMIC_NATIVE &&
               ( address.resultName().compare( RiaResultNames::ternarySaturationResultName(), Qt::CaseInsensitive ) == 0 );
    };

    if ( !isTernaryResult( resultAddress ) )
    {
        std::vector<double> gridResultValues;
        if ( RigContourMapCalculator::isColumnResult( resultAggregation ) )
        {
            bool missingResults = false;

            for ( const auto& resAddr : neededResults( resultAggregation, floodingSettings ) )
            {
                missingResults = missingResults || !resultData.ensureKnownResultLoaded( resAddr );
            }

            if ( !missingResults )
            {
                gridResultValues = calculateColumnResult( resultData, resultAggregation, timeStep, floodingSettings );
            }
        }
        else
        {
            if ( resultAddress.resultCatType() == RiaDefines::ResultCatType::STATIC_NATIVE && timeStep > 0 ) timeStep = 0;

            resultData.ensureKnownResultLoaded( resultAddress );
            // When loading a project file, grid calculator results are not computed the first time this function is
            // called. Must check if result is loaded. See RimReloadCaseTools::updateAll3dViews()
            if ( resultAddress.isValid() && resultData.hasResultEntry( resultAddress ) && resultData.isResultLoaded( resultAddress ) )
            {
                gridResultValues = resultData.cellScalarResults( resultAddress, timeStep );
            }
        }

        if ( !gridResultValues.empty() )
        {
#pragma omp parallel for
            for ( int index = 0; index < static_cast<int>( nCells ); ++index )
            {
                cvf::Vec2ui                                   ij            = contourMapGrid.ijFromCellIndex( index );
                const std::vector<std::pair<size_t, double>>& matchingCells = contourMapProjection.cellsAtIJ( ij.x(), ij.y() );
                aggregatedResults[index] =
                    RigContourMapCalculator::calculateValueInMapCell( contourMapProjection, matchingCells, gridResultValues, resultAggregation );
            }
        }
    }

    return { useActiveCellInfo, aggregatedResults };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigEclipseContourMapProjection::calculateColumnResult( RigCaseCellResultsData&                        resultData,
                                                                           RigContourMapCalculator::ResultAggregationType resultAggregation,
                                                                           int                                            timeStep,
                                                                           RigFloodingSettings&                           floodingSettings )
{
    const std::vector<double>& poroResults =
        resultData.cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PORO" ), 0 );
    const std::vector<double>& ntgResults =
        resultData.cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "NTG" ), 0 );
    const std::vector<double>& dzResults =
        resultData.cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DZ" ), 0 );

    CVF_ASSERT( poroResults.size() == ntgResults.size() && ntgResults.size() == dzResults.size() );

    const auto nSamples = poroResults.size();

    auto mapToFlowResultType = []( RigContourMapCalculator::ResultAggregationType t1 )
    {
        if ( t1 == RigContourMapCalculator::ResultAggregationType::MOBILE_OIL_COLUMN )
            return RigHydrocarbonFlowTools::ResultType::MOBILE_OIL;
        if ( t1 == RigContourMapCalculator::ResultAggregationType::MOBILE_GAS_COLUMN )
            return RigHydrocarbonFlowTools::ResultType::MOBILE_GAS;
        if ( t1 == RigContourMapCalculator::ResultAggregationType::MOBILE_HYDROCARBON_COLUMN )
            return RigHydrocarbonFlowTools::ResultType::MOBILE_HYDROCARBON;
        return RigHydrocarbonFlowTools::ResultType::NONE;
    };
    auto                flowResultType = mapToFlowResultType( resultAggregation );
    std::vector<double> residualOil    = RigHydrocarbonFlowTools::residualOilData( resultData, flowResultType, floodingSettings, nSamples );
    std::vector<double> residualGas    = RigHydrocarbonFlowTools::residualGasData( resultData, flowResultType, floodingSettings, nSamples );

    std::vector<double> resultValues( nSamples, 0.0 );

    if ( ( resultAggregation == RigContourMapCalculator::OIL_COLUMN || resultAggregation == RigContourMapCalculator::HYDROCARBON_COLUMN ) ||
         ( resultAggregation == RigContourMapCalculator::MOBILE_OIL_COLUMN ||
           resultAggregation == RigContourMapCalculator::MOBILE_HYDROCARBON_COLUMN ) )
    {
        const std::vector<double>& soilResults =
            resultData.cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::soil() ),
                                          timeStep );
        for ( size_t n = 0; n < nSamples; n++ )
        {
            resultValues[n] = std::max( soilResults[n] - residualOil[n], 0.0 );
        }
    }

    if ( ( resultAggregation == RigContourMapCalculator::GAS_COLUMN || resultAggregation == RigContourMapCalculator::HYDROCARBON_COLUMN ) ||
         ( resultAggregation == RigContourMapCalculator::MOBILE_GAS_COLUMN ||
           resultAggregation == RigContourMapCalculator::MOBILE_HYDROCARBON_COLUMN ) )
    {
        const std::vector<double>& sgasResults =
            resultData.cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() ),
                                          timeStep );
        for ( size_t n = 0; n < nSamples; n++ )
        {
            resultValues[n] += std::max( sgasResults[n] - residualGas[n], 0.0 );
        }
    }

    for ( size_t n = 0; n < nSamples; n++ )
    {
        resultValues[n] *= poroResults[n] * ntgResults[n] * dzResults[n];
    }

    return resultValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigEclipseContourMapProjection::findIntersectingCells( const cvf::BoundingBox& bbox ) const
{
    return m_mainGrid->findIntersectingCells( bbox );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigEclipseContourMapProjection::kLayer( size_t globalCellIdx ) const
{
    const RigCell& cell            = m_mainGrid->cell( globalCellIdx );
    size_t         mainGridCellIdx = cell.mainGridCellIndex();
    size_t         i, j, k;
    m_mainGrid->ijkFromCellIndex( mainGridCellIdx, &i, &j, &k );
    return k;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigEclipseContourMapProjection::kLayers() const
{
    return m_kLayers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEclipseContourMapProjection::calculateOverlapVolume( size_t globalCellIdx, const cvf::BoundingBox& bbox ) const
{
    std::array<cvf::Vec3d, 8> hexCorners;

    const RigCell& cell = m_mainGrid->cell( globalCellIdx );

    size_t       localCellIdx = cell.gridLocalCellIndex();
    RigGridBase* localGrid    = cell.hostGrid();

    localGrid->cellCornerVertices( localCellIdx, hexCorners.data() );

    cvf::BoundingBox          overlapBBox;
    std::array<cvf::Vec3d, 8> overlapCorners;
    if ( RigCellGeometryTools::estimateHexOverlapWithBoundingBox( hexCorners, bbox, &overlapCorners, &overlapBBox ) )
    {
        double overlapVolume = RigCellGeometryTools::calculateCellVolume( overlapCorners );
        return overlapVolume;
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEclipseContourMapProjection::calculateRayLengthInCell( size_t            globalCellIdx,
                                                                 const cvf::Vec3d& highestPoint,
                                                                 const cvf::Vec3d& lowestPoint ) const
{
    std::array<cvf::Vec3d, 8> hexCorners;

    const RigCell& cell = m_mainGrid->cell( globalCellIdx );

    size_t       localCellIdx = cell.gridLocalCellIndex();
    RigGridBase* localGrid    = cell.hostGrid();

    localGrid->cellCornerVertices( localCellIdx, hexCorners.data() );
    std::vector<HexIntersectionInfo> intersections;

    if ( RigHexIntersectionTools::lineHexCellIntersection( highestPoint, lowestPoint, hexCorners.data(), 0, &intersections ) )
    {
        double lengthInCell = ( intersections.back().m_intersectionPoint - intersections.front().m_intersectionPoint ).length();
        return lengthInCell;
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEclipseContourMapProjection::getParameterWeightForCell( size_t cellResultIdx, const std::vector<double>& cellWeights ) const
{
    if ( cellWeights.empty() ) return 1.0;

    double result = std::max( cellWeights[cellResultIdx], 0.0 );
    if ( result < 1.0e-6 )
    {
        result = 0.0;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigEclipseContourMapProjection::gridResultIndex( size_t globalCellIdx ) const
{
    if ( m_useActiveCellInfo ) return m_activeCellInfo->cellResultIndex( globalCellIdx );

    return globalCellIdx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigEclipseContourMapProjection::isCellActive( size_t globalCellIdx ) const
{
    if ( m_useActiveCellInfo && m_activeCellInfo.notNull() )
    {
        return m_activeCellInfo->isActive( globalCellIdx );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<bool> RigEclipseContourMapProjection::getMapCellVisibility( int                                            viewStepIndex,
                                                                        RigContourMapCalculator::ResultAggregationType resultAggregation )

{
    return std::vector<bool>( numberOfCells(), true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigEclipseResultAddress> RigEclipseContourMapProjection::neededResults( RigContourMapCalculator::ResultAggregationType resultAggregation,
                                                                                 RigFloodingSettings& floodingSettings )
{
    std::set<RigEclipseResultAddress> results;

    if ( RigContourMapCalculator::isColumnResult( resultAggregation ) )
    {
        results.emplace( RiaDefines::ResultCatType::STATIC_NATIVE, "PORO" );
        results.emplace( RiaDefines::ResultCatType::STATIC_NATIVE, "NTG" );
        results.emplace( RiaDefines::ResultCatType::STATIC_NATIVE, "DZ" );

        if ( resultAggregation == RigContourMapCalculator::OIL_COLUMN || resultAggregation == RigContourMapCalculator::HYDROCARBON_COLUMN ||
             resultAggregation == RigContourMapCalculator::MOBILE_OIL_COLUMN ||
             resultAggregation == RigContourMapCalculator::MOBILE_HYDROCARBON_COLUMN )
        {
            results.emplace( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::soil() );
        }
        if ( resultAggregation == RigContourMapCalculator::GAS_COLUMN || resultAggregation == RigContourMapCalculator::HYDROCARBON_COLUMN ||
             resultAggregation == RigContourMapCalculator::MOBILE_GAS_COLUMN ||
             resultAggregation == RigContourMapCalculator::MOBILE_HYDROCARBON_COLUMN )
        {
            results.emplace( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() );
        }
    }

    if ( RigContourMapCalculator::isMobileColumnResult( resultAggregation ) )
    {
        if ( ( resultAggregation == RigContourMapCalculator::ResultAggregationType::MOBILE_OIL_COLUMN ) ||
             ( resultAggregation == RigContourMapCalculator::ResultAggregationType::MOBILE_HYDROCARBON_COLUMN ) )
        {
            if ( floodingSettings.oilFlooding() == RigFloodingSettings::FloodingType::WATER_FLOODING )
            {
                results.emplace( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::sowcr() );
            }
            else if ( floodingSettings.oilFlooding() == RigFloodingSettings::FloodingType::GAS_FLOODING )
            {
                results.emplace( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::sogcr() );
            }
        }
        if ( ( resultAggregation == RigContourMapCalculator::ResultAggregationType::MOBILE_GAS_COLUMN ) ||
             ( resultAggregation == RigContourMapCalculator::ResultAggregationType::MOBILE_HYDROCARBON_COLUMN ) )
        {
            if ( floodingSettings.gasFlooding() == RigFloodingSettings::FloodingType::GAS_FLOODING )
            {
                results.emplace( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::sgcr() );
            }
        }
    }

    return results;
}

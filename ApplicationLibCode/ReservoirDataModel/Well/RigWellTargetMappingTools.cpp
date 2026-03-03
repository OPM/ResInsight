/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-  Equinor ASA
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

#include "RigWellTargetMappingTools.h"

#include "RiaDefines.h"
#include "RiaEclipseUnitTools.h"
#include "RiaLogging.h"
#include "RiaResultNames.h"
#include "RiaWeightedMeanCalculator.h"

#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultInfo.h"
#include "RigFloodingSettings.h"
#include "RigGridBase.h"
#include "RigHydrocarbonFlowTools.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "RigNncConnection.h"
#include "RigStatisticsMath.h"
#include "RigStatisticsTools.h"

#include "RimEclipseCase.h"

#include "cafAssert.h"
#include "cafVecIjk.h"
#include "cvfMath.h"
#include "cvfStructGrid.h"

#include <algorithm>
#include <limits>
#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellTargetMappingTools::getValueForFace( const std::vector<double>& x,
                                                   const std::vector<double>& y,
                                                   const std::vector<double>& z,
                                                   CellFaceType               face,
                                                   ActiveCellIndex            resultIndex )
{
    if ( face == cvf::StructGridInterface::FaceType::POS_I || face == cvf::StructGridInterface::FaceType::NEG_I )
        return x[resultIndex.value()];
    if ( face == cvf::StructGridInterface::FaceType::POS_J || face == cvf::StructGridInterface::FaceType::NEG_J )
        return y[resultIndex.value()];
    if ( face == cvf::StructGridInterface::FaceType::POS_K || face == cvf::StructGridInterface::FaceType::NEG_K )
        return z[resultIndex.value()];
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellTargetMappingTools::getTransmissibilityValueForFace( const std::vector<double>& x,
                                                                   const std::vector<double>& y,
                                                                   const std::vector<double>& z,
                                                                   CellFaceType               face,
                                                                   ActiveCellIndex            resultIndex,
                                                                   ActiveCellIndex            neighborResultIndex )
{
    // For negative directions (NEG_I, NEG_J, NEG_K) use the value from the neighbor cell
    bool isPos = face == cvf::StructGridInterface::FaceType::POS_I || face == cvf::StructGridInterface::FaceType::POS_J ||
                 face == cvf::StructGridInterface::FaceType::POS_K;
    ActiveCellIndex index = isPos ? resultIndex : neighborResultIndex;
    return getValueForFace( x, y, z, face, index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigWellTargetMappingTools::getOilVectorName( VolumesType volumesType )
{
    switch ( volumesType )
    {
        case VolumesType::RESERVOIR_VOLUMES_COMPUTED:
            return RiaResultNames::riPorvSoil();
        case VolumesType::RESERVOIR_VOLUMES:
            return "RFIPOIL";
        case VolumesType::SURFACE_VOLUMES_SFIP:
            return "SFIPOIL";
        case VolumesType::SURFACE_VOLUMES_FIP:
            return "FIPOIL";
        default:
        {
            CAF_ASSERT( false );
            return "";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigWellTargetMappingTools::getGasVectorName( VolumesType volumesType )
{
    switch ( volumesType )
    {
        case VolumesType::RESERVOIR_VOLUMES_COMPUTED:
            return RiaResultNames::riPorvSgas();
        case VolumesType::RESERVOIR_VOLUMES:
            return "RFIPGAS";
        case VolumesType::SURFACE_VOLUMES_SFIP:
            return "SFIPGAS";
        case VolumesType::SURFACE_VOLUMES_FIP:
            return "FIPGAS";
        default:
        {
            CAF_ASSERT( false );
            return "";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWellTargetMappingTools::isSaturationSufficient( VolumeType              volumeType,
                                                        const DataContainer&    data,
                                                        const ClusteringLimits& limits,
                                                        ActiveCellIndex         idx )
{
    bool needsValidOil = volumeType == VolumeType::OIL || volumeType == VolumeType::HYDROCARBON;
    bool needsValidGas = volumeType == VolumeType::GAS || volumeType == VolumeType::HYDROCARBON;
    // For hydrocarbon it is enough that one of the saturations is above the limit.
    if ( needsValidOil && data.saturationOil[idx.value()] >= limits.saturationOil ) return true;
    if ( needsValidGas && data.saturationGas[idx.value()] >= limits.saturationGas ) return true;
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMappingTools::assignClusterIdToCells( const RigActiveCellInfo&               activeCellInfo,
                                                        const std::vector<ReservoirCellIndex>& cells,
                                                        std::vector<int>&                      clusters,
                                                        int                                    clusterId )
{
    for ( ReservoirCellIndex reservoirCellIdx : cells )
    {
        ActiveCellIndex resultIndex = activeCellInfo.cellResultIndex( reservoirCellIdx );
        if ( resultIndex.value() != cvf::UNDEFINED_SIZE_T ) clusters[resultIndex.value()] = clusterId;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<std::pair<std::pair<ReservoirCellIndex, RigWellTargetMappingTools::CellFaceType>, size_t>>
    RigWellTargetMappingTools::nncConnectionCellAndResult( ReservoirCellIndex cellIdx, RigMainGrid* mainGrid )
{
    std::list<std::pair<std::pair<ReservoirCellIndex, CellFaceType>, size_t>> foundCells;

    if ( mainGrid->nncData() == nullptr ) return foundCells;

    auto& connections = mainGrid->nncData()->allConnections();
    for ( size_t i = 0; i < connections.size(); i++ )
    {
        if ( connections[i].c1GlobIdx() == cellIdx.value() )
        {
            foundCells.push_back(
                std::make_pair( std::make_pair( ReservoirCellIndex( connections[i].c2GlobIdx() ), connections[i].face() ), i ) );
        }
    }

    return foundCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMappingTools::createDynamicResultEntry( RigCaseCellResultsData* resultsData, const RigEclipseResultAddress& address )
{
    if ( !resultsData->hasResultEntry( address ) )
    {
        resultsData->createResultEntry( address, false );

        RigEclipseResultAddress addrToMaxTimeStepCountResult;
        resultsData->maxTimeStepCount( &addrToMaxTimeStepCountResult );
        const std::vector<RigEclipseTimeStepInfo> timeStepInfos = resultsData->timeStepInfos( addrToMaxTimeStepCountResult );
        resultsData->setTimeStepInfos( address, timeStepInfos );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMappingTools::createResultVector( RimEclipseCase&         eclipseCase,
                                                    const QString&          resultName,
                                                    const std::vector<int>& clusterIds,
                                                    size_t                  timeStepIdx )
{
    RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::GENERATED, RiaDefines::ResultDataType::INTEGER, resultName );

    auto resultsData = eclipseCase.results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    createDynamicResultEntry( resultsData, resultAddress );

    std::vector<double>* resultVector = resultsData->modifiableCellScalarResult( resultAddress, timeStepIdx );
    resultVector->resize( clusterIds.size(), std::numeric_limits<double>::infinity() );

    std::fill( resultVector->begin(), resultVector->end(), std::numeric_limits<double>::infinity() );

    for ( size_t idx = 0; idx < clusterIds.size(); idx++ )
    {
        if ( clusterIds[idx] > 0 )
        {
            resultVector->at( idx ) = 1.0 * clusterIds[idx];
        }
    }

    std::set<int> uniqueClusterIds( clusterIds.begin(), clusterIds.end() );
    uniqueClusterIds.erase( 0 ); // Remove unassigned cluster id
    resultsData->recalculateStatistics( resultAddress, uniqueClusterIds );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMappingTools::createResultVector( RimEclipseCase&            eclipseCase,
                                                    const QString&             resultName,
                                                    const std::vector<double>& values,
                                                    size_t                     timeStepIdx )
{
    RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::GENERATED, resultName );

    auto resultsData = eclipseCase.results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    createDynamicResultEntry( resultsData, resultAddress );

    auto resultVector = resultsData->modifiableCellScalarResult( resultAddress, timeStepIdx );
    resultVector->resize( values.size(), std::numeric_limits<double>::infinity() );
    std::copy( values.begin(), values.end(), resultVector->begin() );

    resultsData->recalculateStatistics( resultAddress );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMappingTools::createStaticResultVector( RimEclipseCase& eclipseCase, const QString& resultName, const std::vector<int>& intValues )
{
    RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::GENERATED, resultName );

    auto resultsData = eclipseCase.results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    resultsData->addStaticScalarResult( RiaDefines::ResultCatType::GENERATED, resultName, false, intValues.size() );

    std::vector<double>* resultVector = resultsData->modifiableCellScalarResult( resultAddress, 0 );
    resultVector->resize( intValues.size(), std::numeric_limits<double>::infinity() );

    std::fill( resultVector->begin(), resultVector->end(), std::numeric_limits<double>::infinity() );

    for ( size_t idx = 0; idx < intValues.size(); idx++ )
    {
        if ( intValues[idx] > 0 )
        {
            resultVector->at( idx ) = intValues[idx];
        }
    }

    resultsData->recalculateStatistics( resultAddress );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMappingTools::createStaticResultVector( RimEclipseCase& eclipseCase, const QString& resultName, const std::vector<double>& values )
{
    RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::GENERATED, resultName );

    auto resultsData = eclipseCase.results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    resultsData->addStaticScalarResult( RiaDefines::ResultCatType::GENERATED, resultName, false, values.size() );

    std::vector<double>* resultVector = resultsData->modifiableCellScalarResult( resultAddress, 0 );
    resultVector->resize( values.size(), std::numeric_limits<double>::infinity() );

    std::copy( values.begin(), values.end(), resultVector->begin() );

    resultsData->recalculateStatistics( resultAddress );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMappingTools::createResultVectorIfDefined( RimEclipseCase&            eclipseCase,
                                                             const QString&             resultName,
                                                             const std::vector<double>& values,
                                                             int                        timeStepIdx )
{
    // Avoid creating the result vector if all values are inf/nan
    if ( std::all_of( values.begin(), values.end(), []( auto v ) { return std::isinf( v ) || std::isnan( v ); } ) ) return;

    if ( timeStepIdx < 0 )
    {
        createStaticResultVector( eclipseCase, resultName, values );
    }
    else
    {
        createResultVector( eclipseCase, resultName, values, timeStepIdx );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<caf::VecIjk0> RigWellTargetMappingTools::findStartCell( RimEclipseCase*            eclipseCase,
                                                                      size_t                     timeStepIdx,
                                                                      VolumeType                 volumeType,
                                                                      const ClusteringLimits&    limits,
                                                                      const DataContainer&       data,
                                                                      const std::vector<double>& filterVector,
                                                                      const std::vector<int>&    clusters )
{
    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData )
    {
        RiaLogging::error( "No results data found for eclipse case" );
        return {};
    }

    std::optional<ReservoirCellIndex> startCell;
    double                            maxVolume         = -std::numeric_limits<double>::max();
    const size_t                      numReservoirCells = resultsData->activeCellInfo()->reservoirCellCount();
    for ( ReservoirCellIndex reservoirCellIdx( 0 ); reservoirCellIdx.value() < numReservoirCells; ++reservoirCellIdx )
    {
        ActiveCellIndex resultIndex = resultsData->activeCellInfo()->cellResultIndex( reservoirCellIdx );
        if ( resultIndex.value() != cvf::UNDEFINED_SIZE_T && clusters[resultIndex.value()] == 0 )
        {
            const double cellVolume   = data.volume[resultIndex.value()];
            const double cellPressure = data.pressure[resultIndex.value()];

            const bool isSaturationValid = isSaturationSufficient( volumeType, data, limits, resultIndex );

            const double cellPermeabiltyX  = data.permeabilityX[resultIndex.value()];
            const bool   permeabilityValid = ( cellPermeabiltyX >= limits.permeability );

            const bool filterValue = !std::isinf( filterVector[resultIndex.value()] ) && filterVector[resultIndex.value()] > 0.0;

            if ( cellVolume > maxVolume && cellPressure >= limits.pressure && permeabilityValid && filterValue && isSaturationValid )
            {
                maxVolume = cellVolume;
                startCell = ReservoirCellIndex( reservoirCellIdx );
            }
        }
    }

    if ( !startCell ) return {};

    return eclipseCase->mainGrid()->ijkFromCellIndex( startCell->value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMappingTools::growCluster( RimEclipseCase*            eclipseCase,
                                             const caf::VecIjk0&        startCell,
                                             VolumeType                 volumeType,
                                             const ClusteringLimits&    limits,
                                             const DataContainer&       data,
                                             const std::vector<double>& filterVector,
                                             std::vector<int>&          clusters,
                                             int                        clusterId,
                                             size_t                     timeStepIdx,
                                             int                        maxIterations )
{
    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    // Initially only the start cell is found
    ReservoirCellIndex reservoirCellIdx( eclipseCase->mainGrid()->cellIndexFromIJK( startCell.i(), startCell.j(), startCell.k() ) );
    std::vector<ReservoirCellIndex> foundCells = { reservoirCellIdx };
    assignClusterIdToCells( *resultsData->activeCellInfo(), foundCells, clusters, clusterId );

    for ( int i = 0; i < maxIterations; i++ )
    {
        foundCells = findCandidates( eclipseCase, foundCells, volumeType, limits, data, filterVector, clusters );
        if ( foundCells.empty() ) break;
        assignClusterIdToCells( *resultsData->activeCellInfo(), foundCells, clusters, clusterId );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<ReservoirCellIndex> RigWellTargetMappingTools::findCandidates( RimEclipseCase*                        eclipseCase,
                                                                           const std::vector<ReservoirCellIndex>& previousCells,
                                                                           VolumeType                             volumeType,
                                                                           const ClusteringLimits&                limits,
                                                                           const DataContainer&                   data,
                                                                           const std::vector<double>&             filterVector,
                                                                           std::vector<int>&                      clusters )
{
    std::vector<ReservoirCellIndex> candidates;
    auto                            resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    auto                            mainGrid    = eclipseCase->eclipseCaseData()->mainGrid();

    const std::vector<CellFaceType> faces = {
        cvf::StructGridInterface::FaceType::POS_I,
        cvf::StructGridInterface::FaceType::NEG_I,
        cvf::StructGridInterface::FaceType::POS_J,
        cvf::StructGridInterface::FaceType::NEG_J,
        cvf::StructGridInterface::FaceType::POS_K,
        cvf::StructGridInterface::FaceType::NEG_K,
    };

    for ( ReservoirCellIndex cellIdx : previousCells )
    {
        const RigCell& cell = mainGrid->cell( cellIdx.value() );
        if ( cell.isInvalid() ) continue;

        RigGridBase*    grid               = cell.hostGrid();
        size_t          gridLocalCellIndex = cell.gridLocalCellIndex();
        ActiveCellIndex resultIndex        = resultsData->activeCellInfo()->cellResultIndex( cellIdx );

        size_t i, j, k;

        grid->ijkFromCellIndex( gridLocalCellIndex, &i, &j, &k );

        for ( CellFaceType face : faces )
        {
            size_t gridLocalNeighborCellIdx;
            if ( grid->cellIJKNeighbor( i, j, k, face, &gridLocalNeighborCellIdx ) )
            {
                ReservoirCellIndex neighborResvCellIdx( grid->reservoirCellIndex( gridLocalNeighborCellIdx ) );
                ActiveCellIndex    neighborResultIndex = resultsData->activeCellInfo()->cellResultIndex( neighborResvCellIdx );
                if ( neighborResultIndex.value() != cvf::UNDEFINED_SIZE_T && clusters[neighborResultIndex.value()] == 0 )
                {
                    double permeability     = data.permeabilityX[neighborResultIndex.value()];
                    double transmissibility = getTransmissibilityValueForFace( data.transmissibilityX,
                                                                               data.transmissibilityY,
                                                                               data.transmissibilityZ,
                                                                               face,
                                                                               resultIndex,
                                                                               neighborResultIndex );

                    bool filterValue = !std::isinf( filterVector[neighborResultIndex.value()] ) &&
                                       filterVector[neighborResultIndex.value()] > 0.0;

                    const bool isSaturationValid = isSaturationSufficient( volumeType, data, limits, neighborResultIndex );

                    if ( data.pressure[neighborResultIndex.value()] > limits.pressure && permeability > limits.permeability &&
                         transmissibility > limits.transmissibility && filterValue && isSaturationValid )
                    {
                        candidates.push_back( neighborResvCellIdx );
                        clusters[neighborResultIndex.value()] = -1;
                    }
                }
            }
        }

        if ( data.transmissibilityNNC != nullptr )
        {
            auto nncCells = nncConnectionCellAndResult( cellIdx, mainGrid );
            for ( auto& [cellInfo, nncResultIdx] : nncCells )
            {
                auto& [otherCellIdx, face] = cellInfo;
                double transmissibility    = data.transmissibilityNNC->at( nncResultIdx );

                ActiveCellIndex otherResultIndex = resultsData->activeCellInfo()->cellResultIndex( otherCellIdx );

                double permeability = data.permeabilityX[otherResultIndex.value()];

                bool filterValue = !std::isinf( filterVector[otherResultIndex.value()] ) && filterVector[otherResultIndex.value()] > 0.0;

                const bool isSaturationValid = isSaturationSufficient( volumeType, data, limits, otherResultIndex );

                if ( data.pressure[otherResultIndex.value()] > limits.pressure && permeability > limits.permeability &&
                     transmissibility > limits.transmissibility && filterValue && isSaturationValid )
                {
                    candidates.push_back( otherCellIdx );
                    clusters[otherResultIndex.value()] = -1;
                }
            }
        }
    }

    return candidates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<size_t> RigWellTargetMappingTools::getActiveCellCount( RimEclipseCase* eclipseCase )
{
    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData ) return {};

    return resultsData->activeCellInfo()->reservoirActiveCellCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>
    RigWellTargetMappingTools::loadVectorByName( RigCaseCellResultsData& resultsData, const QString& resultName, size_t timeStepIdx )
{
    RigEclipseResultAddress address( RiaDefines::ResultCatType::DYNAMIC_NATIVE, resultName );
    if ( !resultsData.ensureKnownResultLoaded( address ) ) return {};
    return resultsData.cellScalarResults( address, timeStepIdx );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellTargetMappingTools::loadOilVectorByName( RigCaseCellResultsData&    resultsData,
                                                                    VolumesType                volumesType,
                                                                    VolumeResultType           volumeResultType,
                                                                    size_t                     timeStepIdx,
                                                                    const RigFloodingSettings& floodingSettings )
{
    std::vector<double> volume = loadVectorByName( resultsData, getOilVectorName( volumesType ), timeStepIdx );
    if ( volumeResultType == VolumeResultType::MOBILE )
    {
        std::vector<double> residualOil = RigHydrocarbonFlowTools::residualOilData( resultsData,
                                                                                    RigHydrocarbonFlowTools::ResultType::MOBILE_OIL,
                                                                                    floodingSettings,
                                                                                    volume.size() );
        if ( volumesType == VolumesType::RESERVOIR_VOLUMES_COMPUTED )
        {
            const std::vector<double>& porvResults =
                resultsData.cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::porv() ), 0 );

            for ( size_t i = 0; i < volume.size(); i++ )
            {
                volume[i] = std::max( volume[i] - ( porvResults[i] * residualOil[i] ), 0.0 );
            }
        }
        else
        {
            const std::vector<double>& soilResults =
                resultsData.cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::soil() ),
                                               timeStepIdx );

            for ( size_t i = 0; i < volume.size(); i++ )
            {
                if ( soilResults[i] != 0.0 )
                {
                    volume[i] = std::max( volume[i] * ( soilResults[i] - residualOil[i] ) / soilResults[i], 0.0 );
                }
                else
                {
                    volume[i] = 0.0;
                }
            }
        }
    }

    return volume;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellTargetMappingTools::loadGasVectorByName( RigCaseCellResultsData&       resultsData,
                                                                    RiaDefines::EclipseUnitSystem unitsType,
                                                                    VolumesType                   volumesType,
                                                                    VolumeResultType              volumeResultType,
                                                                    size_t                        timeStepIdx,
                                                                    const RigFloodingSettings&    floodingSettings )
{
    std::vector<double> volume = loadVectorByName( resultsData, getGasVectorName( volumesType ), timeStepIdx );

    if ( volumeResultType == VolumeResultType::MOBILE )
    {
        std::vector<double> residualGas = RigHydrocarbonFlowTools::residualGasData( resultsData,
                                                                                    RigHydrocarbonFlowTools::ResultType::MOBILE_GAS,
                                                                                    floodingSettings,
                                                                                    volume.size() );
        if ( volumesType == VolumesType::RESERVOIR_VOLUMES_COMPUTED )
        {
            const std::vector<double>& porvResults =
                resultsData.cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::porv() ), 0 );

            for ( size_t i = 0; i < volume.size(); i++ )
            {
                volume[i] = std::max( volume[i] - ( porvResults[i] * residualGas[i] ), 0.0 );
            }
        }
        else
        {
            const std::vector<double>& sgasResults =
                resultsData.cellScalarResults( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() ),
                                               timeStepIdx );

            for ( size_t i = 0; i < volume.size(); i++ )
            {
                if ( sgasResults[i] != 0.0 )
                {
                    volume[i] = std::max( volume[i] * ( sgasResults[i] - residualGas[i] ) / sgasResults[i], 0.0 );
                }
                else
                {
                    volume[i] = 0.0;
                }
            }
        }
    }

    // Convert gas volumes to oil equivalents
    for ( size_t i = 0; i < volume.size(); i++ )
    {
        volume[i] = RiaEclipseUnitTools::convertSurfaceGasFlowRateToOilEquivalents( unitsType, volume[i] );
    }

    return volume;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigWellTargetMappingTools::ClusterStatistics> RigWellTargetMappingTools::generateStatistics( RimEclipseCase* eclipseCase,
                                                                                                         const std::vector<double>& pressure,
                                                                                                         const std::vector<double>& permeabilityX,
                                                                                                         int            numClustersFound,
                                                                                                         size_t         timeStepIdx,
                                                                                                         const QString& clusterResultName )
{
    std::vector<ClusterStatistics> statistics( numClustersFound );

    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData ) return statistics;

    auto loadData = []( RigCaseCellResultsData* resultsData, RiaDefines::ResultCatType categoryType, const QString& name, size_t timeStepIdx )
    {
        RigEclipseResultAddress address( categoryType, name );
        std::vector<double>     values;
        if ( resultsData->ensureKnownResultLoaded( address ) )
        {
            values = resultsData->cellScalarResults( address, timeStepIdx );
        }

        return values;
    };

    const std::vector<double> porv = loadData( resultsData, RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::porv(), 0 );
    const std::vector<double> porvSoil =
        loadData( resultsData, RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::riPorvSoil(), timeStepIdx );
    const std::vector<double> porvSgas =
        loadData( resultsData, RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::riPorvSgas(), timeStepIdx );
    const std::vector<double> porvSoilAndSgas =
        loadData( resultsData, RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::riPorvSoilSgas(), timeStepIdx );
    const std::vector<double> fipOil  = loadData( resultsData, RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FIPOIL", timeStepIdx );
    const std::vector<double> fipGas  = loadData( resultsData, RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FIPGAS", timeStepIdx );
    const std::vector<double> sfipOil = loadData( resultsData, RiaDefines::ResultCatType::DYNAMIC_NATIVE, "SFIPOIL", timeStepIdx );
    const std::vector<double> sfipGas = loadData( resultsData, RiaDefines::ResultCatType::DYNAMIC_NATIVE, "SFIPGAS", timeStepIdx );
    const std::vector<double> rfipOil = loadData( resultsData, RiaDefines::ResultCatType::DYNAMIC_NATIVE, "RFIPOIL", timeStepIdx );
    const std::vector<double> rfipGas = loadData( resultsData, RiaDefines::ResultCatType::DYNAMIC_NATIVE, "RFIPGAS", timeStepIdx );

    RigEclipseResultAddress clusterAddress( RiaDefines::ResultCatType::GENERATED, clusterResultName );
    resultsData->ensureKnownResultLoaded( clusterAddress );
    const std::vector<double>& clusterIds = resultsData->cellScalarResults( clusterAddress, timeStepIdx );

    std::vector<RiaWeightedMeanCalculator<double>> permeabilityCalculators( numClustersFound );
    std::vector<RiaWeightedMeanCalculator<double>> pressureCalculators( numClustersFound );

    for ( size_t idx = 0; idx < clusterIds.size(); idx++ )
    {
        if ( !std::isinf( clusterIds[idx] ) && static_cast<int>( clusterIds[idx] ) > 0 )
        {
            size_t i = clusterIds[idx] - 1;
            if ( i < static_cast<size_t>( numClustersFound ) )
            {
                statistics[i].id = clusterIds[idx];
                statistics[i].numCells++;
                if ( idx < porvSoil.size() ) statistics[i].totalPorvSoil += porvSoil[idx];
                if ( idx < porvSgas.size() ) statistics[i].totalPorvSgas += porvSgas[idx];
                if ( idx < porvSoilAndSgas.size() ) statistics[i].totalPorvSoilAndSgas += porvSoilAndSgas[idx];

                if ( idx < fipOil.size() ) statistics[i].totalFipOil += fipOil[idx];
                if ( idx < fipGas.size() ) statistics[i].totalFipGas += fipGas[idx];

                if ( idx < rfipOil.size() ) statistics[i].totalRfipOil += rfipOil[idx];
                if ( idx < rfipGas.size() ) statistics[i].totalRfipGas += rfipGas[idx];

                if ( idx < sfipOil.size() ) statistics[i].totalSfipOil += sfipOil[idx];
                if ( idx < sfipGas.size() ) statistics[i].totalSfipGas += sfipGas[idx];

                permeabilityCalculators[i].addValueAndWeight( permeabilityX[idx], porv[idx] );

                pressureCalculators[i].addValueAndWeight( pressure[idx], porv[idx] );
            }
        }
    }

    for ( int i = 0; i < numClustersFound; i++ )
    {
        statistics[i].permeability = permeabilityCalculators[i].weightedMean();
        statistics[i].pressure     = pressureCalculators[i].weightedMean();

        // Invalidate results for empty vectors
        if ( porvSoil.empty() ) statistics[i].totalPorvSoil = std::numeric_limits<double>::infinity();
        if ( porvSgas.empty() ) statistics[i].totalPorvSgas = std::numeric_limits<double>::infinity();
        if ( porvSoilAndSgas.empty() ) statistics[i].totalPorvSoilAndSgas = std::numeric_limits<double>::infinity();
        if ( fipOil.empty() ) statistics[i].totalFipOil = std::numeric_limits<double>::infinity();
        if ( fipGas.empty() ) statistics[i].totalFipGas = std::numeric_limits<double>::infinity();
        if ( rfipOil.empty() ) statistics[i].totalRfipOil = std::numeric_limits<double>::infinity();
        if ( rfipGas.empty() ) statistics[i].totalRfipGas = std::numeric_limits<double>::infinity();
        if ( sfipOil.empty() ) statistics[i].totalSfipOil = std::numeric_limits<double>::infinity();
        if ( sfipGas.empty() ) statistics[i].totalSfipGas = std::numeric_limits<double>::infinity();
    }

    return statistics;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMappingTools::computeStatisticsAndCreateVectors( RimEclipseCase&                         targetCase,
                                                                   const QString&                          resultName,
                                                                   const std::vector<std::vector<double>>& vec )
{
    const RigCaseCellResultsData* targetResultsData = targetCase.results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !targetResultsData ) return;

    const RigActiveCellInfo* targetActiveCellInfo = targetResultsData->activeCellInfo();
    if ( !targetActiveCellInfo ) return;

    const size_t targetNumActiveCells = targetActiveCellInfo->reservoirActiveCellCount();

    int                 nCells = static_cast<int>( targetNumActiveCells );
    std::vector<double> p10Results( nCells, std::numeric_limits<double>::infinity() );
    std::vector<double> p50Results( nCells, std::numeric_limits<double>::infinity() );
    std::vector<double> p90Results( nCells, std::numeric_limits<double>::infinity() );
    std::vector<double> meanResults( nCells, std::numeric_limits<double>::infinity() );
    std::vector<double> minResults( nCells, std::numeric_limits<double>::infinity() );
    std::vector<double> maxResults( nCells, std::numeric_limits<double>::infinity() );

#pragma omp parallel for
    for ( int i = 0; i < nCells; i++ )
    {
        size_t              numSamples = vec.size();
        std::vector<double> samples( numSamples, 0.0 );
        for ( size_t s = 0; s < numSamples; s++ )
            samples[s] = vec[s][i];

        double p10, p50, p90, mean;
        RigStatisticsMath::calculateStatisticsCurves( samples, &p10, &p50, &p90, &mean, RigStatisticsMath::PercentileStyle::SWITCHED );

        if ( RigStatisticsTools::isValidNumber( p10 ) ) p10Results[i] = p10;
        if ( RigStatisticsTools::isValidNumber( p50 ) ) p50Results[i] = p50;
        if ( RigStatisticsTools::isValidNumber( p90 ) ) p90Results[i] = p90;
        if ( RigStatisticsTools::isValidNumber( mean ) ) meanResults[i] = mean;

        double minValue = RigStatisticsTools::minimumValue( samples );
        if ( RigStatisticsTools::isValidNumber( minValue ) && minValue < std::numeric_limits<double>::max() ) minResults[i] = minValue;

        double maxValue = RigStatisticsTools::maximumValue( samples );
        if ( RigStatisticsTools::isValidNumber( maxValue ) && maxValue > -std::numeric_limits<double>::max() ) maxResults[i] = maxValue;
    }

    createResultVectorIfDefined( targetCase, resultName + "_P10", p10Results );
    createResultVectorIfDefined( targetCase, resultName + "_P50", p50Results );
    createResultVectorIfDefined( targetCase, resultName + "_P90", p90Results );
    createResultVectorIfDefined( targetCase, resultName + "_MEAN", meanResults );
    createResultVectorIfDefined( targetCase, resultName + "_MIN", minResults );
    createResultVectorIfDefined( targetCase, resultName + "_MAX", maxResults );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMappingTools::accumulateResultsForSingleCase( RimEclipseCase&                                      eclipseCase,
                                                                RimEclipseCase&                                      targetCase,
                                                                std::map<QString, std::vector<std::vector<double>>>& resultNamesAndSamples,
                                                                std::vector<int>&                                    occupancy,
                                                                size_t                                               timeStepIdx )
{
    RigCaseCellResultsData* resultsData = eclipseCase.results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData ) return;
    const RigMainGrid* mainGrid = eclipseCase.mainGrid();
    if ( !mainGrid ) return;
    const RigActiveCellInfo* activeCellInfo = resultsData->activeCellInfo();
    if ( !activeCellInfo ) return;

    const RigCaseCellResultsData* targetResultsData = targetCase.results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    const RigActiveCellInfo* targetActiveCellInfo = targetResultsData->activeCellInfo();

    const size_t targetNumReservoirCells = targetActiveCellInfo->reservoirCellCount();
    const size_t targetNumActiveCells    = targetActiveCellInfo->reservoirActiveCellCount();

    occupancy.resize( targetNumActiveCells, 0 );

    RigEclipseResultAddress clustersNumAddress( RiaDefines::ResultCatType::GENERATED, RigWellTargetMapping::wellTargetResultName() );
    resultsData->ensureKnownResultLoaded( clustersNumAddress );
    const std::vector<double>& clusterNum = resultsData->cellScalarResults( clustersNumAddress, timeStepIdx );

    std::map<QString, const std::vector<double>*> namedInputVector;

    for ( const auto& [resultName, vec] : resultNamesAndSamples )
    {
        RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::GENERATED, resultName );
        resultsData->ensureKnownResultLoaded( resultAddress );
        const std::vector<double>& resultVector = resultsData->cellScalarResults( resultAddress, timeStepIdx );
        namedInputVector[resultName]            = &resultVector;
    }

    std::map<QString, std::vector<double>> namedOutputVector;
    for ( const auto& [resultName, vec] : resultNamesAndSamples )
    {
        namedOutputVector[resultName] = std::vector( targetNumActiveCells, std::numeric_limits<double>::infinity() );
    }

    for ( ReservoirCellIndex targetCellIdx( 0 ); targetCellIdx.value() < targetNumReservoirCells; ++targetCellIdx )
    {
        const RigCell& nativeCell = targetCase.mainGrid()->cell( targetCellIdx.value() );
        cvf::Vec3d     cellCenter = nativeCell.center();

        ActiveCellIndex targetResultIndex = targetActiveCellInfo->cellResultIndex( targetCellIdx );

        ReservoirCellIndex cellIdx( mainGrid->findReservoirCellIndexFromPoint( cellCenter ) );
        if ( cellIdx.value() != cvf::UNDEFINED_SIZE_T && activeCellInfo->isActive( cellIdx ) &&
             targetResultIndex.value() != cvf::UNDEFINED_SIZE_T )
        {
            ActiveCellIndex resultIndex = resultsData->activeCellInfo()->cellResultIndex( cellIdx );
            if ( !std::isinf( clusterNum[resultIndex.value()] ) && clusterNum[resultIndex.value()] > 0 )
            {
                occupancy[targetResultIndex.value()]++;
                for ( const auto& [resultName, vec] : resultNamesAndSamples )
                {
                    namedOutputVector[resultName][targetResultIndex.value()] = namedInputVector[resultName]->at( resultIndex.value() );
                }
            }
        }
    }

    for ( const auto& [resultName, vec] : resultNamesAndSamples )
    {
        resultNamesAndSamples[resultName].push_back( namedOutputVector[resultName] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox
    RigWellTargetMappingTools::computeBoundingBoxForResult( RimEclipseCase& eclipseCase, const QString& resultName, size_t timeStepIndex )
{
    RigCaseCellResultsData*  resultsData       = eclipseCase.results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    const RigMainGrid*       mainGrid          = eclipseCase.mainGrid();
    const RigActiveCellInfo* activeCellInfo    = resultsData->activeCellInfo();
    const size_t             numReservoirCells = activeCellInfo->reservoirCellCount();

    RigEclipseResultAddress clustersNumAddress( RiaDefines::ResultCatType::GENERATED, resultName );
    resultsData->ensureKnownResultLoaded( clustersNumAddress );
    const std::vector<double>& clusterNum = resultsData->cellScalarResults( clustersNumAddress, timeStepIndex );

    cvf::BoundingBox boundingBox;
    for ( ReservoirCellIndex reservoirCellIdx( 0 ); reservoirCellIdx.value() < numReservoirCells; ++reservoirCellIdx )
    {
        ActiveCellIndex targetResultIndex = activeCellInfo->cellResultIndex( reservoirCellIdx );
        if ( activeCellInfo->isActive( reservoirCellIdx ) && targetResultIndex.value() != cvf::UNDEFINED_SIZE_T &&
             !std::isinf( clusterNum[targetResultIndex.value()] ) && clusterNum[targetResultIndex.value()] > 0 )
        {
            const RigCell& nativeCell = mainGrid->cell( reservoirCellIdx.value() );
            boundingBox.add( nativeCell.boundingBox() );
        }
    }

    return boundingBox;
}

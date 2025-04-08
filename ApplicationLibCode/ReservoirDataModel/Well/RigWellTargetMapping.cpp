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

#include "RigWellTargetMapping.h"

#include "RiaDefines.h"
#include "RiaEclipseUnitTools.h"
#include "RiaLogging.h"
#include "RiaPorosityModel.h"
#include "RiaResultNames.h"
#include "RiaWeightedMeanCalculator.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigFloodingSettings.h"
#include "RigHydrocarbonFlowTools.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "RigStatisticsMath.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimEclipseView.h"
#include "RimProject.h"
#include "RimPropertyFilterCollection.h"
#include "RimRegularGridCase.h"
#include "RimTools.h"

#include "cafProgressInfo.h"
#include "cafVecIjk.h"

#include "cvfBoundingBox.h"
#include "cvfMath.h"
#include "cvfStructGrid.h"

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMapping::generateCandidates( RimEclipseCase*            eclipseCase,
                                               size_t                     timeStepIdx,
                                               VolumeType                 volumeType,
                                               VolumesType                volumesType,
                                               VolumeResultType           volumeResultType,
                                               const RigFloodingSettings& floodingSettings,
                                               const ClusteringLimits&    limits,
                                               bool                       skipUndefinedResults )
{
    if ( !eclipseCase->ensureReservoirCaseIsOpen() ) return;

    auto activeCellCount = getActiveCellCount( eclipseCase );
    if ( !activeCellCount )
    {
        RiaLogging::error( "No active cells found" );
        return;
    }

    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData ) return;

    auto caseData = eclipseCase->eclipseCaseData();
    if ( !caseData ) return;

    RiaDefines::EclipseUnitSystem unitsType = caseData->unitsType();

    DataContainer data;
    data.volume = getVolumeVector( *resultsData, unitsType, volumeType, volumesType, volumeResultType, timeStepIdx, floodingSettings );
    if ( data.volume.empty() )
    {
        RiaLogging::error( "Unable to produce volume vector." );
        return;
    }

    RigEclipseResultAddress soilAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::soil() );
    if ( resultsData->ensureKnownResultLoaded( soilAddress ) )
    {
        data.saturationOil = resultsData->cellScalarResults( soilAddress, timeStepIdx );
    }

    RigEclipseResultAddress sgasAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() );
    if ( resultsData->ensureKnownResultLoaded( sgasAddress ) )
    {
        data.saturationGas = resultsData->cellScalarResults( sgasAddress, timeStepIdx );
    }
    else
    {
        if ( volumeType == VolumeType::GAS || volumeType == VolumeType::HYDROCARBON )
        {
            RiaLogging::error( "Missing SGAS result needed for well target mapping volume type: " +
                               caf::AppEnum<VolumeType>::uiText( volumeType ) );
            return;
        }
    }

    RigEclipseResultAddress pressureAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "PRESSURE" );
    resultsData->ensureKnownResultLoaded( pressureAddress );
    data.pressure = resultsData->cellScalarResults( pressureAddress, timeStepIdx );

    RigEclipseResultAddress permeabilityXAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMX" );
    resultsData->ensureKnownResultLoaded( permeabilityXAddress );
    data.permeabilityX = resultsData->cellScalarResults( permeabilityXAddress, 0 );

    RigEclipseResultAddress transmissibilityXAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANX" );
    resultsData->ensureKnownResultLoaded( transmissibilityXAddress );
    data.transmissibilityX = resultsData->cellScalarResults( transmissibilityXAddress, 0 );

    RigEclipseResultAddress transmissibilityYAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANY" );
    resultsData->ensureKnownResultLoaded( transmissibilityYAddress );
    data.transmissibilityY = resultsData->cellScalarResults( transmissibilityYAddress, 0 );

    RigEclipseResultAddress transmissibilityZAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANZ" );
    resultsData->ensureKnownResultLoaded( transmissibilityZAddress );
    data.transmissibilityZ = resultsData->cellScalarResults( transmissibilityZAddress, 0 );

    auto mainGrid = eclipseCase->eclipseCaseData()->mainGrid();
    if ( mainGrid != nullptr )
    {
        data.transmissibilityNNC = mainGrid->nncData()->staticConnectionScalarResultByName( RiaDefines::propertyNameCombTrans() );
    }
    else
    {
        data.transmissibilityNNC = nullptr;
    }

    std::vector<double> filterVector;
    if ( !limits.filter.empty() )
    {
        filterVector = limits.filter;
    }
    else if ( limits.filterAddress.isValid() )
    {
        resultsData->ensureKnownResultLoaded( limits.filterAddress );
        filterVector = resultsData->cellScalarResults( limits.filterAddress, timeStepIdx );
    }

    // Disable filtering if not configured: set all items to 1 (which includes all cells).
    if ( filterVector.empty() || filterVector.size() != data.pressure.size() )
    {
        filterVector = std::vector<double>( data.pressure.size(), 1.0 );
    }

    const QString logKeyword = "RigWellTargetMapping";

    std::vector<int> clusters( activeCellCount.value(), 0 );
    auto             start            = std::chrono::high_resolution_clock::now();
    int              numClusters      = limits.maxNumTargets;
    int              maxIterations    = limits.maxIterations;
    int              numClustersFound = 0;
    for ( int clusterId = 1; clusterId <= numClusters; clusterId++ )
    {
        std::optional<caf::VecIjk> startCell = findStartCell( eclipseCase, timeStepIdx, volumeType, limits, data, filterVector, clusters );

        if ( startCell.has_value() )
        {
            RiaLogging::info( QString( "Cluster %1 start cell: [%2 %3 %4] " )
                                  .arg( clusterId )
                                  .arg( startCell->i() + 1 )
                                  .arg( startCell->j() + 1 )
                                  .arg( startCell->k() + 1 ),
                              logKeyword );

            growCluster( eclipseCase, startCell.value(), volumeType, limits, data, filterVector, clusters, clusterId, timeStepIdx, maxIterations );
            numClustersFound++;
        }
        else
        {
            RiaLogging::error( "No suitable starting cell found" );
            break;
        }
    }

    RiaLogging::info( QString( "Found %1 clusters." ).arg( numClustersFound ) );

    auto finish = std::chrono::high_resolution_clock::now();

    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>( finish - start );
    RiaLogging::info( QString( "Time spent: %1 ms" ).arg( milliseconds.count() ), logKeyword );

    QString resultName = RigWellTargetMapping::wellTargetResultName();
    createResultVector( *eclipseCase, resultName, clusters );

    // Update views and property filters
    RimProject* proj = RimProject::current();
    proj->scheduleCreateDisplayModelAndRedrawAllViews();
    for ( auto view : eclipseCase->reservoirViews() )
    {
        if ( auto eclipseView = dynamic_cast<RimEclipseView*>( view ) )
        {
            eclipseView->scheduleReservoirGridGeometryRegen();
            eclipseView->propertyFilterCollection()->updateConnectedEditors();
        }
    }

    std::vector<ClusterStatistics> statistics =
        generateStatistics( eclipseCase, data.pressure, data.permeabilityX, numClustersFound, timeStepIdx, resultName );
    std::vector<double> totalPorvSoil( clusters.size(), std::numeric_limits<double>::infinity() );
    std::vector<double> totalPorvSgas( clusters.size(), std::numeric_limits<double>::infinity() );
    std::vector<double> totalPorvSoilAndSgas( clusters.size(), std::numeric_limits<double>::infinity() );
    std::vector<double> totalFipOil( clusters.size(), std::numeric_limits<double>::infinity() );
    std::vector<double> totalFipGas( clusters.size(), std::numeric_limits<double>::infinity() );
    std::vector<double> totalRfipOil( clusters.size(), std::numeric_limits<double>::infinity() );
    std::vector<double> totalRfipGas( clusters.size(), std::numeric_limits<double>::infinity() );
    std::vector<double> totalSfipOil( clusters.size(), std::numeric_limits<double>::infinity() );
    std::vector<double> totalSfipGas( clusters.size(), std::numeric_limits<double>::infinity() );

    auto addValuesForClusterId = []( std::vector<double>& values, const std::vector<int>& clusters, int clusterId, double value )
    {
#pragma omp parallel for
        for ( int i = 0; i < static_cast<int>( clusters.size() ); i++ )
        {
            if ( clusters[i] == clusterId ) values[i] = value;
        }
    };

    int clusterId = 1;
    for ( const auto& s : statistics )
    {
        auto logIfValid = [&logKeyword]( const QString& pattern, double value )
        {
            if ( !std::isinf( value ) && !std::isnan( value ) ) RiaLogging::info( pattern.arg( value ), logKeyword );
        };

        RiaLogging::info( QString( "Cluster #%1 Statistics" ).arg( s.id ), logKeyword );
        RiaLogging::info( QString( "Number of cells: %1" ).arg( s.numCells ), logKeyword );
        logIfValid( "Total PORV*SOIL: %1", s.totalPorvSoil );
        logIfValid( "Total PORV*SOIL: %1", s.totalPorvSoil );
        logIfValid( "Total PORV*SGAS: %1", s.totalPorvSgas );
        logIfValid( "Total PORV*(SOIL+SGAS): %1", s.totalPorvSoilAndSgas );
        logIfValid( "Total FIPOIL: %1", s.totalFipOil );
        logIfValid( "Total FIPGAS: %1", s.totalFipGas );
        logIfValid( "Total RFIPOIL: %1", s.totalRfipOil );
        logIfValid( "Total RFIPGAS: %1", s.totalRfipGas );
        logIfValid( "Total SFIPOIL: %1", s.totalSfipOil );
        logIfValid( "Total SFIPGAS: %1", s.totalSfipGas );
        logIfValid( "Average Permeability: %1", s.permeability );
        logIfValid( "Average Pressure: %1", s.pressure );

        addValuesForClusterId( totalPorvSoil, clusters, clusterId, s.totalPorvSoil );
        addValuesForClusterId( totalPorvSgas, clusters, clusterId, s.totalPorvSgas );
        addValuesForClusterId( totalPorvSoilAndSgas, clusters, clusterId, s.totalPorvSoilAndSgas );
        addValuesForClusterId( totalFipOil, clusters, clusterId, s.totalFipOil );
        addValuesForClusterId( totalFipGas, clusters, clusterId, s.totalFipGas );
        addValuesForClusterId( totalRfipOil, clusters, clusterId, s.totalRfipOil );
        addValuesForClusterId( totalRfipGas, clusters, clusterId, s.totalRfipGas );
        addValuesForClusterId( totalSfipOil, clusters, clusterId, s.totalSfipOil );
        addValuesForClusterId( totalSfipGas, clusters, clusterId, s.totalSfipGas );

        clusterId++;
    }

    if ( skipUndefinedResults )
    {
        createResultVectorIfDefined( *eclipseCase, "TOTAL_PORV_SOIL", totalPorvSoil );
        createResultVectorIfDefined( *eclipseCase, "TOTAL_PORV_SGAS", totalPorvSgas );
        createResultVectorIfDefined( *eclipseCase, "TOTAL_PORV_SOIL_SGAS", totalPorvSoilAndSgas );
        createResultVectorIfDefined( *eclipseCase, "TOTAL_FIPOIL", totalFipOil );
        createResultVectorIfDefined( *eclipseCase, "TOTAL_FIPGAS", totalFipGas );
        createResultVectorIfDefined( *eclipseCase, "TOTAL_RFIPOIL", totalRfipOil );
        createResultVectorIfDefined( *eclipseCase, "TOTAL_RFIPGAS", totalRfipGas );
        createResultVectorIfDefined( *eclipseCase, "TOTAL_SFIPOIL", totalSfipOil );
        createResultVectorIfDefined( *eclipseCase, "TOTAL_SFIPGAS", totalSfipGas );
    }
    else
    {
        createResultVector( *eclipseCase, "TOTAL_PORV_SOIL", totalPorvSoil );
        createResultVector( *eclipseCase, "TOTAL_PORV_SGAS", totalPorvSgas );
        createResultVector( *eclipseCase, "TOTAL_PORV_SOIL_SGAS", totalPorvSoilAndSgas );
        createResultVector( *eclipseCase, "TOTAL_FIPOIL", totalFipOil );
        createResultVector( *eclipseCase, "TOTAL_FIPGAS", totalFipGas );
        createResultVector( *eclipseCase, "TOTAL_RFIPOIL", totalRfipOil );
        createResultVector( *eclipseCase, "TOTAL_RFIPGAS", totalRfipGas );
        createResultVector( *eclipseCase, "TOTAL_SFIPOIL", totalSfipOil );
        createResultVector( *eclipseCase, "TOTAL_SFIPGAS", totalSfipGas );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<caf::VecIjk> RigWellTargetMapping::findStartCell( RimEclipseCase*            eclipseCase,
                                                                size_t                     timeStepIdx,
                                                                const VolumeType           volumeType,
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

    size_t       startCell         = std::numeric_limits<size_t>::max();
    double       maxVolume         = -std::numeric_limits<double>::max();
    const size_t numReservoirCells = resultsData->activeCellInfo()->reservoirCellCount();
    for ( size_t reservoirCellIdx = 0; reservoirCellIdx < numReservoirCells; reservoirCellIdx++ )
    {
        size_t resultIndex = resultsData->activeCellInfo()->cellResultIndex( reservoirCellIdx );
        if ( resultIndex != cvf::UNDEFINED_SIZE_T && clusters[resultIndex] == 0 )
        {
            const double cellVolume   = data.volume[resultIndex];
            const double cellPressure = data.pressure[resultIndex];

            const bool isSaturationValid = isSaturationSufficient( volumeType, data, limits, resultIndex );

            const double cellPermeabiltyX  = data.permeabilityX[resultIndex];
            const bool   permeabilityValid = ( cellPermeabiltyX >= limits.permeability );

            const bool filterValue = !std::isinf( filterVector[resultIndex] ) && filterVector[resultIndex] > 0.0;

            if ( cellVolume > maxVolume && cellPressure >= limits.pressure && permeabilityValid && filterValue && isSaturationValid )
            {
                maxVolume = cellVolume;
                startCell = reservoirCellIdx;
            }
        }
    }

    if ( startCell == std::numeric_limits<size_t>::max() ) return {};

    return eclipseCase->mainGrid()->ijkFromCellIndex( startCell );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMapping::growCluster( RimEclipseCase*            eclipseCase,
                                        const caf::VecIjk&         startCell,
                                        const VolumeType           volumeType,
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
    size_t              reservoirCellIdx = eclipseCase->mainGrid()->cellIndexFromIJK( startCell.i(), startCell.j(), startCell.k() );
    std::vector<size_t> foundCells       = { reservoirCellIdx };
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
std::vector<size_t> RigWellTargetMapping::findCandidates( RimEclipseCase*            eclipseCase,
                                                          const std::vector<size_t>& previousCells,
                                                          const VolumeType           volumeType,
                                                          const ClusteringLimits&    limits,
                                                          const DataContainer&       data,
                                                          const std::vector<double>& filterVector,
                                                          std::vector<int>&          clusters )
{
    std::vector<size_t> candidates;
    auto                resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    auto                mainGrid    = eclipseCase->eclipseCaseData()->mainGrid();

    const std::vector<CellFaceType> faces = {
        cvf::StructGridInterface::FaceType::POS_I,
        cvf::StructGridInterface::FaceType::NEG_I,
        cvf::StructGridInterface::FaceType::POS_J,
        cvf::StructGridInterface::FaceType::NEG_J,
        cvf::StructGridInterface::FaceType::POS_K,
        cvf::StructGridInterface::FaceType::NEG_K,
    };

    for ( size_t cellIdx : previousCells )
    {
        const RigCell& cell = mainGrid->cell( cellIdx );
        if ( cell.isInvalid() ) continue;

        RigGridBase* grid               = cell.hostGrid();
        size_t       gridLocalCellIndex = cell.gridLocalCellIndex();
        size_t       resultIndex        = resultsData->activeCellInfo()->cellResultIndex( cellIdx );

        size_t i, j, k;

        grid->ijkFromCellIndex( gridLocalCellIndex, &i, &j, &k );

        for ( CellFaceType face : faces )
        {
            size_t gridLocalNeighborCellIdx;
            if ( grid->cellIJKNeighbor( i, j, k, face, &gridLocalNeighborCellIdx ) )
            {
                size_t neighborResvCellIdx = grid->reservoirCellIndex( gridLocalNeighborCellIdx );
                size_t neighborResultIndex = resultsData->activeCellInfo()->cellResultIndex( neighborResvCellIdx );
                if ( neighborResultIndex != cvf::UNDEFINED_SIZE_T && clusters[neighborResultIndex] == 0 )
                {
                    double permeability     = data.permeabilityX[neighborResultIndex];
                    double transmissibility = getTransmissibilityValueForFace( data.transmissibilityX,
                                                                               data.transmissibilityY,
                                                                               data.transmissibilityZ,
                                                                               face,
                                                                               resultIndex,
                                                                               neighborResultIndex );
                    bool   filterValue      = !std::isinf( filterVector[neighborResultIndex] ) && filterVector[neighborResultIndex] > 0.0;

                    const bool isSaturationValid = isSaturationSufficient( volumeType, data, limits, neighborResultIndex );

                    if ( data.pressure[neighborResultIndex] > limits.pressure && permeability > limits.permeability &&
                         transmissibility > limits.transmissibility && filterValue && isSaturationValid )
                    {
                        candidates.push_back( neighborResvCellIdx );
                        clusters[neighborResultIndex] = -1;
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

                size_t otherResultIndex = resultsData->activeCellInfo()->cellResultIndex( otherCellIdx );

                double permeability = data.permeabilityX[otherResultIndex];

                bool filterValue = !std::isinf( filterVector[otherResultIndex] ) && filterVector[otherResultIndex] > 0.0;

                const bool isSaturationValid = isSaturationSufficient( volumeType, data, limits, otherResultIndex );

                if ( data.pressure[otherResultIndex] > limits.pressure && permeability > limits.permeability &&
                     transmissibility > limits.transmissibility && filterValue && isSaturationValid )
                {
                    candidates.push_back( otherCellIdx );
                    clusters[otherResultIndex] = -1;
                }
            }
        }
    }

    return candidates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMapping::assignClusterIdToCells( const RigActiveCellInfo&   activeCellInfo,
                                                   const std::vector<size_t>& cells,
                                                   std::vector<int>&          clusters,
                                                   int                        clusterId )
{
    for ( size_t reservoirCellIdx : cells )
    {
        size_t resultIndex = activeCellInfo.cellResultIndex( reservoirCellIdx );
        if ( resultIndex != cvf::UNDEFINED_SIZE_T ) clusters[resultIndex] = clusterId;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMapping::createResultVector( RimEclipseCase& eclipseCase, const QString& resultName, const std::vector<int>& clusterIds )
{
    RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::GENERATED, resultName );

    auto resultsData = eclipseCase.results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    resultsData->addStaticScalarResult( RiaDefines::ResultCatType::GENERATED, resultName, false, clusterIds.size() );

    std::vector<double>* resultVector = resultsData->modifiableCellScalarResult( resultAddress, 0 );
    resultVector->resize( clusterIds.size(), std::numeric_limits<double>::infinity() );

    std::fill( resultVector->begin(), resultVector->end(), std::numeric_limits<double>::infinity() );

    for ( size_t idx = 0; idx < clusterIds.size(); idx++ )
    {
        if ( clusterIds[idx] > 0 )
        {
            resultVector->at( idx ) = clusterIds[idx];
        }
    }

    resultsData->recalculateStatistics( resultAddress );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMapping::createResultVectorIfDefined( RimEclipseCase& eclipseCase, const QString& resultName, const std::vector<double>& values )
{
    // Avoid creating the result vector if all values are inf/nan
    if ( std::all_of( values.begin(), values.end(), []( auto v ) { return std::isinf( v ) || std::isnan( v ); } ) ) return;

    createResultVector( eclipseCase, resultName, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMapping::createResultVector( RimEclipseCase& eclipseCase, const QString& resultName, const std::vector<double>& values )
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
void RigWellTargetMapping::createResultVector( RimEclipseCase&         eclipseCase,
                                               const QString&          resultName,
                                               const std::vector<int>& clusterIds,
                                               double                  value )
{
    RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::GENERATED, resultName );

    auto resultsData = eclipseCase.results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    resultsData->addStaticScalarResult( RiaDefines::ResultCatType::GENERATED, resultName, false, clusterIds.size() );

    std::vector<double>* resultVector = resultsData->modifiableCellScalarResult( resultAddress, 0 );
    resultVector->resize( clusterIds.size(), std::numeric_limits<double>::infinity() );

    std::fill( resultVector->begin(), resultVector->end(), std::numeric_limits<double>::infinity() );

    for ( size_t idx = 0; idx < clusterIds.size(); idx++ )
    {
        if ( clusterIds[idx] > 0 )
        {
            resultVector->at( idx ) = value;
        }
    }

    resultsData->recalculateStatistics( resultAddress );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<size_t> RigWellTargetMapping::getActiveCellCount( RimEclipseCase* eclipseCase )
{
    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData ) return {};

    return resultsData->activeCellInfo()->reservoirActiveCellCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellTargetMapping::getValueForFace( const std::vector<double>& x,
                                              const std::vector<double>& y,
                                              const std::vector<double>& z,
                                              CellFaceType               face,
                                              size_t                     resultIndex )
{
    if ( face == cvf::StructGridInterface::FaceType::POS_I || face == cvf::StructGridInterface::FaceType::NEG_I ) return x[resultIndex];
    if ( face == cvf::StructGridInterface::FaceType::POS_J || face == cvf::StructGridInterface::FaceType::NEG_J ) return y[resultIndex];
    if ( face == cvf::StructGridInterface::FaceType::POS_K || face == cvf::StructGridInterface::FaceType::NEG_K ) return z[resultIndex];
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellTargetMapping::getTransmissibilityValueForFace( const std::vector<double>& x,
                                                              const std::vector<double>& y,
                                                              const std::vector<double>& z,
                                                              CellFaceType               face,
                                                              size_t                     resultIndex,
                                                              size_t                     neighborResultIndex )
{
    // For negative directions (NEG_I, NEG_J, NEG_K) use the value from the neighbor cell
    bool isPos = face == cvf::StructGridInterface::FaceType::POS_I || face == cvf::StructGridInterface::FaceType::POS_J ||
                 face == cvf::StructGridInterface::FaceType::POS_K;
    size_t index = isPos ? resultIndex : neighborResultIndex;
    return getValueForFace( x, y, z, face, index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellTargetMapping::loadVectorByName( RigCaseCellResultsData& resultsData, const QString& resultName, size_t timeStepIdx )
{
    RigEclipseResultAddress address( RiaDefines::ResultCatType::DYNAMIC_NATIVE, resultName );
    if ( !resultsData.ensureKnownResultLoaded( address ) ) return {};
    return resultsData.cellScalarResults( address, timeStepIdx );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigWellTargetMapping::getOilVectorName( VolumesType volumesType )
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
QString RigWellTargetMapping::getGasVectorName( VolumesType volumesType )
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
std::vector<double> RigWellTargetMapping::loadOilVectorByName( RigCaseCellResultsData&    resultsData,
                                                               VolumesType                volumesType,
                                                               VolumeResultType           volumeResultType,
                                                               size_t                     timeStepIdx,
                                                               const RigFloodingSettings& floodingSettings )
{
    std::vector<double> volume = loadVectorByName( resultsData, getOilVectorName( volumesType ), timeStepIdx );
    if ( volumeResultType == RigWellTargetMapping::VolumeResultType::MOBILE )
    {
        std::vector<double> residualOil = RigHydrocarbonFlowTools::residualOilData( resultsData,
                                                                                    RigHydrocarbonFlowTools::ResultType::MOBILE_OIL,
                                                                                    floodingSettings,
                                                                                    volume.size() );
        if ( volumesType == RigWellTargetMapping::VolumesType::RESERVOIR_VOLUMES_COMPUTED )
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
std::vector<double> RigWellTargetMapping::loadGasVectorByName( RigCaseCellResultsData&       resultsData,
                                                               RiaDefines::EclipseUnitSystem unitsType,
                                                               VolumesType                   volumesType,
                                                               VolumeResultType              volumeResultType,
                                                               size_t                        timeStepIdx,
                                                               const RigFloodingSettings&    floodingSettings )
{
    std::vector<double> volume = loadVectorByName( resultsData, getGasVectorName( volumesType ), timeStepIdx );

    if ( volumeResultType == RigWellTargetMapping::VolumeResultType::MOBILE )
    {
        std::vector<double> residualGas = RigHydrocarbonFlowTools::residualGasData( resultsData,
                                                                                    RigHydrocarbonFlowTools::ResultType::MOBILE_GAS,
                                                                                    floodingSettings,
                                                                                    volume.size() );
        if ( volumesType == RigWellTargetMapping::VolumesType::RESERVOIR_VOLUMES_COMPUTED )
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

    // Convert to gas volumes to oil equivalents
    for ( size_t i = 0; i < volume.size(); i++ )
    {
        volume[i] = RiaEclipseUnitTools::convertSurfaceGasFlowRateToOilEquivalents( unitsType, volume[i] );
    }

    return volume;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellTargetMapping::getVolumeVector( RigCaseCellResultsData&       resultsData,
                                                           RiaDefines::EclipseUnitSystem unitsType,
                                                           VolumeType                    volumeType,
                                                           VolumesType                   volumesType,
                                                           VolumeResultType              volumeResultType,
                                                           size_t                        timeStepIdx,
                                                           const RigFloodingSettings&    floodingSettings )
{
    if ( volumeType == VolumeType::OIL )
    {
        return loadOilVectorByName( resultsData, volumesType, volumeResultType, timeStepIdx, floodingSettings );
    }
    else if ( volumeType == VolumeType::GAS )
    {
        return loadGasVectorByName( resultsData, unitsType, volumesType, volumeResultType, timeStepIdx, floodingSettings );
    }
    else if ( volumeType == VolumeType::HYDROCARBON )
    {
        std::vector<double> oilVolume = loadOilVectorByName( resultsData, volumesType, volumeResultType, timeStepIdx, floodingSettings );
        std::vector<double> gasVolume =
            loadGasVectorByName( resultsData, unitsType, volumesType, volumeResultType, timeStepIdx, floodingSettings );
        if ( oilVolume.empty() || gasVolume.empty() || oilVolume.size() != gasVolume.size() ) return {};

        std::vector<double> volume;
        volume.resize( oilVolume.size(), std::numeric_limits<double>::infinity() );
        for ( size_t i = 0; i < oilVolume.size(); i++ )
        {
            volume[i] = oilVolume[i] + gasVolume[i];
        }
        return volume;
    }

    CAF_ASSERT( false && "Unknown volume type" );
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigWellTargetMapping::ClusterStatistics> RigWellTargetMapping::generateStatistics( RimEclipseCase*            eclipseCase,
                                                                                               const std::vector<double>& pressure,
                                                                                               const std::vector<double>& permeabilityX,
                                                                                               int                        numClustersFound,
                                                                                               size_t                     timeStepIdx,
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
    const std::vector<double>& clusterIds = resultsData->cellScalarResults( clusterAddress, 0 );

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
RimRegularGridCase* RigWellTargetMapping::generateEnsembleCandidates( RimEclipseCaseEnsemble&    ensemble,
                                                                      size_t                     timeStepIdx,
                                                                      const cvf::Vec3st&         resultGridCellCount,
                                                                      VolumeType                 volumeType,
                                                                      VolumesType                volumesType,
                                                                      VolumeResultType           volumeResultType,
                                                                      const RigFloodingSettings& floodingSettings,
                                                                      const ClusteringLimits&    limits )
{
    RiaLogging::debug( "Generating ensemble statistics" );

    caf::ProgressInfo progInfo( ensemble.cases().size() * 2, "Generating ensemble statistics" );

    for ( auto eclipseCase : ensemble.cases() )
    {
        auto task = progInfo.task( "Generating realization statistics.", 1 );

        generateCandidates( eclipseCase, timeStepIdx, volumeType, volumesType, volumeResultType, floodingSettings, limits, false );
    }

    cvf::BoundingBox boundingBox;
    for ( auto eclipseCase : ensemble.cases() )
    {
        cvf::BoundingBox bb = computeBoundingBoxForResult( *eclipseCase, RigWellTargetMapping::wellTargetResultName(), 0 );
        boundingBox.add( bb );
    }

    RiaLogging::debug(
        QString( "Clusters bounding box min: [%1 %2 %3]" ).arg( boundingBox.min().x() ).arg( boundingBox.min().y() ).arg( boundingBox.min().z() ) );
    RiaLogging::debug(
        QString( "Clusters bounding box max: [%1 %2 %3]" ).arg( boundingBox.max().x() ).arg( boundingBox.max().y() ).arg( boundingBox.max().z() ) );

    RimRegularGridCase* targetCase = new RimRegularGridCase;
    targetCase->setBoundingBox( boundingBox );
    targetCase->setCellCount( resultGridCellCount );
    targetCase->createModel();

    std::vector<int> occurrence;

    std::map<QString, std::vector<std::vector<double>>> resultNamesAndSamples;
    resultNamesAndSamples["TOTAL_PORV_SOIL"]      = {};
    resultNamesAndSamples["TOTAL_PORV_SGAS"]      = {};
    resultNamesAndSamples["TOTAL_PORV_SOIL_SGAS"] = {};
    resultNamesAndSamples["TOTAL_FIPOIL"]         = {};
    resultNamesAndSamples["TOTAL_FIPGAS"]         = {};
    resultNamesAndSamples["TOTAL_RFIPOIL"]        = {};
    resultNamesAndSamples["TOTAL_RFIPGAS"]        = {};
    resultNamesAndSamples["TOTAL_SFIPOIL"]        = {};
    resultNamesAndSamples["TOTAL_SFIPGAS"]        = {};

    for ( auto eclipseCase : ensemble.cases() )
    {
        auto task = progInfo.task( "Accumulating results.", 1 );

        accumulateResultsForSingleCase( *eclipseCase, *targetCase, resultNamesAndSamples, occurrence );
    }

    auto createFractionVector = []( const std::vector<int>& occurrence, int maxRealizationCount ) -> std::vector<double>
    {
        std::vector<double> fractions( occurrence.size() );
        std::transform( occurrence.begin(),
                        occurrence.end(),
                        fractions.begin(),
                        [maxRealizationCount]( int value ) { return static_cast<double>( value ) / maxRealizationCount; } );

        return fractions;
    };

    createResultVector( *targetCase, "OCCURRENCE", occurrence );
    std::vector<double> probability = createFractionVector( occurrence, static_cast<int>( ensemble.cases().size() ) );
    createResultVector( *targetCase, "PROBABILITY", probability );

    for ( auto [resultName, vec] : resultNamesAndSamples )
    {
        computeStatisticsAndCreateVectors( *targetCase, resultName, vec );
    }

    return targetCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetMapping::computeStatisticsAndCreateVectors( RimEclipseCase&                         targetCase,
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

        if ( RiaStatisticsTools::isValidNumber( p10 ) ) p10Results[i] = p10;
        if ( RiaStatisticsTools::isValidNumber( p50 ) ) p50Results[i] = p50;
        if ( RiaStatisticsTools::isValidNumber( p90 ) ) p90Results[i] = p90;
        if ( RiaStatisticsTools::isValidNumber( mean ) ) meanResults[i] = mean;

        double minValue = RiaStatisticsTools::minimumValue( samples );
        if ( RiaStatisticsTools::isValidNumber( minValue ) && minValue < std::numeric_limits<double>::max() ) minResults[i] = minValue;

        double maxValue = RiaStatisticsTools::maximumValue( samples );
        if ( RiaStatisticsTools::isValidNumber( maxValue ) && maxValue > -std::numeric_limits<double>::max() ) maxResults[i] = maxValue;
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
void RigWellTargetMapping::accumulateResultsForSingleCase( RimEclipseCase&                                      eclipseCase,
                                                           RimEclipseCase&                                      targetCase,
                                                           std::map<QString, std::vector<std::vector<double>>>& resultNamesAndSamples,
                                                           std::vector<int>&                                    occupancy )
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
    const std::vector<double>& clusterNum = resultsData->cellScalarResults( clustersNumAddress, 0 );

    std::map<QString, const std::vector<double>*> namedInputVector;

    for ( const auto& [resultName, vec] : resultNamesAndSamples )
    {
        RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::GENERATED, resultName );
        resultsData->ensureKnownResultLoaded( resultAddress );
        const std::vector<double>& resultVector = resultsData->cellScalarResults( resultAddress, 0 );
        namedInputVector[resultName]            = &resultVector;
    }

    std::map<QString, std::vector<double>> namedOutputVector;
    for ( const auto& [resultName, vec] : resultNamesAndSamples )
    {
        namedOutputVector[resultName] = std::vector( targetNumActiveCells, std::numeric_limits<double>::infinity() );
    }

    for ( size_t targetCellIdx = 0; targetCellIdx < targetNumReservoirCells; targetCellIdx++ )
    {
        const RigCell& nativeCell = targetCase.mainGrid()->cell( targetCellIdx );
        cvf::Vec3d     cellCenter = nativeCell.center();

        size_t targetResultIndex = targetActiveCellInfo->cellResultIndex( targetCellIdx );

        size_t cellIdx = mainGrid->findReservoirCellIndexFromPoint( cellCenter );
        if ( cellIdx != cvf::UNDEFINED_SIZE_T && activeCellInfo->isActive( cellIdx ) && targetResultIndex != cvf::UNDEFINED_SIZE_T )
        {
            size_t resultIndex = resultsData->activeCellInfo()->cellResultIndex( cellIdx );
            if ( !std::isinf( clusterNum[resultIndex] ) && clusterNum[resultIndex] > 0 )
            {
                occupancy[targetResultIndex]++;
                for ( const auto& [resultName, vec] : resultNamesAndSamples )
                {
                    namedOutputVector[resultName][targetResultIndex] = namedInputVector[resultName]->at( resultIndex );
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
cvf::BoundingBox RigWellTargetMapping::computeBoundingBoxForResult( RimEclipseCase& eclipseCase, const QString& resultName, size_t timeStepIndex )
{
    RigCaseCellResultsData*  resultsData       = eclipseCase.results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    const RigMainGrid*       mainGrid          = eclipseCase.mainGrid();
    const RigActiveCellInfo* activeCellInfo    = resultsData->activeCellInfo();
    const size_t             numReservoirCells = activeCellInfo->reservoirCellCount();

    RigEclipseResultAddress clustersNumAddress( RiaDefines::ResultCatType::GENERATED, resultName );
    resultsData->ensureKnownResultLoaded( clustersNumAddress );
    const std::vector<double>& clusterNum = resultsData->cellScalarResults( clustersNumAddress, timeStepIndex );

    cvf::BoundingBox boundingBox;
    for ( size_t reservoirCellIndex = 0; reservoirCellIndex < numReservoirCells; reservoirCellIndex++ )
    {
        size_t targetResultIndex = activeCellInfo->cellResultIndex( reservoirCellIndex );
        if ( reservoirCellIndex != cvf::UNDEFINED_SIZE_T && activeCellInfo->isActive( reservoirCellIndex ) &&
             targetResultIndex != cvf::UNDEFINED_SIZE_T && !std::isinf( clusterNum[targetResultIndex] ) && clusterNum[targetResultIndex] > 0 )
        {
            const RigCell& nativeCell = mainGrid->cell( reservoirCellIndex );
            boundingBox.add( nativeCell.boundingBox() );
        }
    }

    return boundingBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<std::pair<std::pair<size_t, RigWellTargetMapping::CellFaceType>, size_t>>
    RigWellTargetMapping::nncConnectionCellAndResult( size_t cellIdx, RigMainGrid* mainGrid )
{
    std::list<std::pair<std::pair<size_t, CellFaceType>, size_t>> foundCells;

    if ( mainGrid->nncData() == nullptr ) return foundCells;

    auto& connections = mainGrid->nncData()->allConnections();
    for ( size_t i = 0; i < connections.size(); i++ )
    {
        if ( connections[i].c1GlobIdx() == cellIdx )
        {
            foundCells.push_back( std::make_pair( std::make_pair( connections[i].c2GlobIdx(), connections[i].face() ), i ) );
        }
    }

    return foundCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigWellTargetMapping::wellTargetResultName()
{
    return "WELL_TARGET_NUM";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWellTargetMapping::isSaturationSufficient( const VolumeType volumeType, const DataContainer& data, const ClusteringLimits& limits, size_t idx )
{
    bool needsValidOil = volumeType == VolumeType::OIL || volumeType == VolumeType::HYDROCARBON;
    bool needsValidGas = volumeType == VolumeType::GAS || volumeType == VolumeType::HYDROCARBON;
    // For hydrocarbon it is enough that one of the saturations is above the limit.
    if ( needsValidOil && data.saturationOil[idx] >= limits.saturationOil ) return true;
    if ( needsValidGas && data.saturationGas[idx] >= limits.saturationGas ) return true;
    return false;
}

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

#include "RigWellTargetCandidatesGenerator.h"

#include "RiaLogging.h"
#include "RiaPorosityModel.h"
#include "RiaResultNames.h"
#include "RiaWeightedMeanCalculator.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimEclipseView.h"
#include "RimProject.h"
#include "RimPropertyFilterCollection.h"
#include "RimTools.h"

#include "cafVecIjk.h"

#include "cvfMath.h"
#include "cvfStructGrid.h"

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetCandidatesGenerator::generateCandidates( RimEclipseCase*         eclipseCase,
                                                           size_t                  timeStepIdx,
                                                           VolumeType              volumeType,
                                                           VolumesType             volumesType,
                                                           VolumeResultType        volumeResultType,
                                                           const ClusteringLimits& limits )
{
    auto activeCellCount = getActiveCellCount( eclipseCase );
    if ( !activeCellCount )
    {
        RiaLogging::error( "No active cells found" );
        return;
    }

    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData ) return;

    std::vector<double> volume = getVolumeVector( *resultsData, volumeType, volumesType, volumeResultType, timeStepIdx );
    if ( volume.empty() )
    {
        RiaLogging::error( "Unable to produce volume vector." );
        return;
    }

    RigEclipseResultAddress pressureAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "PRESSURE" );
    resultsData->ensureKnownResultLoaded( pressureAddress );
    const std::vector<double>& pressure = resultsData->cellScalarResults( pressureAddress, timeStepIdx );

    RigEclipseResultAddress permeabilityXAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMX" );
    resultsData->ensureKnownResultLoaded( permeabilityXAddress );
    const std::vector<double>& permeabilityX = resultsData->cellScalarResults( permeabilityXAddress, 0 );

    RigEclipseResultAddress permeabilityYAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMY" );
    resultsData->ensureKnownResultLoaded( permeabilityYAddress );
    const std::vector<double>& permeabilityY = resultsData->cellScalarResults( permeabilityYAddress, 0 );

    RigEclipseResultAddress permeabilityZAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMZ" );
    resultsData->ensureKnownResultLoaded( permeabilityZAddress );
    const std::vector<double>& permeabilityZ = resultsData->cellScalarResults( permeabilityZAddress, 0 );

    RigEclipseResultAddress transmissibilityXAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANX" );
    resultsData->ensureKnownResultLoaded( transmissibilityXAddress );
    const std::vector<double>& transmissibilityX = resultsData->cellScalarResults( transmissibilityXAddress, 0 );

    RigEclipseResultAddress transmissibilityYAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANY" );
    resultsData->ensureKnownResultLoaded( transmissibilityYAddress );
    const std::vector<double>& transmissibilityY = resultsData->cellScalarResults( transmissibilityYAddress, 0 );

    RigEclipseResultAddress transmissibilityZAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANZ" );
    resultsData->ensureKnownResultLoaded( transmissibilityZAddress );
    const std::vector<double>& transmissibilityZ = resultsData->cellScalarResults( transmissibilityZAddress, 0 );

    std::vector<int> clusters( activeCellCount.value(), 0 );
    auto             start            = std::chrono::high_resolution_clock::now();
    int              numClusters      = limits.maxClusters;
    int              maxIterations    = limits.maxIterations;
    int              numClustersFound = 0;
    for ( int clusterId = 1; clusterId <= numClusters; clusterId++ )
    {
        std::optional<caf::VecIjk> startCell = findStartCell( eclipseCase,
                                                              timeStepIdx,
                                                              limits,
                                                              volume,
                                                              pressure,
                                                              permeabilityX,
                                                              permeabilityY,
                                                              permeabilityZ,
                                                              transmissibilityX,
                                                              transmissibilityY,
                                                              transmissibilityZ,
                                                              clusters );

        if ( startCell.has_value() )
        {
            RiaLogging::info( QString( "Cluster %1 start cell: [%2 %3 %4] " )
                                  .arg( clusterId )
                                  .arg( startCell->i() + 1 )
                                  .arg( startCell->j() + 1 )
                                  .arg( startCell->k() + 1 ) );

            growCluster( eclipseCase,
                         startCell.value(),
                         limits,
                         volume,
                         pressure,
                         permeabilityX,
                         permeabilityY,
                         permeabilityZ,
                         transmissibilityX,
                         transmissibilityY,
                         transmissibilityZ,
                         clusters,
                         clusterId,
                         timeStepIdx,
                         maxIterations );
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
    RiaLogging::info( QString( "Time spent: %1 ms" ).arg( milliseconds.count() ) );

    QString resultName = "CLUSTERS_NUM";
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
        generateStatistics( eclipseCase, pressure, permeabilityX, permeabilityY, permeabilityZ, numClustersFound, timeStepIdx, resultName );
    for ( auto s : statistics )
    {
        RiaLogging::info( QString( "Cluster #%1 Statistics" ).arg( s.id ) );
        RiaLogging::info( QString( "Number of cells: %1" ).arg( s.numCells ) );
        RiaLogging::info( QString( "Total PORV*SOIL: %1" ).arg( s.totalPorvSoil ) );
        RiaLogging::info( QString( "Total PORV*SGAS: %1" ).arg( s.totalPorvSgas ) );
        RiaLogging::info( QString( "Total PORV*(SOIL+SGAS): %1" ).arg( s.totalPorvSoilAndSgas ) );
        RiaLogging::info( QString( "Total FIPOIL: %1" ).arg( s.totalFipOil ) );
        RiaLogging::info( QString( "Total FIPGAS: %1" ).arg( s.totalFipGas ) );
        RiaLogging::info( QString( "Average Permeability: %1" ).arg( s.permeability ) );
        RiaLogging::info( QString( "Average Pressure: %1" ).arg( s.pressure ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<caf::VecIjk> RigWellTargetCandidatesGenerator::findStartCell( RimEclipseCase*            eclipseCase,
                                                                            size_t                     timeStepIdx,
                                                                            const ClusteringLimits&    limits,
                                                                            const std::vector<double>& volume,
                                                                            const std::vector<double>& pressure,
                                                                            const std::vector<double>& permeabilityX,
                                                                            const std::vector<double>& permeabilityY,
                                                                            const std::vector<double>& permeabilityZ,
                                                                            const std::vector<double>& transmissibilityX,
                                                                            const std::vector<double>& transmissibilityY,
                                                                            const std::vector<double>& transmissibilityZ,
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
            const double cellVolume   = volume[resultIndex];
            const double cellPressure = pressure[resultIndex];

            const double cellPermeabiltyX = permeabilityX[resultIndex];
            const double cellPermeabiltyY = permeabilityY[resultIndex];
            const double cellPermeabiltyZ = permeabilityZ[resultIndex];
            const bool permeabilityValidInAnyDirection = ( cellPermeabiltyX >= limits.permeability || cellPermeabiltyY >= limits.permeability ||
                                                           cellPermeabiltyZ >= limits.permeability );

            if ( cellVolume > maxVolume && cellVolume >= limits.volume && cellPressure >= limits.pressure && permeabilityValidInAnyDirection )
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
void RigWellTargetCandidatesGenerator::growCluster( RimEclipseCase*            eclipseCase,
                                                    const caf::VecIjk&         startCell,
                                                    const ClusteringLimits&    limits,
                                                    const std::vector<double>& volume,
                                                    const std::vector<double>& pressure,
                                                    const std::vector<double>& permeabilityX,
                                                    const std::vector<double>& permeabilityY,
                                                    const std::vector<double>& permeabilityZ,
                                                    const std::vector<double>& transmissibilityX,
                                                    const std::vector<double>& transmissibilityY,
                                                    const std::vector<double>& transmissibilityZ,
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
        foundCells = findCandidates( *eclipseCase,
                                     foundCells,
                                     limits,
                                     volume,
                                     pressure,
                                     permeabilityX,
                                     permeabilityY,
                                     permeabilityZ,
                                     transmissibilityX,
                                     transmissibilityY,
                                     transmissibilityZ,
                                     clusters );
        if ( foundCells.empty() ) break;
        assignClusterIdToCells( *resultsData->activeCellInfo(), foundCells, clusters, clusterId );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigWellTargetCandidatesGenerator::findCandidates( const RimEclipseCase&      eclipseCase,
                                                                      const std::vector<size_t>& previousCells,
                                                                      const ClusteringLimits&    limits,
                                                                      const std::vector<double>& volume,
                                                                      const std::vector<double>& pressure,
                                                                      const std::vector<double>& permeabilityX,
                                                                      const std::vector<double>& permeabilityY,
                                                                      const std::vector<double>& permeabilityZ,
                                                                      const std::vector<double>& transmissibilityX,
                                                                      const std::vector<double>& transmissibilityY,
                                                                      const std::vector<double>& transmissibilityZ,
                                                                      std::vector<int>&          clusters )
{
    std::vector<size_t> candidates;
    auto                resultsData = eclipseCase.results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    const std::vector<cvf::StructGridInterface::FaceType> faces = {
        cvf::StructGridInterface::FaceType::POS_I,
        cvf::StructGridInterface::FaceType::NEG_I,
        cvf::StructGridInterface::FaceType::POS_J,
        cvf::StructGridInterface::FaceType::NEG_J,
        cvf::StructGridInterface::FaceType::POS_K,
        cvf::StructGridInterface::FaceType::NEG_K,
    };

    for ( size_t cellIdx : previousCells )
    {
        const RigCell& cell = eclipseCase.mainGrid()->cell( cellIdx );
        if ( cell.isInvalid() ) continue;

        RigGridBase* grid               = cell.hostGrid();
        size_t       gridLocalCellIndex = cell.gridLocalCellIndex();
        size_t       resultIndex        = resultsData->activeCellInfo()->cellResultIndex( cellIdx );

        size_t i, j, k;

        grid->ijkFromCellIndex( gridLocalCellIndex, &i, &j, &k );

        for ( cvf::StructGridInterface::FaceType face : faces )
        {
            size_t gridLocalNeighborCellIdx;
            if ( grid->cellIJKNeighbor( i, j, k, face, &gridLocalNeighborCellIdx ) )
            {
                size_t neighborResvCellIdx = grid->reservoirCellIndex( gridLocalNeighborCellIdx );
                size_t neighborResultIndex = resultsData->activeCellInfo()->cellResultIndex( neighborResvCellIdx );
                if ( neighborResultIndex != cvf::UNDEFINED_SIZE_T && clusters[neighborResultIndex] == 0 )
                {
                    double permeability     = getValueForFace( permeabilityX, permeabilityY, permeabilityZ, face, neighborResultIndex );
                    double transmissibility = getTransmissibilityValueForFace( transmissibilityX,
                                                                               transmissibilityY,
                                                                               transmissibilityZ,
                                                                               face,
                                                                               resultIndex,
                                                                               neighborResultIndex );
                    if ( volume[neighborResultIndex] > limits.volume && pressure[neighborResultIndex] > limits.pressure &&
                         permeability > limits.permeability && transmissibility > limits.transmissibility )
                    {
                        candidates.push_back( neighborResvCellIdx );
                        clusters[neighborResultIndex] = -1;
                    }
                }
            }
        }
    }

    return candidates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetCandidatesGenerator::assignClusterIdToCells( const RigActiveCellInfo&   activeCellInfo,
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
void RigWellTargetCandidatesGenerator::createResultVector( RimEclipseCase&         eclipseCase,
                                                           const QString&          resultName,
                                                           const std::vector<int>& clusterIds )
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
std::optional<size_t> RigWellTargetCandidatesGenerator::getActiveCellCount( RimEclipseCase* eclipseCase )
{
    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData ) return {};

    return resultsData->activeCellInfo()->reservoirActiveCellCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellTargetCandidatesGenerator::getValueForFace( const std::vector<double>&         x,
                                                          const std::vector<double>&         y,
                                                          const std::vector<double>&         z,
                                                          cvf::StructGridInterface::FaceType face,
                                                          size_t                             resultIndex )
{
    if ( face == cvf::StructGridInterface::FaceType::POS_I || face == cvf::StructGridInterface::FaceType::NEG_I ) return x[resultIndex];
    if ( face == cvf::StructGridInterface::FaceType::POS_J || face == cvf::StructGridInterface::FaceType::NEG_J ) return y[resultIndex];
    if ( face == cvf::StructGridInterface::FaceType::POS_K || face == cvf::StructGridInterface::FaceType::NEG_K ) return z[resultIndex];
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellTargetCandidatesGenerator::getTransmissibilityValueForFace( const std::vector<double>&         x,
                                                                          const std::vector<double>&         y,
                                                                          const std::vector<double>&         z,
                                                                          cvf::StructGridInterface::FaceType face,
                                                                          size_t                             resultIndex,
                                                                          size_t                             neighborResultIndex )
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
std::vector<double> RigWellTargetCandidatesGenerator::getVolumeVector( RigCaseCellResultsData& resultsData,
                                                                       VolumeType              volumeType,
                                                                       VolumesType             volumesType,
                                                                       VolumeResultType        volumeResultType,
                                                                       size_t                  timeStepIdx )
{
    auto loadVectorByName = []( RigCaseCellResultsData& resultsData, const QString& resultName, size_t timeStepIdx ) -> std::vector<double>
    {
        RigEclipseResultAddress address( RiaDefines::ResultCatType::DYNAMIC_NATIVE, resultName );
        if ( !resultsData.ensureKnownResultLoaded( address ) ) return {};
        return resultsData.cellScalarResults( address, timeStepIdx );
    };

    auto getOilVectorName = []( VolumesType volumesType ) -> QString
    {
        switch ( volumesType )
        {
            case VolumesType::COMPUTED_VOLUMES:
                return RiaResultNames::riPorvSoil();
            case VolumesType::RESERVOIR_VOLUMES:
                return "RFIPOIL";
            case VolumesType::SURFACE_VOLUMES:
                return "SFIPOIL";
            default:
            {
                CAF_ASSERT( false );
                return "";
            }
        }
    };

    auto getGasVectorName = []( VolumesType volumesType ) -> QString
    {
        switch ( volumesType )
        {
            case VolumesType::COMPUTED_VOLUMES:
                return RiaResultNames::riPorvSgas();
            case VolumesType::RESERVOIR_VOLUMES:
                return "RFIPGAS";
            case VolumesType::SURFACE_VOLUMES:
                return "SFIPGAS";
            default:
            {
                CAF_ASSERT( false );
                return "";
            }
        }
    };

    std::vector<double> volume;

    if ( volumeType == VolumeType::OIL )
    {
        volume = loadVectorByName( resultsData, getOilVectorName( volumesType ), timeStepIdx );
    }
    else if ( volumeType == VolumeType::GAS )
    {
        volume = loadVectorByName( resultsData, getGasVectorName( volumesType ), timeStepIdx );
    }
    else if ( volumeType == VolumeType::HYDROCARBON )
    {
        std::vector<double> oilVolume = loadVectorByName( resultsData, getOilVectorName( volumesType ), timeStepIdx );
        std::vector<double> gasVolume = loadVectorByName( resultsData, getGasVectorName( volumesType ), timeStepIdx );
        if ( oilVolume.empty() || gasVolume.empty() || oilVolume.size() != gasVolume.size() ) return volume;

        volume.resize( oilVolume.size(), std::numeric_limits<double>::infinity() );
        for ( size_t i = 0; i < oilVolume.size(); i++ )
        {
            volume[i] = oilVolume[i] + gasVolume[i];
        }
    }

    return volume;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigWellTargetCandidatesGenerator::ClusterStatistics>
    RigWellTargetCandidatesGenerator::generateStatistics( RimEclipseCase*            eclipseCase,
                                                          const std::vector<double>& pressure,
                                                          const std::vector<double>& permeabilityX,
                                                          const std::vector<double>& permeabilityY,
                                                          const std::vector<double>& permeabilityZ,
                                                          int                        numClustersFound,
                                                          size_t                     timeStepIdx,
                                                          const QString&             clusterResultName )
{
    std::vector<ClusterStatistics> statistics( numClustersFound );

    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData ) return statistics;

    RigEclipseResultAddress porvAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PORV" );
    resultsData->ensureKnownResultLoaded( porvAddress );
    const std::vector<double>& porv = resultsData->cellScalarResults( porvAddress, 0 );

    RigEclipseResultAddress porvSoilAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::riPorvSoil() );
    resultsData->ensureKnownResultLoaded( porvSoilAddress );
    const std::vector<double>& porvSoil = resultsData->cellScalarResults( porvSoilAddress, timeStepIdx );

    RigEclipseResultAddress porvSgasAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::riPorvSgas() );
    resultsData->ensureKnownResultLoaded( porvSgasAddress );
    const std::vector<double>& porvSgas = resultsData->cellScalarResults( porvSgasAddress, timeStepIdx );

    RigEclipseResultAddress porvSoilAndSgasAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::riPorvSoilSgas() );
    resultsData->ensureKnownResultLoaded( porvSoilAndSgasAddress );
    const std::vector<double>& porvSoilAndSgas = resultsData->cellScalarResults( porvSoilAndSgasAddress, timeStepIdx );

    RigEclipseResultAddress fipOilAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FIPOIL" );
    resultsData->ensureKnownResultLoaded( fipOilAddress );
    const std::vector<double>& fipOil = resultsData->cellScalarResults( fipOilAddress, timeStepIdx );

    RigEclipseResultAddress fipGasAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FIPGAS" );
    resultsData->ensureKnownResultLoaded( fipGasAddress );
    const std::vector<double>& fipGas = resultsData->cellScalarResults( fipGasAddress, timeStepIdx );

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
                statistics[i].totalPorvSoil += porvSoil[idx];
                statistics[i].totalPorvSgas += porvSgas[idx];
                statistics[i].totalPorvSoilAndSgas += porvSoilAndSgas[idx];
                statistics[i].totalFipOil += fipOil[idx];
                statistics[i].totalFipGas += fipGas[idx];

                double meanPermeability = ( permeabilityX[idx] + permeabilityY[idx] + permeabilityZ[idx] ) / 3.0;
                permeabilityCalculators[i].addValueAndWeight( meanPermeability, porv[idx] );

                pressureCalculators[i].addValueAndWeight( pressure[idx], porv[idx] );
            }
        }
    }

    for ( int i = 0; i < numClustersFound; i++ )
    {
        statistics[i].permeability = permeabilityCalculators[i].weightedMean();
        statistics[i].pressure     = pressureCalculators[i].weightedMean();
    }

    return statistics;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellTargetCandidatesGenerator::generateEnsembleCandidates( RimEclipseCase&         targetCase,
                                                                   RimEclipseCaseEnsemble& ensemble,
                                                                   size_t                  timeStepIdx,
                                                                   VolumeType              volumeType,
                                                                   VolumesType             volumesType,
                                                                   VolumeResultType        volumeResultType,
                                                                   const ClusteringLimits& limits )
{
    RiaLogging::debug( "Generating ensemble statistics" );

    for ( auto eclipseCase : ensemble.cases() )
    {
        generateCandidates( eclipseCase, timeStepIdx, volumeType, volumesType, volumeResultType, limits );
    }

    const RigCaseCellResultsData* targetResultsData    = targetCase.results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    const RigActiveCellInfo*      targetActiveCellInfo = targetResultsData->activeCellInfo();

    const size_t targetNumReservoirCells = targetActiveCellInfo->reservoirCellCount();
    const size_t targetNumActiveCells    = targetActiveCellInfo->reservoirActiveCellCount();

    std::vector<int> occupancy( targetNumActiveCells, 0 );
    for ( auto eclipseCase : ensemble.cases() )
    {
        RigCaseCellResultsData*  resultsData    = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
        const RigMainGrid*       mainGrid       = eclipseCase->mainGrid();
        const RigActiveCellInfo* activeCellInfo = resultsData->activeCellInfo();

        RigEclipseResultAddress pressureAddress( RiaDefines::ResultCatType::GENERATED, "CLUSTERS_NUM" );
        resultsData->ensureKnownResultLoaded( pressureAddress );
        const std::vector<double>& clusterNum = resultsData->cellScalarResults( pressureAddress, 0 );

        for ( size_t targetCellIdx = 0; targetCellIdx < targetNumReservoirCells; targetCellIdx++ )
        {
            const RigCell& nativeCell = targetCase.mainGrid()->globalCellArray()[targetCellIdx];
            cvf::Vec3d     cellCenter = nativeCell.center();

            size_t targetResultIndex = targetActiveCellInfo->cellResultIndex( targetCellIdx );

            size_t cellIdx = mainGrid->findReservoirCellIndexFromPoint( cellCenter );
            if ( cellIdx != cvf::UNDEFINED_SIZE_T && activeCellInfo->isActive( cellIdx ) && targetResultIndex != cvf::UNDEFINED_SIZE_T )
            {
                size_t resultIndex = resultsData->activeCellInfo()->cellResultIndex( cellIdx );
                if ( !std::isinf( clusterNum[resultIndex] ) && clusterNum[resultIndex] > 0 )
                {
                    occupancy[targetResultIndex]++;
                }
            }
        }
    }

    createResultVector( targetCase, "OCCUPANCY", occupancy );
}

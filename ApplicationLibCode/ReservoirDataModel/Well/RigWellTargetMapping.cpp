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

#include "RigWellTargetMappingTools.h"

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
#include "RigEclipseResultInfo.h"
#include "RigFloodingSettings.h"
#include "RigHydrocarbonFlowTools.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "RigStatisticsMath.h"

#include "RimEclipseCase.h"
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
                                               bool                       skipUndefinedResults,
                                               bool                       setTimeStepInView = true )
{
    if ( !eclipseCase->ensureReservoirCaseIsOpen() ) return;

    auto activeCellCount = RigWellTargetMappingTools::getActiveCellCount( eclipseCase );
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

    RigWellTargetMappingTools::DataContainer data;
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
        std::optional<caf::VecIjk0> startCell =
            RigWellTargetMappingTools::findStartCell( eclipseCase, timeStepIdx, volumeType, limits, data, filterVector, clusters );

        if ( startCell.has_value() )
        {
            RiaLogging::info( QString( "Cluster %1 start cell: [%2 %3 %4] " )
                                  .arg( clusterId )
                                  .arg( startCell->i() + 1 )
                                  .arg( startCell->j() + 1 )
                                  .arg( startCell->k() + 1 ),
                              logKeyword );

            RigWellTargetMappingTools::growCluster( eclipseCase,
                                                    startCell.value(),
                                                    volumeType,
                                                    limits,
                                                    data,
                                                    filterVector,
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
    RiaLogging::info( QString( "Time spent: %1 ms" ).arg( milliseconds.count() ), logKeyword );

    QString resultName = RigWellTargetMapping::wellTargetResultName();
    RigWellTargetMappingTools::createResultVector( *eclipseCase, resultName, clusters, timeStepIdx );

    std::vector<RigWellTargetMappingTools::ClusterStatistics> statistics =
        RigWellTargetMappingTools::generateStatistics( eclipseCase, data.pressure, data.permeabilityX, numClustersFound, timeStepIdx, resultName );
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
        const int ts = (int)timeStepIdx;
        RigWellTargetMappingTools::createResultVectorIfDefined( *eclipseCase, "TOTAL_PORV_SOIL", totalPorvSoil, ts );
        RigWellTargetMappingTools::createResultVectorIfDefined( *eclipseCase, "TOTAL_PORV_SGAS", totalPorvSgas, ts );
        RigWellTargetMappingTools::createResultVectorIfDefined( *eclipseCase, "TOTAL_PORV_SOIL_SGAS", totalPorvSoilAndSgas, ts );
        RigWellTargetMappingTools::createResultVectorIfDefined( *eclipseCase, "TOTAL_FIPOIL", totalFipOil, ts );
        RigWellTargetMappingTools::createResultVectorIfDefined( *eclipseCase, "TOTAL_FIPGAS", totalFipGas, ts );
        RigWellTargetMappingTools::createResultVectorIfDefined( *eclipseCase, "TOTAL_RFIPOIL", totalRfipOil, ts );
        RigWellTargetMappingTools::createResultVectorIfDefined( *eclipseCase, "TOTAL_RFIPGAS", totalRfipGas, ts );
        RigWellTargetMappingTools::createResultVectorIfDefined( *eclipseCase, "TOTAL_SFIPOIL", totalSfipOil, ts );
        RigWellTargetMappingTools::createResultVectorIfDefined( *eclipseCase, "TOTAL_SFIPGAS", totalSfipGas, ts );
    }
    else
    {
        RigWellTargetMappingTools::createResultVector( *eclipseCase, "TOTAL_PORV_SOIL", totalPorvSoil, timeStepIdx );
        RigWellTargetMappingTools::createResultVector( *eclipseCase, "TOTAL_PORV_SGAS", totalPorvSgas, timeStepIdx );
        RigWellTargetMappingTools::createResultVector( *eclipseCase, "TOTAL_PORV_SOIL_SGAS", totalPorvSoilAndSgas, timeStepIdx );
        RigWellTargetMappingTools::createResultVector( *eclipseCase, "TOTAL_FIPOIL", totalFipOil, timeStepIdx );
        RigWellTargetMappingTools::createResultVector( *eclipseCase, "TOTAL_FIPGAS", totalFipGas, timeStepIdx );
        RigWellTargetMappingTools::createResultVector( *eclipseCase, "TOTAL_RFIPOIL", totalRfipOil, timeStepIdx );
        RigWellTargetMappingTools::createResultVector( *eclipseCase, "TOTAL_RFIPGAS", totalRfipGas, timeStepIdx );
        RigWellTargetMappingTools::createResultVector( *eclipseCase, "TOTAL_SFIPOIL", totalSfipOil, timeStepIdx );
        RigWellTargetMappingTools::createResultVector( *eclipseCase, "TOTAL_SFIPGAS", totalSfipGas, timeStepIdx );
    }
    eclipseCase->updateResultAddressCollection();

    // Update views and property filters
    RimProject* proj = RimProject::current();
    proj->scheduleCreateDisplayModelAndRedrawAllViews();
    for ( auto view : eclipseCase->reservoirViews() )
    {
        if ( auto eclipseView = dynamic_cast<RimEclipseView*>( view ) )
        {
            if ( setTimeStepInView )
            {
                eclipseView->setCurrentTimeStep( (int)timeStepIdx );
            }
            eclipseView->scheduleReservoirGridGeometryRegen();
            eclipseView->propertyFilterCollection()->updateConnectedEditors();
        }
    }
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
        return RigWellTargetMappingTools::loadOilVectorByName( resultsData, volumesType, volumeResultType, timeStepIdx, floodingSettings );
    }
    else if ( volumeType == VolumeType::GAS )
    {
        return RigWellTargetMappingTools::loadGasVectorByName( resultsData, unitsType, volumesType, volumeResultType, timeStepIdx, floodingSettings );
    }
    else if ( volumeType == VolumeType::HYDROCARBON )
    {
        std::vector<double> oilVolume =
            RigWellTargetMappingTools::loadOilVectorByName( resultsData, volumesType, volumeResultType, timeStepIdx, floodingSettings );
        std::vector<double> gasVolume =
            RigWellTargetMappingTools::loadGasVectorByName( resultsData, unitsType, volumesType, volumeResultType, timeStepIdx, floodingSettings );
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
RimRegularGridCase* RigWellTargetMapping::generateEnsembleCandidates( const std::vector<RimEclipseCase*>& cases,
                                                                      size_t                              timeStepIdx,
                                                                      const cvf::Vec3st&                  resultGridCellCount,
                                                                      VolumeType                          volumeType,
                                                                      VolumesType                         volumesType,
                                                                      VolumeResultType                    volumeResultType,
                                                                      const RigFloodingSettings&          floodingSettings,
                                                                      const ClusteringLimits&             limits )
{
    RiaLogging::debug( "Generating ensemble statistics" );

    caf::ProgressInfo progInfo( cases.size() * 2, "Generating ensemble statistics" );

    for ( auto eclipseCase : cases )
    {
        auto task = progInfo.task( "Generating realization statistics.", 1 );

        generateCandidates( eclipseCase, timeStepIdx, volumeType, volumesType, volumeResultType, floodingSettings, limits, false );
    }

    cvf::BoundingBox boundingBox;
    for ( auto eclipseCase : cases )
    {
        cvf::BoundingBox bb =
            RigWellTargetMappingTools::computeBoundingBoxForResult( *eclipseCase, RigWellTargetMapping::wellTargetResultName(), timeStepIdx );
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

    for ( auto eclipseCase : cases )
    {
        auto task = progInfo.task( "Accumulating results.", 1 );

        RigWellTargetMappingTools::accumulateResultsForSingleCase( *eclipseCase, *targetCase, resultNamesAndSamples, occurrence, timeStepIdx );
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

    RigWellTargetMappingTools::createStaticResultVector( *targetCase, "OCCURRENCE", occurrence );
    std::vector<double> probability = createFractionVector( occurrence, static_cast<int>( cases.size() ) );
    RigWellTargetMappingTools::createStaticResultVector( *targetCase, "PROBABILITY", probability );

    for ( auto [resultName, vec] : resultNamesAndSamples )
    {
        RigWellTargetMappingTools::computeStatisticsAndCreateVectors( *targetCase, resultName, vec );
    }

    return targetCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigWellTargetMapping::wellTargetResultName()
{
    return "WELL_TARGET_NUM";
}

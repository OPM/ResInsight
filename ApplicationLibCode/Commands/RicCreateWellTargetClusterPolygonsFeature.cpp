/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RicCreateWellTargetClusterPolygonsFeature.h"

#include "RiaLogging.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigConvexHull.h"
#include "RigMainGrid.h"
#include "RigStatisticsMath.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellTargetCandidatesGenerator.h"

#include "cafSelectionManager.h"
#include "cvfMath.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateWellTargetClusterPolygonsFeature, "RicCreateWellTargetClusterPolygonsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateWellTargetClusterPolygonsFeature::onActionTriggered( bool isChecked )
{
    if ( auto generator = caf::SelectionManager::instance()->selectedItemOfType<RimWellTargetCandidatesGenerator>() )
    {
        if ( auto ensembleStatisticsCase = generator->ensembleStatisticsCase() )
        {
            if ( !ensembleStatisticsCase->reservoirViews().empty() )
            {
                auto view = ensembleStatisticsCase->reservoirViews()[0];
                createWellTargetClusterPolygons( ensembleStatisticsCase, view );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Function to smooth the histogram using a simple moving average
//--------------------------------------------------------------------------------------------------
std::vector<double> RicCreateWellTargetClusterPolygonsFeature::smoothHistogram( const std::vector<size_t>& histogram, size_t windowSize )
{
    std::vector<double> smoothed( histogram.size(), 0.0 );
    size_t              halfWindow = windowSize / 2;

    for ( size_t i = 0; i < histogram.size(); ++i )
    {
        double sum   = 0.0;
        size_t count = 0;
        for ( size_t j = ( i > halfWindow ? i - halfWindow : 0 ); j <= i + halfWindow && j < histogram.size(); ++j )
        {
            sum += histogram[j];
            ++count;
        }
        smoothed[i] = sum / count;
    }

    return smoothed;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<size_t, double>>
    RicCreateWellTargetClusterPolygonsFeature::findPeaksWithDynamicProminence( const std::vector<double>& smoothedHistogram )
{
    std::vector<std::pair<size_t, double>> peaks;
    std::vector<double>                    prominences;

    // Find all peaks and calculate their prominences
    for ( size_t i = 1; i < smoothedHistogram.size() - 1; ++i )
    {
        if ( smoothedHistogram[i] > smoothedHistogram[i - 1] && smoothedHistogram[i] > smoothedHistogram[i + 1] )
        {
            double leftMin = smoothedHistogram[i];
            for ( size_t j = i; j > 0; --j )
            {
                if ( smoothedHistogram[j] < leftMin ) leftMin = smoothedHistogram[j];
                if ( smoothedHistogram[j] > smoothedHistogram[i] ) break;
            }

            double rightMin = smoothedHistogram[i];
            for ( size_t j = i; j < smoothedHistogram.size(); ++j )
            {
                if ( smoothedHistogram[j] < rightMin ) rightMin = smoothedHistogram[j];
                if ( smoothedHistogram[j] > smoothedHistogram[i] ) break;
            }

            double prominence = smoothedHistogram[i] - std::max( leftMin, rightMin );
            prominences.push_back( prominence );
            peaks.emplace_back( i, prominence );
        }
    }

    // Determine dynamic threshold
    double min;
    double max;
    double sum;
    double range;
    double mean;
    double dev;
    RigStatisticsMath::calculateBasicStatistics( prominences, &min, &max, &sum, &range, &mean, &dev );
    double threshold = mean + dev;

    // Filter peaks based on the dynamic threshold
    std::vector<std::pair<size_t, double>> filteredPeaks;
    for ( const auto& [index, prominence] : peaks )
    {
        if ( prominence >= threshold )
        {
            filteredPeaks.emplace_back( index, prominence );
        }
    }

    return filteredPeaks;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RicCreateWellTargetClusterPolygonsFeature::findCellCentersWithResultInRange( RimEclipseCase& eclipseCase,
                                                                                                     const RigEclipseResultAddress& resultAddress,
                                                                                                     size_t timeStepIndex,
                                                                                                     double targetValue,
                                                                                                     double rangeMinimum,
                                                                                                     double rangeMaximum,
                                                                                                     cvf::ref<cvf::UByteArray> visibility,
                                                                                                     std::vector<int>&         clusters,
                                                                                                     int                       clusterId )
{
    RigCaseCellResultsData*  resultsData    = eclipseCase.results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    const RigMainGrid*       mainGrid       = eclipseCase.mainGrid();
    const RigActiveCellInfo* activeCellInfo = resultsData->activeCellInfo();

    resultsData->ensureKnownResultLoaded( resultAddress );
    const std::vector<double>& values = resultsData->cellScalarResults( resultAddress, timeStepIndex );

    size_t startCell = findStartCell( *activeCellInfo, values, targetValue, rangeMinimum, rangeMaximum, visibility, clusters );

    std::vector<cvf::Vec3d> cellCenters;

    if ( startCell != std::numeric_limits<size_t>::max() )
    {
        growCluster( &eclipseCase, startCell, values, rangeMinimum, rangeMaximum, visibility, clusters, clusterId );

        const size_t numReservoirCells = activeCellInfo->reservoirCellCount();

        for ( size_t reservoirCellIndex = 0; reservoirCellIndex < numReservoirCells; reservoirCellIndex++ )
        {
            size_t targetResultIndex = activeCellInfo->cellResultIndex( reservoirCellIndex );
            if ( reservoirCellIndex != cvf::UNDEFINED_SIZE_T && activeCellInfo->isActive( reservoirCellIndex ) &&
                 targetResultIndex != cvf::UNDEFINED_SIZE_T && clusters[targetResultIndex] == clusterId )
            {
                const RigCell& nativeCell = mainGrid->cell( reservoirCellIndex );
                cellCenters.push_back( nativeCell.center() );
            }
        }
    }

    return cellCenters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicCreateWellTargetClusterPolygonsFeature::findStartCell( const RigActiveCellInfo&   activeCellInfo,
                                                                 const std::vector<double>& values,
                                                                 double                     targetValue,
                                                                 double                     minRange,
                                                                 double                     maxRange,
                                                                 cvf::ref<cvf::UByteArray>  visibility,
                                                                 const std::vector<int>&    clusters )
{
    size_t       startCell         = std::numeric_limits<size_t>::max();
    double       minDiff           = std::numeric_limits<double>::max();
    const size_t numReservoirCells = activeCellInfo.reservoirCellCount();
    for ( size_t reservoirCellIdx = 0; reservoirCellIdx < numReservoirCells; reservoirCellIdx++ )
    {
        size_t resultIndex = activeCellInfo.cellResultIndex( reservoirCellIdx );
        if ( resultIndex != cvf::UNDEFINED_SIZE_T && clusters[resultIndex] == 0 && visibility->val( reservoirCellIdx ) )
        {
            const double value = values[resultIndex];
            if ( !std::isinf( value ) && cvf::Math::valueInRange( value, minRange, maxRange ) )
            {
                const double currentDiff = std::fabs( targetValue - value );
                if ( currentDiff < minDiff )
                {
                    minDiff   = currentDiff;
                    startCell = reservoirCellIdx;
                }
            }
        }
    }

    return startCell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateWellTargetClusterPolygonsFeature::assignClusterIdToCells( const RigActiveCellInfo&   activeCellInfo,
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
void RicCreateWellTargetClusterPolygonsFeature::growCluster( RimEclipseCase*            eclipseCase,
                                                             size_t                     startCell,
                                                             const std::vector<double>& volume,
                                                             double                     minRange,
                                                             double                     maxRange,
                                                             cvf::ref<cvf::UByteArray>  visibility,
                                                             std::vector<int>&          clusters,
                                                             int                        clusterId )
{
    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    // Initially only the start cell is found
    std::vector<size_t> foundCells = { startCell };
    assignClusterIdToCells( *resultsData->activeCellInfo(), foundCells, clusters, clusterId );

    while ( !foundCells.empty() )
    {
        foundCells = findCandidates( *eclipseCase, foundCells, volume, minRange, maxRange, visibility, clusters );
        assignClusterIdToCells( *resultsData->activeCellInfo(), foundCells, clusters, clusterId );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RicCreateWellTargetClusterPolygonsFeature::findCandidates( const RimEclipseCase&      eclipseCase,
                                                                               const std::vector<size_t>& previousCells,
                                                                               const std::vector<double>& volume,
                                                                               double                     minRange,
                                                                               double                     maxRange,
                                                                               cvf::ref<cvf::UByteArray>  visibility,
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
        size_t       i, j, k;

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
                    double value = volume[neighborResultIndex];
                    if ( !std::isinf( value ) && cvf::Math::valueInRange( value, minRange, maxRange ) && visibility->val( neighborResvCellIdx ) )
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
void RicCreateWellTargetClusterPolygonsFeature::createWellTargetClusterPolygons( RimEclipseCase* ensembleStatisticsCase, RimEclipseView* view )
{
    auto             overlay       = view->overlayInfoConfig();
    RigHistogramData histogramData = overlay->histogramData();

    auto computeBoundary = []( double histogramMin, double binSize, size_t index ) { return histogramMin + binSize * index; };

    double binSize = ( histogramData.max - histogramData.min ) / histogramData.histogram.size();

    // Smooth the histogram
    size_t              windowSize = 7;
    std::vector<double> smoothed   = smoothHistogram( histogramData.histogram, windowSize );

    // Find peaks with dynamic threshold
    auto peaks = findPeaksWithDynamicProminence( smoothed );

    RigCaseCellResultsData*  resultsData    = ensembleStatisticsCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    const RigActiveCellInfo* activeCellInfo = resultsData->activeCellInfo();

    std::vector<int> clusterIds( activeCellInfo->reservoirActiveCellCount(), 0 );

    int clusterId = 1;
    for ( const auto& [index, prominence] : peaks )
    {
        int minIndex =
            cvf::Math::clamp( static_cast<int>( index - ( windowSize / 2 ) ), 0, static_cast<int>( histogramData.histogram.size() ) );
        int maxIndex =
            cvf::Math::clamp( static_cast<int>( index + ( windowSize / 2 ) ) + 1, 0, static_cast<int>( histogramData.histogram.size() ) );

        double minRange = computeBoundary( histogramData.min, binSize, minIndex );
        double maxRange = computeBoundary( histogramData.min, binSize, maxIndex );

        auto resultAddress = view->cellResult()->eclipseResultAddress();
        auto cellCenters   = findCellCentersWithResultInRange( *ensembleStatisticsCase,
                                                             resultAddress,
                                                             view->currentTimeStep(),
                                                             ( maxRange + minRange ) / 2.0,
                                                             minRange,
                                                             maxRange,
                                                             view->currentTotalCellVisibility(),
                                                             clusterIds,
                                                             clusterId );
        RiaLogging::info( QString( "Got %1 cell centers for range %2 %3" ).arg( cellCenters.size() ).arg( minRange ).arg( maxRange ) );
        clusterId++;

        // Need at least three points to make a polygon
        if ( cellCenters.size() >= 3 )
        {
            std::vector<cvf::Vec3d> polygonBoundary = RigConvexHull::compute2d( cellCenters );
            polygonBoundary.push_back( polygonBoundary.front() );

            auto proj              = RimProject::current();
            auto polygonCollection = proj->activeOilField()->polygonCollection();

            auto newPolygon = polygonCollection->appendUserDefinedPolygon();
            newPolygon->setPointsInDomainCoords( polygonBoundary );
            newPolygon->coordinatesChanged.send();

            polygonCollection->uiCapability()->updateAllRequiredEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateWellTargetClusterPolygonsFeature::setupActionLook( QAction* actionToSetup )
{
    //    actionToSetup->setIcon( QIcon( ":/GeoMechCasePropTable24x24.png" ) );
    actionToSetup->setText( "Create Well Target Cluster Polygons" );
}

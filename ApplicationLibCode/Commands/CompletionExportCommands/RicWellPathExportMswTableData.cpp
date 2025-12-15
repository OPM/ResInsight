/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RicWellPathExportMswTableData.h"

#include "RiaLogging.h"

#include "RicExportCompletionDataSettingsUi.h"
#include "RicExportFractureCompletionsImpl.h"
#include "RicMswCompletions.h"
#include "RicMswExportInfo.h"
#include "RicMswTableDataTools.h"
#include "RicMswTableFormatterTools.h"
#include "RicMswValveAccumulators.h"

#include "CompletionsMsw/RigMswTableData.h"
#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"
#include "Well/RigWellLogExtractor.h"
#include "Well/RigWellPath.h"
#include "Well/RigWellPathIntersectionTools.h"

#include "RimEclipseCase.h"
#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimFractureTemplate.h"
#include "RimMswCompletionParameters.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#include "RimWellPathTieIn.h"
#include "RimWellPathValve.h"

#include <algorithm>

namespace internal
{
constexpr double VALVE_SEGMENT_LENGTH = 0.1;

}; // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RigMswTableData, std::string> RicWellPathExportMswTableData::extractSingleWellMswData( RimEclipseCase* eclipseCase,
                                                                                                     RimWellPath*    wellPath,
                                                                                                     int             timeStep,
                                                                                                     bool exportCompletionsAfterMainBoreSegments,
                                                                                                     CompletionType completionType )
{
    if ( !eclipseCase || !wellPath || eclipseCase->eclipseCaseData() == nullptr )
    {
        return std::unexpected( "Invalid eclipse case or well path provided for MSW data extraction" );
    }

    auto mswParameters = wellPath->mswCompletionParameters();
    if ( !mswParameters )
    {
        return std::unexpected( "Missing MSW completion parameters" );
    }

    auto   cellIntersections = generateCellSegments( eclipseCase, wellPath );
    double initialMD         = computeIntitialMeasuredDepth( eclipseCase, wellPath, mswParameters, cellIntersections );

    RiaDefines::EclipseUnitSystem unitSystem = eclipseCase->eclipseCaseData()->unitsType();
    RicMswExportInfo exportInfo( wellPath, unitSystem, initialMD, mswParameters->lengthAndDepth().text(), mswParameters->pressureDrop().text() );

    // Generate completion data based on the completion type parameter
    const bool createSegmentsForPerforations = ( completionType & CompletionType::PERFORATIONS ) == CompletionType::PERFORATIONS;
    if ( !generateWellSegmentsForMswExportInfo( eclipseCase,
                                                wellPath,
                                                createSegmentsForPerforations,
                                                timeStep,
                                                initialMD,
                                                cellIntersections,
                                                &exportInfo,
                                                exportInfo.mainBoreBranch() ) )
    {
        return std::unexpected( "Failed to generate perforations MSW export info" );
    }

    if ( ( completionType & CompletionType::FISHBONES ) == CompletionType::FISHBONES )
    {
        appendFishbonesMswExportInfo( eclipseCase, wellPath, initialMD, cellIntersections, &exportInfo, exportInfo.mainBoreBranch() );
    }

    if ( ( completionType & CompletionType::FRACTURES ) == CompletionType::FRACTURES )
    {
        appendFracturesMswExportInfo( eclipseCase, wellPath, initialMD, cellIntersections, &exportInfo, exportInfo.mainBoreBranch() );
    }

    updateDataForMultipleItemsInSameGridCell( exportInfo.mainBoreBranch() );

    // Assign branch numbers
    int branchNumber = 1;
    assignBranchNumbersToBranch( eclipseCase, &exportInfo, exportInfo.mainBoreBranch(), &branchNumber );

    // Create table data container and extract data
    RigMswTableData tableData( wellPath->completionSettings()->wellNameForExport().toStdString(), unitSystem );

    // Extract custom segment intervals from MSW parameters
    std::vector<std::pair<double, double>> customSegmentIntervals = mswParameters->getSegmentIntervals();

    // Use the new collection functions to populate the table data
    RicMswTableDataTools::collectWelsegsData( tableData,
                                              exportInfo,
                                              mswParameters->maxSegmentLength(),
                                              customSegmentIntervals,
                                              exportCompletionsAfterMainBoreSegments );

    {
        // Get COMPSEGS for main grid
        bool isLgr = false;
        RicMswTableDataTools::collectCompsegData( tableData, exportInfo, isLgr );
    }

    {
        // Get COMPSEGS for LGR grids
        bool isLgr = true;
        RicMswTableDataTools::collectCompsegData( tableData, exportInfo, isLgr );
    }

    RicMswTableDataTools::collectWsegvalvData( tableData, exportInfo );
    RicMswTableDataTools::collectWsegAicdData( tableData, exportInfo );

    return tableData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellPathExportMswTableData::CompletionType
    RicWellPathExportMswTableData::convertFromExportSettings( const RicExportCompletionDataSettingsUi& settings )
{
    CompletionType result = CompletionType::NONE;

    if ( settings.includePerforations() )
    {
        result |= CompletionType::PERFORATIONS;
    }

    if ( settings.includeFishbones() )
    {
        result |= CompletionType::FISHBONES;
    }

    if ( settings.includeFractures() )
    {
        result |= CompletionType::FRACTURES;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswTableData::generateFishbonesMswExportInfoForWell( const RimEclipseCase* eclipseCase,
                                                                           const RimWellPath*    wellPath,
                                                                           RicMswExportInfo*     exportInfo,
                                                                           RicMswBranch*         branch )
{
    if ( !eclipseCase || !wellPath || !eclipseCase->eclipseCaseData() || !exportInfo || !branch )
    {
        return;
    }

    auto mswParameters = wellPath->mswCompletionParameters();
    if ( !mswParameters )
    {
        return;
    }

    auto   cellIntersections = generateCellSegments( eclipseCase, wellPath );
    double initialMD         = computeIntitialMeasuredDepth( eclipseCase, wellPath, mswParameters, cellIntersections );

    int        timeStep                      = 0;
    const bool createSegmentsForPerforations = true;
    if ( !generateWellSegmentsForMswExportInfo( eclipseCase,
                                                wellPath,
                                                createSegmentsForPerforations,
                                                timeStep,
                                                initialMD,
                                                cellIntersections,
                                                exportInfo,
                                                exportInfo->mainBoreBranch() ) )
    {
        return;
    }

    appendFishbonesMswExportInfo( eclipseCase, wellPath, initialMD, cellIntersections, exportInfo, exportInfo->mainBoreBranch() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswTableData::updateDataForMultipleItemsInSameGridCell( gsl::not_null<RicMswBranch*> branch )
{
    auto allSegments = branch->allSegmentsRecursively();

    {
        // Update effective diameter
        // https://github.com/OPM/ResInsight/issues/7686

        std::map<size_t, std::set<RicMswSegment*>> segmentsInCell;
        {
            for ( auto s : allSegments )
            {
                auto cellsIntersected = s->globalCellsIntersected();
                if ( !cellsIntersected.empty() )
                {
                    for ( auto index : cellsIntersected )
                    {
                        segmentsInCell[index].insert( s );
                    }
                }
            }
        }

        for ( auto [index, segmentsInSameCell] : segmentsInCell )
        {
            // Compute effective diameter based on square root of the sum of diameter squared
            // Deff = sqrt(d1^2 + d2^2 + ..)
            double effectiveDiameter = 0.0;

            for ( auto seg : segmentsInSameCell )
            {
                effectiveDiameter += ( seg->equivalentDiameter() * seg->equivalentDiameter() );
            }

            effectiveDiameter = sqrt( effectiveDiameter );

            for ( auto seg : segmentsInSameCell )
            {
                seg->setEffectiveDiameter( effectiveDiameter );
            }
        }

        {
            // Reduce the diameter for segments in the same cell as main bore
            // https://github.com/OPM/ResInsight/issues/7731

            for ( auto s : allSegments )
            {
                for ( auto completion : s->completions() )
                {
                    if ( completion->completionType() == RigCompletionData::CompletionType::FISHBONES )
                    {
                        auto segments = completion->segments();
                        if ( segments.size() > 1 )
                        {
                            auto firstSegment  = segments[0];
                            auto secondSegment = segments[1];

                            double diameter = secondSegment->effectiveDiameter();

                            firstSegment->setEffectiveDiameter( diameter );
                        }
                    }
                }
            }
        }
    }

    {
        // Update IDC area

        std::map<size_t, std::set<RicMswFishbonesICD*>> icdsInCell;

        {
            for ( auto s : allSegments )
            {
                for ( auto completion : s->completions() )
                {
                    if ( auto icd = dynamic_cast<RicMswFishbonesICD*>( completion ) )
                    {
                        for ( auto icdSegment : icd->segments() )
                        {
                            for ( auto gridIntersection : icdSegment->intersections() )
                            {
                                icdsInCell[gridIntersection->globalCellIndex()].insert( icd );
                            }
                        }
                    }
                }
            }
        }

        for ( auto [Index, icdsInSameCell] : icdsInCell )
        {
            // Compute area sum for all ICDs in same grid cell
            double areaSum = 0.0;

            for ( auto icd : icdsInSameCell )
            {
                areaSum += icd->area();
            }

            for ( auto icd : icdsInSameCell )
            {
                icd->setArea( areaSum );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswTableData::appendFishbonesMswExportInfo( const RimEclipseCase*                            eclipseCase,
                                                                  const RimWellPath*                               wellPath,
                                                                  double                                           initialMD,
                                                                  const std::vector<WellPathCellIntersectionInfo>& cellIntersections,
                                                                  gsl::not_null<RicMswExportInfo*>                 exportInfo,
                                                                  gsl::not_null<RicMswBranch*>                     branch )
{
    std::vector<WellPathCellIntersectionInfo> filteredIntersections =
        filterIntersections( cellIntersections, initialMD, wellPath->wellPathGeometry(), eclipseCase );

    auto mswParameters = wellPath->mswCompletionParameters();

    bool foundSubGridIntersections = false;

    double maxSegmentLength = mswParameters->maxSegmentLength();

    auto unitSystem = exportInfo->unitSystem();

    auto fishbonesSubs = wellPath->completions()->fishbonesCollection()->activeFishbonesSubs();
    for ( RimFishbones* subs : fishbonesSubs )
    {
        std::map<size_t, std::vector<size_t>> subAndLateralIndices;
        for ( const auto& [subIndex, lateralIndex] : subs->installedLateralIndices() )
        {
            subAndLateralIndices[subIndex].push_back( lateralIndex );
        }

        // Find cell intersections closest to each sub location
        std::map<size_t, std::vector<size_t>> closestSubForCellIntersections;
        std::map<size_t, size_t>              cellIntersectionContainingSubIndex;
        {
            auto fishboneSectionStart = subs->startMD();
            auto fishboneSectionEnd   = subs->endMD();

            for ( size_t intersectionIndex = 0; intersectionIndex < filteredIntersections.size(); intersectionIndex++ )
            {
                const auto& cellIntersection = filteredIntersections[intersectionIndex];
                if ( ( fishboneSectionEnd >= cellIntersection.startMD ) && ( fishboneSectionStart <= cellIntersection.endMD ) )

                {
                    double intersectionMidpoint = 0.5 * ( cellIntersection.startMD + cellIntersection.endMD );
                    size_t closestSubIndex      = 0;
                    double closestDistance      = std::numeric_limits<double>::infinity();
                    for ( const auto& [subIndex, lateralIndices] : subAndLateralIndices )
                    {
                        double subMD = subs->measuredDepth( subIndex );

                        if ( ( cellIntersection.startMD <= subMD ) && ( subMD <= cellIntersection.endMD ) )
                        {
                            cellIntersectionContainingSubIndex[subIndex] = intersectionIndex;
                        }

                        auto distanceCandicate = std::abs( subMD - intersectionMidpoint );
                        if ( distanceCandicate < closestDistance )
                        {
                            closestDistance = distanceCandicate;
                            closestSubIndex = subIndex;
                        }
                    }

                    closestSubForCellIntersections[closestSubIndex].push_back( intersectionIndex );
                }
            }
        }

        for ( const auto& [subIndex, lateralIndices] : subAndLateralIndices )
        {
            const double subEndMd      = subs->measuredDepth( subIndex );
            const double subEndTvd     = RicMswTableFormatterTools::tvdFromMeasuredDepth( branch->wellPath(), subEndMd );
            const double startValveMd  = subEndMd - internal::VALVE_SEGMENT_LENGTH;
            const double startValveTvd = RicMswTableFormatterTools::tvdFromMeasuredDepth( branch->wellPath(), startValveMd );

            {
                // Add completion for ICD. Insert the segment at the end of the fishbone section. The laterals flows into the ICD
                // segment, and the simulator requires increasing MD on laterals. Make sure that the lateral MDs are larger than the ICD
                // segment MDs.
                auto icdSegment = std::make_unique<RicMswSegment>( "ICD segment", startValveMd, subEndMd, startValveTvd, subEndTvd, subIndex );

                for ( auto lateralIndex : lateralIndices )
                {
                    QString label = QString( "Lateral %1" ).arg( lateralIndex + 1 );
                    icdSegment->addCompletion( std::make_unique<RicMswFishbones>( label, wellPath, subEndMd, subEndTvd, lateralIndex ) );
                }

                assignFishbonesLateralIntersections( eclipseCase,
                                                     branch->wellPath(),
                                                     subs,
                                                     icdSegment.get(),
                                                     &foundSubGridIntersections,
                                                     maxSegmentLength,
                                                     unitSystem );

                auto icdCompletion = std::make_unique<RicMswFishbonesICD>( QString( "ICD" ), wellPath, subEndMd, subEndTvd, nullptr );
                icdCompletion->setFlowCoefficient( subs->icdFlowCoefficient() );
                double icdOrificeRadius = subs->icdOrificeDiameter( unitSystem ) / 2;
                icdCompletion->setArea( icdOrificeRadius * icdOrificeRadius * cvf::PI_D * subs->icdCount() );

                // assign open hole segments to sub
                {
                    const RigMainGrid* mainGrid = eclipseCase->mainGrid();

                    std::set<size_t> indices;
                    for ( auto intersectionIndex : closestSubForCellIntersections[subIndex] )
                    {
                        indices.insert( intersectionIndex );
                    }

                    indices.insert( cellIntersectionContainingSubIndex[subIndex] );

                    for ( auto intersectionIndex : indices )
                    {
                        auto intersection = filteredIntersections[intersectionIndex];
                        if ( intersection.globCellIndex >= mainGrid->totalCellCount() ) continue;

                        size_t             localGridCellIndex = 0u;
                        const RigGridBase* localGrid =
                            mainGrid->gridAndGridLocalIdxFromGlobalCellIdx( intersection.globCellIndex, &localGridCellIndex );
                        QString gridName;
                        if ( localGrid != mainGrid )
                        {
                            gridName                  = QString::fromStdString( localGrid->gridName() );
                            foundSubGridIntersections = true;
                        }

                        size_t i, j, k;
                        localGrid->ijkFromCellIndex( localGridCellIndex, &i, &j, &k );
                        caf::VecIjk0 localIJK( i, j, k );

                        auto mswIntersect = std::make_shared<RicMswSegmentCellIntersection>( gridName,
                                                                                             intersection.globCellIndex,
                                                                                             localIJK,
                                                                                             intersection.intersectionLengthsInCellCS );
                        icdSegment->addIntersection( mswIntersect );
                    }
                }

                icdCompletion->addSegment( std::move( icdSegment ) );

                RicMswSegment* segmentOnParentBranch = branch->findClosestSegmentWithLowerMD( subEndMd );
                if ( segmentOnParentBranch )
                {
                    segmentOnParentBranch->addCompletion( std::move( icdCompletion ) );
                }
            }
        }
    }
    exportInfo->setHasSubGridIntersections( exportInfo->hasSubGridIntersections() || foundSubGridIntersections );

    branch->branches();
    for ( auto& childBranch : branch->branches() )
    {
        auto childWellPath          = childBranch->wellPath();
        auto childCellIntersections = generateCellSegments( eclipseCase, childWellPath );
        appendFishbonesMswExportInfo( eclipseCase, childWellPath, initialMD, childCellIntersections, exportInfo, childBranch );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellPathExportMswTableData::appendFracturesMswExportInfo( RimEclipseCase*                                  eclipseCase,
                                                                  const RimWellPath*                               wellPath,
                                                                  double                                           initialMD,
                                                                  const std::vector<WellPathCellIntersectionInfo>& cellIntersections,
                                                                  gsl::not_null<RicMswExportInfo*>                 exportInfo,
                                                                  gsl::not_null<RicMswBranch*>                     branch )
{
    auto fractures = wellPath->fractureCollection()->activeFractures();
    if ( !fractures.empty() )
    {
        std::vector<WellPathCellIntersectionInfo> filteredIntersections =
            filterIntersections( cellIntersections, initialMD, wellPath->wellPathGeometry(), eclipseCase );

        bool foundSubGridIntersections = false;

        // Check if fractures are to be assigned to current main bore segment
        for ( RimWellPathFracture* fracture : fractures )
        {
            fracture->ensureValidNonDarcyProperties();

            double fractureStartMD = fracture->fractureMD();
            if ( fracture->fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH )
            {
                double perforationLength = fracture->fractureTemplate()->perforationLength();
                fractureStartMD -= 0.5 * perforationLength;
            }

            auto segment = branch->findClosestSegmentWithLowerMD( fractureStartMD );
            if ( segment )
            {
                std::vector<RigCompletionData> completionData =
                    RicExportFractureCompletionsImpl::generateCompdatValues( eclipseCase,
                                                                             wellPath->completionSettings()->wellNameForExport(),
                                                                             wellPath->wellPathGeometry(),
                                                                             { fracture },
                                                                             nullptr,
                                                                             nullptr );

                assignFractureCompletionsToCellSegment( eclipseCase, wellPath, fracture, completionData, segment, &foundSubGridIntersections );
            }
        }

        exportInfo->setHasSubGridIntersections( exportInfo->hasSubGridIntersections() || foundSubGridIntersections );
    }

    for ( auto& childBranch : branch->branches() )
    {
        auto childWellPath          = childBranch->wellPath();
        auto childCellIntersections = generateCellSegments( eclipseCase, childWellPath );

        appendFracturesMswExportInfo( eclipseCase, childWellPath, initialMD, childCellIntersections, exportInfo, childBranch );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellPathExportMswTableData::generateWellSegmentsForMswExportInfo( const RimEclipseCase* eclipseCase,
                                                                          const RimWellPath*    wellPath,
                                                                          bool                  createSegmentsForPerforations,
                                                                          int                   timeStep,
                                                                          double                initialMD,
                                                                          const std::vector<WellPathCellIntersectionInfo>& cellIntersections,
                                                                          gsl::not_null<RicMswExportInfo*> exportInfo,
                                                                          gsl::not_null<RicMswBranch*>     branch )
{
    auto perforationIntervals = createSegmentsForPerforations ? wellPath->perforationIntervalCollection()->activePerforations()
                                                              : std::vector<const RimPerforationInterval*>();

    // Check if there exist overlap between valves in a perforation interval
    for ( const auto& perfInterval : perforationIntervals )
    {
        for ( const auto& valve : perfInterval->valves() )
        {
            for ( const auto& otherValve : perfInterval->valves() )
            {
                if ( otherValve != valve )
                {
                    bool hasIntersection = ( valve->endMD() >= otherValve->startMD() ) && ( otherValve->endMD() >= valve->startMD() );

                    if ( hasIntersection )
                    {
                        RiaLogging::error( QString( "Valve overlap detected for perforation interval : %1" ).arg( perfInterval->name() ) );

                        RiaLogging::error( "Name of valves" );
                        RiaLogging::error( valve->name() );
                        RiaLogging::error( otherValve->name() );

                        RiaLogging::error( "Failed to export well segments" );

                        return false;
                    }
                }
            }
        }
    }

    std::vector<WellPathCellIntersectionInfo> filteredIntersections =
        filterIntersections( cellIntersections, initialMD, wellPath->wellPathGeometry(), eclipseCase );

    bool foundSubGridIntersections = false;

    createWellPathSegments( branch, filteredIntersections, perforationIntervals, wellPath, timeStep, eclipseCase, &foundSubGridIntersections );

    createValveCompletions( branch, perforationIntervals, wellPath, exportInfo->unitSystem() );

    const RigActiveCellInfo* activeCellInfo = eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    assignValveContributionsToSuperICDsOrAICDs( branch, perforationIntervals, filteredIntersections, activeCellInfo, exportInfo->unitSystem() );
    moveIntersectionsToICVs( branch, perforationIntervals, exportInfo->unitSystem() );
    moveIntersectionsToSuperICDsOrAICDs( branch );

    exportInfo->setHasSubGridIntersections( exportInfo->hasSubGridIntersections() || foundSubGridIntersections );
    branch->sortSegments();

    auto connectedWellPaths = wellPathsWithTieIn( wellPath );

    for ( auto childWellPath : connectedWellPaths )
    {
        auto childMswBranch         = createChildMswBranch( childWellPath );
        auto childCellIntersections = generateCellSegments( eclipseCase, childWellPath );

        // Start MD of child well path at the tie in location
        const double tieInOnParentWellPath = childWellPath->wellPathTieIn() ? childWellPath->wellPathTieIn()->tieInMeasuredDepth() : initialMD;

        if ( generateWellSegmentsForMswExportInfo( eclipseCase,
                                                   childWellPath,
                                                   createSegmentsForPerforations,
                                                   timeStep,
                                                   tieInOnParentWellPath,
                                                   childCellIntersections,
                                                   exportInfo,
                                                   childMswBranch.get() ) )
        {
            branch->addChildBranch( std::move( childMswBranch ) );
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo> RicWellPathExportMswTableData::generateCellSegments( const RimEclipseCase* eclipseCase,
                                                                                               const RimWellPath*    wellPath )
{
    auto wellPathGeometry = wellPath->wellPathGeometry();
    CVF_ASSERT( wellPathGeometry );

    const std::vector<cvf::Vec3d>& coords = wellPathGeometry->uniqueWellPathPoints();
    const std::vector<double>&     mds    = wellPathGeometry->uniqueMeasuredDepths();
    CVF_ASSERT( !coords.empty() && !mds.empty() );

    const RigMainGrid* mainGrid = eclipseCase->mainGrid();

    std::vector<WellPathCellIntersectionInfo> allIntersections =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( eclipseCase->eclipseCaseData(), wellPath->name(), coords, mds );
    if ( allIntersections.empty() ) return {};

    std::vector<WellPathCellIntersectionInfo> continuousIntersections =
        RigWellPathIntersectionTools::buildContinuousIntersections( allIntersections, mainGrid );

    return continuousIntersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathExportMswTableData::computeIntitialMeasuredDepth( const RimEclipseCase*                            eclipseCase,
                                                                    const RimWellPath*                               wellPath,
                                                                    const RimMswCompletionParameters*                mswParameters,
                                                                    const std::vector<WellPathCellIntersectionInfo>& allIntersections )
{
    if ( allIntersections.empty() ) return 0.0;

    const RigActiveCellInfo* activeCellInfo = eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    double candidateMeasuredDepth = 0.0;
    if ( mswParameters->referenceMDType() == RimMswCompletionParameters::ReferenceMDType::MANUAL_REFERENCE_MD )
    {
        candidateMeasuredDepth = mswParameters->manualReferenceMD();
    }
    else
    {
        for ( const WellPathCellIntersectionInfo& intersection : allIntersections )
        {
            if ( activeCellInfo->isActive( intersection.globCellIndex ) )
            {
                candidateMeasuredDepth = intersection.startMD;
                break;
            }
        }

        double startOfFirstCompletion = std::numeric_limits<double>::infinity();
        {
            std::vector<const RimWellPathComponentInterface*> allCompletions = wellPath->completions()->allCompletions();

            for ( const RimWellPathComponentInterface* completion : allCompletions )
            {
                if ( completion->isEnabled() && completion->startMD() < startOfFirstCompletion )
                {
                    startOfFirstCompletion = completion->startMD();
                }
            }
        }

        // Initial MD is the lowest MD based on grid intersection and start of fracture completions
        // https://github.com/OPM/ResInsight/issues/6071
        candidateMeasuredDepth = std::min( candidateMeasuredDepth, startOfFirstCompletion );
    }

    return candidateMeasuredDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo>
    RicWellPathExportMswTableData::filterIntersections( const std::vector<WellPathCellIntersectionInfo>& intersections,
                                                        double                                           initialMD,
                                                        gsl::not_null<const RigWellPath*>                wellPathGeometry,
                                                        gsl::not_null<const RimEclipseCase*>             eclipseCase )
{
    std::vector<WellPathCellIntersectionInfo> filteredIntersections;

    if ( !intersections.empty() && intersections[0].startMD > initialMD )
    {
        WellPathCellIntersectionInfo firstIntersection = intersections[0];

        // Add a segment from user defined MD to start of grid
        cvf::Vec3d intersectionPoint = wellPathGeometry->interpolatedPointAlongWellPath( initialMD );

        WellPathCellIntersectionInfo extraIntersection;

        extraIntersection.globCellIndex         = std::numeric_limits<size_t>::max();
        extraIntersection.startPoint            = intersectionPoint;
        extraIntersection.endPoint              = firstIntersection.startPoint;
        extraIntersection.startMD               = initialMD;
        extraIntersection.endMD                 = firstIntersection.startMD;
        extraIntersection.intersectedCellFaceIn = cvf::StructGridInterface::NO_FACE;

        if ( firstIntersection.intersectedCellFaceIn != cvf::StructGridInterface::NO_FACE )

        {
            extraIntersection.intersectedCellFaceOut = cvf::StructGridInterface::oppositeFace( firstIntersection.intersectedCellFaceIn );
        }
        else if ( firstIntersection.intersectedCellFaceOut != cvf::StructGridInterface::NO_FACE )
        {
            extraIntersection.intersectedCellFaceOut = firstIntersection.intersectedCellFaceOut;
        }

        extraIntersection.intersectionLengthsInCellCS = cvf::Vec3d::ZERO;

        filteredIntersections.push_back( extraIntersection );
    }

    const double epsilon = 1.0e-3;

    for ( const WellPathCellIntersectionInfo& intersection : intersections )
    {
        if ( ( intersection.endMD - initialMD ) < epsilon )
        {
            // Skip all intersections before initial measured depth
            continue;
        }

        if ( ( intersection.startMD - initialMD ) > epsilon )
        {
            filteredIntersections.push_back( intersection );
        }
        else
        {
            // InitialMD is inside intersection, split based on intersection point

            cvf::Vec3d intersectionPoint = wellPathGeometry->interpolatedPointAlongWellPath( initialMD );

            WellPathCellIntersectionInfo extraIntersection;

            extraIntersection.globCellIndex          = intersection.globCellIndex;
            extraIntersection.startPoint             = intersectionPoint;
            extraIntersection.endPoint               = intersection.endPoint;
            extraIntersection.startMD                = initialMD;
            extraIntersection.endMD                  = intersection.endMD;
            extraIntersection.intersectedCellFaceIn  = cvf::StructGridInterface::NO_FACE;
            extraIntersection.intersectedCellFaceOut = intersection.intersectedCellFaceOut;

            const RigMainGrid* grid = eclipseCase->mainGrid();

            if ( intersection.globCellIndex < grid->cellCount() )
            {
                extraIntersection.intersectionLengthsInCellCS =
                    RigWellPathIntersectionTools::calculateLengthInCell( grid, intersection.globCellIndex, intersectionPoint, intersection.endPoint );
            }
            else
            {
                extraIntersection.intersectionLengthsInCellCS = cvf::Vec3d::ZERO;
            }

            filteredIntersections.push_back( extraIntersection );
        }
    }

    return filteredIntersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswTableData::createWellPathSegments( gsl::not_null<RicMswBranch*>                      branch,
                                                            const std::vector<WellPathCellIntersectionInfo>&  cellSegmentIntersections,
                                                            const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                                            const RimWellPath*                                wellPath,
                                                            int                                               timeStep,
                                                            const RimEclipseCase*                             eclipseCase,
                                                            bool*                                             foundSubGridIntersections )
{
    // Intersections along the well path with grid geometry is handled by well log extraction tools.
    // The threshold in RigWellLogExtractionTools::isEqualDepth is currently set to 0.1m, and this
    // is a pretty large threshold based on the indicated threshold of 0.001m for MSW segments
    const double segmentLengthThreshold = 1.0e-3;

    for ( const auto& cellIntInfo : cellSegmentIntersections )
    {
        const double segmentLength = std::fabs( cellIntInfo.endMD - cellIntInfo.startMD );

        if ( segmentLength > segmentLengthThreshold )
        {
            auto segment = std::make_unique<RicMswSegment>( QString( "%1 segment" ).arg( branch->label() ),
                                                            cellIntInfo.startMD,
                                                            cellIntInfo.endMD,
                                                            cellIntInfo.startTVD(),
                                                            cellIntInfo.endTVD() );

            for ( const RimPerforationInterval* interval : perforationIntervals )
            {
                double overlapStart = std::max( interval->startMD(), segment->startMD() );
                double overlapEnd   = std::min( interval->endMD(), segment->endMD() );
                double overlap      = std::max( 0.0, overlapEnd - overlapStart );
                if ( overlap > 0.0 )
                {
                    double overlapStartTVD = -wellPath->wellPathGeometry()->interpolatedPointAlongWellPath( overlapStart ).z();
                    auto intervalCompletion = std::make_unique<RicMswPerforation>( interval->name(), wellPath, overlapStart, overlapStartTVD );
                    std::vector<RigCompletionData> completionData =
                        generatePerforationIntersections( wellPath, interval, timeStep, eclipseCase );
                    assignPerforationIntersections( completionData,
                                                    intervalCompletion.get(),
                                                    cellIntInfo,
                                                    overlapStart,
                                                    overlapEnd,
                                                    foundSubGridIntersections );
                    segment->addCompletion( std::move( intervalCompletion ) );
                }
            }
            branch->addSegment( std::move( segment ) );
        }
        else
        {
            QString text = QString( "Skipping segment , threshold = %1, length = %2" ).arg( segmentLengthThreshold ).arg( segmentLength );
            RiaLogging::info( text );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswTableData::createValveCompletions( gsl::not_null<RicMswBranch*>                      branch,
                                                            const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                                            const RimWellPath*                                wellPath,
                                                            RiaDefines::EclipseUnitSystem                     unitSystem )
{
    int  nMainSegment = 0;
    auto segments     = branch->segments();
    for ( auto segment : segments )
    {
        std::unique_ptr<RicMswPerforationICV>  ICV;
        std::unique_ptr<RicMswPerforationICD>  superICD;
        std::unique_ptr<RicMswPerforationAICD> superAICD;

        double totalICDOverlap  = 0.0;
        double totalAICDOverlap = 0.0;

        for ( const RimPerforationInterval* interval : perforationIntervals )
        {
            if ( !interval->isChecked() ) continue;

            auto perforationValves = interval->descendantsIncludingThisOfType<RimWellPathValve>();
            for ( const RimWellPathValve* valve : perforationValves )
            {
                if ( !valve->isChecked() ) continue;

                for ( size_t nSubValve = 0u; nSubValve < valve->valveLocations().size(); ++nSubValve )
                {
                    double valveMD = valve->valveLocations()[nSubValve];

                    std::pair<double, double> valveSegment = valve->valveSegments()[nSubValve];
                    double                    overlapStart = std::max( valveSegment.first, segment->startMD() );
                    double                    overlapEnd   = std::min( valveSegment.second, segment->endMD() );
                    double                    overlap      = std::max( 0.0, overlapEnd - overlapStart );

                    double exportStartMD = valveMD;
                    double exportEndMD   = valveMD + internal::VALVE_SEGMENT_LENGTH;

                    double exportStartTVD = RicMswTableFormatterTools::tvdFromMeasuredDepth( wellPath, exportStartMD );
                    double exportEndTVD   = RicMswTableFormatterTools::tvdFromMeasuredDepth( wellPath, exportEndMD );

                    if ( segment->startMD() <= valveMD && valveMD < segment->endMD() )
                    {
                        if ( valve->componentType() == RiaDefines::WellPathComponentType::AICD )
                        {
                            QString valveLabel = QString( "%1 #%2" ).arg( "Combined Valve for segment" ).arg( nMainSegment + 2 );
                            auto    subSegment =
                                std::make_unique<RicMswSegment>( "Valve segment", exportStartMD, exportEndMD, exportStartTVD, exportEndTVD );

                            superAICD = std::make_unique<RicMswPerforationAICD>( valveLabel, wellPath, exportStartMD, exportStartTVD, valve );
                            superAICD->addSegment( std::move( subSegment ) );
                        }
                        else if ( valve->componentType() == RiaDefines::WellPathComponentType::ICD )
                        {
                            QString valveLabel = QString( "%1 #%2" ).arg( "Combined Valve for segment" ).arg( nMainSegment + 2 );
                            auto    subSegment =
                                std::make_unique<RicMswSegment>( "Valve segment", exportStartMD, exportEndMD, exportStartTVD, exportEndTVD );

                            superICD = std::make_unique<RicMswPerforationICD>( valveLabel, wellPath, exportStartMD, exportStartTVD, valve );
                            superICD->addSegment( std::move( subSegment ) );
                        }
                        else if ( valve->componentType() == RiaDefines::WellPathComponentType::ICV )
                        {
                            QString valveLabel = QString( "ICV %1 at segment #%2" ).arg( valve->name() ).arg( nMainSegment + 2 );
                            auto    subSegment =
                                std::make_unique<RicMswSegment>( "Valve segment", exportStartMD, exportEndMD, exportStartTVD, exportEndTVD );

                            ICV = std::make_unique<RicMswPerforationICV>( valveLabel, wellPath, exportStartMD, exportStartTVD, valve );
                            ICV->addSegment( std::move( subSegment ) );
                        }
                    }
                    else if ( overlap > 0.0 && ( valve->componentType() == RiaDefines::WellPathComponentType::ICD && !superICD ) )
                    {
                        QString valveLabel = QString( "%1 #%2" ).arg( "Combined Valve for segment" ).arg( nMainSegment + 2 );

                        auto subSegment =
                            std::make_unique<RicMswSegment>( "Valve segment", exportStartMD, exportEndMD, exportStartTVD, exportEndTVD );
                        superICD = std::make_unique<RicMswPerforationICD>( valveLabel, wellPath, exportStartMD, exportStartTVD, valve );
                        superICD->addSegment( std::move( subSegment ) );
                    }
                    else if ( overlap > 0.0 && ( valve->componentType() == RiaDefines::WellPathComponentType::AICD && !superAICD ) )
                    {
                        QString valveLabel = QString( "%1 #%2" ).arg( "Combined Valve for segment" ).arg( nMainSegment + 2 );

                        auto subSegment =
                            std::make_unique<RicMswSegment>( "Valve segment", exportStartMD, exportEndMD, exportStartTVD, exportEndTVD );
                        superAICD = std::make_unique<RicMswPerforationAICD>( valveLabel, wellPath, exportStartMD, exportStartTVD, valve );
                        superAICD->addSegment( std::move( subSegment ) );
                    }

                    if ( valve->componentType() == RiaDefines::WellPathComponentType::AICD )
                    {
                        totalAICDOverlap += overlap;
                    }
                    else if ( valve->componentType() == RiaDefines::WellPathComponentType::ICD )
                    {
                        totalICDOverlap += overlap;
                    }
                }
            }
        }

        if ( ICV )
        {
            segment->addCompletion( std::move( ICV ) );
        }
        else
        {
            if ( totalICDOverlap > 0.0 || totalAICDOverlap > 0.0 )
            {
                if ( totalAICDOverlap > totalICDOverlap )
                {
                    segment->addCompletion( std::move( superAICD ) );
                }
                else
                {
                    segment->addCompletion( std::move( superICD ) );
                }
            }
        }
        nMainSegment++;
    }
}

//--------------------------------------------------------------------------------------------------
/// Aggregates individual valve parameters from perforation intervals into segment-level "super"
/// valves (ICDs or AICDs) for MSW export. Calculates weighted averages based on overlap with
/// active reservoir cells.
///
/// Key steps:
/// 1. Setup Phase: Creates accumulator objects (ICD or AICD) for each segment containing a super valve
/// 2. First Pass: Calculates total perforation length overlapping with active cells for each valve
/// 3. Second Pass: For each valve, calculates overlap with segments and accumulates parameters
///    weighted by overlap length. Only considers overlaps with active cells.
/// 4. Apply Phase: Applies accumulated weighted averages to super valve parameters
/// 5. Label Update: Appends contributor valve names to super valve labels for documentation
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswTableData::assignValveContributionsToSuperICDsOrAICDs(
    gsl::not_null<RicMswBranch*>                      branch,
    const std::vector<const RimPerforationInterval*>& perforationIntervals,
    const std::vector<WellPathCellIntersectionInfo>&  wellPathIntersections,
    const RigActiveCellInfo*                          activeCellInfo,
    RiaDefines::EclipseUnitSystem                     unitSystem )
{
    using ValveContributionMap = std::map<RicMswCompletion*, std::vector<const RimWellPathValve*>>;

    ValveContributionMap assignedRegularValves;

    std::map<RicMswSegment*, std::unique_ptr<RicMswValveAccumulator>> accumulators;

    for ( auto segment : branch->segments() )
    {
        RicMswValve* superValve = nullptr;
        for ( auto completion : segment->completions() )
        {
            auto valve = dynamic_cast<RicMswValve*>( completion );
            if ( valve )
            {
                superValve = valve;
                break;
            }
        }
        if ( dynamic_cast<RicMswPerforationICD*>( superValve ) )
        {
            accumulators[segment] = std::make_unique<RicMswICDAccumulator>( superValve, unitSystem );
        }
        else if ( dynamic_cast<RicMswPerforationAICD*>( superValve ) )
        {
            accumulators[segment] = std::make_unique<RicMswAICDAccumulator>( superValve, unitSystem );
        }
    }

    for ( const RimPerforationInterval* interval : perforationIntervals )
    {
        if ( !interval->isChecked() ) continue;

        std::vector<const RimWellPathValve*> perforationValves      = interval->descendantsIncludingThisOfType<const RimWellPathValve>();
        double                               totalPerforationLength = 0.0;
        for ( const RimWellPathValve* valve : perforationValves )
        {
            if ( !valve->isChecked() ) continue;

            for ( auto segment : branch->segments() )
            {
                double intervalOverlapStart = std::max( interval->startMD(), segment->startMD() );
                double intervalOverlapEnd   = std::min( interval->endMD(), segment->endMD() );
                auto   intervalOverlapWithActiveCells =
                    calculateOverlapWithActiveCells( intervalOverlapStart, intervalOverlapEnd, wellPathIntersections, activeCellInfo );

                totalPerforationLength += intervalOverlapWithActiveCells.second - intervalOverlapWithActiveCells.first;
            }
        }

        for ( const RimWellPathValve* valve : perforationValves )
        {
            if ( !valve->isChecked() ) continue;

            for ( auto segment : branch->segments() )
            {
                double intervalOverlapStart = std::max( interval->startMD(), segment->startMD() );
                double intervalOverlapEnd   = std::min( interval->endMD(), segment->endMD() );

                auto intervalOverlapWithActiveCells =
                    calculateOverlapWithActiveCells( intervalOverlapStart, intervalOverlapEnd, wellPathIntersections, activeCellInfo );

                double overlapLength = intervalOverlapWithActiveCells.second - intervalOverlapWithActiveCells.first;
                if ( overlapLength > 0.0 )
                {
                    auto it = accumulators.find( segment );

                    if ( it != accumulators.end() )
                    {
                        it->second->accumulateValveParameters( valve, overlapLength, totalPerforationLength );
                        assignedRegularValves[it->second->superValve()].push_back( valve );
                    }
                }
            }
        }
    }

    for ( const auto& accumulator : accumulators )
    {
        accumulator.second->applyToSuperValve();
    }

    for ( auto regularValvePair : assignedRegularValves )
    {
        if ( !regularValvePair.second.empty() )
        {
            QStringList valveLabels;
            for ( const RimWellPathValve* regularValve : regularValvePair.second )
            {
                QString valveLabel = QString( "%1" ).arg( regularValve->name() );
                valveLabels.push_back( valveLabel );
            }
            QString valveContribLabel = QString( " with contribution from: %1" ).arg( valveLabels.join( ", " ) );
            regularValvePair.first->setLabel( regularValvePair.first->label() + valveContribLabel );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswTableData::moveIntersectionsToICVs( gsl::not_null<RicMswBranch*>                      branch,
                                                             const std::vector<const RimPerforationInterval*>& perforationIntervals,
                                                             RiaDefines::EclipseUnitSystem                     unitSystem )
{
    std::map<const RimWellPathValve*, RicMswPerforationICV*> icvCompletionMap;

    for ( auto segment : branch->segments() )
    {
        for ( auto completion : segment->completions() )
        {
            auto icv = dynamic_cast<RicMswPerforationICV*>( completion );
            if ( icv )
            {
                icvCompletionMap[icv->wellPathValve()] = icv;
            }
        }
    }

    for ( auto segment : branch->segments() )
    {
        std::vector<RicMswCompletion*> perforations;
        for ( auto completion : segment->completions() )
        {
            if ( completion->completionType() == RigCompletionData::CompletionType::PERFORATION )
            {
                perforations.push_back( completion );
            }
        }

        for ( const RimPerforationInterval* interval : perforationIntervals )
        {
            if ( !interval->isChecked() ) continue;

            std::vector<const RimWellPathValve*> perforationValves = interval->descendantsIncludingThisOfType<const RimWellPathValve>();

            for ( const RimWellPathValve* valve : perforationValves )
            {
                if ( !valve->isChecked() ) continue;
                if ( valve->componentType() != RiaDefines::WellPathComponentType::ICV ) continue;

                auto icvIt = icvCompletionMap.find( valve );
                if ( icvIt == icvCompletionMap.end() ) continue;

                auto icvCompletion = icvIt->second;
                CVF_ASSERT( icvCompletion );

                std::pair<double, double> valveSegment = valve->valveSegments().front();
                double                    overlapStart = std::max( valveSegment.first, segment->startMD() );
                double                    overlapEnd   = std::min( valveSegment.second, segment->endMD() );
                double                    overlap      = std::max( 0.0, overlapEnd - overlapStart );

                if ( overlap > 0.0 )
                {
                    CVF_ASSERT( icvCompletion->segments().size() == 1u );
                    for ( auto perforationPtr : perforations )
                    {
                        for ( auto subSegmentPtr : perforationPtr->segments() )
                        {
                            for ( auto intersectionPtr : subSegmentPtr->intersections() )
                            {
                                icvCompletion->segments()[0]->addIntersection( intersectionPtr );
                            }
                        }
                        segment->removeCompletion( perforationPtr );
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswTableData::moveIntersectionsToSuperICDsOrAICDs( gsl::not_null<RicMswBranch*> branch )
{
    for ( auto segment : branch->segments() )
    {
        RicMswCompletion*              superValve = nullptr;
        std::vector<RicMswCompletion*> perforations;
        for ( auto completion : segment->completions() )
        {
            if ( completion->completionType() == RigCompletionData::CompletionType::PERFORATION_ICD ||
                 completion->completionType() == RigCompletionData::CompletionType::PERFORATION_AICD )
            {
                superValve = completion;
            }
            else if ( completion->completionType() == RigCompletionData::CompletionType::PERFORATION )
            {
                perforations.push_back( completion );
            }
        }

        if ( superValve == nullptr ) continue;

        CVF_ASSERT( superValve->segments().size() == 1u );

        // Remove and take over ownership of the superValve completion
        auto completionPtr = segment->removeCompletion( superValve );
        for ( auto perforation : perforations )
        {
            for ( auto subSegment : perforation->segments() )
            {
                // The valve completions on the main branch will be deleted. Create a segment with startMD and
                // endMD representing the perforation along main well path to be connected to the valve. When COMPSEGS
                // data is exported, the startMD and endMD of the segment is used to define the Start Length and End
                // Length of the COMPSEGS keyword
                //
                // Example output
                //
                // COMPSEGS
                // --Name
                //     Well - 1 /
                // --I      J      K      Branch no     Start Length     End Length
                //   17     17     9      2             3030.71791       3034.01331 /
                //   17     18     9      3             3034.01331       3125.47617 /

                auto valveInflowSegment = std::make_unique<RicMswSegment>( QString( "%1 real valve segment " ).arg( branch->label() ),
                                                                           subSegment->startMD(),
                                                                           subSegment->endMD(),
                                                                           subSegment->startTVD(),
                                                                           subSegment->endTVD() );

                for ( auto intersectionPtr : subSegment->intersections() )
                {
                    valveInflowSegment->addIntersection( intersectionPtr );
                }

                {
                    double midpoint = ( segment->startMD() + segment->endMD() ) * 0.5;

                    // Set the output MD to the midpoint of the segment, this info is used when exporting WELSEGS in
                    // RicMswTableFormatterTools::writeValveWelsegsSegment
                    completionPtr->segments()[0]->setOutputMD( midpoint );
                }
                completionPtr->addSegment( std::move( valveInflowSegment ) );
            }
        }

        // Remove all completions and re-add the super valve
        segment->deleteAllCompletions();
        segment->addCompletion( std::move( completionPtr ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswTableData::assignFishbonesLateralIntersections( const RimEclipseCase*         eclipseCase,
                                                                         const RimWellPath*            wellPath,
                                                                         const RimFishbones*           fishbonesSubs,
                                                                         gsl::not_null<RicMswSegment*> segment,
                                                                         bool*                         foundSubGridIntersections,
                                                                         double                        maxSegmentLength,
                                                                         RiaDefines::EclipseUnitSystem unitSystem )
{
    CVF_ASSERT( foundSubGridIntersections != nullptr );

    const RigMainGrid* grid = eclipseCase->eclipseCaseData()->mainGrid();

    for ( auto completion : segment->completions() )
    {
        if ( completion->completionType() != RigCompletionData::CompletionType::FISHBONES )
        {
            continue;
        }

        std::vector<std::pair<cvf::Vec3d, double>> lateralCoordMDPairs =
            fishbonesSubs->coordsAndMDForLateral( segment->subIndex(), completion->index() );

        if ( lateralCoordMDPairs.empty() )
        {
            continue;
        }

        std::vector<cvf::Vec3d> lateralCoords;
        std::vector<double>     lateralMDs;

        lateralCoords.reserve( lateralCoordMDPairs.size() );
        lateralMDs.reserve( lateralCoordMDPairs.size() );

        for ( auto& coordMD : lateralCoordMDPairs )
        {
            lateralCoords.push_back( coordMD.first );
            lateralMDs.push_back( coordMD.second );
        }

        std::vector<WellPathCellIntersectionInfo> intersections =
            RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( eclipseCase->eclipseCaseData(),
                                                                              wellPath->name(),
                                                                              lateralCoords,
                                                                              lateralMDs );

        double previousExitMD  = lateralMDs.front();
        double previousExitTVD = -lateralCoords.front().z();

        for ( const auto& cellIntInfo : intersections )
        {
            size_t             localGridCellIndex = 0u;
            const RigGridBase* localGrid = grid->gridAndGridLocalIdxFromGlobalCellIdx( cellIntInfo.globCellIndex, &localGridCellIndex );
            QString            gridName;
            if ( localGrid != grid )
            {
                gridName                   = QString::fromStdString( localGrid->gridName() );
                *foundSubGridIntersections = true;
            }

            size_t i = 0u, j = 0u, k = 0u;
            localGrid->ijkFromCellIndex( localGridCellIndex, &i, &j, &k );
            auto subSegment = std::make_unique<RicMswSegment>( "Sub segment",
                                                               previousExitMD,
                                                               cellIntInfo.endMD,
                                                               previousExitTVD,
                                                               cellIntInfo.endTVD(),
                                                               segment->subIndex() );

            subSegment->setEquivalentDiameter( fishbonesSubs->equivalentDiameter( unitSystem ) );
            subSegment->setHoleDiameter( fishbonesSubs->holeDiameter( unitSystem ) );
            subSegment->setOpenHoleRoughnessFactor( fishbonesSubs->openHoleRoughnessFactor( unitSystem ) );
            subSegment->setSkinFactor( fishbonesSubs->skinFactor() );
            subSegment->setSourcePdmObject( fishbonesSubs );
            subSegment->setIntersectedGlobalCells( { cellIntInfo.globCellIndex } );

            auto intersection = std::make_shared<RicMswSegmentCellIntersection>( gridName,
                                                                                 cellIntInfo.globCellIndex,
                                                                                 caf::VecIjk0( i, j, k ),
                                                                                 cellIntInfo.intersectionLengthsInCellCS );
            subSegment->addIntersection( std::move( intersection ) );
            completion->addSegment( std::move( subSegment ) );

            previousExitMD  = cellIntInfo.endMD;
            previousExitTVD = cellIntInfo.endTVD();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswTableData::assignFractureCompletionsToCellSegment( const RimEclipseCase*                 eclipseCase,
                                                                            const RimWellPath*                    wellPath,
                                                                            const RimWellPathFracture*            fracture,
                                                                            const std::vector<RigCompletionData>& completionData,
                                                                            gsl::not_null<RicMswSegment*>         segment,
                                                                            bool* foundSubGridIntersections )
{
    CVF_ASSERT( foundSubGridIntersections != nullptr );

    double position = fracture->fractureMD();
    double width    = fracture->fractureTemplate()->computeFractureWidth( fracture );

    auto fractureCompletion = std::make_unique<RicMswFracture>( fracture->name(), wellPath, position, position + width );

    if ( fracture->fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH )
    {
        double perforationLength = fracture->fractureTemplate()->perforationLength();
        position -= 0.5 * perforationLength;
        width = perforationLength;
    }

    auto subSegment = std::make_unique<RicMswSegment>( "Fracture segment", position, position + width, 0.0, 0.0 );
    for ( const RigCompletionData& compIntersection : completionData )
    {
        const RigCompletionDataGridCell& cell = compIntersection.completionDataGridCell();

        if ( !cell.isMainGridCell() )
        {
            *foundSubGridIntersections = true;
        }

        caf::VecIjk0 localIJK( cell.localCellIndexI(), cell.localCellIndexJ(), cell.localCellIndexK() );

        auto intersection =
            std::make_shared<RicMswSegmentCellIntersection>( cell.lgrName(), cell.globalCellIndex(), localIJK, cvf::Vec3d::ZERO );
        subSegment->addIntersection( intersection );
    }
    fractureCompletion->addSegment( std::move( subSegment ) );
    segment->addCompletion( std::move( fractureCompletion ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicWellPathExportMswTableData::generatePerforationIntersections( gsl::not_null<const RimWellPath*>            wellPath,
                                                                     gsl::not_null<const RimPerforationInterval*> perforationInterval,
                                                                     int                                          timeStep,
                                                                     gsl::not_null<const RimEclipseCase*>         eclipseCase )
{
    std::vector<RigCompletionData> completionData;
    const RigActiveCellInfo* activeCellInfo = eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    auto wellPathGeometry = wellPath->wellPathGeometry();
    CVF_ASSERT( wellPathGeometry );
    bool hasDate  = (size_t)timeStep < eclipseCase->timeStepDates().size();
    bool isActive = !hasDate || perforationInterval->isActiveOnDate( eclipseCase->timeStepDates()[timeStep] );

    if ( wellPath->perforationIntervalCollection()->isChecked() && perforationInterval->isChecked() && isActive )
    {
        std::pair<std::vector<cvf::Vec3d>, std::vector<double>> perforationPointsAndMD =
            wellPathGeometry->clippedPointSubset( perforationInterval->startMD(), perforationInterval->endMD() );

        std::vector<WellPathCellIntersectionInfo> intersectedCells =
            RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( eclipseCase->eclipseCaseData(),
                                                                              wellPath->name(),
                                                                              perforationPointsAndMD.first,
                                                                              perforationPointsAndMD.second );

        for ( auto& cell : intersectedCells )
        {
            bool cellIsActive = activeCellInfo->isActive( cell.globCellIndex );
            if ( !cellIsActive ) continue;

            RigCompletionData completion( wellPath->completionSettings()->wellNameForExport(),
                                          RigCompletionDataGridCell( cell.globCellIndex, eclipseCase->mainGrid() ),
                                          cell.startMD );

            completion.setSourcePdmObject( perforationInterval );
            completionData.push_back( completion );
        }
    }

    return completionData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswTableData::assignPerforationIntersections( const std::vector<RigCompletionData>& completionData,
                                                                    gsl::not_null<RicMswCompletion*>      perforationCompletion,
                                                                    const WellPathCellIntersectionInfo&   cellIntInfo,
                                                                    double                                overlapStart,
                                                                    double                                overlapEnd,
                                                                    bool*                                 foundSubGridIntersections )
{
    size_t currCellId = cellIntInfo.globCellIndex;

    auto subSegment =
        std::make_unique<RicMswSegment>( "Perforation segment", overlapStart, overlapEnd, cellIntInfo.startTVD(), cellIntInfo.endTVD() );
    for ( const RigCompletionData& compIntersection : completionData )
    {
        const RigCompletionDataGridCell& cell = compIntersection.completionDataGridCell();
        if ( !cell.isMainGridCell() )
        {
            *foundSubGridIntersections = true;
        }

        if ( cell.globalCellIndex() != currCellId ) continue;

        caf::VecIjk0 localIJK( cell.localCellIndexI(), cell.localCellIndexJ(), cell.localCellIndexK() );

        auto intersection = std::make_shared<RicMswSegmentCellIntersection>( cell.lgrName(),
                                                                             cell.globalCellIndex(),
                                                                             localIJK,
                                                                             cellIntInfo.intersectionLengthsInCellCS );
        subSegment->addIntersection( intersection );
    }
    perforationCompletion->addSegment( std::move( subSegment ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswTableData::assignBranchNumbersToPerforations( const RimEclipseCase*         eclipseCase,
                                                                       gsl::not_null<RicMswSegment*> segment,
                                                                       int                           branchNumber )
{
    for ( auto completion : segment->completions() )
    {
        if ( completion->completionType() == RigCompletionData::CompletionType::PERFORATION )
        {
            completion->setBranchNumber( branchNumber );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswTableData::assignBranchNumbersToOtherCompletions( const RimEclipseCase*         eclipseCase,
                                                                           gsl::not_null<RicMswSegment*> segment,
                                                                           gsl::not_null<int*>           branchNumber )
{
    for ( auto completion : segment->completions() )
    {
        if ( completion->completionType() != RigCompletionData::CompletionType::PERFORATION )
        {
            completion->setBranchNumber( ++( *branchNumber ) );

            for ( auto seg : completion->segments() )
            {
                assignBranchNumbersToOtherCompletions( eclipseCase, seg, branchNumber );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswTableData::assignBranchNumbersToBranch( const RimEclipseCase*        eclipseCase,
                                                                 RicMswExportInfo*            exportInfo,
                                                                 gsl::not_null<RicMswBranch*> branch,
                                                                 gsl::not_null<int*>          branchNumber )
{
    const auto currentBranchNumber = *branchNumber;
    branch->setBranchNumber( currentBranchNumber );

    for ( auto childBranch : branch->branches() )
    {
        ( *branchNumber )++;
        assignBranchNumbersToBranch( eclipseCase, exportInfo, childBranch, branchNumber );
    }

    // Assign perforations first to ensure the same branch number as the segment
    for ( auto segment : branch->segments() )
    {
        assignBranchNumbersToPerforations( eclipseCase, segment, currentBranchNumber );
    }

    // Assign other completions with an incremented branch number
    for ( auto segment : branch->segments() )
    {
        assignBranchNumbersToOtherCompletions( eclipseCase, segment, branchNumber );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RicMswBranch> RicWellPathExportMswTableData::createChildMswBranch( const RimWellPath* childWellPath )
{
    auto initialChildMD  = childWellPath->wellPathTieIn()->tieInMeasuredDepth();
    auto initialChildTVD = -childWellPath->wellPathGeometry()->interpolatedPointAlongWellPath( initialChildMD ).z();

    const RimWellPathValve* outletValve = childWellPath->wellPathTieIn()->outletValve();
    if ( outletValve )
    {
        auto branchStartingWithValve =
            RicMswValve::createTieInValve( QString( "%1 valve for %2" ).arg( outletValve->componentLabel() ).arg( childWellPath->name() ),
                                           childWellPath,
                                           initialChildMD,
                                           initialChildTVD,
                                           outletValve );
        if ( branchStartingWithValve )
        {
            const auto segmentEndMd = initialChildMD + internal::VALVE_SEGMENT_LENGTH;
            auto       dummySegment =
                std::make_unique<RicMswSegment>( QString( "%1 segment" ).arg( outletValve->componentLabel() ),
                                                 initialChildMD,
                                                 segmentEndMd,
                                                 initialChildTVD,
                                                 RicMswTableFormatterTools::tvdFromMeasuredDepth( childWellPath, segmentEndMd ) );
            branchStartingWithValve->addSegment( std::move( dummySegment ) );

            return branchStartingWithValve;
        }
    }

    auto childBranch = std::make_unique<RicMswBranch>( childWellPath->name(), childWellPath, initialChildMD, initialChildTVD );

    return childBranch;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicWellPathExportMswTableData::wellPathsWithTieIn( const RimWellPath* wellPath )
{
    std::vector<RimWellPath*> connectedWellPaths;
    {
        auto wellPaths = RimProject::current()->allWellPaths();
        for ( auto well : wellPaths )
        {
            if ( well && well->isEnabled() && well->wellPathTieIn() && well->wellPathTieIn()->parentWell() == wellPath )
            {
                connectedWellPaths.push_back( well );
            }
        }
    }

    return connectedWellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double>
    RicWellPathExportMswTableData::calculateOverlapWithActiveCells( double                                           startMD,
                                                                    double                                           endMD,
                                                                    const std::vector<WellPathCellIntersectionInfo>& wellPathIntersections,
                                                                    const RigActiveCellInfo*                         activeCellInfo )
{
    for ( const WellPathCellIntersectionInfo& intersection : wellPathIntersections )
    {
        if ( intersection.globCellIndex < activeCellInfo->reservoirCellCount() && activeCellInfo->isActive( intersection.globCellIndex ) )
        {
            double overlapStart = std::max( startMD, intersection.startMD );
            double overlapEnd   = std::min( endMD, intersection.endMD );
            if ( overlapEnd > overlapStart )
            {
                return std::make_pair( overlapStart, overlapEnd );
            }
        }
    }
    return std::make_pair( 0.0, 0.0 );
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RicWellPathExportMswCompletionsImpl.h"

#include "RiaLogging.h"
#include "RiaWeightedMeanCalculator.h"

#include "RicExportCompletionDataSettingsUi.h"
#include "RicExportFractureCompletionsImpl.h"
#include "RicMswExportInfo.h"
#include "RicMswTableFormatterTools.h"
#include "RicMswValveAccumulators.h"
#include "RicWellPathExportCompletionsFileTools.h"

#include "RifTextDataTableFormatter.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"
#include "RigWellLogExtractor.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"

#include "RimEclipseCase.h"
#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimFractureTemplate.h"
#include "RimModeledWellPath.h"
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

#include <QFile>

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::exportWellSegmentsForAllCompletions(
    const RicExportCompletionDataSettingsUi& exportSettings,
    const std::vector<RimWellPath*>&         wellPaths )
{
    std::shared_ptr<QFile> unifiedExportFile;
    if ( exportSettings.fileSplit() == RicExportCompletionDataSettingsUi::UNIFIED_FILE )
    {
        QString unifiedFileName =
            QString( "UnifiedCompletions_MSW_%1" ).arg( exportSettings.caseToApply->caseUserDescription() );
        unifiedExportFile =
            RicWellPathExportCompletionsFileTools::openFileForExport( exportSettings.folder, unifiedFileName );
    }

    for ( const auto& wellPath : wellPaths )
    {
        std::shared_ptr<QFile> unifiedWellPathFile;

        auto allCompletions = wellPath->allCompletionsRecursively();

        bool exportFractures = exportSettings.includeFractures() &&
                               std::any_of( allCompletions.begin(), allCompletions.end(), []( auto completion ) {
                                   return completion->isEnabled() &&
                                          completion->componentType() == RiaDefines::WellPathComponentType::FRACTURE;
                               } );

        bool exportFishbones = exportSettings.includeFishbones() &&
                               std::any_of( allCompletions.begin(), allCompletions.end(), []( auto completion ) {
                                   return completion->isEnabled() &&
                                          completion->componentType() == RiaDefines::WellPathComponentType::FISHBONES;
                               } );

        if ( exportSettings.fileSplit() == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL && !unifiedWellPathFile )
        {
            QString wellFileName = QString( "%1_UnifiedCompletions_MSW_%2" )
                                       .arg( wellPath->name(), exportSettings.caseToApply->caseUserDescription() );
            unifiedWellPathFile =
                RicWellPathExportCompletionsFileTools::openFileForExport( exportSettings.folder, wellFileName );
        }

        {
            // Always use perforation functions to export well segments along well path.
            // If no perforations are present, skip Perforation from file name

            std::shared_ptr<QFile> perforationsExportFile;
            if ( unifiedExportFile )
                perforationsExportFile = unifiedExportFile;
            else if ( unifiedWellPathFile )
                perforationsExportFile = unifiedWellPathFile;
            else
            {
                bool anyPerforationsPresent =
                    exportSettings.includeFractures() &&
                    std::any_of( allCompletions.begin(), allCompletions.end(), []( auto completion ) {
                        return completion->isEnabled() &&
                               completion->componentType() == RiaDefines::WellPathComponentType::PERFORATION_INTERVAL;
                    } );

                QString perforationText = anyPerforationsPresent ? "Perforation_" : "";
                QString fileName =
                    QString( "%1_%2MSW_%3" )
                        .arg( wellPath->name(), perforationText, exportSettings.caseToApply->caseUserDescription() );
                perforationsExportFile =
                    RicWellPathExportCompletionsFileTools::openFileForExport( exportSettings.folder, fileName );
            }
            exportWellSegmentsForPerforations( exportSettings.caseToApply,
                                               perforationsExportFile,
                                               wellPath,
                                               exportSettings.timeStep,
                                               exportSettings.exportDataSourceAsComment(),
                                               exportSettings.exportCompletionWelspecAfterMainBore() );
        }

        if ( exportFractures )
        {
            std::shared_ptr<QFile> fractureExportFile;
            if ( unifiedExportFile )
                fractureExportFile = unifiedExportFile;
            else if ( unifiedWellPathFile )
                fractureExportFile = unifiedWellPathFile;
            else
            {
                QString fileName =
                    QString( "%1_Fracture_MSW_%2" ).arg( wellPath->name(), exportSettings.caseToApply->caseUserDescription() );
                fractureExportFile =
                    RicWellPathExportCompletionsFileTools::openFileForExport( exportSettings.folder, fileName );
            }
            exportWellSegmentsForFractures( exportSettings.caseToApply,
                                            fractureExportFile,
                                            wellPath,
                                            exportSettings.exportDataSourceAsComment(),
                                            exportSettings.exportCompletionWelspecAfterMainBore() );
        }

        if ( exportFishbones )
        {
            std::shared_ptr<QFile> fishbonesExportFile;
            if ( unifiedExportFile )
                fishbonesExportFile = unifiedExportFile;
            else if ( unifiedWellPathFile )
                fishbonesExportFile = unifiedWellPathFile;
            else
            {
                QString fileName =
                    QString( "%1_Fishbones_MSW_%2" ).arg( wellPath->name(), exportSettings.caseToApply->caseUserDescription() );
                fishbonesExportFile =
                    RicWellPathExportCompletionsFileTools::openFileForExport( exportSettings.folder, fileName );
            }
            exportWellSegmentsForFishbones( exportSettings.caseToApply,
                                            fishbonesExportFile,
                                            wellPath,
                                            exportSettings.exportDataSourceAsComment(),
                                            exportSettings.exportCompletionWelspecAfterMainBore() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::exportWellSegmentsForPerforations( RimEclipseCase*        eclipseCase,
                                                                             std::shared_ptr<QFile> exportFile,
                                                                             const RimWellPath*     wellPath,
                                                                             int                    timeStep,
                                                                             bool exportDataSourceAsComment,
                                                                             bool completionSegmentsAfterMainBore )
{
    RiaDefines::EclipseUnitSystem unitSystem = eclipseCase->eclipseCaseData()->unitsType();

    auto mswParameters = wellPath->mswCompletionParameters();

    if ( !mswParameters ) return;

    double initialMD = 0.0; // Start measured depth location to export MSW data for. Either based on first intersection
                            // with active grid, or user defined value.

    auto cellIntersections = generateCellSegments( eclipseCase, wellPath, mswParameters, &initialMD );

    RicMswExportInfo exportInfo( wellPath,
                                 unitSystem,
                                 initialMD,
                                 mswParameters->lengthAndDepth().text(),
                                 mswParameters->pressureDrop().text() );

    if ( generatePerforationsMswExportInfo( eclipseCase,
                                            wellPath,
                                            timeStep,
                                            initialMD,
                                            cellIntersections,
                                            &exportInfo,
                                            exportInfo.mainBoreBranch() ) )
    {
        int branchNumber = 1;

        assignBranchNumbersToBranch( eclipseCase, &exportInfo, exportInfo.mainBoreBranch(), &branchNumber );

        QTextStream               stream( exportFile.get() );
        RifTextDataTableFormatter formatter( stream );

        double maxSegmentLength = mswParameters->maxSegmentLength();

        RicMswTableFormatterTools::generateWelsegsTable( formatter,
                                                         exportInfo,
                                                         maxSegmentLength,
                                                         completionSegmentsAfterMainBore );
        RicMswTableFormatterTools::generateCompsegTables( formatter, exportInfo );
        RicMswTableFormatterTools::generateWsegvalvTable( formatter, exportInfo );
        RicMswTableFormatterTools::generateWsegAicdTable( formatter, exportInfo );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::exportWellSegmentsForFractures( RimEclipseCase*        eclipseCase,
                                                                          std::shared_ptr<QFile> exportFile,
                                                                          const RimWellPath*     wellPath,
                                                                          bool exportDataSourceAsComment,
                                                                          bool completionSegmentsAfterMainBore )
{
    RiaDefines::EclipseUnitSystem unitSystem = eclipseCase->eclipseCaseData()->unitsType();

    if ( eclipseCase == nullptr )
    {
        RiaLogging::error(
            "Export Fracture Well Segments: Cannot export completions data without specified eclipse case" );
        return;
    }

    auto mswParameters = wellPath->mswCompletionParameters();

    if ( !mswParameters ) return;

    double initialMD = 0.0; // Start measured depth location to export MSW data for. Either based on first intersection
                            // with active grid, or user defined value.

    auto cellIntersections = generateCellSegments( eclipseCase, wellPath, mswParameters, &initialMD );

    RicMswExportInfo exportInfo( wellPath,
                                 unitSystem,
                                 initialMD,
                                 mswParameters->lengthAndDepth().text(),
                                 mswParameters->pressureDrop().text() );

    generateFracturesMswExportInfo( eclipseCase,
                                    wellPath,
                                    initialMD,
                                    cellIntersections,
                                    &exportInfo,
                                    exportInfo.mainBoreBranch() );

    int branchNumber = 1;
    assignBranchNumbersToBranch( eclipseCase, &exportInfo, exportInfo.mainBoreBranch(), &branchNumber );

    QTextStream               stream( exportFile.get() );
    RifTextDataTableFormatter formatter( stream );
    formatter.setOptionalComment( exportDataSourceAsComment );

    double maxSegmentLength = wellPath->mswCompletionParameters()->maxSegmentLength();

    RicMswTableFormatterTools::generateWelsegsTable( formatter, exportInfo, maxSegmentLength, completionSegmentsAfterMainBore );
    RicMswTableFormatterTools::generateCompsegTables( formatter, exportInfo );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::exportWellSegmentsForFishbones( RimEclipseCase*        eclipseCase,
                                                                          std::shared_ptr<QFile> exportFile,
                                                                          const RimWellPath*     wellPath,
                                                                          bool exportDataSourceAsComment,
                                                                          bool completionSegmentsAfterMainBore )
{
    auto fishbonesSubs = wellPath->fishbonesCollection()->activeFishbonesSubs();

    if ( eclipseCase == nullptr )
    {
        RiaLogging::error( "Export Well Segments: Cannot export completions data without specified eclipse case" );
        return;
    }

    double initialMD = 0.0; // Start measured depth location to export MSW data for. Either based on first intersection
                            // with active grid, or user defined value.

    auto mswParameters     = wellPath->mswCompletionParameters();
    auto cellIntersections = generateCellSegments( eclipseCase, wellPath, mswParameters, &initialMD );

    RiaDefines::EclipseUnitSystem unitSystem = eclipseCase->eclipseCaseData()->unitsType();

    RicMswExportInfo exportInfo( wellPath,
                                 unitSystem,
                                 initialMD,
                                 mswParameters->lengthAndDepth().text(),
                                 mswParameters->pressureDrop().text() );

    generateFishbonesMswExportInfo( eclipseCase,
                                    wellPath,
                                    initialMD,
                                    cellIntersections,
                                    fishbonesSubs,
                                    true,
                                    &exportInfo,
                                    exportInfo.mainBoreBranch() );

    int branchNumber = 1;

    assignBranchNumbersToBranch( eclipseCase, &exportInfo, exportInfo.mainBoreBranch(), &branchNumber );

    QTextStream               stream( exportFile.get() );
    RifTextDataTableFormatter formatter( stream );
    formatter.setOptionalComment( exportDataSourceAsComment );

    double maxSegmentLength = wellPath->mswCompletionParameters()->maxSegmentLength();

    RicMswTableFormatterTools::generateWelsegsTable( formatter, exportInfo, maxSegmentLength, completionSegmentsAfterMainBore );
    RicMswTableFormatterTools::generateCompsegTables( formatter, exportInfo );
    RicMswTableFormatterTools::generateWsegvalvTable( formatter, exportInfo );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::generateFishbonesMswExportInfo(
    const RimEclipseCase*                            eclipseCase,
    const RimWellPath*                               wellPath,
    double                                           initialMD,
    const std::vector<WellPathCellIntersectionInfo>& cellIntersections,
    bool                                             enableSegmentSplitting,
    gsl::not_null<RicMswExportInfo*>                 exportInfo,
    gsl::not_null<RicMswBranch*>                     branch )
{
    std::vector<RimFishbones*> fishbonesSubs = wellPath->fishbonesCollection()->activeFishbonesSubs();
    if ( fishbonesSubs.empty() ) return;

    generateFishbonesMswExportInfo( eclipseCase,
                                    wellPath,
                                    initialMD,
                                    cellIntersections,
                                    fishbonesSubs,
                                    enableSegmentSplitting,
                                    exportInfo,
                                    branch );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::generateFishbonesMswExportInfo(
    const RimEclipseCase*                            eclipseCase,
    const RimWellPath*                               wellPath,
    double                                           initialMD,
    const std::vector<WellPathCellIntersectionInfo>& cellIntersections,
    const std::vector<RimFishbones*>&                fishbonesSubs,
    bool                                             enableSegmentSplitting,
    gsl::not_null<RicMswExportInfo*>                 exportInfo,
    gsl::not_null<RicMswBranch*>                     branch )
{
    std::vector<WellPathCellIntersectionInfo> filteredIntersections =
        filterIntersections( cellIntersections, initialMD, wellPath->wellPathGeometry(), eclipseCase );

    auto mswParameters = wellPath->mswCompletionParameters();

    bool foundSubGridIntersections = false;

    // Create a dummy perforation interval
    RimPerforationInterval perfInterval;

    createWellPathSegments( branch, filteredIntersections, { &perfInterval }, wellPath, -1, eclipseCase, &foundSubGridIntersections );

    double maxSegmentLength = enableSegmentSplitting ? mswParameters->maxSegmentLength()
                                                     : std::numeric_limits<double>::infinity();

    double subStartMD  = wellPath->fishbonesCollection()->startMD();
    double subStartTVD = RicMswTableFormatterTools::tvdFromMeasuredDepth( branch->wellPath(), subStartMD );

    auto unitSystem = exportInfo->unitSystem();

    for ( RimFishbones* subs : fishbonesSubs )
    {
        std::map<size_t, std::vector<size_t>> subAndLateralIndices;
        for ( const auto& [subIndex, lateralIndex] : subs->installedLateralIndices() )
        {
            subAndLateralIndices[subIndex].push_back( lateralIndex );
        }

        // Find cell intersections closest to each sub location
        std::map<size_t, std::vector<size_t>> subAndCellIntersectionIndices;
        {
            auto fishboneSectionStart = subs->startMD();
            auto fishboneSectionEnd   = subs->endMD();

            for ( size_t intersectionIndex = 0; intersectionIndex < filteredIntersections.size(); intersectionIndex++ )
            {
                auto cellIntersection = filteredIntersections[intersectionIndex];
                if ( fishboneSectionStart <= cellIntersection.startMD && cellIntersection.startMD < fishboneSectionEnd )
                {
                    double intersectionMidpoint = 0.5 * ( cellIntersection.startMD + cellIntersection.endMD );
                    size_t closestSubIndex      = 0;
                    double closestDistance      = std::numeric_limits<double>::infinity();
                    for ( const auto& sub : subAndLateralIndices )
                    {
                        double subMD = subs->measuredDepth( sub.first );

                        auto distanceCandicate = std::abs( subMD - intersectionMidpoint );
                        if ( distanceCandicate < closestDistance )
                        {
                            closestDistance = distanceCandicate;
                            closestSubIndex = sub.first;
                        }
                    }

                    subAndCellIntersectionIndices[closestSubIndex].push_back( intersectionIndex );
                }
            }
        }

        for ( const auto& sub : subAndLateralIndices )
        {
            double subEndMD  = subs->measuredDepth( sub.first );
            double subEndTVD = RicMswTableFormatterTools::tvdFromMeasuredDepth( branch->wellPath(), subEndMD );

            {
                // Add completion for ICD
                auto icdSegment =
                    std::make_unique<RicMswSegment>( "ICD segment", subEndMD, subEndMD + 0.1, subEndTVD, subEndTVD, sub.first );

                for ( auto lateralIndex : sub.second )
                {
                    QString label = QString( "Lateral %1" ).arg( lateralIndex + 1 );
                    icdSegment->addCompletion(
                        std::make_unique<RicMswFishbones>( label, wellPath, subEndMD, subEndTVD, lateralIndex ) );
                }

                assignFishbonesLateralIntersections( eclipseCase,
                                                     branch->wellPath(),
                                                     subs,
                                                     icdSegment.get(),
                                                     &foundSubGridIntersections,
                                                     maxSegmentLength,
                                                     unitSystem );

                auto icdCompletion =
                    std::make_unique<RicMswFishbonesICD>( QString( "ICD" ), wellPath, subEndMD, subEndTVD, nullptr );
                icdCompletion->setFlowCoefficient( subs->icdFlowCoefficient() );
                double icdOrificeRadius = subs->icdOrificeDiameter( unitSystem ) / 2;
                icdCompletion->setArea( icdOrificeRadius * icdOrificeRadius * cvf::PI_D * subs->icdCount() );

                // assign open hole segments to sub
                {
                    const RigMainGrid* mainGrid = eclipseCase->mainGrid();

                    for ( auto intersectionIndex : subAndCellIntersectionIndices[sub.first] )
                    {
                        auto intersection = filteredIntersections[intersectionIndex];
                        if ( intersection.globCellIndex >= mainGrid->globalCellArray().size() ) continue;

                        size_t             localGridCellIndex = 0u;
                        const RigGridBase* localGrid =
                            mainGrid->gridAndGridLocalIdxFromGlobalCellIdx( intersection.globCellIndex,
                                                                            &localGridCellIndex );
                        QString gridName;
                        if ( localGrid != mainGrid )
                        {
                            gridName                  = QString::fromStdString( localGrid->gridName() );
                            foundSubGridIntersections = true;
                        }

                        size_t i, j, k;
                        localGrid->ijkFromCellIndex( localGridCellIndex, &i, &j, &k );
                        cvf::Vec3st localIJK( i, j, k );

                        auto mswIntersect =
                            std::make_shared<RicMswSegmentCellIntersection>( gridName,
                                                                             intersection.globCellIndex,
                                                                             localIJK,
                                                                             intersection.intersectionLengthsInCellCS );
                        icdSegment->addIntersection( mswIntersect );
                    }
                }

                icdCompletion->addSegment( std::move( icdSegment ) );

                RicMswSegment* segmentOnParentBranch = branch->findClosestSegmentWithLowerMD( subEndMD );
                if ( segmentOnParentBranch )
                {
                    segmentOnParentBranch->addCompletion( std::move( icdCompletion ) );
                }
            }

            subStartMD  = subEndMD;
            subStartTVD = subEndTVD;
        }
    }
    exportInfo->setHasSubGridIntersections( exportInfo->hasSubGridIntersections() || foundSubGridIntersections );
    // branch->sortSegments();

    std::vector<RimModeledWellPath*> connectedWellPaths = wellPathsWithTieIn( wellPath );
    for ( auto childWellPath : connectedWellPaths )
    {
        auto childMswBranch = createChildMswBranch( childWellPath );
        auto mswParameters  = childWellPath->mswCompletionParameters();

        double startOfChildMD       = 0.0; // this is currently not used, as the tie-in MD is used
        auto childCellIntersections = generateCellSegments( eclipseCase, childWellPath, mswParameters, &startOfChildMD );
        auto initialChildMD         = childWellPath->wellPathTieIn()->tieInMeasuredDepth();

        generateFishbonesMswExportInfo( eclipseCase,
                                        childWellPath,
                                        initialChildMD,
                                        childCellIntersections,
                                        enableSegmentSplitting,
                                        exportInfo,
                                        childMswBranch.get() );

        branch->addChildBranch( std::move( childMswBranch ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::generateFishbonesMswExportInfoForWell( const RimEclipseCase* eclipseCase,
                                                                                 const RimWellPath*    wellPath,
                                                                                 gsl::not_null<RicMswExportInfo*> exportInfo,
                                                                                 gsl::not_null<RicMswBranch*> branch )
{
    double initialMD = 0.0; // Start measured depth location to export MSW data for. Either based on first intersection
                            // with active grid, or user defined value.

    auto mswParameters     = wellPath->mswCompletionParameters();
    auto cellIntersections = generateCellSegments( eclipseCase, wellPath, mswParameters, &initialMD );

    RiaDefines::EclipseUnitSystem unitSystem = eclipseCase->eclipseCaseData()->unitsType();

    bool enableSegmentSplitting = false;
    generateFishbonesMswExportInfo( eclipseCase, wellPath, initialMD, cellIntersections, enableSegmentSplitting, exportInfo, branch );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellPathExportMswCompletionsImpl::generateFracturesMswExportInfo(
    RimEclipseCase*                                  eclipseCase,
    const RimWellPath*                               wellPath,
    double                                           initialMD,
    const std::vector<WellPathCellIntersectionInfo>& cellIntersections,
    gsl::not_null<RicMswExportInfo*>                 exportInfo,
    gsl::not_null<RicMswBranch*>                     branch )
{
    auto mswParameters = wellPath->mswCompletionParameters();
    auto fractures     = wellPath->fractureCollection()->activeFractures();

    std::vector<WellPathCellIntersectionInfo> filteredIntersections =
        filterIntersections( cellIntersections, initialMD, wellPath->wellPathGeometry(), eclipseCase );

    // Create a dummy perforation interval
    RimPerforationInterval perfInterval;

    bool foundSubGridIntersections = false;
    createWellPathSegments( branch, filteredIntersections, { &perfInterval }, wellPath, -1, eclipseCase, &foundSubGridIntersections );

    // Check if fractures are to be assigned to current main bore segment
    for ( RimWellPathFracture* fracture : fractures )
    {
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

            assignFractureCompletionsToCellSegment( eclipseCase,
                                                    wellPath,
                                                    fracture,
                                                    completionData,
                                                    segment,
                                                    &foundSubGridIntersections );
        }
    }

    exportInfo->setHasSubGridIntersections( exportInfo->hasSubGridIntersections() || foundSubGridIntersections );
    branch->sortSegments();

    std::vector<RimModeledWellPath*> connectedWellPaths = wellPathsWithTieIn( wellPath );
    for ( auto childWellPath : connectedWellPaths )
    {
        auto childMswBranch = createChildMswBranch( childWellPath );
        auto mswParameters  = childWellPath->mswCompletionParameters();

        double startOfChildMD       = 0.0; // this is currently not used, as the tie-in MD is used
        auto childCellIntersections = generateCellSegments( eclipseCase, childWellPath, mswParameters, &startOfChildMD );
        auto initialChildMD         = childWellPath->wellPathTieIn()->tieInMeasuredDepth();

        if ( generateFracturesMswExportInfo( eclipseCase,
                                             childWellPath,
                                             initialChildMD,
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
bool RicWellPathExportMswCompletionsImpl::generatePerforationsMswExportInfo(
    RimEclipseCase*                                  eclipseCase,
    const RimWellPath*                               wellPath,
    int                                              timeStep,
    double                                           initialMD,
    const std::vector<WellPathCellIntersectionInfo>& cellIntersections,
    gsl::not_null<RicMswExportInfo*>                 exportInfo,
    gsl::not_null<RicMswBranch*>                     branch )
{
    auto perforationIntervals = wellPath->perforationIntervalCollection()->activePerforations();

    // Check if there exist overlap between valves in a perforation interval
    for ( const auto& perfInterval : perforationIntervals )
    {
        for ( const auto& valve : perfInterval->valves() )
        {
            for ( const auto& otherValve : perfInterval->valves() )
            {
                if ( otherValve != valve )
                {
                    bool hasIntersection =
                        !( ( valve->endMD() < otherValve->startMD() ) || ( otherValve->endMD() < valve->startMD() ) );

                    if ( hasIntersection )
                    {
                        RiaLogging::error(
                            QString( "Valve overlap detected for perforation interval : %1" ).arg( perfInterval->name() ) );

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

    createWellPathSegments( branch,
                            filteredIntersections,
                            perforationIntervals,
                            wellPath,
                            timeStep,
                            eclipseCase,
                            &foundSubGridIntersections );

    createValveCompletions( branch, perforationIntervals, wellPath, exportInfo->unitSystem() );

    const RigActiveCellInfo* activeCellInfo =
        eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    assignValveContributionsToSuperICDsOrAICDs( branch,
                                                perforationIntervals,
                                                filteredIntersections,
                                                activeCellInfo,
                                                exportInfo->unitSystem() );
    moveIntersectionsToICVs( branch, perforationIntervals, exportInfo->unitSystem() );
    moveIntersectionsToSuperICDsOrAICDs( branch );

    exportInfo->setHasSubGridIntersections( exportInfo->hasSubGridIntersections() || foundSubGridIntersections );
    branch->sortSegments();

    std::vector<RimModeledWellPath*> connectedWellPaths = wellPathsWithTieIn( wellPath );

    for ( auto childWellPath : connectedWellPaths )
    {
        auto childMswBranch = createChildMswBranch( childWellPath );
        auto mswParameters  = childWellPath->mswCompletionParameters();

        double startOfChildMD       = 0.0; // this is currently not used, as the tie-in MD is used
        auto childCellIntersections = generateCellSegments( eclipseCase, childWellPath, mswParameters, &startOfChildMD );
        auto initialChildMD         = childWellPath->wellPathTieIn()->tieInMeasuredDepth();

        if ( generatePerforationsMswExportInfo( eclipseCase,
                                                childWellPath,
                                                timeStep,
                                                initialChildMD,
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
std::vector<WellPathCellIntersectionInfo>
    RicWellPathExportMswCompletionsImpl::generateCellSegments( const RimEclipseCase*             eclipseCase,
                                                               const RimWellPath*                wellPath,
                                                               const RimMswCompletionParameters* mswParameters,
                                                               gsl::not_null<double*>            initialMD )
{
    const RigActiveCellInfo* activeCellInfo =
        eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    auto wellPathGeometry = wellPath->wellPathGeometry();
    CVF_ASSERT( wellPathGeometry );

    const std::vector<cvf::Vec3d>& coords = wellPathGeometry->uniqueWellPathPoints();
    const std::vector<double>&     mds    = wellPathGeometry->uniqueMeasuredDepths();
    CVF_ASSERT( !coords.empty() && !mds.empty() );

    const RigMainGrid* mainGrid = eclipseCase->mainGrid();

    std::vector<WellPathCellIntersectionInfo> allIntersections =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( eclipseCase->eclipseCaseData(),
                                                                          wellPath->name(),
                                                                          coords,
                                                                          mds );
    if ( allIntersections.empty() ) return {};

    std::vector<WellPathCellIntersectionInfo> continuousIntersections =
        RigWellPathIntersectionTools::buildContinuousIntersections( allIntersections, mainGrid );

    if ( mswParameters->referenceMDType() == RimMswCompletionParameters::ReferenceMDType::MANUAL_REFERENCE_MD )
    {
        *initialMD = mswParameters->manualReferenceMD();
    }
    else
    {
        for ( const WellPathCellIntersectionInfo& intersection : continuousIntersections )
        {
            if ( activeCellInfo->isActive( intersection.globCellIndex ) )
            {
                *initialMD = intersection.startMD;
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
        *initialMD = std::min( *initialMD, startOfFirstCompletion );
    }

    return continuousIntersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo>
    RicWellPathExportMswCompletionsImpl::filterIntersections( const std::vector<WellPathCellIntersectionInfo>& intersections,
                                                              double                               initialMD,
                                                              gsl::not_null<const RigWellPath*>    wellPathGeometry,
                                                              gsl::not_null<const RimEclipseCase*> eclipseCase )
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
        extraIntersection.intersectedCellFaceOut =
            cvf::StructGridInterface::oppositeFace( firstIntersection.intersectedCellFaceIn );
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
                    RigWellPathIntersectionTools::calculateLengthInCell( grid,
                                                                         intersection.globCellIndex,
                                                                         intersectionPoint,
                                                                         intersection.endPoint );
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
void RicWellPathExportMswCompletionsImpl::createWellPathSegments(
    gsl::not_null<RicMswBranch*>                      branch,
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
                    double overlapStartTVD =
                        -wellPath->wellPathGeometry()->interpolatedPointAlongWellPath( overlapStart ).z();
                    auto intervalCompletion =
                        std::make_unique<RicMswPerforation>( interval->name(), wellPath, overlapStart, overlapStartTVD );
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
            QString text =
                QString( "Skipping segment , threshold = %1, length = %2" ).arg( segmentLengthThreshold ).arg( segmentLength );
            RiaLogging::info( text );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::createValveCompletions(
    gsl::not_null<RicMswBranch*>                      branch,
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

            std::vector<const RimWellPathValve*> perforationValves;
            interval->descendantsIncludingThisOfType( perforationValves );

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
                    double exportEndMD   = valveMD + 0.1;

                    double exportStartTVD = RicMswTableFormatterTools::tvdFromMeasuredDepth( wellPath, exportStartMD );
                    double exportEndTVD   = RicMswTableFormatterTools::tvdFromMeasuredDepth( wellPath, exportEndMD );

                    double overlapStartTVD = RicMswTableFormatterTools::tvdFromMeasuredDepth( wellPath, overlapStart );
                    double overlapEndTVD   = RicMswTableFormatterTools::tvdFromMeasuredDepth( wellPath, overlapEnd );

                    if ( segment->startMD() <= valveMD && valveMD < segment->endMD() )
                    {
                        if ( valve->componentType() == RiaDefines::WellPathComponentType::AICD )
                        {
                            QString valveLabel =
                                QString( "%1 #%2" ).arg( "Combined Valve for segment" ).arg( nMainSegment + 2 );
                            auto subSegment = std::make_unique<RicMswSegment>( "Valve segment",
                                                                               exportStartMD,
                                                                               exportEndMD,
                                                                               exportStartTVD,
                                                                               exportEndTVD );

                            superAICD = std::make_unique<RicMswPerforationAICD>( valveLabel,
                                                                                 wellPath,
                                                                                 exportStartMD,
                                                                                 exportStartTVD,
                                                                                 valve );
                            superAICD->addSegment( std::move( subSegment ) );
                        }
                        else if ( valve->componentType() == RiaDefines::WellPathComponentType::ICD )
                        {
                            QString valveLabel =
                                QString( "%1 #%2" ).arg( "Combined Valve for segment" ).arg( nMainSegment + 2 );
                            auto subSegment = std::make_unique<RicMswSegment>( "Valve segment",
                                                                               exportStartMD,
                                                                               exportEndMD,
                                                                               exportStartTVD,
                                                                               exportEndTVD );

                            superICD = std::make_unique<RicMswPerforationICD>( valveLabel,
                                                                               wellPath,
                                                                               exportStartMD,
                                                                               exportStartTVD,
                                                                               valve );
                            superICD->addSegment( std::move( subSegment ) );
                        }
                        else if ( valve->componentType() == RiaDefines::WellPathComponentType::ICV )
                        {
                            QString valveLabel =
                                QString( "ICV %1 at segment #%2" ).arg( valve->name() ).arg( nMainSegment + 2 );
                            auto subSegment = std::make_unique<RicMswSegment>( "Valve segment",
                                                                               exportStartMD,
                                                                               exportEndMD,
                                                                               exportStartTVD,
                                                                               exportEndTVD );

                            ICV = std::make_unique<RicMswPerforationICV>( valveLabel,
                                                                          wellPath,
                                                                          exportStartMD,
                                                                          exportStartTVD,
                                                                          valve );
                            ICV->addSegment( std::move( subSegment ) );
                        }
                    }
                    else if ( overlap > 0.0 &&
                              ( valve->componentType() == RiaDefines::WellPathComponentType::ICD && !superICD ) )
                    {
                        QString valveLabel =
                            QString( "%1 #%2" ).arg( "Combined Valve for segment" ).arg( nMainSegment + 2 );

                        auto subSegment = std::make_unique<RicMswSegment>( "Valve segment",
                                                                           exportStartMD,
                                                                           exportEndMD,
                                                                           exportStartTVD,
                                                                           exportEndTVD );
                        superICD =
                            std::make_unique<RicMswPerforationICD>( valveLabel, wellPath, exportStartMD, exportStartTVD, valve );
                        superICD->addSegment( std::move( subSegment ) );
                    }
                    else if ( overlap > 0.0 &&
                              ( valve->componentType() == RiaDefines::WellPathComponentType::AICD && !superAICD ) )
                    {
                        QString valveLabel =
                            QString( "%1 #%2" ).arg( "Combined Valve for segment" ).arg( nMainSegment + 2 );

                        auto subSegment = std::make_unique<RicMswSegment>( "Valve segment",
                                                                           exportStartMD,
                                                                           exportEndMD,
                                                                           exportStartTVD,
                                                                           exportEndTVD );
                        superAICD       = std::make_unique<RicMswPerforationAICD>( valveLabel,
                                                                             wellPath,
                                                                             exportStartMD,
                                                                             exportStartTVD,
                                                                             valve );
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
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::assignValveContributionsToSuperICDsOrAICDs(
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

        std::vector<const RimWellPathValve*> perforationValves;
        interval->descendantsIncludingThisOfType( perforationValves );

        double totalPerforationLength = 0.0;
        for ( const RimWellPathValve* valve : perforationValves )
        {
            if ( !valve->isChecked() ) continue;

            for ( auto segment : branch->segments() )
            {
                double intervalOverlapStart           = std::max( interval->startMD(), segment->startMD() );
                double intervalOverlapEnd             = std::min( interval->endMD(), segment->endMD() );
                auto   intervalOverlapWithActiveCells = calculateOverlapWithActiveCells( intervalOverlapStart,
                                                                                       intervalOverlapEnd,
                                                                                       wellPathIntersections,
                                                                                       activeCellInfo );

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

                auto intervalOverlapWithActiveCells = calculateOverlapWithActiveCells( intervalOverlapStart,
                                                                                       intervalOverlapEnd,
                                                                                       wellPathIntersections,
                                                                                       activeCellInfo );

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
void RicWellPathExportMswCompletionsImpl::moveIntersectionsToICVs(
    gsl::not_null<RicMswBranch*>                      branch,
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
            if ( completion->completionType() == RigCompletionData::PERFORATION )
            {
                perforations.push_back( completion );
            }
        }

        for ( const RimPerforationInterval* interval : perforationIntervals )
        {
            if ( !interval->isChecked() ) continue;

            std::vector<const RimWellPathValve*> perforationValves;
            interval->descendantsIncludingThisOfType( perforationValves );

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
void RicWellPathExportMswCompletionsImpl::moveIntersectionsToSuperICDsOrAICDs( gsl::not_null<RicMswBranch*> branch )
{
    for ( auto segment : branch->segments() )
    {
        RicMswCompletion*              superValve = nullptr;
        std::vector<RicMswCompletion*> perforations;
        for ( auto completion : segment->completions() )
        {
            if ( RigCompletionData::isPerforationValve( completion->completionType() ) )
            {
                superValve = completion;
            }
            else
            {
                CVF_ASSERT( completion->completionType() == RigCompletionData::PERFORATION );
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
                for ( auto intersectionPtr : subSegment->intersections() )
                {
                    completionPtr->segments()[0]->addIntersection( intersectionPtr );
                }
            }
        }
        // Remove all completions and re-add the super valve
        segment->completions().clear();
        segment->addCompletion( std::move( completionPtr ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::assignFishbonesLateralIntersections( const RimEclipseCase* eclipseCase,
                                                                               const RimWellPath*    wellPath,
                                                                               const RimFishbones*   fishbonesSubs,
                                                                               gsl::not_null<RicMswSegment*> segment,
                                                                               bool*  foundSubGridIntersections,
                                                                               double maxSegmentLength,
                                                                               RiaDefines::EclipseUnitSystem unitSystem )
{
    CVF_ASSERT( foundSubGridIntersections != nullptr );

    const RigMainGrid* grid = eclipseCase->eclipseCaseData()->mainGrid();

    for ( auto completion : segment->completions() )
    {
        if ( completion->completionType() != RigCompletionData::FISHBONES )
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

        RigWellPath pathGeometry( lateralCoords, lateralMDs );

        double previousExitMD  = lateralMDs.front();
        double previousExitTVD = -lateralCoords.front().z();

        for ( const auto& cellIntInfo : intersections )
        {
            size_t             localGridCellIndex = 0u;
            const RigGridBase* localGrid =
                grid->gridAndGridLocalIdxFromGlobalCellIdx( cellIntInfo.globCellIndex, &localGridCellIndex );
            QString gridName;
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

            auto intersection = std::make_shared<RicMswSegmentCellIntersection>( gridName,
                                                                                 cellIntInfo.globCellIndex,
                                                                                 cvf::Vec3st( i, j, k ),
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
void RicWellPathExportMswCompletionsImpl::assignFractureCompletionsToCellSegment( const RimEclipseCase* eclipseCase,
                                                                                  const RimWellPath*    wellPath,
                                                                                  const RimWellPathFracture* fracture,
                                                                                  const std::vector<RigCompletionData>& completionData,
                                                                                  gsl::not_null<RicMswSegment*> segment,
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

        cvf::Vec3st localIJK( cell.localCellIndexI(), cell.localCellIndexJ(), cell.localCellIndexK() );

        auto intersection = std::make_shared<RicMswSegmentCellIntersection>( cell.lgrName(),
                                                                             cell.globalCellIndex(),
                                                                             localIJK,
                                                                             cvf::Vec3d::ZERO );
        subSegment->addIntersection( intersection );
    }
    fractureCompletion->addSegment( std::move( subSegment ) );
    segment->addCompletion( std::move( fractureCompletion ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicWellPathExportMswCompletionsImpl::generatePerforationIntersections(
    gsl::not_null<const RimWellPath*>            wellPath,
    gsl::not_null<const RimPerforationInterval*> perforationInterval,
    int                                          timeStep,
    gsl::not_null<const RimEclipseCase*>         eclipseCase )
{
    std::vector<RigCompletionData> completionData;
    const RigActiveCellInfo*       activeCellInfo =
        eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

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
void RicWellPathExportMswCompletionsImpl::assignPerforationIntersections(
    const std::vector<RigCompletionData>& completionData,
    gsl::not_null<RicMswCompletion*>      perforationCompletion,
    const WellPathCellIntersectionInfo&   cellIntInfo,
    double                                overlapStart,
    double                                overlapEnd,
    bool*                                 foundSubGridIntersections )
{
    size_t currCellId = cellIntInfo.globCellIndex;

    auto subSegment = std::make_unique<RicMswSegment>( "Perforation segment",
                                                       overlapStart,
                                                       overlapEnd,
                                                       cellIntInfo.startTVD(),
                                                       cellIntInfo.endTVD() );
    for ( const RigCompletionData& compIntersection : completionData )
    {
        const RigCompletionDataGridCell& cell = compIntersection.completionDataGridCell();
        if ( !cell.isMainGridCell() )
        {
            *foundSubGridIntersections = true;
        }

        if ( cell.globalCellIndex() != currCellId ) continue;

        cvf::Vec3st localIJK( cell.localCellIndexI(), cell.localCellIndexJ(), cell.localCellIndexK() );

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
void RicWellPathExportMswCompletionsImpl::assignBranchNumbersToPerforations( const RimEclipseCase*         eclipseCase,
                                                                             gsl::not_null<RicMswSegment*> segment,
                                                                             gsl::not_null<int*> branchNumber )
{
    for ( auto completion : segment->completions() )
    {
        if ( completion->completionType() == RigCompletionData::PERFORATION )
        {
            completion->setBranchNumber( *branchNumber );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportMswCompletionsImpl::assignBranchNumbersToOtherCompletions( const RimEclipseCase* eclipseCase,
                                                                                 gsl::not_null<RicMswSegment*> segment,
                                                                                 gsl::not_null<int*> branchNumber )
{
    for ( auto completion : segment->completions() )
    {
        if ( completion->completionType() != RigCompletionData::PERFORATION )
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
void RicWellPathExportMswCompletionsImpl::assignBranchNumbersToBranch( const RimEclipseCase*        eclipseCase,
                                                                       RicMswExportInfo*            exportInfo,
                                                                       gsl::not_null<RicMswBranch*> branch,
                                                                       gsl::not_null<int*>          branchNumber )
{
    branch->setBranchNumber( *branchNumber );

    // Assign perforations first to ensure the same branch number as the segment
    for ( auto segment : branch->segments() )
    {
        assignBranchNumbersToPerforations( eclipseCase, segment, branchNumber );
    }

    // Assign other completions with an incremented branch number
    for ( auto segment : branch->segments() )
    {
        assignBranchNumbersToOtherCompletions( eclipseCase, segment, branchNumber );
    }

    ( *branchNumber )++;

    for ( auto childBranch : branch->branches() )
    {
        assignBranchNumbersToBranch( eclipseCase, exportInfo, childBranch, branchNumber );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RicMswBranch>
    RicWellPathExportMswCompletionsImpl::createChildMswBranch( const RimModeledWellPath* childWellPath )
{
    auto initialChildMD  = childWellPath->wellPathTieIn()->tieInMeasuredDepth();
    auto initialChildTVD = -childWellPath->wellPathGeometry()->interpolatedPointAlongWellPath( initialChildMD ).z();

    const RimWellPathValve* outletValve = childWellPath->wellPathTieIn()->outletValve();
    if ( outletValve )
    {
        auto branchStartingWithValve = RicMswValve::createTieInValve( QString( "%1 valve for %2" )
                                                                          .arg( outletValve->componentLabel() )
                                                                          .arg( childWellPath->name() ),
                                                                      childWellPath,
                                                                      initialChildMD,
                                                                      initialChildTVD,
                                                                      outletValve );
        if ( branchStartingWithValve )
        {
            auto dummySegment =
                std::make_unique<RicMswSegment>( QString( "%1 segment" ).arg( outletValve->componentLabel() ),
                                                 initialChildMD,
                                                 initialChildMD + 0.1,
                                                 initialChildTVD,
                                                 RicMswTableFormatterTools::tvdFromMeasuredDepth( childWellPath,
                                                                                                  initialChildMD + 0.1 ) );
            branchStartingWithValve->addSegment( std::move( dummySegment ) );

            return branchStartingWithValve;
        }
    }

    auto childBranch =
        std::make_unique<RicMswBranch>( childWellPath->name(), childWellPath, initialChildMD, initialChildTVD );

    return childBranch;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimModeledWellPath*> RicWellPathExportMswCompletionsImpl::wellPathsWithTieIn( const RimWellPath* wellPath )
{
    std::vector<RimModeledWellPath*> connectedWellPaths;
    {
        auto wellPaths = RimProject::current()->allWellPaths();
        for ( auto w : wellPaths )
        {
            auto modelWellPath = dynamic_cast<RimModeledWellPath*>( w );
            if ( modelWellPath && modelWellPath->wellPathTieIn() && modelWellPath->wellPathTieIn()->parentWell() == wellPath )
            {
                connectedWellPaths.push_back( modelWellPath );
            }
        }
    }

    return connectedWellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RicWellPathExportMswCompletionsImpl::calculateOverlapWithActiveCells(
    double                                           startMD,
    double                                           endMD,
    const std::vector<WellPathCellIntersectionInfo>& wellPathIntersections,
    const RigActiveCellInfo*                         activeCellInfo )
{
    for ( const WellPathCellIntersectionInfo& intersection : wellPathIntersections )
    {
        if ( intersection.globCellIndex < activeCellInfo->reservoirCellCount() &&
             activeCellInfo->isActive( intersection.globCellIndex ) )
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

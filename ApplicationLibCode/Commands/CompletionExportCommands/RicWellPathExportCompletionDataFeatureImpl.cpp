/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicWellPathExportCompletionDataFeatureImpl.h"

#include "RiaLogging.h"
#include "RiaPreferencesSystem.h"
#include "RiaQStringFormatter.h"
#include "RiaWeightedMeanCalculator.h"
#include "RigStatisticsTools.h"
#include "RiuMessageDialog.h"

#include "ExportCommands/RicExportLgrFeature.h"
#include "RicCompletionDataExportTools.h"
#include "RicExportCompletionDataSettingsUi.h"
#include "RicExportFractureCompletionsImpl.h"
#include "RicFishbonesTransmissibilityCalculationFeatureImp.h"
#include "RicScheduleDataGenerator.h"
#include "RicTransmissibilityCalculator.h"
#include "RicWellPathExportCompletionsFileTools.h"
#include "RicWellPathExportMswCompletionsImpl.h"
#include "RicWellPathFractureReportItem.h"
#include "RicWellPathFractureTextReportFeatureImpl.h"

#include "RifTextDataTableFormatter.h"
#include "RifThermalToStimPlanFractureXmlOutput.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "Well/RigWellLogExtractor.h"
#include "Well/RigWellPath.h"
#include "Well/RigWellPathIntersectionTools.h"

#include "RicPerforationCellFilterEvaluator.h"

#include "RimCellFilter.h"
#include "RimEclipseCase.h"
#include "RimFishbonesCollection.h"
#include "RimNonDarcyPerforationParameters.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimThermalFractureTemplate.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafProgressInfo.h"

#include <QDir>

#include <algorithm>
#include <limits>
#include <map>
#include <optional>
#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportCompletions( const std::vector<RimWellPath*>&         allWellPaths,
                                                                    const RicExportCompletionDataSettingsUi& exportSettings )
{
    // Laterals are exported together with their top level well path
    const auto wellPaths = RicWellPathExportCompletionsFileTools::uniqueTopLevelWellPaths( allWellPaths );

    if ( exportSettings.caseToApply() == nullptr )
    {
        RiaLogging::error( "Export Completions Data: Cannot export completions data a valid Eclipse case" );
        return;
    }

    // Ensure that the case is open. This will enable export without any open views.
    // https://github.com/OPM/ResInsight/issues/11134
    exportSettings.caseToApply()->ensureReservoirCaseIsOpen();

    if ( exportSettings.caseToApply()->eclipseCaseData() == nullptr )
    {
        RiaLogging::error( "Export Completions Data: No data available for Eclipse Case" );
        return;
    }

    if ( exportSettings.customFileName().isEmpty() )
    {
        QDir folder( exportSettings.folder );
        if ( !folder.exists() )
        {
            QString txt = QString( "The path '%1' does not exist. Aborting export." ).arg( exportSettings.folder );
            RiuMessageDialog::showError( Riu3DMainWindowTools::mainWindowWidget(), "Export", txt );

            return;
        }
    }

    exportCarfinForTemporaryLgrs( exportSettings.caseToApply(), exportSettings.folder );

    {
        std::vector<RicWellPathFractureReportItem> fractureDataReportItems;
        std::unique_ptr<QTextStream>               fractureTransmissibilityExportInformationStream = nullptr;
        QFile                                      fractureTransmissibilityExportInformationFile;

        RiaPreferencesSystem* prefs = RiaPreferencesSystem::current();
        if ( prefs->includeFractureDebugInfoFile() )
        {
            QDir outputDir = QDir( exportSettings.folder );
            if ( !outputDir.mkpath( "." ) )
            {
                QString errMsg = QString( "Could not create export folder: %1" ).arg( exportSettings.folder );
                RiaLogging::error( errMsg.toStdString() );
                return;
            }

            QString fractureTransmisibillityExportInformationPath =
                QDir( exportSettings.folder ).absoluteFilePath( "FractureTransmissibilityExportInformation" );

            fractureTransmissibilityExportInformationFile.setFileName( fractureTransmisibillityExportInformationPath );
            if ( !fractureTransmissibilityExportInformationFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
            {
                RiaLogging::error(
                    std::format( "Export Completions Data: Could not open the file: {}", fractureTransmisibillityExportInformationPath ) );
            }
            else
            {
                fractureTransmissibilityExportInformationStream =
                    std::make_unique<QTextStream>( &fractureTransmissibilityExportInformationFile );
            }

            // Extra debugging for thermal fractures
            if ( exportSettings.includeFractures() )
            {
                for ( RimWellPath* wellPath : wellPaths )
                {
                    std::vector<RimWellPath*> allWellPathLaterals = wellPath->allWellPathLaterals();
                    for ( auto wellPathLateral : allWellPathLaterals )
                    {
                        for ( auto& frac : wellPathLateral->fractureCollection()->activeFractures() )
                        {
                            if ( RimThermalFractureTemplate* fractureTemplate =
                                     dynamic_cast<RimThermalFractureTemplate*>( frac->fractureTemplate() ) )
                            {
                                QString name            = frac->name();
                                QString fractureXmlPath = QDir( exportSettings.folder ).absoluteFilePath( name + ".xml" );

                                RifThermalToStimPlanFractureXmlOutput::writeToFile( fractureTemplate, fractureXmlPath );
                            }
                        }
                    }
                }
            }
        }

        std::vector<RigCompletionData> completions;

        {
            caf::ProgressInfo progress( wellPaths.size(), "Extracting Completion Data For Well Paths" );

            for ( RimWellPath* wellPath : wellPaths )
            {
                std::vector<RimWellPath*> allWellPathLaterals;
                if ( wellPath->unitSystem() == exportSettings.caseToApply->eclipseCaseData()->unitsType() )
                {
                    allWellPathLaterals = wellPath->allWellPathLaterals();
                }
                else
                {
                    int     caseId = exportSettings.caseToApply->caseId();
                    QString format = QString( "Unit systems for well path \"%1\" must match unit system of chosen eclipse case \"%2\"" );
                    QString errMsg = format.arg( wellPath->name() ).arg( caseId );
                    RiaLogging::error( errMsg.toStdString() );
                }

                std::map<size_t, std::vector<RigCompletionData>> completionsPerEclipseCellAllCompletionTypes;

                for ( auto wellPathLateral : allWellPathLaterals )
                {
                    // Skip well paths that are not enabled, no export of WELSEGS or COMPDAT
                    // https://github.com/OPM/ResInsight/issues/10754
                    //
                    if ( !wellPathLateral->isEnabled() ) continue;

                    // Generate completion data

                    if ( exportSettings.includePerforations )
                    {
                        auto exportDate = exportDateForTimeStep( exportSettings.caseToApply, exportSettings.timeStep );
                        std::vector<RigCompletionData> perforationCompletionData =
                            RicCompletionDataExportTools::generatePerforationsCompdatValues( wellPathLateral,
                                                                                             wellPathLateral
                                                                                                 ->perforationIntervalCollection()
                                                                                                 ->perforations(),
                                                                                             exportSettings,
                                                                                             exportDate );

                        RicCompletionDataExportTools::appendCompletionData( &completionsPerEclipseCellAllCompletionTypes,
                                                                            perforationCompletionData );
                    }

                    if ( exportSettings.includeFishbones )
                    {
                        // Make sure the start and end location is computed if needed
                        wellPathLateral->fishbonesCollection()->computeStartAndEndLocation();

                        std::vector<RigCompletionData> fishbonesCompletionData =
                            RicFishbonesTransmissibilityCalculationFeatureImp::generateFishboneCompdatValuesUsingAdjustedCellVolume( wellPathLateral,
                                                                                                                                     exportSettings );

                        RicCompletionDataExportTools::appendCompletionData( &completionsPerEclipseCellAllCompletionTypes,
                                                                            fishbonesCompletionData );
                    }

                    if ( exportSettings.includeFractures() )
                    {
                        // If no report is wanted, set reportItems = nullptr
                        std::vector<RicWellPathFractureReportItem>* reportItems = &fractureDataReportItems;

                        std::vector<RigCompletionData> fractureCompletionData = RicExportFractureCompletionsImpl::
                            generateCompdatValuesForWellPath( wellPathLateral,
                                                              exportSettings.caseToApply(),
                                                              reportItems,
                                                              fractureTransmissibilityExportInformationStream.get(),
                                                              RicExportFractureCompletionsImpl::
                                                                  PressureDepletionParameters( exportSettings.performTransScaling(),
                                                                                               exportSettings.transScalingTimeStep(),
                                                                                               exportSettings.transScalingWBHPSource(),
                                                                                               exportSettings.transScalingWBHP() ) );

                        RicCompletionDataExportTools::appendCompletionData( &completionsPerEclipseCellAllCompletionTypes,
                                                                            fractureCompletionData );
                    }
                }

                for ( auto& data : completionsPerEclipseCellAllCompletionTypes )
                {
                    completions.push_back( RicCompletionDataExportTools::combineEclipseCellCompletions( data.second, exportSettings ) );
                }

                progress.incrementProgress();
            }
        }

        // make sure we use per-connection D-factors, must be done after combining completions
        for ( auto& completion : completions )
        {
            completion.setPerConnectionDfactor();
        }

        const QString eclipseCaseName = exportSettings.caseToApply->caseUserDescription();

        if ( exportSettings.fileSplit == RicExportCompletionDataSettingsUi::ExportSplit::UNIFIED_FILE )
        {
            QString fileName = exportSettings.customFileName();
            if ( fileName.isEmpty() )
            {
                fileName = QString( "UnifiedCompletions_%1" ).arg( eclipseCaseName );
            }

            sortAndExportCompletionsToFile( exportSettings.caseToApply,
                                            exportSettings.folder,
                                            fileName,
                                            completions,
                                            fractureDataReportItems,
                                            exportSettings.compdatExport,
                                            exportSettings.exportDataSourceAsComment(),
                                            exportSettings.exportWelspec() );
        }
        else if ( exportSettings.fileSplit == RicExportCompletionDataSettingsUi::ExportSplit::SPLIT_ON_WELL )
        {
            for ( auto wellPath : wellPaths )
            {
                std::vector<RigCompletionData> completionsForWell;
                for ( const auto& completion : completions )
                {
                    if ( wellPath == RicCompletionDataExportTools::topLevelWellPath( completion ) )
                    {
                        completionsForWell.push_back( completion );
                    }
                }

                if ( completionsForWell.empty() ) continue;

                std::vector<RicWellPathFractureReportItem> reportItemsForWell;
                for ( const auto& fracItem : fractureDataReportItems )
                {
                    if ( fracItem.wellPathNameForExport() == wellPath->completionSettings()->wellNameForExport() )
                    {
                        reportItemsForWell.push_back( fracItem );
                    }
                }

                QString fileName = QString( "%1_Completions_%2" ).arg( wellPath->name() ).arg( eclipseCaseName );
                sortAndExportCompletionsToFile( exportSettings.caseToApply,
                                                exportSettings.folder,
                                                fileName,
                                                completionsForWell,
                                                reportItemsForWell,
                                                exportSettings.compdatExport,
                                                exportSettings.exportDataSourceAsComment(),
                                                exportSettings.exportWelspec() );
            }
        }
    }

    if ( exportSettings.includeMsw )
    {
        RicWellPathExportMswCompletionsImpl::exportWellSegmentsForAllCompletions( exportSettings, wellPaths );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicWellPathExportCompletionDataFeatureImpl::computeStaticCompletionsForWellPath( RimWellPath* wellPath,
                                                                                                                RimEclipseCase* eclipseCase )
{
    std::vector<RigCompletionData> completionsPerEclipseCell;

    if ( eclipseCase && eclipseCase->eclipseCaseData() )
    {
        RicExportCompletionDataSettingsUi exportSettings;
        exportSettings.caseToApply         = eclipseCase;
        exportSettings.timeStep            = 0;
        exportSettings.includeFishbones    = true;
        exportSettings.includePerforations = true;
        exportSettings.includeFractures    = true;

        {
            std::vector<RigCompletionData> completionData =
                RicFishbonesTransmissibilityCalculationFeatureImp::generateFishboneCompdatValuesUsingAdjustedCellVolume( wellPath,
                                                                                                                         exportSettings );

            std::copy( completionData.begin(), completionData.end(), std::back_inserter( completionsPerEclipseCell ) );
        }

        {
            std::vector<RigCompletionData> completionData =
                RicExportFractureCompletionsImpl::generateCompdatValuesForWellPath( wellPath, eclipseCase, nullptr, nullptr );

            std::copy( completionData.begin(), completionData.end(), std::back_inserter( completionsPerEclipseCell ) );
        }
    }

    return completionsPerEclipseCell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicWellPathExportCompletionDataFeatureImpl::computeDynamicCompletionsForWellPath( RimWellPath* wellPath,
                                                                                                                 RimEclipseCase* eclipseCase,
                                                                                                                 size_t timeStepIndex )
{
    CAF_ASSERT( eclipseCase );
    std::vector<RigCompletionData> completionsPerEclipseCell;

    if ( eclipseCase && eclipseCase->eclipseCaseData() )
    {
        RicExportCompletionDataSettingsUi exportSettings;
        exportSettings.caseToApply         = eclipseCase;
        exportSettings.timeStep            = static_cast<int>( timeStepIndex );
        exportSettings.includeFishbones    = true;
        exportSettings.includePerforations = true;
        exportSettings.includeFractures    = true;

        auto exportDate = exportDateForTimeStep( eclipseCase, timeStepIndex );

        completionsPerEclipseCell =
            RicCompletionDataExportTools::generatePerforationsCompdatValues( wellPath,
                                                                             wellPath->perforationIntervalCollection()->perforations(),
                                                                             exportSettings,
                                                                             exportDate );
    }

    return completionsPerEclipseCell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<int> RicWellPathExportCompletionDataFeatureImpl::completionNumberForCell( const std::vector<RigCompletionData>& completions )
{
    // One perforation interval can cross the same cell more than once, so lengths are summed per number
    std::map<int, double> overlapPerNumber;
    for ( const auto& completion : completions )
    {
        const auto number = completion.completionNumber();
        if ( !number.has_value() ) continue;

        double length = 0.0;
        if ( completion.startMD().has_value() && completion.endMD().has_value() )
        {
            length = std::max( 0.0, completion.endMD().value() - completion.startMD().value() );
        }
        overlapPerNumber[number.value()] += length;
    }

    if ( overlapPerNumber.empty() ) return std::nullopt;

    // Iterating in ascending number order and requiring a strictly larger length makes ties go to the lowest number
    const double tolerance = 1.0e-6;

    auto best = overlapPerNumber.begin();
    for ( auto it = std::next( best ); it != overlapPerNumber.end(); ++it )
    {
        if ( it->second > best->second + tolerance ) best = it;
    }
    return best->first;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportWellPathFractureReport(
    RimEclipseCase*                                   sourceCase,
    QFilePtr                                          exportFile,
    const std::vector<RicWellPathFractureReportItem>& wellPathFractureReportItems )
{
    QTextStream stream( exportFile.get() );

    if ( !wellPathFractureReportItems.empty() )
    {
        std::vector<RicWellPathFractureReportItem> sortedReportItems;
        {
            std::set<RicWellPathFractureReportItem> fractureReportItemsSet;

            for ( const auto& reportItem : wellPathFractureReportItems )
            {
                fractureReportItemsSet.insert( reportItem );
            }

            for ( const auto& reportItem : fractureReportItemsSet )
            {
                sortedReportItems.emplace_back( reportItem );
            }
        }

        std::vector<RimWellPath*> wellPathsToReport;
        {
            std::set<RimWellPath*> wellPathsSet;

            auto allWellPaths = RicWellPathFractureTextReportFeatureImpl::wellPathsWithActiveFractures();
            for ( const auto& wellPath : allWellPaths )
            {
                for ( const auto& reportItem : sortedReportItems )
                {
                    if ( reportItem.wellPathNameForExport() == wellPath->completionSettings()->wellNameForExport() )
                    {
                        wellPathsSet.insert( wellPath );
                    }
                }
            }

            std::copy( wellPathsSet.begin(), wellPathsSet.end(), std::back_inserter( wellPathsToReport ) );
        }

        RicWellPathFractureTextReportFeatureImpl reportGenerator;
        QString summaryText = reportGenerator.wellPathFractureReport( sourceCase, wellPathsToReport, sortedReportItems );

        stream << summaryText;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportWelspecsToFile( RimEclipseCase*                       gridCase,
                                                                       QFilePtr                              exportFile,
                                                                       const std::vector<RigCompletionData>& completions,
                                                                       bool                                  exportDataSourceAsComment )
{
    QTextStream stream( exportFile.get() );

    RifTextDataTableFormatter formatter( stream );
    formatter.setColumnSpacing( 2 );
    formatter.setOptionalComment( exportDataSourceAsComment );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "WELL", "NAME" ), // well
        RifTextDataTableColumn( "GROUP", "NAME" ), // group
        RifTextDataTableColumn( "", "I" ), // I
        RifTextDataTableColumn( "", "J" ), // J
        RifTextDataTableColumn( "BHP", "DEPTH" ), // RefDepth
        RifTextDataTableColumn( "PHASE", "FLUID" ), // Type
        RifTextDataTableColumn( "DRAIN", "AREA" ), // DrainRad
        RifTextDataTableColumn( "INFLOW", "EQUANS" ), // GasInEq
        RifTextDataTableColumn( "OPEN", "SHUT" ), // AutoShut
        RifTextDataTableColumn( "CROSS", "FLOW" ), // XFlow
        RifTextDataTableColumn( "PVT", "TABLE" ), // FluidPVT
        RifTextDataTableColumn( "HYDS", "DENS" ), // HydrDens
        RifTextDataTableColumn( "FIP", "REGN" ) // FluidInPla) };
    };

    formatter.header( header );
    formatter.keyword( "WELSPECS" );

    std::set<const RimWellPath*> wellPathSet;

    // Build list of unique RimWellPath
    for ( const auto& completion : completions )
    {
        const auto wellPath = RicWellPathExportCompletionsFileTools::findWellPathFromExportName( completion.wellName() );
        if ( wellPath )
        {
            wellPathSet.insert( wellPath );
        }
    }

    // Export
    for ( const auto wellPath : wellPathSet )
    {
        auto completionSettings = wellPath->completionSettings();
        auto ijIntersection     = wellPathUpperGridIntersectionIJ( gridCase, wellPath );

        formatter.add( completionSettings->wellNameForExport() )
            .add( completionSettings->groupNameForExport() )
            .add( ijIntersection.second.x() + 1 )
            .add( ijIntersection.second.y() + 1 )
            .add( completionSettings->referenceDepthForExport() )
            .add( completionSettings->wellTypeNameForExport() )
            .add( completionSettings->drainageRadiusForExport() )
            .add( completionSettings->gasInflowEquationForExport() )
            .add( completionSettings->automaticWellShutInForExport() )
            .add( completionSettings->allowWellCrossFlowForExport() )
            .add( completionSettings->wellBoreFluidPVTForExport() )
            .add( completionSettings->hydrostaticDensityForExport() )
            .add( completionSettings->fluidInPlaceRegionForExport() )
            .rowCompleted();
    }

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportWelspeclToFile( RimEclipseCase* gridCase,
                                                                       QFilePtr        exportFile,
                                                                       const std::map<QString, std::vector<RigCompletionData>>& completions )
{
    QTextStream stream( exportFile.get() );

    RifTextDataTableFormatter formatter( stream );
    formatter.setColumnSpacing( 2 );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "WELL", "NAME" ), // well
        RifTextDataTableColumn( "GROUP", "NAME" ), // group
        RifTextDataTableColumn( "", "LGR" ),
        RifTextDataTableColumn( "", "I" ), // I
        RifTextDataTableColumn( "", "J" ), // J
        RifTextDataTableColumn( "BHP", "DEPTH" ), // RefDepth
        RifTextDataTableColumn( "PHASE", "FLUID" ), // Type
        RifTextDataTableColumn( "DRAIN", "AREA" ), // DrainRad
        RifTextDataTableColumn( "INFLOW", "EQUANS" ), // GasInEq
        RifTextDataTableColumn( "OPEN", "SHUT" ), // AutoShut
        RifTextDataTableColumn( "CROSS", "FLOW" ), // XFlow
        RifTextDataTableColumn( "PVT", "TABLE" ), // FluidPVT
        RifTextDataTableColumn( "HYDS", "DENS" ), // HydrDens
        RifTextDataTableColumn( "FIP", "REGN" ) // FluidInPla) };
    };

    formatter.header( header );
    formatter.keyword( "WELSPECL" );

    std::map<const RimWellPath*, std::set<QString>> wellPathToLgrNameMap;

    for ( const auto& completionsForLgr : completions )
    {
        for ( const auto& completion : completionsForLgr.second )
        {
            const auto wellPath = RicWellPathExportCompletionsFileTools::findWellPathFromExportName( completion.wellName() );
            if ( !wellPath ) continue;
            wellPathToLgrNameMap[wellPath].insert( completionsForLgr.first );
        }
    }

    // See similar logic in RiaGrpcWellPathService::GetCompletionData()

    // A well must be defined only once. Use the LGR the well path enters first.
    for ( const auto& [wellPath, gridNames] : wellPathToLgrNameMap )
    {
        QString    firstLgrName;
        double     firstMeasuredDepth = std::numeric_limits<double>::max();
        cvf::Vec2i firstIJ;
        bool       isFound = false;

        for ( const auto& lgrName : gridNames )
        {
            const auto& [measuredDepth, ijIntersection] = wellPathUpperGridIntersectionIJ( gridCase, wellPath, lgrName );

            const bool isValid = measuredDepth != cvf::UNDEFINED_DOUBLE;
            if ( !isFound || ( isValid && measuredDepth < firstMeasuredDepth ) )
            {
                firstLgrName       = lgrName;
                firstMeasuredDepth = isValid ? measuredDepth : std::numeric_limits<double>::max();
                firstIJ            = ijIntersection;
                isFound            = true;
            }
        }

        if ( !isFound ) continue;

        auto completionSettings = wellPath->completionSettings();

        formatter.add( completionSettings->wellNameForExport() )
            .add( completionSettings->groupNameForExport() )
            .add( firstLgrName )
            .add( firstIJ.x() + 1 )
            .add( firstIJ.y() + 1 )
            .add( completionSettings->referenceDepthForExport() )
            .add( completionSettings->wellTypeNameForExport() )
            .add( completionSettings->drainageRadiusForExport() )
            .add( completionSettings->gasInflowEquationForExport() )
            .add( completionSettings->automaticWellShutInForExport() )
            .add( completionSettings->allowWellCrossFlowForExport() )
            .add( completionSettings->wellBoreFluidPVTForExport() )
            .add( completionSettings->hydrostaticDensityForExport() )
            .add( completionSettings->fluidInPlaceRegionForExport() )
            .rowCompleted();
    }
    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::sortAndExportCompletionsToFile(
    RimEclipseCase*                                      eclipseCase,
    const QString&                                       folderName,
    const QString&                                       fileName,
    const std::vector<RigCompletionData>&                completions,
    const std::vector<RicWellPathFractureReportItem>&    wellPathFractureReportItems,
    RicExportCompletionDataSettingsUi::CompdatExportType exportType,
    bool                                                 exportDataSourceAsComment,
    bool                                                 exportWelspec )
{
    // Sort completions based on grid they belong to
    std::vector<RigCompletionData> completionsForMainGrid = RicCompletionDataExportTools::mainGridCompletions( completions );
    std::map<QString, std::vector<RigCompletionData>> completionsForSubGrids = RicCompletionDataExportTools::subGridsCompletions( completions );

    if ( !completionsForMainGrid.empty() )
    {
        QFileInfo              fi( fileName );
        std::shared_ptr<QFile> exportFile =
            RicWellPathExportCompletionsFileTools::openFileForExport( folderName, fi.baseName(), fi.suffix(), exportDataSourceAsComment );

        if ( exportFile )
        {
            std::map<QString, std::vector<RigCompletionData>> completionsForGrid;
            completionsForGrid.insert( std::pair<QString, std::vector<RigCompletionData>>( "", completionsForMainGrid ) );

            exportWellPathFractureReport( eclipseCase, exportFile, wellPathFractureReportItems );
            if ( exportWelspec )
            {
                exportWelspecsToFile( eclipseCase, exportFile, completionsForMainGrid, exportDataSourceAsComment );
            }
            exportCompdatAndWpimultTables( eclipseCase, exportFile, completionsForGrid, exportType, exportDataSourceAsComment );
        }
    }

    if ( !completionsForSubGrids.empty() )
    {
        QFileInfo fi( fileName );

        QString                lgrFileName = fi.baseName() + "_LGR";
        std::shared_ptr<QFile> exportFile =
            RicWellPathExportCompletionsFileTools::openFileForExport( folderName, lgrFileName, fi.suffix(), exportDataSourceAsComment );

        if ( exportFile )
        {
            exportWellPathFractureReport( eclipseCase, exportFile, wellPathFractureReportItems );
            if ( exportWelspec )
            {
                exportWelspeclToFile( eclipseCase, exportFile, completionsForSubGrids );
            }
            exportCompdatAndWpimultTables( eclipseCase, exportFile, completionsForSubGrids, exportType, exportDataSourceAsComment );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportCompdatAndWpimultTables(
    RimEclipseCase*                                          sourceCase,
    QFilePtr                                                 exportFile,
    const std::map<QString, std::vector<RigCompletionData>>& completionsPerGrid,
    RicExportCompletionDataSettingsUi::CompdatExportType     exportType,
    bool                                                     exportDataSourceAsComment )
{
    if ( completionsPerGrid.empty() ) return;

    QTextStream stream( exportFile.get() );

    RifTextDataTableFormatter formatter( stream );
    formatter.setColumnSpacing( 3 );
    formatter.setOptionalComment( exportDataSourceAsComment );

    // All LGRs share one set of tables (COMPDATL, WPIMULTL, COMPLMPL) where each row carries its own LGR name
    std::map<QString, std::vector<RigCompletionData>> completionsPerTable;
    for ( const auto& [gridName, gridCompletions] : completionsPerGrid )
    {
        auto& tableCompletions = completionsPerTable[gridName.isEmpty() ? QString() : QString( "LGR" )];
        tableCompletions.insert( tableCompletions.end(), gridCompletions.begin(), gridCompletions.end() );
    }

    for ( const auto& gridCompletions : completionsPerTable )
    {
        std::vector<RigCompletionData> completions = gridCompletions.second;

        // Sort by well name / cell index
        std::sort( completions.begin(), completions.end() );

        // Print completion data
        QString gridName = gridCompletions.first;
        RicCompletionDataExportTools::exportCompdatTableUsingFormatter( formatter, gridName, completions );

        if ( exportType == RicExportCompletionDataSettingsUi::CompdatExport::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS )
        {
            RicCompletionDataExportTools::exportWpimultTableUsingFormatter( formatter, gridName, completions );
        }

        for ( auto& completion : completions )
        {
            if ( completion.completionNumber().has_value() )
            {
                RicCompletionDataExportTools::exportComplumpTableUsingFormatter( formatter, gridName, completions );
                break;
            }
        }
    }

    RiaLogging::info( std::format( "Successfully exported completion data to {}", exportFile->fileName() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, cvf::Vec2i> RicWellPathExportCompletionDataFeatureImpl::wellPathUpperGridIntersectionIJ( const RimEclipseCase* gridCase,
                                                                                                           const RimWellPath*    wellPath,
                                                                                                           const QString&        gridName )
{
    const RigEclipseCaseData* caseData       = gridCase->eclipseCaseData();
    const RigMainGrid*        mainGrid       = caseData->mainGrid();
    const RigActiveCellInfo*  activeCellInfo = caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    auto wellPathGeometry = wellPath->wellPathGeometry();
    if ( !wellPathGeometry ) return std::make_pair( cvf::UNDEFINED_DOUBLE, cvf::Vec2i() );

    const std::vector<cvf::Vec3d>& coords = wellPathGeometry->wellPathPoints();
    const std::vector<double>&     mds    = wellPathGeometry->measuredDepths();
    if ( coords.empty() || mds.empty() ) return std::make_pair( cvf::UNDEFINED_DOUBLE, cvf::Vec2i() );

    std::vector<WellPathCellIntersectionInfo> intersections =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( caseData, wellPath->name(), coords, mds );

    int gridId = 0;

    if ( !gridName.isEmpty() )
    {
        const auto grid = caseData->grid( gridName );
        if ( grid ) gridId = grid->gridId();
    }

    for ( const WellPathCellIntersectionInfo& intersection : intersections )
    {
        size_t             gridLocalCellIndex = 0;
        const RigGridBase* grid = mainGrid->gridAndGridLocalIdxFromGlobalCellIdx( intersection.globCellIndex, &gridLocalCellIndex );

        if ( grid->gridId() == gridId && activeCellInfo->isActive( ReservoirCellIndex( intersection.globCellIndex ) ) )
        {
            size_t i, j, k;
            if ( grid->ijkFromCellIndex( gridLocalCellIndex, &i, &j, &k ) )
            {
                return std::make_pair( intersection.startMD, cvf::Vec2i( (int)i, (int)j ) );
            }
        }
    }
    return std::make_pair( cvf::UNDEFINED_DOUBLE, cvf::Vec2i() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportCarfinForTemporaryLgrs( const RimEclipseCase* sourceCase, const QString& folder )
{
    if ( !sourceCase || !sourceCase->mainGrid() ) return;

    const auto  mainGrid         = sourceCase->mainGrid();
    const auto& lgrInfosForWells = RicExportLgrFeature::createLgrInfoListForTemporaryLgrs( mainGrid );

    for ( const auto& lgrInfoForWell : lgrInfosForWells )
    {
        RicExportLgrFeature::exportLgrs( folder, lgrInfoForWell.first, lgrInfoForWell.second );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicWellPathExportCompletionDataFeatureImpl::completionDataForWellPath( RimWellPath*    wellPath,
                                                                                                      RimEclipseCase* eCase,
                                                                                                      const std::optional<QDateTime>& exportDate )
{
    if ( wellPath == nullptr ) return {};

    return completionDataForWellPaths( { wellPath }, eCase, exportDate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicWellPathExportCompletionDataFeatureImpl::completionDataForWellPaths( const std::vector<RimWellPath*>& wellPaths,
                                                                            RimEclipseCase*                  eCase,
                                                                            const std::optional<QDateTime>&  exportDate )
{
    if ( eCase == nullptr ) return {};

    // Ensure that the case is open. This will enable export without any open views.
    eCase->ensureReservoirCaseIsOpen();
    if ( eCase->eclipseCaseData() == nullptr )
    {
        RiaLogging::error( "Export Completions Data: No data available for Eclipse Case" );
        return {};
    }

    RicExportCompletionDataSettingsUi exportSettings;
    exportSettings.caseToApply = eCase;
    exportSettings.includeMsw  = false;
    exportSettings.setExportDataSourceAsComment( true );
    exportSettings.includePerforations = true;
    exportSettings.includeFishbones    = true;
    exportSettings.includeFractures    = true;

    std::vector<RigCompletionData> completions;

    // Completions from all given well paths (and their laterals) are combined per cell, so that cells shared
    // by several well paths are exported as a single connection.
    std::vector<RimWellPath*> allWellPathLaterals;
    for ( auto* wellPath : wellPaths )
    {
        if ( !wellPath || wellPath->unitSystem() != eCase->eclipseCaseData()->unitsType() ) continue;

        for ( auto* lateral : wellPath->allWellPathLaterals() )
        {
            if ( std::find( allWellPathLaterals.begin(), allWellPathLaterals.end(), lateral ) == allWellPathLaterals.end() )
            {
                allWellPathLaterals.push_back( lateral );
            }
        }
    }

    std::map<size_t, std::vector<RigCompletionData>> completionsPerEclipseCellAllCompletionTypes;

    for ( auto wellPathLateral : allWellPathLaterals )
    {
        // Skip well paths that are not enabled, no export of WELSEGS or COMPDAT
        // https://github.com/OPM/ResInsight/issues/10754
        //
        if ( !wellPathLateral->isEnabled() ) continue;

        // Generate completion data

        if ( exportSettings.includePerforations )
        {
            std::vector<RigCompletionData> perforationCompletionData =
                RicCompletionDataExportTools::generatePerforationsCompdatValues( wellPathLateral,
                                                                                 wellPathLateral->perforationIntervalCollection()->perforations(),
                                                                                 exportSettings,
                                                                                 exportDate );

            RicCompletionDataExportTools::appendCompletionData( &completionsPerEclipseCellAllCompletionTypes, perforationCompletionData );
        }

        if ( exportSettings.includeFishbones )
        {
            // Make sure the start and end location is computed if needed
            wellPathLateral->fishbonesCollection()->computeStartAndEndLocation();

            std::vector<RigCompletionData> fishbonesCompletionData =
                RicFishbonesTransmissibilityCalculationFeatureImp::generateFishboneCompdatValuesUsingAdjustedCellVolume( wellPathLateral,
                                                                                                                         exportSettings );

            RicCompletionDataExportTools::appendCompletionData( &completionsPerEclipseCellAllCompletionTypes, fishbonesCompletionData );
        }

        if ( exportSettings.includeFractures() )
        {
            // If no report is wanted, set reportItems = nullptr
            std::vector<RicWellPathFractureReportItem>* reportItems = nullptr;

            std::vector<RigCompletionData> fractureCompletionData = RicExportFractureCompletionsImpl::
                generateCompdatValuesForWellPath( wellPathLateral,
                                                  exportSettings.caseToApply(),
                                                  reportItems,
                                                  nullptr,
                                                  RicExportFractureCompletionsImpl::
                                                      PressureDepletionParameters( exportSettings.performTransScaling(),
                                                                                   exportSettings.transScalingTimeStep(),
                                                                                   exportSettings.transScalingWBHPSource(),
                                                                                   exportSettings.transScalingWBHP() ) );

            RicCompletionDataExportTools::appendCompletionData( &completionsPerEclipseCellAllCompletionTypes, fractureCompletionData );
        }
    }

    for ( auto& data : completionsPerEclipseCellAllCompletionTypes )
    {
        completions.push_back( RicCompletionDataExportTools::combineEclipseCellCompletions( data.second, exportSettings ) );
    }

    // Order completions with each well's mainbore connections first, followed by each lateral's
    // (see https://github.com/OPM/ResInsight/issues/14963), instead of the plain well
    // name/completion type/measured depth ordering from RigCompletionData::operator<(), since each
    // lateral has its own independent MD scale.
    RicScheduleDataGenerator::sortCompletionsMainBoreFirst( wellPaths, completions );

    // make sure we set per-connection D-factors, must be done after combining completions
    for ( auto& completion : completions )
    {
        completion.setPerConnectionDfactor(); // we calculate per connection D-factors, which are negative per definition (ref. OPM Flow
                                              // manual)
    }

    return completions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<QDateTime> RicWellPathExportCompletionDataFeatureImpl::exportDateForTimeStep( const RimEclipseCase* eclipseCase,
                                                                                            size_t                timeStepIndex )
{
    if ( !eclipseCase ) return std::nullopt;

    auto timeSteps = eclipseCase->timeStepDates();
    if ( timeStepIndex < timeSteps.size() ) return timeSteps[timeStepIndex];
    return std::nullopt;
}

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
#include "RiaWeightedMeanCalculator.h"
#include "RigStatisticsTools.h"

#include "ExportCommands/RicExportLgrFeature.h"
#include "RicExportCompletionDataSettingsUi.h"
#include "RicExportFractureCompletionsImpl.h"
#include "RicFishbonesTransmissibilityCalculationFeatureImp.h"
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

#include <map>
#include <optional>
#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportCompletions( const std::vector<RimWellPath*>&         wellPaths,
                                                                    const RicExportCompletionDataSettingsUi& exportSettings )
{
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
            RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(), "Export", txt );

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
                RiaLogging::error( errMsg );
                return;
            }

            QString fractureTransmisibillityExportInformationPath =
                QDir( exportSettings.folder ).absoluteFilePath( "FractureTransmissibilityExportInformation" );

            fractureTransmissibilityExportInformationFile.setFileName( fractureTransmisibillityExportInformationPath );
            if ( !fractureTransmissibilityExportInformationFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
            {
                RiaLogging::error(
                    QString( "Export Completions Data: Could not open the file: %1" ).arg( fractureTransmisibillityExportInformationPath ) );
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
                    RiaLogging::error( errMsg );
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
                            generatePerforationsCompdatValues( wellPathLateral,
                                                               wellPathLateral->perforationIntervalCollection()->perforations(),
                                                               exportSettings,
                                                               exportDate );

                        appendCompletionData( &completionsPerEclipseCellAllCompletionTypes, perforationCompletionData );
                    }

                    if ( exportSettings.includeFishbones )
                    {
                        // Make sure the start and end location is computed if needed
                        wellPathLateral->fishbonesCollection()->computeStartAndEndLocation();

                        std::vector<RigCompletionData> fishbonesCompletionData =
                            RicFishbonesTransmissibilityCalculationFeatureImp::generateFishboneCompdatValuesUsingAdjustedCellVolume( wellPathLateral,
                                                                                                                                     exportSettings );

                        appendCompletionData( &completionsPerEclipseCellAllCompletionTypes, fishbonesCompletionData );
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

                        appendCompletionData( &completionsPerEclipseCellAllCompletionTypes, fractureCompletionData );
                    }
                }

                for ( auto& data : completionsPerEclipseCellAllCompletionTypes )
                {
                    completions.push_back( combineEclipseCellCompletions( data.second, exportSettings ) );
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
                    if ( wellPath == topLevelWellPath( completion ) )
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
            generatePerforationsCompdatValues( wellPath, wellPath->perforationIntervalCollection()->perforations(), exportSettings, exportDate );
    }

    return completionsPerEclipseCell;
}

//==================================================================================================
///
//==================================================================================================
RigCompletionData RicWellPathExportCompletionDataFeatureImpl::combineEclipseCellCompletions( const std::vector<RigCompletionData>& completions,
                                                                                             const RicExportCompletionDataSettingsUi& settings )
{
    CVF_ASSERT( !completions.empty() );

    // For detailed description of how the combined completion data is computed, see
    // https://github.com/OPM/ResInsight/issues/7049

    const RigCompletionData& firstCompletion = completions[0];

    const QString&                    wellName       = firstCompletion.wellName();
    const RigCompletionDataGridCell&  cellIndexIJK   = firstCompletion.completionDataGridCell();
    RigCompletionData::CompletionType completionType = firstCompletion.completionType();

    RigCompletionData resultCompletion( wellName, cellIndexIJK, firstCompletion.firstOrderingValue() );
    resultCompletion.setSecondOrderingValue( firstCompletion.secondOrderingValue() );
    resultCompletion.setSourcePdmObject( firstCompletion.sourcePdmObject() );

    RigCompletionData::CellDirection cellDirection                = firstCompletion.direction();
    double                           largestTransmissibilityValue = firstCompletion.transmissibility();

    RiaWeightedMeanCalculator<double> diameterCalculator;
    RiaWeightedMeanCalculator<double> skinFactorCalculator;

    auto isValidTransmissibility = []( double transmissibility )
    { return RigStatisticsTools::isValidNumber<double>( transmissibility ) && transmissibility >= 0.0; };

    auto startMD          = completions[0].startMD();
    auto endMD            = completions[0].endMD();
    auto completionNumber = completions[0].completionNumber();

    for ( const RigCompletionData& completion : completions )
    {
        if ( !startMD.has_value() )
        {
            startMD = completion.startMD();
        }
        else
        {
            if ( completion.startMD().has_value() && ( completion.startMD().value() < startMD.value() ) )
            {
                startMD = completion.startMD();
            }
        }

        if ( !endMD.has_value() )
        {
            endMD = completion.endMD();
        }
        else
        {
            if ( completion.endMD().has_value() && ( completion.endMD().value() > endMD.value() ) )
            {
                endMD = completion.endMD();
            }
        }

        if ( !completionNumber.has_value() )
        {
            if ( completion.completionNumber().has_value() )
            {
                completionNumber = completion.completionNumber();
            }
        }

        double transmissibility = completion.transmissibility();

        if ( !isValidTransmissibility( transmissibility ) )
        {
            QString errorMessage = QString( "Invalid or negative transmissibility value (%1) in cell %3" )
                                       .arg( transmissibility )
                                       .arg( cellIndexIJK.oneBasedLocalCellIndexString() );
            RiaLogging::error( errorMessage );
            resultCompletion.addMetadata( "ERROR", errorMessage );
            continue;
        }

        diameterCalculator.addValueAndWeight( completion.diameter(), transmissibility );
        skinFactorCalculator.addValueAndWeight( completion.skinFactor(), transmissibility );

        if ( transmissibility > largestTransmissibilityValue )
        {
            largestTransmissibilityValue = transmissibility;
            cellDirection                = completion.direction();
        }
    }

    if ( startMD.has_value() && endMD.has_value() )
    {
        resultCompletion.setDepthRange( startMD.value(), endMD.value() );
    }
    if ( completionNumber.has_value() ) resultCompletion.setCompletionNumber( completionNumber.value() );

    double combinedDiameter   = diameterCalculator.weightedMean();
    double combinedSkinFactor = skinFactorCalculator.weightedMean();

    double combinedTrans   = 0.0;
    double combinedKh      = 0.0;
    double combinedDFactor = 0.0;

    {
        RiaWeightedMeanCalculator<double> dFactorCalculator;

        for ( const RigCompletionData& completion : completions )
        {
            // Error is reported in the loop above, so we can skip this completion
            if ( !isValidTransmissibility( completion.transmissibility() ) ) continue;

            resultCompletion.m_metadata.reserve( resultCompletion.m_metadata.size() + completion.m_metadata.size() );
            resultCompletion.m_metadata.insert( resultCompletion.m_metadata.end(), completion.m_metadata.begin(), completion.m_metadata.end() );

            if ( completion.wellName() != firstCompletion.wellName() )
            {
                QString errorMessage =
                    QString( "Cannot combine completions of different types in same cell %1" ).arg( cellIndexIJK.oneBasedLocalCellIndexString() );
                RiaLogging::error( errorMessage );
                resultCompletion.addMetadata( "ERROR", errorMessage );
                return resultCompletion; // Returning empty completion, should not be exported
            }

            if ( completion.transmissibility() == HUGE_VAL )
            {
                QString errorMessage =
                    QString( "Transmissibility calculation has failed for cell %1" ).arg( cellIndexIJK.oneBasedLocalCellIndexString() );
                RiaLogging::error( errorMessage );
                resultCompletion.addMetadata( "ERROR", errorMessage );
                return resultCompletion; // Returning empty completion, should not be exported
            }

            combinedTrans = combinedTrans + completion.transmissibility();
            combinedKh    = combinedKh + completion.kh();

            dFactorCalculator.addValueAndWeight( completion.dFactor(), completion.transmissibility() );
        }

        // Arithmetic MEAN dFactor weighted by Tj/SumTj from the completions
        // Note : Divide by n is intentional, based on input from @hhgs in mail dated 18.01.2020
        combinedDFactor = dFactorCalculator.weightedMean() / completions.size();
    }

    if ( settings.compdatExport == RicExportCompletionDataSettingsUi::CompdatExport::TRANSMISSIBILITIES )
    {
        resultCompletion.setCombinedValuesExplicitTrans( combinedTrans,
                                                         combinedKh,
                                                         combinedDFactor,
                                                         combinedSkinFactor,
                                                         combinedDiameter,
                                                         cellDirection,
                                                         completionType );
    }
    else if ( settings.compdatExport == RicExportCompletionDataSettingsUi::CompdatExport::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS )
    {
        // calculate trans for main bore - but as Eclipse will do it!
        double transmissibilityEclipseCalculation =
            RicTransmissibilityCalculator::calculateTransmissibilityAsEclipseDoes( settings.caseToApply(),
                                                                                   combinedSkinFactor,
                                                                                   combinedDiameter / 2,
                                                                                   cellIndexIJK.globalCellIndex(),
                                                                                   cellDirection );

        double defaultKh = RigCompletionData::defaultValue();
        double wpimult   = combinedTrans / transmissibilityEclipseCalculation;
        resultCompletion.setCombinedValuesImplicitTransWPImult( wpimult,
                                                                defaultKh,
                                                                combinedDFactor,
                                                                combinedSkinFactor,
                                                                combinedDiameter,
                                                                cellDirection,
                                                                completionType );
    }

    return resultCompletion;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicWellPathExportCompletionDataFeatureImpl::mainGridCompletions( const std::vector<RigCompletionData>& allCompletions )
{
    std::vector<RigCompletionData> completions;

    for ( const auto& completion : allCompletions )
    {
        QString gridName = completion.completionDataGridCell().lgrName();
        if ( gridName.isEmpty() )
        {
            completions.push_back( completion );
        }
    }
    return completions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, std::vector<RigCompletionData>>
    RicWellPathExportCompletionDataFeatureImpl::subGridsCompletions( const std::vector<RigCompletionData>& allCompletions )
{
    std::map<QString, std::vector<RigCompletionData>> completions;

    for ( const auto& completion : allCompletions )
    {
        QString gridName = completion.completionDataGridCell().lgrName();
        if ( !gridName.isEmpty() )
        {
            auto it = completions.find( gridName );
            if ( it == completions.end() )
            {
                completions.insert( std::pair<QString, std::vector<RigCompletionData>>( gridName, { completion } ) );
            }
            else
            {
                it->second.push_back( completion );
            }
        }
    }
    return completions;
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

    for ( const auto& [wellPath, gridNames] : wellPathToLgrNameMap )
    {
        for ( const auto& lgrName : gridNames )
        {
            const auto& [measuredDepth, ijIntersection] = wellPathUpperGridIntersectionIJ( gridCase, wellPath, lgrName );

            auto completionSettings = wellPath->completionSettings();

            formatter.add( completionSettings->wellNameForExport() )
                .add( completionSettings->groupNameForExport() )
                .add( lgrName )
                .add( ijIntersection.x() + 1 )
                .add( ijIntersection.y() + 1 )
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
    std::vector<RigCompletionData>                    completionsForMainGrid = mainGridCompletions( completions );
    std::map<QString, std::vector<RigCompletionData>> completionsForSubGrids = subGridsCompletions( completions );

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

    for ( const auto& gridCompletions : completionsPerGrid )
    {
        std::vector<RigCompletionData> completions = gridCompletions.second;

        // Sort by well name / cell index
        std::sort( completions.begin(), completions.end() );

        // Print completion data
        QString gridName = gridCompletions.first;
        exportCompdatTableUsingFormatter( formatter, gridName, completions );

        if ( exportType == RicExportCompletionDataSettingsUi::CompdatExport::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS )
        {
            exportWpimultTableUsingFormatter( formatter, gridName, completions );
        }

        for ( auto& completion : completions )
        {
            if ( completion.completionNumber().has_value() )
            {
                exportComplumpTableUsingFormatter( formatter, gridName, completions );
                break;
            }
        }
    }

    RiaLogging::info( QString( "Successfully exported completion data to %1" ).arg( exportFile->fileName() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportCompdatTableUsingFormatter( RifTextDataTableFormatter&            formatter,
                                                                                   const QString&                        gridName,
                                                                                   const std::vector<RigCompletionData>& completionData )
{
    if ( gridName.isEmpty() )
    {
        std::vector<RifTextDataTableColumn> header =
            { RifTextDataTableColumn( "WELL", "NAME" ),
              RifTextDataTableColumn( "", "I" ),
              RifTextDataTableColumn( "", "J" ),
              RifTextDataTableColumn( "", "K1" ),
              RifTextDataTableColumn( "", "K2" ),
              RifTextDataTableColumn( "OPEN", "SHUT" ),
              RifTextDataTableColumn( "SAT", "TAB" ),
              RifTextDataTableColumn( "CONN", "FACT", RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
              RifTextDataTableColumn( "WELL", "DIA" ),
              RifTextDataTableColumn( "KH", "FACT", RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
              RifTextDataTableColumn( "SKIN", "FACT" ),
              RifTextDataTableColumn( "D", "FACT", RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
              RifTextDataTableColumn( "DIR", "PEN" ) };

        formatter.header( header );
        formatter.keyword( "COMPDAT" );
    }
    else
    {
        std::vector<RifTextDataTableColumn> header =
            { RifTextDataTableColumn( "WELL", "NAME" ),
              RifTextDataTableColumn( "LGR", "NAME" ),
              RifTextDataTableColumn( "", "I" ),
              RifTextDataTableColumn( "", "J" ),
              RifTextDataTableColumn( "", "K1" ),
              RifTextDataTableColumn( "", "K2" ),
              RifTextDataTableColumn( "OPEN", "SHUT" ),
              RifTextDataTableColumn( "SAT", "TAB" ),
              RifTextDataTableColumn( "CONN", "FACT", RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
              RifTextDataTableColumn( "WELL", "DIA" ),
              RifTextDataTableColumn( "KH", "FACT", RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
              RifTextDataTableColumn( "SKIN", "FACT" ),
              RifTextDataTableColumn( "D", "FACT", RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
              RifTextDataTableColumn( "DIR", "PEN" ) };

        formatter.header( header );
        formatter.keyword( "COMPDATL" );
    }

    RigCompletionData::CompletionType currentCompletionType = RigCompletionData::CompletionType::CT_UNDEFINED;

    for ( const RigCompletionData& data : completionData )
    {
        if ( currentCompletionType != data.completionType() )
        {
            // The completions are sorted by completion type, write out a heading when completion type changes

            QString txt;
            if ( data.completionType() == RigCompletionData::CompletionType::FISHBONES ) txt = "Fishbones";
            if ( data.completionType() == RigCompletionData::CompletionType::FRACTURE ) txt = "Fracture";
            if ( data.completionType() == RigCompletionData::CompletionType::PERFORATION ) txt = "Perforation";

            formatter.addOptionalComment( "---- Completions for completion type " + txt + " ----" );

            currentCompletionType = data.completionType();
        }

        for ( const RigCompletionMetaData& metadata : data.metadata() )
        {
            formatter.addOptionalComment( QString( "%1 : %2" ).arg( metadata.name ).arg( metadata.comment ) );
        }

        if ( data.transmissibility() == 0.0 || data.wpimult() == 0.0 )
        {
            // Don't export completions without transmissibility
            continue;
        }

        formatter.add( data.wellName() );

        if ( !gridName.isEmpty() )
        {
            formatter.add( gridName );
        }

        formatter.add( data.completionDataGridCell().localCellIndexI() + 1 )
            .add( data.completionDataGridCell().localCellIndexJ() + 1 )
            .add( data.completionDataGridCell().localCellIndexK() + 1 )
            .add( data.completionDataGridCell().localCellIndexK() + 1 );

        formatter.add( "OPEN" );

        formatter.addValueOrDefaultMarker( data.saturation(), RigCompletionData::defaultValue() );
        formatter.addValueOrDefaultMarker( data.transmissibility(), RigCompletionData::defaultValue() );
        formatter.addValueOrDefaultMarker( data.diameter(), RigCompletionData::defaultValue() );
        formatter.addValueOrDefaultMarker( data.kh(), RigCompletionData::defaultValue() );
        formatter.addValueOrDefaultMarker( data.skinFactor(), RigCompletionData::defaultValue() );
        formatter.addValueOrDefaultMarker( data.dFactor(), RigCompletionData::defaultValue() );

        switch ( data.direction() )
        {
            case RigCompletionData::CellDirection::DIR_I:
                formatter.add( "'X'" );
                break;
            case RigCompletionData::CellDirection::DIR_J:
                formatter.add( "'Y'" );
                break;
            case RigCompletionData::CellDirection::DIR_K:
            default:
                formatter.add( "'Z'" );
                break;
        }

        formatter.rowCompleted();
    }
    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportComplumpTableUsingFormatter( RifTextDataTableFormatter&            formatter,
                                                                                    const QString&                        gridName,
                                                                                    const std::vector<RigCompletionData>& completionData )
{
    if ( gridName.isEmpty() )
    {
        std::vector<RifTextDataTableColumn> header = { RifTextDataTableColumn( "WELL", "NAME" ),
                                                       RifTextDataTableColumn( "", "I" ),
                                                       RifTextDataTableColumn( "", "J" ),
                                                       RifTextDataTableColumn( "", "K1" ),
                                                       RifTextDataTableColumn( "", "K2" ),
                                                       RifTextDataTableColumn( "COMP", "NUM" ) };

        formatter.header( header );
        formatter.keyword( "COMPLUMP" );
    }
    else
    {
        std::vector<RifTextDataTableColumn> header = { RifTextDataTableColumn( "WELL", "NAME" ),
                                                       RifTextDataTableColumn( "LGR", "NAME" ),
                                                       RifTextDataTableColumn( "", "I" ),
                                                       RifTextDataTableColumn( "", "J" ),
                                                       RifTextDataTableColumn( "", "K1" ),
                                                       RifTextDataTableColumn( "", "K2" ),
                                                       RifTextDataTableColumn( "COMP", "NUM" ) };

        formatter.header( header );
        formatter.keyword( "COMPLMPL" );
    }

    for ( const RigCompletionData& data : completionData )
    {
        if ( !data.completionNumber().has_value() || data.transmissibility() == 0.0 || data.wpimult() == 0.0 )
        {
            // Don't export completions without transmissibility or completion number set
            continue;
        }

        formatter.add( data.wellName() );

        if ( !gridName.isEmpty() )
        {
            formatter.add( gridName );
        }

        formatter.add( data.completionDataGridCell().localCellIndexI() + 1 )
            .add( data.completionDataGridCell().localCellIndexJ() + 1 )
            .add( data.completionDataGridCell().localCellIndexK() + 1 )
            .add( data.completionDataGridCell().localCellIndexK() + 1 )
            .add( data.completionNumber().value() );

        formatter.rowCompleted();
    }
    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportWpimultTableUsingFormatter( RifTextDataTableFormatter&            formatter,
                                                                                   const QString&                        gridName,
                                                                                   const std::vector<RigCompletionData>& completionData )
{
    std::vector<RifTextDataTableColumn> header;

    if ( gridName.isEmpty() )
    {
        header = {
            RifTextDataTableColumn( "Well" ),
            RifTextDataTableColumn( "Mult" ),
            RifTextDataTableColumn( "I" ),
            RifTextDataTableColumn( "J" ),
            RifTextDataTableColumn( "K" ),
        };
        formatter.keyword( "WPIMULT" );
    }
    else
    {
        header = {
            RifTextDataTableColumn( "Well" ),
            RifTextDataTableColumn( "LgrName" ),
            RifTextDataTableColumn( "Mult" ),
            RifTextDataTableColumn( "I" ),
            RifTextDataTableColumn( "J" ),
            RifTextDataTableColumn( "K" ),
        };
        formatter.keyword( "WPIMULTL" );
    }
    formatter.header( header );

    for ( auto& completion : completionData )
    {
        if ( completion.wpimult() == 0.0 || RigCompletionData::isDefaultValue( completion.wpimult() ) )
        {
            continue;
        }

        formatter.add( completion.wellName() );

        if ( !gridName.isEmpty() )
        {
            formatter.add( gridName );
        }

        formatter.add( completion.wpimult() );

        formatter.add( completion.completionDataGridCell().localCellIndexI() + 1 )
            .add( completion.completionDataGridCell().localCellIndexJ() + 1 )
            .add( completion.completionDataGridCell().localCellIndexK() + 1 );
        formatter.rowCompleted();
    }

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicWellPathExportCompletionDataFeatureImpl::generatePerforationsCompdatValues( gsl::not_null<const RimWellPath*> wellPath,
                                                                                   const std::vector<const RimPerforationInterval*>& intervals,
                                                                                   const RicExportCompletionDataSettingsUi& settings,
                                                                                   const std::optional<QDateTime>&          exportDate )
{
    RiaDefines::EclipseUnitSystem unitSystem = settings.caseToApply->eclipseCaseData()->unitsType();

    std::vector<RigCompletionData> completionData;

    const RigActiveCellInfo* activeCellInfo =
        settings.caseToApply->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    auto wellPathGeometry = wellPath->wellPathGeometry();
    if ( !wellPathGeometry ) return completionData;

    const RimNonDarcyPerforationParameters* nonDarcyParameters = wellPath->perforationIntervalCollection()->nonDarcyParameters();

    if ( wellPath->perforationIntervalCollection()->isChecked() )
    {
        for ( const RimPerforationInterval* interval : intervals )
        {
            if ( !interval->isChecked() ) continue;
            if ( exportDate.has_value() && !interval->isActiveOnDate( *exportDate ) ) continue;

            std::pair<std::vector<cvf::Vec3d>, std::vector<double>> perforationPointsAndMD =
                wellPathGeometry->clippedPointSubset( interval->startMD(), interval->endMD() );

            std::vector<WellPathCellIntersectionInfo> intersectedCells =
                RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( settings.caseToApply->eclipseCaseData(),
                                                                                  wellPath->name(),
                                                                                  perforationPointsAndMD.first,
                                                                                  perforationPointsAndMD.second );

            for ( auto& cell : intersectedCells )
            {
                bool cellIsActive = activeCellInfo->isActive( ReservoirCellIndex( cell.globCellIndex ) );
                if ( !cellIsActive ) continue;

                RigCompletionData completion( wellPath->completionSettings()->wellNameForExport(),
                                              RigCompletionDataGridCell( cell.globCellIndex, settings.caseToApply->mainGrid() ),
                                              cell.startMD );

                RigCompletionData::CellDirection direction =
                    RicTransmissibilityCalculator::calculateCellMainDirection( settings.caseToApply,
                                                                               cell.globCellIndex,
                                                                               cell.intersectionLengthsInCellCS );

                double transmissibility = 0.0;
                double kh               = RigCompletionData::defaultValue();
                double dFactor          = RigCompletionData::defaultValue();

                {
                    auto transmissibilityData =
                        RicTransmissibilityCalculator::calculateTransmissibilityData( settings.caseToApply,
                                                                                      wellPath,
                                                                                      cell.intersectionLengthsInCellCS,
                                                                                      interval->skinFactor(),
                                                                                      interval->diameter( unitSystem ) / 2,
                                                                                      cell.globCellIndex,
                                                                                      settings.useLateralNTG );

                    transmissibility = transmissibilityData.connectionFactor();
                    kh               = transmissibilityData.kh();

                    if ( nonDarcyParameters->nonDarcyFlowType() == RimNonDarcyPerforationParameters::NON_DARCY_COMPUTED )
                    {
                        const double effectiveH = transmissibilityData.effectiveH();

                        const double effectivePermeability =
                            nonDarcyParameters->gridPermeabilityScalingFactor() * transmissibilityData.effectiveK();

                        dFactor =
                            RicTransmissibilityCalculator::calculateDFactor( settings.caseToApply,
                                                                             effectiveH,
                                                                             cell.globCellIndex,
                                                                             wellPath->perforationIntervalCollection()->nonDarcyParameters(),
                                                                             effectivePermeability );
                    }
                }

                completion.setTransAndWPImultBackgroundDataFromPerforation( transmissibility,
                                                                            interval->skinFactor(),
                                                                            interval->diameter( unitSystem ),
                                                                            dFactor,
                                                                            kh,
                                                                            direction );
                completion.setDepthRange( cell.startMD, cell.endMD );
                if ( interval->completionNumber() > 0 ) completion.setCompletionNumber( interval->completionNumber() );

                completion.addMetadata( "Perforation Completion",
                                        QString( "MD In: %1 - MD Out: %2" ).arg( cell.startMD ).arg( cell.endMD ) +
                                            QString( " Transmissibility: " ) + QString::number( transmissibility ) );
                completion.setSourcePdmObject( interval );
                completionData.push_back( completion );
            }
        }
    }

    if ( nonDarcyParameters->nonDarcyFlowType() == RimNonDarcyPerforationParameters::NON_DARCY_USER_DEFINED )
    {
        // Compute D-factor for completion as
        // D_cell = D_well * Sum_Tran_cells / Tran_cell
        //
        // See https://github.com/OPM/ResInsight/issues/7450

        double accumulatedTransmissibility = 0.0;
        for ( const auto& completion : completionData )
        {
            accumulatedTransmissibility += completion.transmissibility();
        }

        double userDefinedDFactor = nonDarcyParameters->userDefinedDFactor();
        for ( auto& completion : completionData )
        {
            double dFactorForCompletion = userDefinedDFactor * accumulatedTransmissibility / completion.transmissibility();
            completion.setDFactor( dFactorForCompletion );
        }
    }

    return completionData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::appendCompletionData( std::map<size_t, std::vector<RigCompletionData>>* completionData,
                                                                       const std::vector<RigCompletionData>& completionsToAppend )
{
    for ( const auto& completion : completionsToAppend )
    {
        auto it = completionData->find( completion.completionDataGridCell().globalCellIndex() );
        if ( it != completionData->end() )
        {
            it->second.push_back( completion );
        }
        else
        {
            completionData->insert( std::pair<size_t, std::vector<RigCompletionData>>( completion.completionDataGridCell().globalCellIndex(),
                                                                                       std::vector<RigCompletionData>{ completion } ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, cvf::Vec2i>
    RicWellPathExportCompletionDataFeatureImpl::wellPathUpperGridIntersectionIJ( gsl::not_null<const RimEclipseCase*> gridCase,
                                                                                 gsl::not_null<const RimWellPath*>    wellPath,
                                                                                 const QString&                       gridName )
{
    const RigEclipseCaseData* caseData       = gridCase->eclipseCaseData();
    const RigMainGrid*        mainGrid       = caseData->mainGrid();
    const RigActiveCellInfo*  activeCellInfo = caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    auto wellPathGeometry = wellPath->wellPathGeometry();
    CVF_ASSERT( wellPathGeometry );

    const std::vector<cvf::Vec3d>& coords = wellPathGeometry->wellPathPoints();
    const std::vector<double>&     mds    = wellPathGeometry->measuredDepths();
    CVF_ASSERT( !coords.empty() && !mds.empty() );

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
RimWellPath* RicWellPathExportCompletionDataFeatureImpl::topLevelWellPath( const RigCompletionData& completion )
{
    if ( completion.sourcePdmObject() )
    {
        auto parentWellPath = completion.sourcePdmObject()->firstAncestorOrThisOfType<RimWellPath>();
        if ( parentWellPath )
        {
            return parentWellPath->topLevelWellPath();
        }
    }

    return nullptr;
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

    std::vector<RimWellPath*> allWellPathLaterals;
    if ( wellPath->unitSystem() == eCase->eclipseCaseData()->unitsType() )
    {
        allWellPathLaterals = wellPath->allWellPathLaterals();
    }
    else
    {
        return {};
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
                generatePerforationsCompdatValues( wellPathLateral,
                                                   wellPathLateral->perforationIntervalCollection()->perforations(),
                                                   exportSettings,
                                                   exportDate );

            appendCompletionData( &completionsPerEclipseCellAllCompletionTypes, perforationCompletionData );
        }

        if ( exportSettings.includeFishbones )
        {
            // Make sure the start and end location is computed if needed
            wellPathLateral->fishbonesCollection()->computeStartAndEndLocation();

            std::vector<RigCompletionData> fishbonesCompletionData =
                RicFishbonesTransmissibilityCalculationFeatureImp::generateFishboneCompdatValuesUsingAdjustedCellVolume( wellPathLateral,
                                                                                                                         exportSettings );

            appendCompletionData( &completionsPerEclipseCellAllCompletionTypes, fishbonesCompletionData );
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

            appendCompletionData( &completionsPerEclipseCellAllCompletionTypes, fractureCompletionData );
        }
    }

    for ( auto& data : completionsPerEclipseCellAllCompletionTypes )
    {
        completions.push_back( combineEclipseCellCompletions( data.second, exportSettings ) );
    }

    std::sort( completions.begin(), completions.end() );

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

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

#include "RiaEclipseUnitTools.h"
#include "RiaFilePathTools.h"
#include "RiaFractureDefines.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaWeightedMeanCalculator.h"

#include "ExportCommands/RicExportLgrFeature.h"
#include "RicExportCompletionDataSettingsUi.h"
#include "RicExportFeatureImpl.h"
#include "RicExportFractureCompletionsImpl.h"
#include "RicFishbonesTransmissibilityCalculationFeatureImp.h"
#include "RicWellPathExportCompletionsFileTools.h"
#include "RicWellPathExportMswCompletionsImpl.h"
#include "RicWellPathFractureReportItem.h"
#include "RicWellPathFractureTextReportFeatureImpl.h"

#include "RifTextDataTableFormatter.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigPerforationTransmissibilityEquations.h"
#include "RigResultAccessorFactory.h"
#include "RigTransmissibilityEquations.h"
#include "RigVirtualPerforationTransmissibilities.h"
#include "RigWellLogExtractionTools.h"
#include "RigWellLogExtractor.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"

#include "RimFileWellPath.h"
#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimFractureTemplate.h"
#include "RimNonDarcyPerforationParameters.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#include "RimWellPathValve.h"

#include "Riu3DMainWindowTools.h"
#include "RiuMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"
#include "cafSelectionManager.h"
#include "cafUtils.h"

#include "cvfPlane.h"

#include <QDir>

#include <map>
#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportCompletions( const std::vector<RimWellPath*>& topLevelWellPaths,
                                                                    const std::vector<RimSimWellInView*>& simWells,
                                                                    const RicExportCompletionDataSettingsUi& exportSettings )
{
    if ( exportSettings.caseToApply() == nullptr || exportSettings.caseToApply()->eclipseCaseData() == nullptr )
    {
        RiaLogging::error( "Export Completions Data: Cannot export completions data without specified eclipse case" );
        return;
    }

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

        RiaPreferences* prefs = RiaPreferences::current();
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
                RiaLogging::error( QString( "Export Completions Data: Could not open the file: %1" )
                                       .arg( fractureTransmisibillityExportInformationPath ) );
            }
            else
            {
                fractureTransmissibilityExportInformationStream =
                    std::unique_ptr<QTextStream>( new QTextStream( &fractureTransmissibilityExportInformationFile ) );
            }
        }

        std::vector<RigCompletionData> completions;

        {
            caf::ProgressInfo progress( topLevelWellPaths.size(), "Extracting Completion Data For Well Paths" );

            for ( RimWellPath* wellPath : topLevelWellPaths )
            {
                std::vector<RimWellPath*> allWellPathLaterals;
                if ( wellPath->unitSystem() == exportSettings.caseToApply->eclipseCaseData()->unitsType() )
                {
                    auto tieInWells = wellPath->wellPathLateralsRecursively();
                    for ( auto w : tieInWells )
                    {
                        allWellPathLaterals.push_back( w );
                    }
                }
                else
                {
                    int     caseId = exportSettings.caseToApply->caseId();
                    QString format = QString(
                        "Unit systems for well path \"%1\" must match unit system of chosen eclipse case \"%2\"" );
                    QString errMsg = format.arg( wellPath->name() ).arg( caseId );
                    RiaLogging::error( errMsg );
                }

                std::map<size_t, std::vector<RigCompletionData>> completionsPerEclipseCellAllCompletionTypes;
                std::map<size_t, std::vector<RigCompletionData>> completionsPerEclipseCellFishbones;
                std::map<size_t, std::vector<RigCompletionData>> completionsPerEclipseCellFracture;
                std::map<size_t, std::vector<RigCompletionData>> completionsPerEclipseCellPerforations;

                for ( auto wellPath : allWellPathLaterals )
                {
                    // Generate completion data

                    if ( exportSettings.includePerforations )
                    {
                        std::vector<RigCompletionData> perforationCompletionData =
                            generatePerforationsCompdatValues( wellPath,
                                                               wellPath->perforationIntervalCollection()->perforations(),
                                                               exportSettings );

                        appendCompletionData( &completionsPerEclipseCellAllCompletionTypes, perforationCompletionData );
                        appendCompletionData( &completionsPerEclipseCellPerforations, perforationCompletionData );
                    }

                    if ( exportSettings.includeFishbones )
                    {
                        std::vector<RigCompletionData> fishbonesCompletionData =
                            RicFishbonesTransmissibilityCalculationFeatureImp::
                                generateFishboneCompdatValuesUsingAdjustedCellVolume( wellPath, exportSettings );

                        appendCompletionData( &completionsPerEclipseCellAllCompletionTypes, fishbonesCompletionData );
                        appendCompletionData( &completionsPerEclipseCellFishbones, fishbonesCompletionData );
                    }

                    if ( exportSettings.includeFractures() )
                    {
                        // If no report is wanted, set reportItems = nullptr
                        std::vector<RicWellPathFractureReportItem>* reportItems = &fractureDataReportItems;

                        std::vector<RigCompletionData> fractureCompletionData = RicExportFractureCompletionsImpl::
                            generateCompdatValuesForWellPath( wellPath,
                                                              exportSettings.caseToApply(),
                                                              reportItems,
                                                              fractureTransmissibilityExportInformationStream.get(),
                                                              RicExportFractureCompletionsImpl::
                                                                  PressureDepletionParameters( exportSettings.performTransScaling(),
                                                                                               exportSettings.transScalingTimeStep(),
                                                                                               exportSettings.transScalingWBHPSource(),
                                                                                               exportSettings
                                                                                                   .transScalingWBHP() ) );

                        appendCompletionData( &completionsPerEclipseCellAllCompletionTypes, fractureCompletionData );
                        appendCompletionData( &completionsPerEclipseCellFracture, fractureCompletionData );
                    }
                }

                if ( exportSettings.reportCompletionsTypesIndividually() )
                {
                    for ( auto& data : completionsPerEclipseCellFracture )
                    {
                        completions.push_back( combineEclipseCellCompletions( data.second, exportSettings ) );
                    }

                    for ( auto& data : completionsPerEclipseCellFishbones )
                    {
                        completions.push_back( combineEclipseCellCompletions( data.second, exportSettings ) );
                    }

                    for ( auto& data : completionsPerEclipseCellPerforations )
                    {
                        completions.push_back( combineEclipseCellCompletions( data.second, exportSettings ) );
                    }
                }
                else
                {
                    for ( auto& data : completionsPerEclipseCellAllCompletionTypes )
                    {
                        completions.push_back( combineEclipseCellCompletions( data.second, exportSettings ) );
                    }
                }

                progress.incrementProgress();
            }
        }

        for ( auto simWell : simWells )
        {
            std::map<size_t, std::vector<RigCompletionData>> completionsPerEclipseCell;

            std::vector<RigCompletionData> fractureCompletionData = RicExportFractureCompletionsImpl::
                generateCompdatValuesForSimWell( exportSettings.caseToApply(),
                                                 simWell,
                                                 fractureTransmissibilityExportInformationStream.get(),
                                                 RicExportFractureCompletionsImpl::
                                                     PressureDepletionParameters( exportSettings.performTransScaling(),
                                                                                  exportSettings.transScalingTimeStep(),
                                                                                  exportSettings.transScalingWBHPSource(),
                                                                                  exportSettings.transScalingWBHP() ) );

            appendCompletionData( &completionsPerEclipseCell, fractureCompletionData );

            for ( auto& data : completionsPerEclipseCell )
            {
                completions.push_back( combineEclipseCellCompletions( data.second, exportSettings ) );
            }
        }

        const QString eclipseCaseName = exportSettings.caseToApply->caseUserDescription();

        if ( exportSettings.fileSplit == RicExportCompletionDataSettingsUi::UNIFIED_FILE )
        {
            QString fileName = exportSettings.customFileName();
            if ( fileName.isEmpty() ) fileName = QString( "UnifiedCompletions_%1" ).arg( eclipseCaseName );

            sortAndExportCompletionsToFile( exportSettings.caseToApply,
                                            exportSettings.folder,
                                            fileName,
                                            completions,
                                            fractureDataReportItems,
                                            exportSettings.compdatExport,
                                            exportSettings.exportDataSourceAsComment(),
                                            exportSettings.exportWelspec() );
        }
        else if ( exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL )
        {
            for ( auto wellPath : topLevelWellPaths )
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

                QString fileName = QString( "%1_UnifiedCompletions_%2" ).arg( wellPath->name() ).arg( eclipseCaseName );
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
        else if ( exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL_AND_COMPLETION_TYPE )
        {
            std::vector<RigCompletionData::CompletionType> completionTypes;
            completionTypes.push_back( RigCompletionData::CompletionType::FISHBONES );
            completionTypes.push_back( RigCompletionData::CompletionType::FRACTURE );
            completionTypes.push_back( RigCompletionData::CompletionType::PERFORATION );

            for ( const auto& completionType : completionTypes )
            {
                for ( auto wellPath : topLevelWellPaths )
                {
                    std::vector<RigCompletionData> completionsForWell;
                    for ( const auto& completion : completions )
                    {
                        if ( completionType == completion.completionType() )
                        {
                            if ( wellPath == topLevelWellPath( completion ) )
                            {
                                completionsForWell.push_back( completion );
                            }
                        }
                    }

                    if ( completionsForWell.empty() ) continue;

                    std::vector<RicWellPathFractureReportItem> reportItemsForWell;
                    if ( completionType == RigCompletionData::CompletionType::FRACTURE )
                    {
                        for ( const auto& fracItem : fractureDataReportItems )
                        {
                            if ( fracItem.wellPathNameForExport() == wellPath->completionSettings()->wellNameForExport() )
                            {
                                reportItemsForWell.push_back( fracItem );
                            }
                        }
                    }

                    {
                        QString completionTypeText;
                        if ( completionType == RigCompletionData::CompletionType::FISHBONES )
                            completionTypeText = "Fishbones";
                        if ( completionType == RigCompletionData::CompletionType::FRACTURE )
                            completionTypeText = "Fracture";
                        if ( completionType == RigCompletionData::CompletionType::PERFORATION )
                            completionTypeText = "Perforation";

                        QString fileName =
                            QString( "%1_%2_%3" ).arg( wellPath->name() ).arg( completionTypeText ).arg( eclipseCaseName );
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
        }

        // Export sim wells
        if ( exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL ||
             exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL_AND_COMPLETION_TYPE )
        {
            for ( auto simWell : simWells )
            {
                std::vector<RigCompletionData> wellCompletions;
                for ( const auto& completion : completions )
                {
                    if ( completion.wellName() == simWell->name() )
                    {
                        wellCompletions.push_back( completion );
                    }
                }

                if ( wellCompletions.empty() ) continue;

                QString fileName = QString( "%1_Fractures_%2" ).arg( simWell->name() ).arg( eclipseCaseName );
                sortAndExportCompletionsToFile( exportSettings.caseToApply,
                                                exportSettings.folder,
                                                fileName,
                                                wellCompletions,
                                                fractureDataReportItems,
                                                exportSettings.compdatExport,
                                                exportSettings.exportDataSourceAsComment(),
                                                exportSettings.exportWelspec() );
            }
        }
    }

    if ( exportSettings.includeMsw )
    {
        RicWellPathExportMswCompletionsImpl::exportWellSegmentsForAllCompletions( exportSettings, topLevelWellPaths );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicWellPathExportCompletionDataFeatureImpl::computeStaticCompletionsForWellPath( RimWellPath*    wellPath,
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
std::vector<RigCompletionData>
    RicWellPathExportCompletionDataFeatureImpl::computeDynamicCompletionsForWellPath( RimWellPath*    wellPath,
                                                                                      RimEclipseCase* eclipseCase,
                                                                                      size_t          timeStepIndex )
{
    std::vector<RigCompletionData> completionsPerEclipseCell;

    if ( eclipseCase && eclipseCase->eclipseCaseData() )
    {
        RicExportCompletionDataSettingsUi exportSettings;
        exportSettings.caseToApply         = eclipseCase;
        exportSettings.timeStep            = static_cast<int>( timeStepIndex );
        exportSettings.includeFishbones    = true;
        exportSettings.includePerforations = true;
        exportSettings.includeFractures    = true;

        completionsPerEclipseCell =
            generatePerforationsCompdatValues( wellPath,
                                               wellPath->perforationIntervalCollection()->perforations(),
                                               exportSettings );
    }

    return completionsPerEclipseCell;
}

//==================================================================================================
///
//==================================================================================================
RigCompletionData RicWellPathExportCompletionDataFeatureImpl::combineEclipseCellCompletions(
    const std::vector<RigCompletionData>&    completions,
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

    for ( const RigCompletionData& completion : completions )
    {
        double transmissibility = completion.transmissibility();

        diameterCalculator.addValueAndWeight( completion.diameter(), transmissibility );
        skinFactorCalculator.addValueAndWeight( completion.skinFactor(), transmissibility );

        if ( transmissibility > largestTransmissibilityValue )
        {
            largestTransmissibilityValue = transmissibility;
            cellDirection                = completion.direction();
        }
    }

    double combinedDiameter   = diameterCalculator.weightedMean();
    double combinedSkinFactor = skinFactorCalculator.weightedMean();

    double combinedTrans   = 0.0;
    double combinedKh      = 0.0;
    double combinedDFactor = 0.0;

    {
        RiaWeightedMeanCalculator<double> dFactorCalculator;

        for ( const RigCompletionData& completion : completions )
        {
            resultCompletion.m_metadata.reserve( resultCompletion.m_metadata.size() + completion.m_metadata.size() );
            resultCompletion.m_metadata.insert( resultCompletion.m_metadata.end(),
                                                completion.m_metadata.begin(),
                                                completion.m_metadata.end() );

            if ( completion.wellName() != firstCompletion.wellName() )
            {
                QString errorMessage = QString( "Cannot combine completions of different types in same cell %1" )
                                           .arg( cellIndexIJK.oneBasedLocalCellIndexString() );
                RiaLogging::error( errorMessage );
                resultCompletion.addMetadata( "ERROR", errorMessage );
                return resultCompletion; // Returning empty completion, should not be exported
            }

            if ( completion.transmissibility() == HUGE_VAL )
            {
                QString errorMessage = QString( "Transmissibility calculation has failed for cell %1" )
                                           .arg( cellIndexIJK.oneBasedLocalCellIndexString() );
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

    if ( settings.compdatExport == RicExportCompletionDataSettingsUi::TRANSMISSIBILITIES )
    {
        resultCompletion.setCombinedValuesExplicitTrans( combinedTrans,
                                                         combinedKh,
                                                         combinedDFactor,
                                                         combinedSkinFactor,
                                                         combinedDiameter,
                                                         cellDirection,
                                                         completionType );
    }
    else if ( settings.compdatExport == RicExportCompletionDataSettingsUi::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS )
    {
        // calculate trans for main bore - but as Eclipse will do it!
        double transmissibilityEclipseCalculation =
            RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibilityAsEclipseDoes( settings.caseToApply(),
                                                                                                combinedSkinFactor,
                                                                                                combinedDiameter / 2,
                                                                                                cellIndexIJK.globalCellIndex(),
                                                                                                cellDirection );

        double wpimult = combinedTrans / transmissibilityEclipseCalculation;
        resultCompletion.setCombinedValuesImplicitTransWPImult( wpimult,
                                                                combinedKh,
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
                                                                       bool exportDataSourceAsComment )
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
            .add( completionSettings->wellGroupNameForExport() )
            .addOneBasedCellIndex( ijIntersection.second.x() )
            .addOneBasedCellIndex( ijIntersection.second.y() )
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
void RicWellPathExportCompletionDataFeatureImpl::exportWelspeclToFile(
    RimEclipseCase*                                          gridCase,
    QFilePtr                                                 exportFile,
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
            const auto wellPath =
                RicWellPathExportCompletionsFileTools::findWellPathFromExportName( completion.wellName() );
            wellPathToLgrNameMap[wellPath].insert( completionsForLgr.first );
        }
    }

    for ( const auto& wellPathsForLgr : wellPathToLgrNameMap )
    {
        const RimWellPath* wellPath = wellPathsForLgr.first;

        std::tuple<double, cvf::Vec2i, QString> itemWithLowestMD =
            std::make_tuple( std::numeric_limits<double>::max(), cvf::Vec2i(), "" );

        // Find first LGR-intersection along the well path

        for ( const auto& lgrName : wellPathsForLgr.second )
        {
            auto ijIntersection = wellPathUpperGridIntersectionIJ( gridCase, wellPath, lgrName );
            if ( ijIntersection.first < std::get<0>( itemWithLowestMD ) )
            {
                itemWithLowestMD = std::make_tuple( ijIntersection.first, ijIntersection.second, lgrName );
            }
        }

        {
            double     measuredDepth = 0.0;
            cvf::Vec2i ijIntersection;
            QString    lgrName;

            std::tie( measuredDepth, ijIntersection, lgrName ) = itemWithLowestMD;

            auto completionSettings = wellPath->completionSettings();

            formatter.add( completionSettings->wellNameForExport() )
                .add( completionSettings->wellGroupNameForExport() )
                .add( lgrName )
                .addOneBasedCellIndex( ijIntersection.x() )
                .addOneBasedCellIndex( ijIntersection.y() )
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
        try
        {
            QFileInfo              fi( fileName );
            std::shared_ptr<QFile> exportFile =
                RicWellPathExportCompletionsFileTools::openFileForExport( folderName, fi.baseName(), fi.suffix() );

            std::map<QString, std::vector<RigCompletionData>> completionsForGrid;
            completionsForGrid.insert( std::pair<QString, std::vector<RigCompletionData>>( "", completionsForMainGrid ) );

            exportWellPathFractureReport( eclipseCase, exportFile, wellPathFractureReportItems );
            if ( exportWelspec )
            {
                exportWelspecsToFile( eclipseCase, exportFile, completionsForMainGrid, exportDataSourceAsComment );
            }
            exportCompdatAndWpimultTables( eclipseCase, exportFile, completionsForGrid, exportType, exportDataSourceAsComment );
        }
        catch ( RicWellPathExportCompletionsFileTools::OpenFileException )
        {
        }
    }

    if ( !completionsForSubGrids.empty() )
    {
        try
        {
            QFileInfo fi( fileName );

            QString                lgrFileName = fi.baseName() + "_LGR";
            std::shared_ptr<QFile> exportFile =
                RicWellPathExportCompletionsFileTools::openFileForExport( folderName, lgrFileName, fi.suffix() );

            exportWellPathFractureReport( eclipseCase, exportFile, wellPathFractureReportItems );
            if ( exportWelspec )
            {
                exportWelspeclToFile( eclipseCase, exportFile, completionsForSubGrids );
            }
            exportCompdatAndWpimultTables( eclipseCase, exportFile, completionsForSubGrids, exportType, exportDataSourceAsComment );
        }
        catch ( RicWellPathExportCompletionsFileTools::OpenFileException )
        {
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

        if ( exportType == RicExportCompletionDataSettingsUi::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS )
        {
            exportWpimultTableUsingFormatter( formatter, gridName, completions );
        }
    }

    RiaLogging::info( QString( "Successfully exported completion data to %1" ).arg( exportFile->fileName() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportCompdatTableUsingFormatter(
    RifTextDataTableFormatter&            formatter,
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
              RifTextDataTableColumn( "CONN",
                                      "FACT",
                                      RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
              RifTextDataTableColumn( "WELL", "DIA" ),
              RifTextDataTableColumn( "KH",
                                      "FACT",
                                      RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
              RifTextDataTableColumn( "SKIN", "FACT" ),
              RifTextDataTableColumn( "D",
                                      "FACT",
                                      RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
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
              RifTextDataTableColumn( "CONN",
                                      "FACT",
                                      RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
              RifTextDataTableColumn( "WELL", "DIA" ),
              RifTextDataTableColumn( "KH",
                                      "FACT",
                                      RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
              RifTextDataTableColumn( "SKIN", "FACT" ),
              RifTextDataTableColumn( "D",
                                      "FACT",
                                      RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
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

        formatter.addOneBasedCellIndex( data.completionDataGridCell().localCellIndexI() )
            .addOneBasedCellIndex( data.completionDataGridCell().localCellIndexJ() )
            .addOneBasedCellIndex( data.completionDataGridCell().localCellIndexK() )
            .addOneBasedCellIndex( data.completionDataGridCell().localCellIndexK() );

        formatter.add( "OPEN" );

        formatter.addValueOrDefaultMarker( data.saturation(), RigCompletionData::defaultValue() );
        formatter.addValueOrDefaultMarker( data.transmissibility(), RigCompletionData::defaultValue() );
        formatter.addValueOrDefaultMarker( data.diameter(), RigCompletionData::defaultValue() );
        formatter.addValueOrDefaultMarker( data.kh(), RigCompletionData::defaultValue() );
        formatter.addValueOrDefaultMarker( data.skinFactor(), RigCompletionData::defaultValue() );
        if ( RigCompletionData::isDefaultValue( data.dFactor() ) )
            formatter.add( "1*" );
        else
            formatter.add( -data.dFactor() );

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
void RicWellPathExportCompletionDataFeatureImpl::exportWpimultTableUsingFormatter(
    RifTextDataTableFormatter&            formatter,
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
        if ( completion.wpimult() == 0.0 || completion.isDefaultValue( completion.wpimult() ) )
        {
            continue;
        }

        formatter.add( completion.wellName() );

        if ( !gridName.isEmpty() )
        {
            formatter.add( gridName );
        }

        formatter.add( completion.wpimult() );

        formatter.addOneBasedCellIndex( completion.completionDataGridCell().localCellIndexI() )
            .addOneBasedCellIndex( completion.completionDataGridCell().localCellIndexJ() )
            .addOneBasedCellIndex( completion.completionDataGridCell().localCellIndexK() );
        formatter.rowCompleted();
    }

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicWellPathExportCompletionDataFeatureImpl::generatePerforationsCompdatValues(
    gsl::not_null<const RimWellPath*>                 wellPath,
    const std::vector<const RimPerforationInterval*>& intervals,
    const RicExportCompletionDataSettingsUi&          settings )
{
    RiaDefines::EclipseUnitSystem unitSystem = settings.caseToApply->eclipseCaseData()->unitsType();

    std::vector<RigCompletionData> completionData;

    const RigActiveCellInfo* activeCellInfo =
        settings.caseToApply->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    auto wellPathGeometry = wellPath->wellPathGeometry();
    if ( !wellPathGeometry ) return completionData;
    auto timeSteps = settings.caseToApply->timeStepDates();

    if ( wellPath->perforationIntervalCollection()->isChecked() )
    {
        for ( const RimPerforationInterval* interval : intervals )
        {
            if ( !interval->isChecked() ) continue;
            if ( (size_t)settings.timeStep < timeSteps.size() &&
                 !interval->isActiveOnDate( settings.caseToApply->timeStepDates()[settings.timeStep] ) )
                continue;

            std::pair<std::vector<cvf::Vec3d>, std::vector<double>> perforationPointsAndMD =
                wellPathGeometry->clippedPointSubset( interval->startMD(), interval->endMD() );

            std::vector<WellPathCellIntersectionInfo> intersectedCells =
                RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( settings.caseToApply->eclipseCaseData(),
                                                                                  wellPath->name(),
                                                                                  perforationPointsAndMD.first,
                                                                                  perforationPointsAndMD.second );

            for ( auto& cell : intersectedCells )
            {
                bool cellIsActive = activeCellInfo->isActive( cell.globCellIndex );
                if ( !cellIsActive ) continue;

                RigCompletionData completion( wellPath->completionSettings()->wellNameForExport(),
                                              RigCompletionDataGridCell( cell.globCellIndex,
                                                                         settings.caseToApply->mainGrid() ),
                                              cell.startMD );

                RigCompletionData::CellDirection direction =
                    calculateCellMainDirection( settings.caseToApply, cell.globCellIndex, cell.intersectionLengthsInCellCS );

                const RimNonDarcyPerforationParameters* nonDarcyParameters =
                    wellPath->perforationIntervalCollection()->nonDarcyParameters();

                double transmissibility = 0.0;
                double kh               = RigCompletionData::defaultValue();
                double dFactor          = RigCompletionData::defaultValue();

                {
                    auto transmissibilityData = calculateTransmissibilityData( settings.caseToApply,
                                                                               wellPath,
                                                                               cell.intersectionLengthsInCellCS,
                                                                               interval->skinFactor(),
                                                                               interval->diameter( unitSystem ) / 2,
                                                                               cell.globCellIndex,
                                                                               settings.useLateralNTG );

                    transmissibility = transmissibilityData.connectionFactor();
                    kh               = transmissibilityData.kh();

                    if ( nonDarcyParameters->nonDarcyFlowType() == RimNonDarcyPerforationParameters::NON_DARCY_USER_DEFINED )
                    {
                        dFactor = nonDarcyParameters->userDefinedDFactor();
                    }
                    else if ( nonDarcyParameters->nonDarcyFlowType() == RimNonDarcyPerforationParameters::NON_DARCY_COMPUTED )
                    {
                        const double effectiveH = transmissibilityData.effectiveH();

                        const double effectivePermeability =
                            nonDarcyParameters->gridPermeabilityScalingFactor() * transmissibilityData.effectiveK();

                        dFactor = calculateDFactor( settings.caseToApply,
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
                completion.addMetadata( "Perforation Completion",
                                        QString( "MD In: %1 - MD Out: %2" ).arg( cell.startMD ).arg( cell.endMD ) +
                                            QString( " Transmissibility: " ) + QString::number( transmissibility ) );
                completion.setSourcePdmObject( interval );
                completionData.push_back( completion );
            }
        }
    }

    return completionData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::appendCompletionData(
    std::map<size_t, std::vector<RigCompletionData>>* completionData,
    const std::vector<RigCompletionData>&             completionsToAppend )
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
            completionData->insert(
                std::pair<size_t, std::vector<RigCompletionData>>( completion.completionDataGridCell().globalCellIndex(),
                                                                   std::vector<RigCompletionData>{ completion } ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData::CellDirection
    RicWellPathExportCompletionDataFeatureImpl::calculateCellMainDirection( RimEclipseCase*   eclipseCase,
                                                                            size_t            globalCellIndex,
                                                                            const cvf::Vec3d& lengthsInCell )
{
    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( "DX" ) );
    cvf::ref<RigResultAccessor> dxAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( "DX" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( "DY" ) );
    cvf::ref<RigResultAccessor> dyAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( "DY" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( "DZ" ) );
    cvf::ref<RigResultAccessor> dzAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( "DZ" ) );

    double xLengthFraction = fabs( lengthsInCell.x() / dxAccessObject->cellScalarGlobIdx( globalCellIndex ) );
    double yLengthFraction = fabs( lengthsInCell.y() / dyAccessObject->cellScalarGlobIdx( globalCellIndex ) );
    double zLengthFraction = fabs( lengthsInCell.z() / dzAccessObject->cellScalarGlobIdx( globalCellIndex ) );

    if ( xLengthFraction > yLengthFraction && xLengthFraction > zLengthFraction )
    {
        return RigCompletionData::CellDirection::DIR_I;
    }
    else if ( yLengthFraction > xLengthFraction && yLengthFraction > zLengthFraction )
    {
        return RigCompletionData::CellDirection::DIR_J;
    }
    else
    {
        return RigCompletionData::CellDirection::DIR_K;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TransmissibilityData RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibilityData(
    RimEclipseCase*                  eclipseCase,
    const RimWellPath*               wellPath,
    const cvf::Vec3d&                internalCellLengths,
    double                           skinFactor,
    double                           wellRadius,
    size_t                           globalCellIndex,
    bool                             useLateralNTG,
    size_t                           volumeScaleConstant,
    RigCompletionData::CellDirection directionForVolumeScaling )
{
    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( "DX" ) );
    cvf::ref<RigResultAccessor> dxAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( "DX" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( "DY" ) );
    cvf::ref<RigResultAccessor> dyAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( "DY" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( "DZ" ) );
    cvf::ref<RigResultAccessor> dzAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( "DZ" ) );

    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( "PERMX" ) );
    cvf::ref<RigResultAccessor> permxAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( "PERMX" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( "PERMY" ) );
    cvf::ref<RigResultAccessor> permyAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( "PERMY" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( "PERMZ" ) );
    cvf::ref<RigResultAccessor> permzAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( "PERMZ" ) );

    if ( dxAccessObject.isNull() || dyAccessObject.isNull() || dzAccessObject.isNull() || permxAccessObject.isNull() ||
         permyAccessObject.isNull() || permzAccessObject.isNull() )
    {
        return TransmissibilityData();
    }

    double ntg = 1.0;
    {
        // Trigger loading from file
        eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
            ->ensureKnownResultLoaded( RigEclipseResultAddress( "NTG" ) );

        cvf::ref<RigResultAccessor> ntgAccessObject =
            RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                               0,
                                                               RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                               0,
                                                               RigEclipseResultAddress( "NTG" ) );

        if ( ntgAccessObject.notNull() )
        {
            ntg = ntgAccessObject->cellScalarGlobIdx( globalCellIndex );
        }
    }
    double latNtg = useLateralNTG ? ntg : 1.0;

    double dx    = dxAccessObject->cellScalarGlobIdx( globalCellIndex );
    double dy    = dyAccessObject->cellScalarGlobIdx( globalCellIndex );
    double dz    = dzAccessObject->cellScalarGlobIdx( globalCellIndex );
    double permx = permxAccessObject->cellScalarGlobIdx( globalCellIndex );
    double permy = permyAccessObject->cellScalarGlobIdx( globalCellIndex );
    double permz = permzAccessObject->cellScalarGlobIdx( globalCellIndex );

    const double totalKh = RigTransmissibilityEquations::totalKh( permx, permy, permz, internalCellLengths, latNtg, ntg );

    const double effectiveK =
        RigTransmissibilityEquations::effectiveK( permx, permy, permz, internalCellLengths, latNtg, ntg );
    const double effectiveH = RigTransmissibilityEquations::effectiveH( internalCellLengths, latNtg, ntg );

    double darcy = RiaEclipseUnitTools::darcysConstant( wellPath->unitSystem() );

    if ( volumeScaleConstant != 1 )
    {
        if ( directionForVolumeScaling == RigCompletionData::CellDirection::DIR_I ) dx = dx / volumeScaleConstant;
        if ( directionForVolumeScaling == RigCompletionData::CellDirection::DIR_J ) dy = dy / volumeScaleConstant;
        if ( directionForVolumeScaling == RigCompletionData::CellDirection::DIR_K ) dz = dz / volumeScaleConstant;
    }

    const double transx = RigTransmissibilityEquations::wellBoreTransmissibilityComponent( internalCellLengths.x() * latNtg,
                                                                                           permy,
                                                                                           permz,
                                                                                           dy,
                                                                                           dz,
                                                                                           wellRadius,
                                                                                           skinFactor,
                                                                                           darcy );
    const double transy = RigTransmissibilityEquations::wellBoreTransmissibilityComponent( internalCellLengths.y() * latNtg,
                                                                                           permx,
                                                                                           permz,
                                                                                           dx,
                                                                                           dz,
                                                                                           wellRadius,
                                                                                           skinFactor,
                                                                                           darcy );
    const double transz = RigTransmissibilityEquations::wellBoreTransmissibilityComponent( internalCellLengths.z() * ntg,
                                                                                           permy,
                                                                                           permx,
                                                                                           dy,
                                                                                           dx,
                                                                                           wellRadius,
                                                                                           skinFactor,
                                                                                           darcy );

    const double totalConnectionFactor = RigTransmissibilityEquations::totalConnectionFactor( transx, transy, transz );

    TransmissibilityData trData;
    trData.setData( effectiveH, effectiveK, totalConnectionFactor, totalKh );
    return trData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathExportCompletionDataFeatureImpl::calculateDFactor( RimEclipseCase* eclipseCase,
                                                                     double          effectiveH,
                                                                     size_t          globalCellIndex,
                                                                     const RimNonDarcyPerforationParameters* nonDarcyParameters,
                                                                     const double effectivePermeability )
{
    using EQ = RigPerforationTransmissibilityEquations;

    if ( !eclipseCase || !eclipseCase->eclipseCaseData() )
    {
        return std::numeric_limits<double>::infinity();
    }

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    double porosity = 0.0;
    {
        eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
            ->ensureKnownResultLoaded( RigEclipseResultAddress( "PORO" ) );
        cvf::ref<RigResultAccessor> poroAccessObject =
            RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                               0,
                                                               RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                               0,
                                                               RigEclipseResultAddress( "PORO" ) );

        if ( poroAccessObject.notNull() )
        {
            porosity = poroAccessObject->cellScalar( globalCellIndex );
        }
    }

    const double betaFactor = EQ::betaFactor( nonDarcyParameters->inertialCoefficientBeta0(),
                                              effectivePermeability,
                                              nonDarcyParameters->permeabilityScalingFactor(),
                                              porosity,
                                              nonDarcyParameters->porosityScalingFactor() );

    const double alpha = RiaDefines::nonDarcyFlowAlpha( eclipseCaseData->unitsType() );

    return EQ::dFactor( alpha,
                        betaFactor,
                        effectivePermeability,
                        effectiveH,
                        nonDarcyParameters->wellRadius(),
                        nonDarcyParameters->relativeGasDensity(),
                        nonDarcyParameters->gasViscosity() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibilityAsEclipseDoes( RimEclipseCase* eclipseCase,
                                                                                           double          skinFactor,
                                                                                           double          wellRadius,
                                                                                           size_t globalCellIndex,
                                                                                           RigCompletionData::CellDirection direction )
{
    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( "DX" ) );
    cvf::ref<RigResultAccessor> dxAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( "DX" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( "DY" ) );
    cvf::ref<RigResultAccessor> dyAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( "DY" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( "DZ" ) );
    cvf::ref<RigResultAccessor> dzAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( "DZ" ) );

    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( "PERMX" ) );
    cvf::ref<RigResultAccessor> permxAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( "PERMX" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( "PERMY" ) );
    cvf::ref<RigResultAccessor> permyAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( "PERMY" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( "PERMZ" ) );
    cvf::ref<RigResultAccessor> permzAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( "PERMZ" ) );

    double ntg = 1.0;
    if ( eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
             ->ensureKnownResultLoaded( RigEclipseResultAddress( "NTG" ) ) )
    {
        cvf::ref<RigResultAccessor> ntgAccessObject =
            RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                               0,
                                                               RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                               0,
                                                               RigEclipseResultAddress( "NTG" ) );
        ntg = ntgAccessObject->cellScalarGlobIdx( globalCellIndex );
    }

    double dx    = dxAccessObject->cellScalarGlobIdx( globalCellIndex );
    double dy    = dyAccessObject->cellScalarGlobIdx( globalCellIndex );
    double dz    = dzAccessObject->cellScalarGlobIdx( globalCellIndex );
    double permx = permxAccessObject->cellScalarGlobIdx( globalCellIndex );
    double permy = permyAccessObject->cellScalarGlobIdx( globalCellIndex );
    double permz = permzAccessObject->cellScalarGlobIdx( globalCellIndex );

    RiaDefines::EclipseUnitSystem units = eclipseCaseData->unitsType();
    double                        darcy = RiaEclipseUnitTools::darcysConstant( units );

    double trans = cvf::UNDEFINED_DOUBLE;
    if ( direction == RigCompletionData::CellDirection::DIR_I )
    {
        trans = RigTransmissibilityEquations::wellBoreTransmissibilityComponent( dx,
                                                                                 permy,
                                                                                 permz,
                                                                                 dy,
                                                                                 dz,
                                                                                 wellRadius,
                                                                                 skinFactor,
                                                                                 darcy );
    }
    else if ( direction == RigCompletionData::CellDirection::DIR_J )
    {
        trans = RigTransmissibilityEquations::wellBoreTransmissibilityComponent( dy,
                                                                                 permx,
                                                                                 permz,
                                                                                 dx,
                                                                                 dz,
                                                                                 wellRadius,
                                                                                 skinFactor,
                                                                                 darcy );
    }
    else if ( direction == RigCompletionData::CellDirection::DIR_K )
    {
        trans = RigTransmissibilityEquations::wellBoreTransmissibilityComponent( dz * ntg,
                                                                                 permy,
                                                                                 permx,
                                                                                 dy,
                                                                                 dx,
                                                                                 wellRadius,
                                                                                 skinFactor,
                                                                                 darcy );
    }

    return trans;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, cvf::Vec2i> RicWellPathExportCompletionDataFeatureImpl::wellPathUpperGridIntersectionIJ(
    gsl::not_null<const RimEclipseCase*> gridCase,
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

    for ( WellPathCellIntersectionInfo intersection : intersections )
    {
        size_t             gridLocalCellIndex = 0;
        const RigGridBase* grid =
            mainGrid->gridAndGridLocalIdxFromGlobalCellIdx( intersection.globCellIndex, &gridLocalCellIndex );

        if ( grid->gridId() == gridId && activeCellInfo->isActive( intersection.globCellIndex ) )
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
    RimWellPath* parentWellPath = nullptr;
    if ( completion.sourcePdmObject() )
    {
        completion.sourcePdmObject()->firstAncestorOrThisOfType( parentWellPath );
    }

    if ( parentWellPath )
    {
        return parentWellPath->topLevelWellPath();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportCarfinForTemporaryLgrs( const RimEclipseCase* sourceCase,
                                                                               const QString&        folder )
{
    if ( !sourceCase || !sourceCase->mainGrid() ) return;

    const auto  mainGrid         = sourceCase->mainGrid();
    const auto& lgrInfosForWells = RicExportLgrFeature::createLgrInfoListForTemporaryLgrs( mainGrid );

    for ( const auto& lgrInfoForWell : lgrInfosForWells )
    {
        RicExportLgrFeature::exportLgrs( folder, lgrInfoForWell.first, lgrInfoForWell.second );
    }
}

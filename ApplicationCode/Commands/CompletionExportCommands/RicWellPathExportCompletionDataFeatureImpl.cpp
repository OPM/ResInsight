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

#include "RiaApplication.h"
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
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
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

#include "RiuMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"
#include "cafSelectionManager.h"
#include "cafUtils.h"

#include "cvfPlane.h"

#include "RicWellPathExportCompletionsFileTools.h"
#include <QDir>
#include <map>
#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportCompletions( const std::vector<RimWellPath*>&      wellPaths,
                                                                    const std::vector<RimSimWellInView*>& simWells,
                                                                    const RicExportCompletionDataSettingsUi& exportSettings )
{
    if ( exportSettings.caseToApply() == nullptr || exportSettings.caseToApply()->eclipseCaseData() == nullptr )
    {
        RiaLogging::error( "Export Completions Data: Cannot export completions data without specified eclipse case" );
        return;
    }

    exportCarfinForTemporaryLgrs( exportSettings.caseToApply(), exportSettings.folder );

    if ( exportSettings.compdatExport == RicExportCompletionDataSettingsUi::TRANSMISSIBILITIES ||
         exportSettings.compdatExport == RicExportCompletionDataSettingsUi::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS )
    {
        std::vector<RimWellPath*> usedWellPaths;
        for ( RimWellPath* wellPath : wellPaths )
        {
            if ( wellPath->unitSystem() == exportSettings.caseToApply->eclipseCaseData()->unitsType() )
            {
                usedWellPaths.push_back( wellPath );
            }
            else
            {
                int     caseId = exportSettings.caseToApply->caseId();
                QString format =
                    QString( "Unit systems for well path \"%1\" must match unit system of chosen eclipse case \"%2\"" );
                QString errMsg = format.arg( wellPath->name() ).arg( caseId );
                RiaLogging::error( errMsg );
            }
        }

        std::vector<RicWellPathFractureReportItem> fractureDataReportItems;

        // FractureTransmissibilityExportInformation
        std::unique_ptr<QTextStream> fractureTransmissibilityExportInformationStream = nullptr;
        QFile                        fractureTransmissibilityExportInformationFile;

        RiaPreferences* prefs = RiaApplication::instance()->preferences();
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

        size_t maxProgress =
            usedWellPaths.size() * 3 + simWells.size() +
            ( exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL
                  ? usedWellPaths.size()
                  : exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL_AND_COMPLETION_TYPE
                        ? usedWellPaths.size() * 3
                        : 1 ) +
            simWells.size();

        caf::ProgressInfo progress( maxProgress, "Export Completions" );

        progress.setProgressDescription( "Read Completion Data" );

        std::vector<RigCompletionData> completions;

        for ( auto wellPath : usedWellPaths )
        {
            std::map<size_t, std::vector<RigCompletionData>> completionsPerEclipseCellAllCompletionTypes;
            std::map<size_t, std::vector<RigCompletionData>> completionsPerEclipseCellFishbones;
            std::map<size_t, std::vector<RigCompletionData>> completionsPerEclipseCellFracture;
            std::map<size_t, std::vector<RigCompletionData>> completionsPerEclipseCellPerforations;

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
            progress.incrementProgress();

            if ( exportSettings.includeFishbones )
            {
                std::vector<RigCompletionData> fishbonesCompletionData = RicFishbonesTransmissibilityCalculationFeatureImp::
                    generateFishboneCompdatValuesUsingAdjustedCellVolume( wellPath, exportSettings );

                appendCompletionData( &completionsPerEclipseCellAllCompletionTypes, fishbonesCompletionData );
                appendCompletionData( &completionsPerEclipseCellFishbones, fishbonesCompletionData );
            }
            progress.incrementProgress();

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
                                                                                       exportSettings.transScalingWBHP() ) );

                appendCompletionData( &completionsPerEclipseCellAllCompletionTypes, fractureCompletionData );
                appendCompletionData( &completionsPerEclipseCellFracture, fractureCompletionData );
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

            progress.incrementProgress();
        }

        const QString eclipseCaseName = exportSettings.caseToApply->caseUserDescription();

        progress.setProgressDescription( "Write Export Files" );
        if ( exportSettings.fileSplit == RicExportCompletionDataSettingsUi::UNIFIED_FILE )
        {
            QString fileName = QString( "UnifiedCompletions_%1" ).arg( eclipseCaseName );
            sortAndExportCompletionsToFile( exportSettings.caseToApply,
                                            exportSettings.folder,
                                            fileName,
                                            completions,
                                            fractureDataReportItems,
                                            exportSettings.compdatExport );
            progress.incrementProgress();
        }
        else if ( exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL )
        {
            for ( auto wellPath : usedWellPaths )
            {
                std::vector<RigCompletionData> completionsForWell;
                for ( const auto& completion : completions )
                {
                    if ( RicWellPathExportCompletionDataFeatureImpl::isCompletionWellPathEqual( completion, wellPath ) )
                    {
                        completionsForWell.push_back( completion );
                    }
                }

                if ( completionsForWell.empty() ) continue;

                std::vector<RicWellPathFractureReportItem> reportItemsForWell;
                for ( const auto& fracItem : fractureDataReportItems )
                {
                    if ( fracItem.wellPathNameForExport() == wellPath->completions()->wellNameForExport() )
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
                                                exportSettings.compdatExport );
                progress.incrementProgress();
            }
        }
        else if ( exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL_AND_COMPLETION_TYPE )
        {
            std::vector<RigCompletionData::CompletionType> completionTypes;
            completionTypes.push_back( RigCompletionData::FISHBONES );
            completionTypes.push_back( RigCompletionData::FRACTURE );
            completionTypes.push_back( RigCompletionData::PERFORATION );

            for ( const auto& completionType : completionTypes )
            {
                for ( auto wellPath : usedWellPaths )
                {
                    std::vector<RigCompletionData> completionsForWell;
                    for ( const auto& completion : completions )
                    {
                        if ( completionType == completion.completionType() )
                        {
                            if ( RicWellPathExportCompletionDataFeatureImpl::isCompletionWellPathEqual( completion,
                                                                                                        wellPath ) )
                            {
                                completionsForWell.push_back( completion );
                            }
                        }
                    }

                    if ( completionsForWell.empty() ) continue;

                    {
                        QString completionTypeText;
                        if ( completionType == RigCompletionData::FISHBONES ) completionTypeText = "Fishbones";
                        if ( completionType == RigCompletionData::FRACTURE ) completionTypeText = "Fracture";
                        if ( completionType == RigCompletionData::PERFORATION ) completionTypeText = "Perforation";

                        QString fileName =
                            QString( "%1_%2_%3" ).arg( wellPath->name() ).arg( completionTypeText ).arg( eclipseCaseName );
                        if ( completionType == RigCompletionData::FRACTURE )
                        {
                            std::vector<RicWellPathFractureReportItem> reportItemsForWell;
                            for ( const auto& fracItem : fractureDataReportItems )
                            {
                                if ( fracItem.wellPathNameForExport() == wellPath->completions()->wellNameForExport() )
                                {
                                    reportItemsForWell.push_back( fracItem );
                                }
                            }

                            sortAndExportCompletionsToFile( exportSettings.caseToApply,
                                                            exportSettings.folder,
                                                            fileName,
                                                            completionsForWell,
                                                            reportItemsForWell,
                                                            exportSettings.compdatExport );
                        }
                        else
                        {
                            std::vector<RicWellPathFractureReportItem> emptyReportItemVector;
                            sortAndExportCompletionsToFile( exportSettings.caseToApply,
                                                            exportSettings.folder,
                                                            fileName,
                                                            completionsForWell,
                                                            emptyReportItemVector,
                                                            exportSettings.compdatExport );
                        }
                    }

                    progress.incrementProgress();
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
                                                exportSettings.compdatExport );

                progress.incrementProgress();
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

    const RigCompletionData& firstCompletion = completions[0];

    const QString&                    wellName       = firstCompletion.wellName();
    const RigCompletionDataGridCell&  cellIndexIJK   = firstCompletion.completionDataGridCell();
    RigCompletionData::CompletionType completionType = firstCompletion.completionType();

    RigCompletionData resultCompletion( wellName, cellIndexIJK, firstCompletion.firstOrderingValue() );
    resultCompletion.setSecondOrderingValue( firstCompletion.secondOrderingValue() );
    resultCompletion.setSourcePdmObject( firstCompletion.sourcePdmObject() );

    // completion type, skin factor, well bore diameter and cell direction are taken from (first) main bore,
    // if no main bore they are taken from first completion
    double        skinfactor       = firstCompletion.skinFactor();
    double        wellBoreDiameter = firstCompletion.diameter();
    CellDirection cellDirection    = firstCompletion.direction();

    for ( const RigCompletionData& completion : completions )
    {
        // Use data from the completion with largest diameter
        // This is more robust than checking for main bore flag
        // See also https://github.com/OPM/ResInsight/issues/2765
        if ( completion.diameter() > wellBoreDiameter )
        {
            skinfactor       = completion.skinFactor();
            wellBoreDiameter = completion.diameter();
            cellDirection    = completion.direction();
        }
    }

    double combinedTrans   = 0.0;
    double combinedKh      = 0.0;
    double combinedDFactor = 0.0;

    if ( completions.size() == 1 )
    {
        resultCompletion.m_metadata = completions[0].m_metadata;
        combinedTrans               = completions[0].transmissibility();
        combinedKh                  = completions[0].kh();
        combinedDFactor             = completions[0].dFactor();
    }
    else
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
                                                         skinfactor,
                                                         wellBoreDiameter,
                                                         cellDirection,
                                                         completionType );
    }
    else if ( settings.compdatExport == RicExportCompletionDataSettingsUi::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS )
    {
        // calculate trans for main bore - but as Eclipse will do it!
        double transmissibilityEclipseCalculation =
            RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibilityAsEclipseDoes( settings.caseToApply(),
                                                                                                skinfactor,
                                                                                                wellBoreDiameter / 2,
                                                                                                cellIndexIJK.globalCellIndex(),
                                                                                                cellDirection );

        double wpimult = combinedTrans / transmissibilityEclipseCalculation;
        resultCompletion.setCombinedValuesImplicitTransWPImult( wpimult,
                                                                combinedKh,
                                                                combinedDFactor,
                                                                skinfactor,
                                                                wellBoreDiameter,
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
                completions.insert( std::pair<QString, std::vector<RigCompletionData>>( gridName, {completion} ) );
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
                    if ( reportItem.wellPathNameForExport() == wellPath->completions()->wellNameForExport() )
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
                                                                       const std::vector<RigCompletionData>& completions )
{
    QTextStream stream( exportFile.get() );

    RifTextDataTableFormatter formatter( stream );
    formatter.setColumnSpacing( 2 );

    std::vector<RifTextDataTableColumn> header = {RifTextDataTableColumn( "Well" ),
                                                  RifTextDataTableColumn( "Grp" ),
                                                  RifTextDataTableColumn( "I" ),
                                                  RifTextDataTableColumn( "J" ),
                                                  RifTextDataTableColumn( "RefDepth" ),
                                                  RifTextDataTableColumn( "Type" ),
                                                  RifTextDataTableColumn( "DrainRad" ),
                                                  RifTextDataTableColumn( "GasInEq" ),
                                                  RifTextDataTableColumn( "AutoShut" ),
                                                  RifTextDataTableColumn( "XFlow" ),
                                                  RifTextDataTableColumn( "FluidPVT" ),
                                                  RifTextDataTableColumn( "HydSDens" ),
                                                  RifTextDataTableColumn( "FluidInPlReg" )};

    formatter.keyword( "WELSPECS" );
    formatter.header( header );

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
        auto rimCompletions = wellPath->completions();
        auto ijIntersection = wellPathUpperGridIntersectionIJ( gridCase, wellPath );

        formatter.add( rimCompletions->wellNameForExport() )
            .add( rimCompletions->wellGroupNameForExport() )
            .addOneBasedCellIndex( ijIntersection.second.x() )
            .addOneBasedCellIndex( ijIntersection.second.y() )
            .add( rimCompletions->referenceDepthForExport() )
            .add( rimCompletions->wellTypeNameForExport() )
            .add( rimCompletions->drainageRadiusForExport() )
            .add( rimCompletions->gasInflowEquationForExport() )
            .add( rimCompletions->automaticWellShutInForExport() )
            .add( rimCompletions->allowWellCrossFlowForExport() )
            .add( rimCompletions->wellBoreFluidPVTForExport() )
            .add( rimCompletions->hydrostaticDensityForExport() )
            .add( rimCompletions->fluidInPlaceRegionForExport() )
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

    std::vector<RifTextDataTableColumn> header = {RifTextDataTableColumn( "Well" ),
                                                  RifTextDataTableColumn( "Grp" ),
                                                  RifTextDataTableColumn( "LGR" ),
                                                  RifTextDataTableColumn( "I" ),
                                                  RifTextDataTableColumn( "J" ),
                                                  RifTextDataTableColumn( "RefDepth" ),
                                                  RifTextDataTableColumn( "Type" ),
                                                  RifTextDataTableColumn( "DrainRad" ),
                                                  RifTextDataTableColumn( "GasInEq" ),
                                                  RifTextDataTableColumn( "AutoShut" ),
                                                  RifTextDataTableColumn( "XFlow" ),
                                                  RifTextDataTableColumn( "FluidPVT" ),
                                                  RifTextDataTableColumn( "HydSDens" ),
                                                  RifTextDataTableColumn( "FluidInPlReg" )};

    formatter.keyword( "WELSPECL" );
    formatter.header( header );

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

            auto rimCompletions = wellPath->completions();

            formatter.add( rimCompletions->wellNameForExport() )
                .add( rimCompletions->wellGroupNameForExport() )
                .add( lgrName )
                .addOneBasedCellIndex( ijIntersection.x() )
                .addOneBasedCellIndex( ijIntersection.y() )
                .add( rimCompletions->referenceDepthForExport() )
                .add( rimCompletions->wellTypeNameForExport() )
                .add( rimCompletions->drainageRadiusForExport() )
                .add( rimCompletions->gasInflowEquationForExport() )
                .add( rimCompletions->automaticWellShutInForExport() )
                .add( rimCompletions->allowWellCrossFlowForExport() )
                .add( rimCompletions->wellBoreFluidPVTForExport() )
                .add( rimCompletions->hydrostaticDensityForExport() )
                .add( rimCompletions->fluidInPlaceRegionForExport() )
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
    RicExportCompletionDataSettingsUi::CompdatExportType exportType )
{
    // Sort completions based on grid they belong to
    std::vector<RigCompletionData>                    completionsForMainGrid = mainGridCompletions( completions );
    std::map<QString, std::vector<RigCompletionData>> completionsForSubGrids = subGridsCompletions( completions );

    if ( !completionsForMainGrid.empty() )
    {
        try
        {
            std::shared_ptr<QFile> exportFile =
                RicWellPathExportCompletionsFileTools::openFileForExport( folderName, fileName );

            std::map<QString, std::vector<RigCompletionData>> completionsForGrid;
            completionsForGrid.insert( std::pair<QString, std::vector<RigCompletionData>>( "", completionsForMainGrid ) );

            exportWellPathFractureReport( eclipseCase, exportFile, wellPathFractureReportItems );
            exportWelspecsToFile( eclipseCase, exportFile, completionsForMainGrid );
            exportCompdatAndWpimultTables( eclipseCase, exportFile, completionsForGrid, exportType );
        }
        catch ( RicWellPathExportCompletionsFileTools::OpenFileException )
        {
        }
    }

    if ( !completionsForSubGrids.empty() )
    {
        try
        {
            QString                lgrFileName = fileName + "_LGR";
            std::shared_ptr<QFile> exportFile =
                RicWellPathExportCompletionsFileTools::openFileForExport( folderName, lgrFileName );

            exportWellPathFractureReport( eclipseCase, exportFile, wellPathFractureReportItems );
            exportWelspeclToFile( eclipseCase, exportFile, completionsForSubGrids );
            exportCompdatAndWpimultTables( eclipseCase, exportFile, completionsForSubGrids, exportType );
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
    RicExportCompletionDataSettingsUi::CompdatExportType     exportType )
{
    if ( completionsPerGrid.empty() ) return;

    QTextStream stream( exportFile.get() );

    RifTextDataTableFormatter formatter( stream );
    formatter.setColumnSpacing( 3 );

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
    std::vector<RifTextDataTableColumn> header;

    if ( gridName.isEmpty() )
    {
        header = {RifTextDataTableColumn( "Well" ),
                  RifTextDataTableColumn( "I" ),
                  RifTextDataTableColumn( "J" ),
                  RifTextDataTableColumn( "K1" ),
                  RifTextDataTableColumn( "K2" ),
                  RifTextDataTableColumn( "Status" ),
                  RifTextDataTableColumn( "SAT" ),
                  RifTextDataTableColumn( "TR",
                                          RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
                  RifTextDataTableColumn( "DIAM" ),
                  RifTextDataTableColumn( "KH",
                                          RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
                  RifTextDataTableColumn( "S" ),
                  RifTextDataTableColumn( "Df",
                                          RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
                  RifTextDataTableColumn( "DIR" )};

        formatter.keyword( "COMPDAT" );
    }
    else
    {
        header = {RifTextDataTableColumn( "Well" ),
                  RifTextDataTableColumn( "LgrName" ),
                  RifTextDataTableColumn( "I" ),
                  RifTextDataTableColumn( "J" ),
                  RifTextDataTableColumn( "K1" ),
                  RifTextDataTableColumn( "K2" ),
                  RifTextDataTableColumn( "Status" ),
                  RifTextDataTableColumn( "SAT" ),
                  RifTextDataTableColumn( "TR",
                                          RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
                  RifTextDataTableColumn( "DIAM" ),
                  RifTextDataTableColumn( "KH",
                                          RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
                  RifTextDataTableColumn( "S" ),
                  RifTextDataTableColumn( "Df",
                                          RifTextDataTableDoubleFormatting( RifTextDataTableDoubleFormat::RIF_SCIENTIFIC ) ),
                  RifTextDataTableColumn( "DIR" )};

        formatter.keyword( "COMPDATL" );
    }
    formatter.header( header );

    RigCompletionData::CompletionType currentCompletionType = RigCompletionData::CT_UNDEFINED;

    for ( const RigCompletionData& data : completionData )
    {
        if ( currentCompletionType != data.completionType() )
        {
            // The completions are sorted by completion type, write out a heading when completion type changes

            QString txt;
            if ( data.completionType() == RigCompletionData::FISHBONES ) txt = "Fishbones";
            if ( data.completionType() == RigCompletionData::FRACTURE ) txt = "Fracture";
            if ( data.completionType() == RigCompletionData::PERFORATION ) txt = "Perforation";

            formatter.comment( "---- Completions for completion type " + txt + " ----" );

            currentCompletionType = data.completionType();
        }

        for ( const RigCompletionMetaData& metadata : data.metadata() )
        {
            formatter.comment( QString( "%1 : %2" ).arg( metadata.name ).arg( metadata.comment ) );
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
        switch ( data.connectionState() )
        {
            case OPEN:
                formatter.add( "OPEN" );
                break;
            case SHUT:
                formatter.add( "SHUT" );
                break;
            case AUTO:
                formatter.add( "AUTO" );
                break;
        }

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
            case DIR_I:
                formatter.add( "'X'" );
                break;
            case DIR_J:
                formatter.add( "'Y'" );
                break;
            case DIR_K:
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
    const RimWellPath*                                wellPath,
    const std::vector<const RimPerforationInterval*>& intervals,
    const RicExportCompletionDataSettingsUi&          settings )
{
    RiaEclipseUnitTools::UnitSystem unitSystem = settings.caseToApply->eclipseCaseData()->unitsType();

    std::vector<RigCompletionData> completionData;
    if ( !wellPath || !wellPath->wellPathGeometry() )
    {
        return completionData;
    }

    const RigActiveCellInfo* activeCellInfo =
        settings.caseToApply->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    if ( wellPath->perforationIntervalCollection()->isChecked() )
    {
        for ( const RimPerforationInterval* interval : intervals )
        {
            if ( !interval->isChecked() ) continue;
            if ( !interval->isActiveOnDate( settings.caseToApply->timeStepDates()[settings.timeStep] ) ) continue;

            using namespace std;
            pair<vector<cvf::Vec3d>, vector<double>> perforationPointsAndMD =
                wellPath->wellPathGeometry()->clippedPointSubset( interval->startMD(), interval->endMD() );

            std::vector<WellPathCellIntersectionInfo> intersectedCells =
                RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( settings.caseToApply->eclipseCaseData(),
                                                                                  perforationPointsAndMD.first,
                                                                                  perforationPointsAndMD.second );

            for ( auto& cell : intersectedCells )
            {
                bool cellIsActive = activeCellInfo->isActive( cell.globCellIndex );
                if ( !cellIsActive ) continue;

                RigCompletionData completion( wellPath->completions()->wellNameForExport(),
                                              RigCompletionDataGridCell( cell.globCellIndex,
                                                                         settings.caseToApply->mainGrid() ),
                                              cell.startMD );

                CellDirection direction = calculateCellMainDirection( settings.caseToApply,
                                                                      cell.globCellIndex,
                                                                      cell.intersectionLengthsInCellCS );

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

                    if ( nonDarcyParameters->nonDarcyFlowType() == RimNonDarcyPerforationParameters::NON_DARCY_USER_DEFINED )
                    {
                        kh      = transmissibilityData.kh();
                        dFactor = nonDarcyParameters->userDefinedDFactor();
                    }
                    else if ( nonDarcyParameters->nonDarcyFlowType() == RimNonDarcyPerforationParameters::NON_DARCY_COMPUTED )
                    {
                        kh = transmissibilityData.kh();

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
                                                                   std::vector<RigCompletionData>{completion} ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CellDirection RicWellPathExportCompletionDataFeatureImpl::calculateCellMainDirection( RimEclipseCase*   eclipseCase,
                                                                                      size_t            globalCellIndex,
                                                                                      const cvf::Vec3d& lengthsInCell )
{
    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DX" ) );
    cvf::ref<RigResultAccessor> dxAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    "DX" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DY" ) );
    cvf::ref<RigResultAccessor> dyAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    "DY" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DZ" ) );
    cvf::ref<RigResultAccessor> dzAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    "DZ" ) );

    double xLengthFraction = fabs( lengthsInCell.x() / dxAccessObject->cellScalarGlobIdx( globalCellIndex ) );
    double yLengthFraction = fabs( lengthsInCell.y() / dyAccessObject->cellScalarGlobIdx( globalCellIndex ) );
    double zLengthFraction = fabs( lengthsInCell.z() / dzAccessObject->cellScalarGlobIdx( globalCellIndex ) );

    if ( xLengthFraction > yLengthFraction && xLengthFraction > zLengthFraction )
    {
        return CellDirection::DIR_I;
    }
    else if ( yLengthFraction > xLengthFraction && yLengthFraction > zLengthFraction )
    {
        return CellDirection::DIR_J;
    }
    else
    {
        return CellDirection::DIR_K;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TransmissibilityData
    RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibilityData( RimEclipseCase*    eclipseCase,
                                                                               const RimWellPath* wellPath,
                                                                               const cvf::Vec3d&  internalCellLengths,
                                                                               double             skinFactor,
                                                                               double             wellRadius,
                                                                               size_t             globalCellIndex,
                                                                               bool               useLateralNTG,
                                                                               size_t             volumeScaleConstant,
                                                                               CellDirection directionForVolumeScaling )
{
    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DX" ) );
    cvf::ref<RigResultAccessor> dxAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    "DX" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DY" ) );
    cvf::ref<RigResultAccessor> dyAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    "DY" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DZ" ) );
    cvf::ref<RigResultAccessor> dzAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    "DZ" ) );

    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMX" ) );
    cvf::ref<RigResultAccessor> permxAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    "PERMX" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMY" ) );
    cvf::ref<RigResultAccessor> permyAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    "PERMY" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMZ" ) );
    cvf::ref<RigResultAccessor> permzAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    "PERMZ" ) );

    if ( dxAccessObject.isNull() || dyAccessObject.isNull() || dzAccessObject.isNull() || permxAccessObject.isNull() ||
         permyAccessObject.isNull() || permzAccessObject.isNull() )
    {
        return TransmissibilityData();
    }

    double ntg = 1.0;
    {
        // Trigger loading from file
        eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
            ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "NTG" ) );

        cvf::ref<RigResultAccessor> ntgAccessObject =
            RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                               0,
                                                               RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                               0,
                                                               RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                        "NTG" ) );

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
        if ( directionForVolumeScaling == CellDirection::DIR_I ) dx = dx / volumeScaleConstant;
        if ( directionForVolumeScaling == CellDirection::DIR_J ) dy = dy / volumeScaleConstant;
        if ( directionForVolumeScaling == CellDirection::DIR_K ) dz = dz / volumeScaleConstant;
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
            ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PORO" ) );
        cvf::ref<RigResultAccessor> poroAccessObject =
            RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                               0,
                                                               RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                               0,
                                                               RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                        "PORO" ) );

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
                                                                                           CellDirection direction )
{
    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DX" ) );
    cvf::ref<RigResultAccessor> dxAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    "DX" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DY" ) );
    cvf::ref<RigResultAccessor> dyAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    "DY" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DZ" ) );
    cvf::ref<RigResultAccessor> dzAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    "DZ" ) );

    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMX" ) );
    cvf::ref<RigResultAccessor> permxAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    "PERMX" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMY" ) );
    cvf::ref<RigResultAccessor> permyAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    "PERMY" ) );
    eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
        ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMZ" ) );
    cvf::ref<RigResultAccessor> permzAccessObject =
        RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                           0,
                                                           RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                           0,
                                                           RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                    "PERMZ" ) );

    double ntg = 1.0;
    if ( eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
             ->ensureKnownResultLoaded( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "NTG" ) ) )
    {
        cvf::ref<RigResultAccessor> ntgAccessObject =
            RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                               0,
                                                               RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                               0,
                                                               RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                        "NTG" ) );
        ntg = ntgAccessObject->cellScalarGlobIdx( globalCellIndex );
    }

    double dx    = dxAccessObject->cellScalarGlobIdx( globalCellIndex );
    double dy    = dyAccessObject->cellScalarGlobIdx( globalCellIndex );
    double dz    = dzAccessObject->cellScalarGlobIdx( globalCellIndex );
    double permx = permxAccessObject->cellScalarGlobIdx( globalCellIndex );
    double permy = permyAccessObject->cellScalarGlobIdx( globalCellIndex );
    double permz = permzAccessObject->cellScalarGlobIdx( globalCellIndex );

    RiaEclipseUnitTools::UnitSystem units = eclipseCaseData->unitsType();
    double                          darcy = RiaEclipseUnitTools::darcysConstant( units );

    double trans = cvf::UNDEFINED_DOUBLE;
    if ( direction == CellDirection::DIR_I )
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
    else if ( direction == CellDirection::DIR_J )
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
    else if ( direction == CellDirection::DIR_K )
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
std::pair<double, cvf::Vec2i>
    RicWellPathExportCompletionDataFeatureImpl::wellPathUpperGridIntersectionIJ( const RimEclipseCase* gridCase,
                                                                                 const RimWellPath*    wellPath,
                                                                                 const QString&        gridName )
{
    const RigEclipseCaseData* caseData       = gridCase->eclipseCaseData();
    const RigMainGrid*        mainGrid       = caseData->mainGrid();
    const RigActiveCellInfo*  activeCellInfo = caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    const RigWellPath*        wellPathGeometry = wellPath->wellPathGeometry();
    const std::vector<cvf::Vec3d>& coords      = wellPathGeometry->wellPathPoints();
    const std::vector<double>&     mds         = wellPathGeometry->measureDepths();
    CVF_ASSERT( !coords.empty() && !mds.empty() );

    std::vector<WellPathCellIntersectionInfo> intersections =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( caseData, coords, mds );

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
bool RicWellPathExportCompletionDataFeatureImpl::isCompletionWellPathEqual( const RigCompletionData& completion,
                                                                            const RimWellPath*       wellPath )
{
    if ( !wellPath ) return false;

    RimWellPath* parentWellPath = nullptr;
    if ( completion.sourcePdmObject() )
    {
        completion.sourcePdmObject()->firstAncestorOrThisOfType( parentWellPath );
    }

    return ( parentWellPath == wellPath );
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

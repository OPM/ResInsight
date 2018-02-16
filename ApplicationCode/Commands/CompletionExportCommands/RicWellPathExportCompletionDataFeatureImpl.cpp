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
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RicExportCompletionDataSettingsUi.h"
#include "RicExportFeatureImpl.h"
#include "RicExportFractureCompletionsImpl.h"
#include "RicFishbonesTransmissibilityCalculationFeatureImp.h"

#include "RifEclipseDataTableFormatter.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigResultAccessorFactory.h"
#include "RigTransmissibilityEquations.h"
#include "RigWellLogExtractionTools.h"
#include "RigWellLogExtractor.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"

#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimSimWellInView.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"

#include "RiuMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"
#include "cafSelectionManager.h"

#include "cvfPlane.h"

#include <QDir>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportCompletions(const std::vector<RimWellPath*>&         wellPaths,
                                                                   const std::vector<RimSimWellInView*>&    simWells,
                                                                   const RicExportCompletionDataSettingsUi& exportSettings)
{
    if (exportSettings.caseToApply() == nullptr)
    {
        RiaLogging::error("Export Completions Data: Cannot export completions data without specified eclipse case");
        return;
    }

    std::vector<RimWellPath*> usedWellPaths;
    if (exportSettings.wellSelection == RicExportCompletionDataSettingsUi::ALL_WELLS ||
        exportSettings.wellSelection == RicExportCompletionDataSettingsUi::SELECTED_WELLS)
    {
        usedWellPaths = wellPaths;
    }
    else if (exportSettings.wellSelection == RicExportCompletionDataSettingsUi::CHECKED_WELLS)
    {
        for (auto wellPath : wellPaths)
        {
            if (wellPath->showWellPath)
            {
                usedWellPaths.push_back(wellPath);
            }
        }
    }

    {
        bool unitSystemMismatch = false;
        for (const RimWellPath* wellPath : usedWellPaths)
        {
            if (wellPath->unitSystem() != exportSettings.caseToApply->eclipseCaseData()->unitsType())
            {
                unitSystemMismatch = true;
                break;
            }
        }

        for (const RimSimWellInView* simWell : simWells)
        {
            RimEclipseCase* eclipseCase;
            simWell->firstAncestorOrThisOfType(eclipseCase);
            if (exportSettings.caseToApply->eclipseCaseData()->unitsType() != eclipseCase->eclipseCaseData()->unitsType())
            {
                unitSystemMismatch = true;
                break;
            }
        }
        if (unitSystemMismatch)
        {
            RiaLogging::error("Well path unit systems must match unit system of chosen eclipse case.");
            return;
        }
    }

    std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>> completionsPerEclipseCell;

    // FractureTransmissibilityExportInformation
    std::unique_ptr<QTextStream> fractureTransmissibilityExportInformationStream = nullptr;

    QString fractureTransmisibillityExportInformationPath =
        QDir(exportSettings.folder).filePath("FractureTransmissibilityExportInformation");
    QFile fractureTransmissibilityExportInformationFile(fractureTransmisibillityExportInformationPath);

    RiaPreferences* prefs = RiaApplication::instance()->preferences();
    if (prefs->includeFractureDebugInfoFile())
    {
        if (!fractureTransmissibilityExportInformationFile.open(QIODevice::WriteOnly))
        {
            RiaLogging::error(QString("Export Completions Data: Could not open the file: %1")
                                  .arg(fractureTransmisibillityExportInformationPath));
            return;
        }

        fractureTransmissibilityExportInformationStream =
            std::unique_ptr<QTextStream>(new QTextStream(&fractureTransmissibilityExportInformationFile));
    }

    size_t maxProgress = usedWellPaths.size() * 3 +
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
                         simWells.size() +
#endif // USE_PROTOTYPE_FEATURE_FRACTURES
                         (exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL
                              ? usedWellPaths.size()
                              : exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL_AND_COMPLETION_TYPE
                                    ? usedWellPaths.size() * 3
                                    : 1) +
                         simWells.size();

    caf::ProgressInfo progress(maxProgress, "Export Completions");

    progress.setProgressDescription("Read Completion Data");
    for (auto wellPath : usedWellPaths)
    {
        // Generate completion data

        if (exportSettings.includePerforations)
        {
            std::vector<RigCompletionData> perforationCompletionData =
                generatePerforationsCompdatValues(wellPath, exportSettings);
            appendCompletionData(&completionsPerEclipseCell, perforationCompletionData);
        }
        progress.incrementProgress();

        if (exportSettings.includeFishbones)
        {
            std::vector<RigCompletionData> fishbonesCompletionData =
                RicFishbonesTransmissibilityCalculationFeatureImp::generateFishboneCompdatValuesUsingAdjustedCellVolume(
                    wellPath, exportSettings);
            appendCompletionData(&completionsPerEclipseCell, fishbonesCompletionData);
        }
        progress.incrementProgress();

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
        if (exportSettings.includeFractures())
        {
            std::vector<RigCompletionData> fractureCompletionData =
                RicExportFractureCompletionsImpl::generateCompdatValuesForWellPath(
                    wellPath, exportSettings, fractureTransmissibilityExportInformationStream.get());
            appendCompletionData(&completionsPerEclipseCell, fractureCompletionData);
        }
#endif // USE_PROTOTYPE_FEATURE_FRACTURES
        progress.incrementProgress();
    }

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
    for (auto simWell : simWells)
    {
        std::vector<RigCompletionData> fractureCompletionData = RicExportFractureCompletionsImpl::generateCompdatValuesForSimWell(
            exportSettings.caseToApply(), simWell, fractureTransmissibilityExportInformationStream.get());
        appendCompletionData(&completionsPerEclipseCell, fractureCompletionData);
        progress.incrementProgress();
    }
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

    const QString eclipseCaseName = exportSettings.caseToApply->caseUserDescription();

    progress.setProgressDescription("Write Export Files");
    if (exportSettings.fileSplit == RicExportCompletionDataSettingsUi::UNIFIED_FILE)
    {
        std::vector<RigCompletionData> completions;
        for (auto& data : completionsPerEclipseCell)
        {
            completions.push_back(combineEclipseCellCompletions(data.second, exportSettings));
        }

        const QString fileName = QString("UnifiedCompletions_%1").arg(eclipseCaseName);
        printCompletionsToFiles(exportSettings.folder, fileName, completions, exportSettings.compdatExport);
        progress.incrementProgress();
    }
    else if (exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL)
    {
        for (auto wellPath : usedWellPaths)
        {
            std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>> filteredWellCompletions =
                getCompletionsForWell(completionsPerEclipseCell, wellPath->completions()->wellNameForExport());
            std::vector<RigCompletionData> completions;
            for (auto& data : filteredWellCompletions)
            {
                completions.push_back(combineEclipseCellCompletions(data.second, exportSettings));
            }
            std::vector<RigCompletionData> wellCompletions;
            for (const auto& completion : completions)
            {
                if (completion.wellName() == wellPath->completions()->wellNameForExport())
                {
                    wellCompletions.push_back(completion);
                }
            }

            if (wellCompletions.empty()) continue;

            QString fileName = QString("%1_unifiedCompletions_%2").arg(wellPath->name()).arg(eclipseCaseName);
            printCompletionsToFiles(exportSettings.folder, fileName, wellCompletions, exportSettings.compdatExport);
            progress.incrementProgress();
        }
    }
    else if (exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL_AND_COMPLETION_TYPE)
    {
        for (auto wellPath : usedWellPaths)
        {
            std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>> filteredWellCompletions =
                getCompletionsForWell(completionsPerEclipseCell, wellPath->completions()->wellNameForExport());
            std::vector<RigCompletionData> completions;
            for (auto& data : filteredWellCompletions)
            {
                completions.push_back(combineEclipseCellCompletions(data.second, exportSettings));
            }
            {
                std::vector<RigCompletionData> fishbonesCompletions = getCompletionsForWellAndCompletionType(
                    completions, wellPath->completions()->wellNameForExport(), RigCompletionData::FISHBONES);
                if (!fishbonesCompletions.empty())
                {
                    QString fileName = QString("%1_Fishbones_%2").arg(wellPath->name()).arg(eclipseCaseName);
                    printCompletionsToFiles(exportSettings.folder, fileName, fishbonesCompletions, exportSettings.compdatExport);
                }
                progress.incrementProgress();
            }
            {
                std::vector<RigCompletionData> perforationCompletions = getCompletionsForWellAndCompletionType(
                    completions, wellPath->completions()->wellNameForExport(), RigCompletionData::PERFORATION);
                if (!perforationCompletions.empty())
                {
                    QString fileName = QString("%1_Perforations_%2").arg(wellPath->name()).arg(eclipseCaseName);
                    printCompletionsToFiles(
                        exportSettings.folder, fileName, perforationCompletions, exportSettings.compdatExport);
                }
                progress.incrementProgress();
            }
            {
                std::vector<RigCompletionData> fractureCompletions = getCompletionsForWellAndCompletionType(
                    completions, wellPath->completions()->wellNameForExport(), RigCompletionData::FRACTURE);
                if (!fractureCompletions.empty())
                {
                    QString fileName = QString("%1_Fractures_%2").arg(wellPath->name()).arg(eclipseCaseName);
                    printCompletionsToFiles(exportSettings.folder, fileName, fractureCompletions, exportSettings.compdatExport);
                }
                progress.incrementProgress();
            }
        }
    }

    // Export sim wells
    if (exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL ||
        exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL_AND_COMPLETION_TYPE)
    {
        for (auto simWell : simWells)
        {
            std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>> filteredWellCompletions =
                getCompletionsForWell(completionsPerEclipseCell, simWell->name());
            std::vector<RigCompletionData> completions;
            for (auto& data : filteredWellCompletions)
            {
                completions.push_back(combineEclipseCellCompletions(data.second, exportSettings));
            }
            std::vector<RigCompletionData> wellCompletions;
            for (const auto& completion : completions)
            {
                if (completion.wellName() == simWell->name())
                {
                    wellCompletions.push_back(completion);
                }
            }

            if (wellCompletions.empty()) continue;

            QString fileName = exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL
                                   ? QString("%1_unifiedCompletions_%2").arg(simWell->name()).arg(eclipseCaseName)
                                   : QString("%1_Fractures_%2").arg(simWell->name()).arg(eclipseCaseName);
            printCompletionsToFiles(exportSettings.folder, fileName, wellCompletions, exportSettings.compdatExport);
            progress.incrementProgress();
        }
    }
}

//==================================================================================================
///
//==================================================================================================
RigCompletionData
    RicWellPathExportCompletionDataFeatureImpl::combineEclipseCellCompletions(const std::vector<RigCompletionData>& completions,
                                                                              const RicExportCompletionDataSettingsUi& settings)
{
    CVF_ASSERT(!completions.empty());

    const RigCompletionData& firstCompletion = completions[0];

    const QString&                    wellName       = firstCompletion.wellName();
    const RigCompletionDataGridCell&  cellIndexIJK   = firstCompletion.completionDataGridCell();
    RigCompletionData::CompletionType completionType = firstCompletion.completionType();

    RigCompletionData resultCompletion(wellName, cellIndexIJK, firstCompletion.firstOrderingValue());
    resultCompletion.setSecondOrderingValue(firstCompletion.secondOrderingValue());

    bool anyNonDarcyFlowPresent = false;
    for (const auto& c : completions)
    {
        if (c.isNonDarcyFlow()) anyNonDarcyFlowPresent = true;
    }

    if (anyNonDarcyFlowPresent && completions.size() > 1)
    {
        QString errorMessage =
            QString("Cannot combine multiple completions when Non-Darcy Flow contribution is present in same cell %1")
                .arg(cellIndexIJK.oneBasedLocalCellIndexString());
        RiaLogging::error(errorMessage);
        resultCompletion.addMetadata("ERROR", errorMessage);
        return resultCompletion; // Returning empty completion, should not be exported
    }

    if (firstCompletion.isNonDarcyFlow())
    {
        resultCompletion.setKh(firstCompletion.kh());
        resultCompletion.setDFactor(firstCompletion.dFactor());

        resultCompletion.setCombinedValuesExplicitTrans(firstCompletion.transmissibility(), completionType);

        resultCompletion.m_metadata = firstCompletion.m_metadata;

        return resultCompletion;
    }

    // completion type, skin factor, well bore diameter and cell direction are taken from (first) main bore,
    // if no main bore they are taken from first completion
    double        skinfactor       = firstCompletion.skinFactor();
    double        wellBoreDiameter = firstCompletion.diameter();
    CellDirection cellDirection    = firstCompletion.direction();

    for (const RigCompletionData& completion : completions)
    {
        if (completion.isMainBore())
        {
            skinfactor       = completion.skinFactor();
            wellBoreDiameter = completion.diameter();
            cellDirection    = completion.direction();
            break;
        }
    }

    double totalTrans = 0.0;

    for (const RigCompletionData& completion : completions)
    {
        resultCompletion.m_metadata.reserve(resultCompletion.m_metadata.size() + completion.m_metadata.size());
        resultCompletion.m_metadata.insert(
            resultCompletion.m_metadata.end(), completion.m_metadata.begin(), completion.m_metadata.end());

        if (completion.completionType() != firstCompletion.completionType())
        {
            QString errorMessage = QString("Cannot combine completions of different types in same cell %1")
                                       .arg(cellIndexIJK.oneBasedLocalCellIndexString());
            RiaLogging::error(errorMessage);
            resultCompletion.addMetadata("ERROR", errorMessage);
            return resultCompletion; // Returning empty completion, should not be exported
        }

        if (completion.wellName() != firstCompletion.wellName())
        {
            QString errorMessage = QString("Cannot combine completions of different types in same cell %1")
                                       .arg(cellIndexIJK.oneBasedLocalCellIndexString());
            RiaLogging::error(errorMessage);
            resultCompletion.addMetadata("ERROR", errorMessage);
            return resultCompletion; // Returning empty completion, should not be exported
        }

        if (completion.transmissibility() == HUGE_VAL)
        {
            QString errorMessage =
                QString("Transmissibility calculation has failed for cell %1").arg(cellIndexIJK.oneBasedLocalCellIndexString());
            RiaLogging::error(errorMessage);
            resultCompletion.addMetadata("ERROR", errorMessage);
            return resultCompletion; // Returning empty completion, should not be exported
        }

        totalTrans = totalTrans + completion.transmissibility();
    }

    if (settings.compdatExport == RicExportCompletionDataSettingsUi::TRANSMISSIBILITIES)
    {
        resultCompletion.setCombinedValuesExplicitTrans(totalTrans, completionType);
    }
    else if (settings.compdatExport == RicExportCompletionDataSettingsUi::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS)
    {
        // calculate trans for main bore - but as Eclipse will do it!
        double transmissibilityEclipseCalculation =
            RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibilityAsEclipseDoes(
                settings.caseToApply(), skinfactor, wellBoreDiameter / 2, cellIndexIJK.globalCellIndex(), cellDirection);

        double wpimult = totalTrans / transmissibilityEclipseCalculation;
        resultCompletion.setCombinedValuesImplicitTransWPImult(
            wpimult, cellDirection, skinfactor, wellBoreDiameter, completionType);
    }

    return resultCompletion;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::printCompletionsToFiles(
    const QString&                                       folderName,
    const QString&                                       fileName,
    std::vector<RigCompletionData>&                      completions,
    RicExportCompletionDataSettingsUi::CompdatExportType exportType)
{
    // Sort completions based on grid they belong to
    std::vector<RigCompletionData> completionsForMainGrid;

    std::map<QString, std::vector<RigCompletionData>> completionsForSubGrids;

    for (const auto& completion : completions)
    {
        QString gridName = completion.completionDataGridCell().lgrName();
        if (gridName.isEmpty())
        {
            completionsForMainGrid.push_back(completion);
        }
        else
        {
            auto it = completionsForSubGrids.find(gridName);
            if (it == completionsForSubGrids.end())
            {
                completionsForSubGrids.insert(
                    std::pair<QString, std::vector<RigCompletionData>>(gridName, std::vector<RigCompletionData>{completion}));
            }
            else
            {
                it->second.push_back(completion);
            }
        }
    }

    if (!completionsForMainGrid.empty())
    {
        std::map<QString, std::vector<RigCompletionData>> completionsForGrid;
        completionsForGrid.insert(std::pair<QString, std::vector<RigCompletionData>>("", completionsForMainGrid));

        printCompletionsToFile(folderName, fileName, completionsForGrid, exportType);
    }

    if (!completionsForSubGrids.empty())
    {
        QString lgrFileName = fileName + "_LGR";
        printCompletionsToFile(folderName, lgrFileName, completionsForSubGrids, exportType);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::printCompletionsToFile(
    const QString&                                           folderName,
    const QString&                                           fileName,
    const std::map<QString, std::vector<RigCompletionData>>& completionsPerGrid,
    RicExportCompletionDataSettingsUi::CompdatExportType     exportType)
{
    if (completionsPerGrid.empty()) return;

    QDir exportFolder(folderName);

    if (!exportFolder.exists())
    {
        bool createdPath = exportFolder.mkpath(folderName);
        if (createdPath)
            RiaLogging::info("Created export folder " + folderName);
        else
            RiaLogging::error("Selected output folder does not exist, and could not be created.");
    }

    QString filePath = exportFolder.filePath(fileName);
    QFile   exportFile(filePath);
    if (!exportFile.open(QIODevice::WriteOnly))
    {
        RiaLogging::error(QString("Export Completions Data: Could not open the file: %1").arg(filePath));
        return;
    }

    QTextStream                  stream(&exportFile);
    RifEclipseDataTableFormatter formatter(stream);

    for (const auto& gridCompletions : completionsPerGrid)
    {
        std::vector<RigCompletionData> completions = gridCompletions.second;

        // Sort by well name / cell index
        std::sort(completions.begin(), completions.end());

        // Print completion data
        QString gridName = gridCompletions.first;
        generateCompdatTable(formatter, gridName, completions);

        if (exportType == RicExportCompletionDataSettingsUi::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS)
        {
            generateWpimultTable(formatter, gridName, completions);
        }
    }

    RiaLogging::info(QString("Successfully exported completion data to %1").arg(filePath));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicWellPathExportCompletionDataFeatureImpl::getCompletionsForWellAndCompletionType(
    const std::vector<RigCompletionData>& completions,
    const QString&                        wellName,
    RigCompletionData::CompletionType     completionType)
{
    std::vector<RigCompletionData> filteredCompletions;
    for (const auto& completion : completions)
    {
        if (completion.wellName() == wellName && completion.completionType() == completionType)
        {
            filteredCompletions.push_back(completion);
        }
    }
    return filteredCompletions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>>
    RicWellPathExportCompletionDataFeatureImpl::getCompletionsForWell(
        const std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>>& cellToCompletionMap,
        const QString&                                                             wellName)
{
    std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>> wellCompletions;

    for (const auto& it : cellToCompletionMap)
    {
        for (auto& completion : it.second)
        {
            if (completion.wellName() == wellName)
            {
                wellCompletions[it.first].push_back(completion);
            }
        }
    }

    return wellCompletions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::generateCompdatTable(RifEclipseDataTableFormatter&         formatter,
                                                                      const QString&                        gridName,
                                                                      const std::vector<RigCompletionData>& completionData)
{
    std::vector<RifEclipseOutputTableColumn> header;

    if (gridName.isEmpty())
    {
        header = {RifEclipseOutputTableColumn("Well"),
                  RifEclipseOutputTableColumn("I"),
                  RifEclipseOutputTableColumn("J"),
                  RifEclipseOutputTableColumn("K1"),
                  RifEclipseOutputTableColumn("K2"),
                  RifEclipseOutputTableColumn("Status"),
                  RifEclipseOutputTableColumn("SAT"),
                  RifEclipseOutputTableColumn(
                      "TR", RifEclipseOutputTableDoubleFormatting(RifEclipseOutputTableDoubleFormat::RIF_SCIENTIFIC)),
                  RifEclipseOutputTableColumn("DIAM"),
                  RifEclipseOutputTableColumn("KH"),
                  RifEclipseOutputTableColumn("S"),
                  RifEclipseOutputTableColumn("Df"),
                  RifEclipseOutputTableColumn("DIR"),
                  RifEclipseOutputTableColumn("r0")};

        formatter.keyword("COMPDAT");
    }
    else
    {
        header = {RifEclipseOutputTableColumn("Well"),
                  RifEclipseOutputTableColumn("LgrName"),
                  RifEclipseOutputTableColumn("I"),
                  RifEclipseOutputTableColumn("J"),
                  RifEclipseOutputTableColumn("K1"),
                  RifEclipseOutputTableColumn("K2"),
                  RifEclipseOutputTableColumn("Status"),
                  RifEclipseOutputTableColumn("SAT"),
                  RifEclipseOutputTableColumn(
                      "TR", RifEclipseOutputTableDoubleFormatting(RifEclipseOutputTableDoubleFormat::RIF_SCIENTIFIC)),
                  RifEclipseOutputTableColumn("DIAM"),
                  RifEclipseOutputTableColumn("KH"),
                  RifEclipseOutputTableColumn("S"),
                  RifEclipseOutputTableColumn("Df"),
                  RifEclipseOutputTableColumn("DIR"),
                  RifEclipseOutputTableColumn("r0")};

        formatter.keyword("COMPDATL");
    }
    formatter.header(header);

    for (const RigCompletionData& data : completionData)
    {
        if (data.transmissibility() == 0.0 || data.wpimult() == 0.0)
        {
            // Don't export completions without transmissibility
            continue;
        }

        for (const RigCompletionMetaData& metadata : data.metadata())
        {
            formatter.comment(QString("%1 : %2").arg(metadata.name).arg(metadata.comment));
        }
        formatter.add(data.wellName());

        if (!gridName.isEmpty())
        {
            formatter.add(gridName);
        }

        formatter.addZeroBasedCellIndex(data.completionDataGridCell().localCellIndexI())
            .addZeroBasedCellIndex(data.completionDataGridCell().localCellIndexJ())
            .addZeroBasedCellIndex(data.completionDataGridCell().localCellIndexK())
            .addZeroBasedCellIndex(data.completionDataGridCell().localCellIndexK());
        switch (data.connectionState())
        {
            case OPEN: formatter.add("OPEN"); break;
            case SHUT: formatter.add("SHUT"); break;
            case AUTO: formatter.add("AUTO"); break;
        }

        if (RigCompletionData::isDefaultValue(data.saturation()))
            formatter.add("1*");
        else
            formatter.add(data.saturation());

        if (data.isNonDarcyFlow() || RigCompletionData::isDefaultValue(data.transmissibility()))
        {
            if (RigCompletionData::isDefaultValue(data.transmissibility()))
                formatter.add("1*");
            else
                formatter.add(data.transmissibility());

            if (RigCompletionData::isDefaultValue(data.diameter()))
                formatter.add("1*");
            else
                formatter.add(data.diameter());
            if (RigCompletionData::isDefaultValue(data.kh()))
                formatter.add("1*");
            else
                formatter.add(data.kh());
            if (RigCompletionData::isDefaultValue(data.skinFactor()))
                formatter.add("1*");
            else
                formatter.add(data.skinFactor());
            if (RigCompletionData::isDefaultValue(data.dFactor()))
                formatter.add("1*");
            else
                formatter.add(-data.dFactor());

            switch (data.direction())
            {
                case DIR_I: formatter.add("'X'"); break;
                case DIR_J: formatter.add("'Y'"); break;
                case DIR_K: formatter.add("'Z'"); break;
                default: formatter.add("'Z'"); break;
            }
        }
        else
        {
            formatter.add(data.transmissibility());
        }

        formatter.rowCompleted();
    }
    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::generateWpimultTable(RifEclipseDataTableFormatter&         formatter,
                                                                      const QString&                        gridName,
                                                                      const std::vector<RigCompletionData>& completionData)
{
    std::vector<RifEclipseOutputTableColumn> header;

    if (gridName.isEmpty())
    {
        header = {
            RifEclipseOutputTableColumn("Well"),
            RifEclipseOutputTableColumn("Mult"),
            RifEclipseOutputTableColumn("I"),
            RifEclipseOutputTableColumn("J"),
            RifEclipseOutputTableColumn("K"),
        };
        formatter.keyword("WPIMULT");
    }
    else
    {
        header = {
            RifEclipseOutputTableColumn("Well"),
            RifEclipseOutputTableColumn("LgrName"),
            RifEclipseOutputTableColumn("Mult"),
            RifEclipseOutputTableColumn("I"),
            RifEclipseOutputTableColumn("J"),
            RifEclipseOutputTableColumn("K"),
        };
        formatter.keyword("WPIMULTL");
    }
    formatter.header(header);

    for (auto& completion : completionData)
    {
        if (completion.wpimult() == 0.0 || completion.isDefaultValue(completion.wpimult()))
        {
            continue;
        }

        formatter.add(completion.wellName());
        formatter.add(completion.wpimult());

        if (!gridName.isEmpty())
        {
            formatter.add(gridName);
        }

        formatter.addZeroBasedCellIndex(completion.completionDataGridCell().localCellIndexI())
            .addZeroBasedCellIndex(completion.completionDataGridCell().localCellIndexJ())
            .addZeroBasedCellIndex(completion.completionDataGridCell().localCellIndexK());
        formatter.rowCompleted();
    }

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicWellPathExportCompletionDataFeatureImpl::generatePerforationsCompdatValues(
    const RimWellPath*                       wellPath,
    const RicExportCompletionDataSettingsUi& settings)
{
    RiaEclipseUnitTools::UnitSystem unitSystem = settings.caseToApply->eclipseCaseData()->unitsType();

    std::vector<RigCompletionData> completionData;
    const RigActiveCellInfo* activeCellInfo = settings.caseToApply->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);

    if (wellPath->perforationIntervalCollection()->isChecked())
    {
        for (const RimPerforationInterval* interval : wellPath->perforationIntervalCollection()->perforations())
        {
            if (!interval->isChecked()) continue;
            if (!interval->isActiveOnDate(settings.caseToApply->timeStepDates()[settings.timeStep])) continue;

            using namespace std;
            pair<vector<cvf::Vec3d>, vector<double>> perforationPointsAndMD =
                wellPath->wellPathGeometry()->clippedPointSubset(interval->startMD(), interval->endMD());

            std::vector<WellPathCellIntersectionInfo> intersectedCells =
                RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath(
                    settings.caseToApply->eclipseCaseData(), perforationPointsAndMD.first, perforationPointsAndMD.second);

            for (auto& cell : intersectedCells)
            {
                bool cellIsActive = activeCellInfo->isActive(cell.globCellIndex);
                if (!cellIsActive) continue;

                RigCompletionData completion(wellPath->completions()->wellNameForExport(),
                                             RigCompletionDataGridCell(cell.globCellIndex, settings.caseToApply->mainGrid()),
                                             cell.startMD);

                CellDirection direction =
                    calculateDirectionInCell(settings.caseToApply, cell.globCellIndex, cell.intersectionLengthsInCellCS);

                double transmissibility =
                    RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibility(settings.caseToApply,
                                                                                          wellPath,
                                                                                          cell.intersectionLengthsInCellCS,
                                                                                          interval->skinFactor(),
                                                                                          interval->diameter(unitSystem) / 2,
                                                                                          cell.globCellIndex,
                                                                                          settings.useLateralNTG);

                completion.setTransAndWPImultBackgroundDataFromPerforation(
                    transmissibility, interval->skinFactor(), interval->diameter(unitSystem), direction);
                completion.addMetadata("Perforation",
                                       QString("StartMD: %1 - EndMD: %2").arg(interval->startMD()).arg(interval->endMD()) +
                                           QString(" : ") + QString::number(transmissibility));
                completionData.push_back(completion);
            }
        }
    }

    return completionData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellPathExportCompletionDataFeatureImpl::wellSegmentLocationOrdering(const WellSegmentLocation& first,
                                                                             const WellSegmentLocation& second)
{
    return first.measuredDepth < second.measuredDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellSegmentLocation>
    RicWellPathExportCompletionDataFeatureImpl::findWellSegmentLocations(const RimEclipseCase* caseToApply,
                                                                         const RimWellPath*    wellPath)
{
    std::vector<RimFishbonesMultipleSubs*> fishbonesSubs;

    if (wellPath->fishbonesCollection()->isChecked())
    {
        for (RimFishbonesMultipleSubs* subs : wellPath->fishbonesCollection()->fishbonesSubs())
        {
            if (subs->isActive())
            {
                fishbonesSubs.push_back(subs);
            }
        }
    }

    return findWellSegmentLocations(caseToApply, wellPath, fishbonesSubs);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellSegmentLocation> RicWellPathExportCompletionDataFeatureImpl::findWellSegmentLocations(
    const RimEclipseCase*                         caseToApply,
    const RimWellPath*                            wellPath,
    const std::vector<RimFishbonesMultipleSubs*>& fishbonesSubs)
{
    std::vector<WellSegmentLocation> wellSegmentLocations;
    for (RimFishbonesMultipleSubs* subs : fishbonesSubs)
    {
        for (auto& sub : subs->installedLateralIndices())
        {
            double              measuredDepth = subs->measuredDepth(sub.subIndex);
            cvf::Vec3d          position      = wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(measuredDepth);
            WellSegmentLocation location      = WellSegmentLocation(subs, measuredDepth, -position.z(), sub.subIndex);

            for (size_t lateralIndex : sub.lateralIndices)
            {
                location.laterals.push_back(WellSegmentLateral(lateralIndex));
            }
            wellSegmentLocations.push_back(location);
        }
    }
    std::sort(wellSegmentLocations.begin(), wellSegmentLocations.end(), wellSegmentLocationOrdering);

    assignLateralIntersectionsAndBranchAndSegmentNumbers(caseToApply, &wellSegmentLocations);

    return wellSegmentLocations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::assignLateralIntersections(const RimEclipseCase* caseToApply,
                                                                            WellSegmentLocation*  location,
                                                                            int*                  branchNum,
                                                                            int*                  segmentNum)
{
    for (WellSegmentLateral& lateral : location->laterals)
    {
        ++(*branchNum);
        lateral.branchNumber = (*branchNum);

        std::vector<std::pair<cvf::Vec3d, double>> lateralCoordMDPairs =
            location->fishbonesSubs->coordsAndMDForLateral(location->subIndex, lateral.lateralIndex);

        if (lateralCoordMDPairs.empty())
        {
            continue;
        }

        std::vector<cvf::Vec3d> lateralCoords;
        std::vector<double>     lateralMDs;

        lateralCoords.reserve(lateralCoordMDPairs.size());
        lateralMDs.reserve(lateralCoordMDPairs.size());

        for (auto& coordMD : lateralCoordMDPairs)
        {
            lateralCoords.push_back(coordMD.first);
            lateralMDs.push_back(coordMD.second);
        }

        std::vector<WellPathCellIntersectionInfo> intersections =
            RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath(
                caseToApply->eclipseCaseData(), lateralCoords, lateralMDs);
        double previousExitMD  = lateralMDs.front();
        double previousExitTVD = lateralCoords.front().z();

        int attachedSegmentNumber = location->icdSegmentNumber;
        for (const auto& cellIntInfo : intersections)
        {
            ++(*segmentNum);
            WellSegmentLateralIntersection lateralIntersection((*segmentNum),
                                                               attachedSegmentNumber,
                                                               cellIntInfo.globCellIndex,
                                                               cellIntInfo.endMD - previousExitMD,
                                                               cellIntInfo.endPoint.z() - previousExitTVD,
                                                               cellIntInfo.intersectionLengthsInCellCS);

            lateral.intersections.push_back(lateralIntersection);

            attachedSegmentNumber = (*segmentNum);
            previousExitMD        = cellIntInfo.endMD;
            previousExitTVD       = cellIntInfo.endPoint.z();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::assignLateralIntersectionsAndBranchAndSegmentNumbers(
    const RimEclipseCase*             caseToApply,
    std::vector<WellSegmentLocation>* locations)
{
    int segmentNumber = 1;
    int branchNumber  = 1;

    // First loop over the locations so that each segment on the main stem is an incremental number
    for (WellSegmentLocation& location : *locations)
    {
        location.segmentNumber    = ++segmentNumber;
        location.icdBranchNumber  = ++branchNumber;
        location.icdSegmentNumber = ++segmentNumber;
    }

    // Then assign branch and segment numbers to each lateral parts
    for (WellSegmentLocation& location : *locations)
    {
        assignLateralIntersections(caseToApply, &location, &branchNumber, &segmentNumber);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::appendCompletionData(
    std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>>* completionData,
    const std::vector<RigCompletionData>&                                data)
{
    for (auto& completion : data)
    {
        auto it = completionData->find(completion.completionDataGridCell());
        if (it != completionData->end())
        {
            it->second.push_back(completion);
        }
        else
        {
            completionData->insert(std::pair<RigCompletionDataGridCell, std::vector<RigCompletionData>>(
                completion.completionDataGridCell(), std::vector<RigCompletionData>{completion}));
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CellDirection RicWellPathExportCompletionDataFeatureImpl::calculateDirectionInCell(RimEclipseCase*   eclipseCase,
                                                                                   size_t            globalCellIndex,
                                                                                   const cvf::Vec3d& lengthsInCell)
{
    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DX");
    cvf::ref<RigResultAccessor> dxAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "DX");
    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DY");
    cvf::ref<RigResultAccessor> dyAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "DY");
    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DZ");
    cvf::ref<RigResultAccessor> dzAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "DZ");

    double xLengthFraction = fabs(lengthsInCell.x() / dxAccessObject->cellScalarGlobIdx(globalCellIndex));
    double yLengthFraction = fabs(lengthsInCell.y() / dyAccessObject->cellScalarGlobIdx(globalCellIndex));
    double zLengthFraction = fabs(lengthsInCell.z() / dzAccessObject->cellScalarGlobIdx(globalCellIndex));

    if (xLengthFraction > yLengthFraction && xLengthFraction > zLengthFraction)
    {
        return CellDirection::DIR_I;
    }
    else if (yLengthFraction > xLengthFraction && yLengthFraction > zLengthFraction)
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
double RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibility(RimEclipseCase*    eclipseCase,
                                                                             const RimWellPath* wellPath,
                                                                             const cvf::Vec3d&  internalCellLengths,
                                                                             double             skinFactor,
                                                                             double             wellRadius,
                                                                             size_t             globalCellIndex,
                                                                             bool               useLateralNTG,
                                                                             size_t             volumeScaleConstant,
                                                                             CellDirection      directionForVolumeScaling)
{
    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DX");
    cvf::ref<RigResultAccessor> dxAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "DX");
    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DY");
    cvf::ref<RigResultAccessor> dyAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "DY");
    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DZ");
    cvf::ref<RigResultAccessor> dzAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "DZ");

    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PERMX");
    cvf::ref<RigResultAccessor> permxAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "PERMX");
    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PERMY");
    cvf::ref<RigResultAccessor> permyAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "PERMY");
    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PERMZ");
    cvf::ref<RigResultAccessor> permzAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "PERMZ");

    double ntg       = 1.0;
    size_t ntgResIdx = eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "NTG");
    if (ntgResIdx != cvf::UNDEFINED_SIZE_T)
    {
        cvf::ref<RigResultAccessor> ntgAccessObject =
            RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "NTG");
        ntg = ntgAccessObject->cellScalarGlobIdx(globalCellIndex);
    }
    double latNtg = useLateralNTG ? ntg : 1.0;

    double dx    = dxAccessObject->cellScalarGlobIdx(globalCellIndex);
    double dy    = dyAccessObject->cellScalarGlobIdx(globalCellIndex);
    double dz    = dzAccessObject->cellScalarGlobIdx(globalCellIndex);
    double permx = permxAccessObject->cellScalarGlobIdx(globalCellIndex);
    double permy = permyAccessObject->cellScalarGlobIdx(globalCellIndex);
    double permz = permzAccessObject->cellScalarGlobIdx(globalCellIndex);

    double darcy = RiaEclipseUnitTools::darcysConstant(wellPath->unitSystem());

    if (volumeScaleConstant != 1)
    {
        if (directionForVolumeScaling == CellDirection::DIR_I) dx = dx / volumeScaleConstant;
        if (directionForVolumeScaling == CellDirection::DIR_J) dy = dy / volumeScaleConstant;
        if (directionForVolumeScaling == CellDirection::DIR_K) dz = dz / volumeScaleConstant;
    }

    double transx = RigTransmissibilityEquations::wellBoreTransmissibilityComponent(
        internalCellLengths.x() * latNtg, permy, permz, dy, dz, wellRadius, skinFactor, darcy);
    double transy = RigTransmissibilityEquations::wellBoreTransmissibilityComponent(
        internalCellLengths.y() * latNtg, permx, permz, dx, dz, wellRadius, skinFactor, darcy);
    double transz = RigTransmissibilityEquations::wellBoreTransmissibilityComponent(
        internalCellLengths.z() * ntg, permy, permx, dy, dx, wellRadius, skinFactor, darcy);

    return RigTransmissibilityEquations::totalConnectionFactor(transx, transy, transz);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibilityAsEclipseDoes(RimEclipseCase* eclipseCase,
                                                                                          double          skinFactor,
                                                                                          double          wellRadius,
                                                                                          size_t          globalCellIndex,
                                                                                          CellDirection   direction)
{
    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DX");
    cvf::ref<RigResultAccessor> dxAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "DX");
    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DY");
    cvf::ref<RigResultAccessor> dyAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "DY");
    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DZ");
    cvf::ref<RigResultAccessor> dzAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "DZ");

    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PERMX");
    cvf::ref<RigResultAccessor> permxAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "PERMX");
    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PERMY");
    cvf::ref<RigResultAccessor> permyAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "PERMY");
    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PERMZ");
    cvf::ref<RigResultAccessor> permzAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "PERMZ");

    double ntg       = 1.0;
    size_t ntgResIdx = eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "NTG");
    if (ntgResIdx != cvf::UNDEFINED_SIZE_T)
    {
        cvf::ref<RigResultAccessor> ntgAccessObject =
            RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "NTG");
        ntg = ntgAccessObject->cellScalarGlobIdx(globalCellIndex);
    }

    double dx    = dxAccessObject->cellScalarGlobIdx(globalCellIndex);
    double dy    = dyAccessObject->cellScalarGlobIdx(globalCellIndex);
    double dz    = dzAccessObject->cellScalarGlobIdx(globalCellIndex);
    double permx = permxAccessObject->cellScalarGlobIdx(globalCellIndex);
    double permy = permyAccessObject->cellScalarGlobIdx(globalCellIndex);
    double permz = permzAccessObject->cellScalarGlobIdx(globalCellIndex);

    RiaEclipseUnitTools::UnitSystem units = eclipseCaseData->unitsType();
    double                          darcy = RiaEclipseUnitTools::darcysConstant(units);

    double trans = cvf::UNDEFINED_DOUBLE;
    if (direction == CellDirection::DIR_I)
    {
        trans = RigTransmissibilityEquations::wellBoreTransmissibilityComponent(
            dx, permy, permz, dy, dz, wellRadius, skinFactor, darcy);
    }
    else if (direction == CellDirection::DIR_J)
    {
        trans = RigTransmissibilityEquations::wellBoreTransmissibilityComponent(
            dy, permx, permz, dx, dz, wellRadius, skinFactor, darcy);
    }
    else if (direction == CellDirection::DIR_K)
    {
        trans = RigTransmissibilityEquations::wellBoreTransmissibilityComponent(
            dz * ntg, permy, permx, dy, dx, wellRadius, skinFactor, darcy);
    }

    return trans;
}

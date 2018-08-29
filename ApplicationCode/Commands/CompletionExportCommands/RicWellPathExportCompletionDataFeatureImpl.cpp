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
#include "RiaFilePathTools.h"

#include "RicExportCompletionDataSettingsUi.h"
#include "RicExportFeatureImpl.h"
#include "RicExportFractureCompletionsImpl.h"
#include "RicFishbonesTransmissibilityCalculationFeatureImp.h"
#include "RicWellPathFractureReportItem.h"

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
 
#include "RimFileWellPath.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimFractureTemplate.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimSimWellInView.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#include "RimProject.h"

#include "RiuMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"
#include "cafSelectionManager.h"

#include "cvfPlane.h"

#include "RicWellPathFractureTextReportFeatureImpl.h"
#include "RigVirtualPerforationTransmissibilities.h"
#include <QDir>

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
const RimWellPath* findWellPathFromExportName(const QString& wellNameForExport);

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class OpenFileException
{
public:
    OpenFileException(const QString &message) : message(message) {}
    QString  message;
};

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
    for (RimWellPath* wellPath : wellPaths)
    {
        if (wellPath->unitSystem() == exportSettings.caseToApply->eclipseCaseData()->unitsType())
        {
            usedWellPaths.push_back(wellPath);
        }
        else
        {
            int     caseId = exportSettings.caseToApply->caseId();
            QString format = QString("Unit systems for well path \"%1\" must match unit system of chosen eclipse case \"%2\"");
            QString errMsg = format.arg(wellPath->name()).arg(caseId);
            RiaLogging::error(errMsg);
        }
    }

    std::vector<RicWellPathFractureReportItem> fractureDataReportItems;

    // FractureTransmissibilityExportInformation
    std::unique_ptr<QTextStream> fractureTransmissibilityExportInformationStream = nullptr;
    QFile                        fractureTransmissibilityExportInformationFile;

    RiaPreferences* prefs = RiaApplication::instance()->preferences();
    if (prefs->includeFractureDebugInfoFile())
    {
        QDir outputDir = QDir(exportSettings.folder);
        outputDir.mkpath(".");

        QString fractureTransmisibillityExportInformationPath =
            QDir(exportSettings.folder).absoluteFilePath("FractureTransmissibilityExportInformation");

        fractureTransmissibilityExportInformationFile.setFileName(fractureTransmisibillityExportInformationPath);
        if (!fractureTransmissibilityExportInformationFile.open(QIODevice::WriteOnly))
        {
            RiaLogging::error(QString("Export Completions Data: Could not open the file: %1")
                                  .arg(fractureTransmisibillityExportInformationPath));
        }
        else
        {
            fractureTransmissibilityExportInformationStream =
                std::unique_ptr<QTextStream>(new QTextStream(&fractureTransmissibilityExportInformationFile));
        }
    }

    size_t maxProgress = usedWellPaths.size() * 3 + simWells.size() +
                         (exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL
                              ? usedWellPaths.size()
                              : exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL_AND_COMPLETION_TYPE
                                    ? usedWellPaths.size() * 3
                                    : 1) +
                         simWells.size();

    caf::ProgressInfo progress(maxProgress, "Export Completions");

    progress.setProgressDescription("Read Completion Data");

    std::vector<RigCompletionData> completions;

    for (auto wellPath : usedWellPaths)
    {
        std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>> completionsPerEclipseCellAllCompletionTypes;
        std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>> completionsPerEclipseCellFishbones;
        std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>> completionsPerEclipseCellFracture;
        std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>> completionsPerEclipseCellPerforations;

        // Generate completion data

        if (exportSettings.includePerforations)
        {
            std::vector<RigCompletionData> perforationCompletionData =
                generatePerforationsCompdatValues(wellPath, exportSettings);

            appendCompletionData(&completionsPerEclipseCellAllCompletionTypes, perforationCompletionData);
            appendCompletionData(&completionsPerEclipseCellPerforations, perforationCompletionData);
        }
        progress.incrementProgress();

        if (exportSettings.includeFishbones)
        {
            std::vector<RigCompletionData> fishbonesCompletionData =
                RicFishbonesTransmissibilityCalculationFeatureImp::generateFishboneCompdatValuesUsingAdjustedCellVolume(
                    wellPath, exportSettings);

            appendCompletionData(&completionsPerEclipseCellAllCompletionTypes, fishbonesCompletionData);
            appendCompletionData(&completionsPerEclipseCellFishbones, fishbonesCompletionData);
        }
        progress.incrementProgress();

        if (exportSettings.includeFractures())
        {
            std::vector<RicWellPathFractureReportItem>* reportItems = nullptr;
            if (exportSettings.includeFracturesSummaryHeader())
            {
                reportItems = &fractureDataReportItems;
            }

            std::vector<RigCompletionData> fractureCompletionData =
                RicExportFractureCompletionsImpl::generateCompdatValuesForWellPath(
                    wellPath, exportSettings.caseToApply(), reportItems, fractureTransmissibilityExportInformationStream.get());

            appendCompletionData(&completionsPerEclipseCellAllCompletionTypes, fractureCompletionData);
            appendCompletionData(&completionsPerEclipseCellFracture, fractureCompletionData);
        }

        if (exportSettings.reportCompletionsTypesIndividually())
        {
            for (auto& data : completionsPerEclipseCellFracture)
            {
                completions.push_back(combineEclipseCellCompletions(data.second, exportSettings));
            }

            for (auto& data : completionsPerEclipseCellFishbones)
            {
                completions.push_back(combineEclipseCellCompletions(data.second, exportSettings));
            }

            for (auto& data : completionsPerEclipseCellPerforations)
            {
                completions.push_back(combineEclipseCellCompletions(data.second, exportSettings));
            }
        }
        else
        {
            for (auto& data : completionsPerEclipseCellAllCompletionTypes)
            {
                completions.push_back(combineEclipseCellCompletions(data.second, exportSettings));
            }
        }

        progress.incrementProgress();
    }

    for (auto simWell : simWells)
    {
        std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>> completionsPerEclipseCell;

        std::vector<RigCompletionData> fractureCompletionData = RicExportFractureCompletionsImpl::generateCompdatValuesForSimWell(
            exportSettings.caseToApply(), simWell, fractureTransmissibilityExportInformationStream.get());
        appendCompletionData(&completionsPerEclipseCell, fractureCompletionData);

        for (auto& data : completionsPerEclipseCell)
        {
            completions.push_back(combineEclipseCellCompletions(data.second, exportSettings));
        }

        progress.incrementProgress();
    }

    const QString eclipseCaseName = exportSettings.caseToApply->caseUserDescription();

    progress.setProgressDescription("Write Export Files");
    if (exportSettings.fileSplit == RicExportCompletionDataSettingsUi::UNIFIED_FILE)
    {
        const QString fileName = QString("UnifiedCompletions_%1").arg(eclipseCaseName);
        sortAndExportCompletionsToFile(exportSettings.caseToApply,
                                       exportSettings.folder,
                                       fileName,
                                       completions,
                                       fractureDataReportItems,
                                       exportSettings.compdatExport);
        progress.incrementProgress();
    }
    else if (exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL)
    {
        for (auto wellPath : usedWellPaths)
        {
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
            sortAndExportCompletionsToFile(exportSettings.caseToApply,
                                           exportSettings.folder,
                                           fileName,
                                           wellCompletions,
                                           fractureDataReportItems,
                                           exportSettings.compdatExport);
            progress.incrementProgress();
        }
    }
    else if (exportSettings.fileSplit == RicExportCompletionDataSettingsUi::SPLIT_ON_WELL_AND_COMPLETION_TYPE)
    {
        std::vector<RigCompletionData::CompletionType> completionTypes;
        completionTypes.push_back(RigCompletionData::FISHBONES);
        completionTypes.push_back(RigCompletionData::FRACTURE);
        completionTypes.push_back(RigCompletionData::PERFORATION);

        for (const auto& completionType : completionTypes)
        {
            for (auto wellPath : usedWellPaths)
            {
                std::vector<RigCompletionData> wellCompletions;
                for (const auto& completion : completions)
                {
                    if (completion.wellName() == wellPath->completions()->wellNameForExport() &&
                        completionType == completion.completionType())
                    {
                        wellCompletions.push_back(completion);
                    }
                }

                if (wellCompletions.empty()) continue;

                {
                    QString completionTypeText;
                    if (completionType == RigCompletionData::FISHBONES) completionTypeText = "Fishbones";
                    if (completionType == RigCompletionData::FRACTURE) completionTypeText = "Fracture";
                    if (completionType == RigCompletionData::PERFORATION) completionTypeText = "Perforation";

                    QString fileName = QString("%1_%2_%3").arg(wellPath->name()).arg(completionTypeText).arg(eclipseCaseName);
                    sortAndExportCompletionsToFile(exportSettings.caseToApply,
                                                   exportSettings.folder,
                                                   fileName,
                                                   wellCompletions,
                                                   fractureDataReportItems,
                                                   exportSettings.compdatExport);
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
            std::vector<RigCompletionData> wellCompletions;
            for (const auto& completion : completions)
            {
                if (completion.wellName() == simWell->name())
                {
                    wellCompletions.push_back(completion);
                }
            }

            if (wellCompletions.empty()) continue;

            QString fileName = QString("%1_Fractures_%2").arg(simWell->name()).arg(eclipseCaseName);
            sortAndExportCompletionsToFile(exportSettings.caseToApply,
                                           exportSettings.folder,
                                           fileName,
                                           wellCompletions,
                                           fractureDataReportItems,
                                           exportSettings.compdatExport);

            progress.incrementProgress();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicWellPathExportCompletionDataFeatureImpl::computeStaticCompletionsForWellPath(RimWellPath*    wellPath,
                                                                                    RimEclipseCase* eclipseCase)
{
    std::vector<RigCompletionData> completionsPerEclipseCell;

    if (eclipseCase && eclipseCase->eclipseCaseData())
    {
        RicExportCompletionDataSettingsUi exportSettings;
        exportSettings.caseToApply         = eclipseCase;
        exportSettings.timeStep            = 0;
        exportSettings.includeFishbones    = true;
        exportSettings.includePerforations = true;
        exportSettings.includeFractures    = true;

        {
            std::vector<RigCompletionData> completionData =
                RicFishbonesTransmissibilityCalculationFeatureImp::generateFishboneCompdatValuesUsingAdjustedCellVolume(
                    wellPath, exportSettings);

            std::copy(completionData.begin(), completionData.end(), std::back_inserter(completionsPerEclipseCell));
        }

        {
            std::vector<RigCompletionData> completionData =
                RicExportFractureCompletionsImpl::generateCompdatValuesForWellPath(wellPath, eclipseCase, nullptr, nullptr);

            std::copy(completionData.begin(), completionData.end(), std::back_inserter(completionsPerEclipseCell));
        }
    }

    return completionsPerEclipseCell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicWellPathExportCompletionDataFeatureImpl::computeDynamicCompletionsForWellPath(RimWellPath*    wellPath,
                                                                                     RimEclipseCase* eclipseCase,
                                                                                     size_t          timeStepIndex)
{
    std::vector<RigCompletionData> completionsPerEclipseCell;

    if (eclipseCase && eclipseCase->eclipseCaseData())
    {
        RicExportCompletionDataSettingsUi exportSettings;
        exportSettings.caseToApply         = eclipseCase;
        exportSettings.timeStep            = static_cast<int>(timeStepIndex);
        exportSettings.includeFishbones    = true;
        exportSettings.includePerforations = true;
        exportSettings.includeFractures    = true;

        completionsPerEclipseCell = generatePerforationsCompdatValues(wellPath, exportSettings);
    }

    return completionsPerEclipseCell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::generateWelsegsTable(RifEclipseDataTableFormatter& formatter,
                                                                      const RicMswExportInfo&       exportInfo)
{
    formatter.keyword("WELSEGS");

    double startMD  = exportInfo.initialMD();
    double startTVD = exportInfo.initialTVD();

    {
        std::vector<RifEclipseOutputTableColumn> header = {
            RifEclipseOutputTableColumn("Name"),
            RifEclipseOutputTableColumn("Dep 1"),
            RifEclipseOutputTableColumn("Tlen 1"),
            RifEclipseOutputTableColumn("Vol 1"),
            RifEclipseOutputTableColumn("Len&Dep"),
            RifEclipseOutputTableColumn("PresDrop"),
        };
        formatter.header(header);

        formatter.add(exportInfo.wellPath()->name());
        formatter.add(startTVD);
        formatter.add(startMD);
        formatter.addValueOrDefaultMarker(exportInfo.topWellBoreVolume(), RicMswExportInfo::defaultDoubleValue());
        formatter.add(exportInfo.lengthAndDepthText());
        formatter.add(exportInfo.pressureDropText());

        formatter.rowCompleted();
    }

    {
        std::vector<RifEclipseOutputTableColumn> header = {
            RifEclipseOutputTableColumn("First Seg"),
            RifEclipseOutputTableColumn("Last Seg"),
            RifEclipseOutputTableColumn("Branch Num"),
            RifEclipseOutputTableColumn("Outlet Seg"),
            RifEclipseOutputTableColumn("Length"),
            RifEclipseOutputTableColumn("Depth Change"),
            RifEclipseOutputTableColumn("Diam"),
            RifEclipseOutputTableColumn("Rough"),
        };
        formatter.header(header);
    }

    {
        double prevMD  = exportInfo.initialMD();
        double prevTVD = exportInfo.initialTVD();
        for (const RicMswSegment& location : exportInfo.wellSegmentLocations())
        {
            double depth  = 0;
            double length = 0;

            if (exportInfo.lengthAndDepthText() == QString("INC"))
            {
                depth  = location.endTVD() - prevTVD;
                length = location.endMD() - prevMD;
            }
            else
            {
                depth  = location.endTVD();
                length = location.endMD();
            }
            QString comment = location.label();
            if (location.subIndex() != cvf::UNDEFINED_SIZE_T)
            {
                comment += QString(", sub %1").arg(location.subIndex());
            }
            formatter.comment(comment);
            formatter.add(location.segmentNumber()).add(location.segmentNumber());
            formatter.add(1); // All segments on main stem are branch 1
            formatter.add(location.segmentNumber() - 1); // All main stem segments are connected to the segment below them
            formatter.add(length);
            formatter.add(depth);
            formatter.add(exportInfo.linerDiameter());
            formatter.add(exportInfo.roughnessFactor());
            formatter.rowCompleted();
            prevMD  = location.endMD();
            prevTVD = location.endTVD();
        }
    }

    {
        generateWelsegsSegments(formatter, exportInfo, {RigCompletionData::ICD, RigCompletionData::FISHBONES});
        generateWelsegsSegments(formatter, exportInfo, {RigCompletionData::FRACTURE});
    }

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::generateWelsegsSegments(
    RifEclipseDataTableFormatter&                      formatter,
    const RicMswExportInfo&                            exportInfo,
    const std::set<RigCompletionData::CompletionType>& exportCompletionTypes)
{
    bool generatedHeader = false;
    for (const RicMswSegment& segment : exportInfo.wellSegmentLocations())
    {
        for (const RicMswCompletion& completion : segment.completions())
        {
            if (exportCompletionTypes.count(completion.completionType()))
            {
                if (!generatedHeader)
                {
                    generateWelsegsCompletionCommentHeader(formatter, completion.completionType());
                    generatedHeader = true;
                }

                if (completion.completionType() == RigCompletionData::ICD) // Found ICD
                {
                    formatter.comment(completion.label());
                    formatter.add(completion.subSegments().front().segmentNumber());
                    formatter.add(completion.subSegments().front().segmentNumber());
                    formatter.add(completion.branchNumber());
                    formatter.add(segment.segmentNumber());
                    formatter.add(0.1); // ICDs have 0.1 length
                    formatter.add(0); // Depth change
                    formatter.add(exportInfo.linerDiameter());
                    formatter.add(exportInfo.roughnessFactor());
                    formatter.rowCompleted();
                }
                else
                {
                    if (completion.completionType() == RigCompletionData::FISHBONES)
                    {
                        formatter.comment(QString("%1 : Sub index %2 - %3")
                                              .arg(segment.label())
                                              .arg(segment.subIndex())
                                              .arg(completion.label()));
                    }
                    else if (completion.completionType() == RigCompletionData::FRACTURE)
                    {
                        formatter.comment(QString("%1 connected to %2").arg(completion.label()).arg(segment.label()));
                    }

                    for (const RicMswSubSegment& subSegment : completion.subSegments())
                    {
                        double depth  = 0;
                        double length = 0;

                        if (exportInfo.lengthAndDepthText() == QString("INC"))
                        {
                            depth  = subSegment.deltaTVD();
                            length = subSegment.deltaMD();
                        }
                        else
                        {
                            depth  = subSegment.startTVD() + subSegment.deltaTVD();
                            length = subSegment.startMD() + subSegment.deltaMD();
                        }
                        double diameter = segment.effectiveDiameter();
                        formatter.add(subSegment.segmentNumber());
                        formatter.add(subSegment.segmentNumber());
                        formatter.add(completion.branchNumber());
                        formatter.add(subSegment.attachedSegmentNumber());
                        formatter.add(length);
                        formatter.add(depth);
                        formatter.add(diameter);
                        formatter.add(segment.openHoleRoughnessFactor());
                        formatter.rowCompleted();
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::generateWelsegsCompletionCommentHeader(
    RifEclipseDataTableFormatter&     formatter,
    RigCompletionData::CompletionType completionType)
{
    if (completionType == RigCompletionData::CT_UNDEFINED)
    {
        formatter.comment("Main stem");
    }
    else if (completionType == RigCompletionData::ICD)
    {
        formatter.comment("Fishbone Laterals");
        formatter.comment("Diam: MSW - Tubing Radius");
        formatter.comment("Rough: MSW - Open Hole Roughness Factor");
    }
    else if (completionType == RigCompletionData::FRACTURE)
    {
        formatter.comment("Fracture Segments");
        formatter.comment("Diam: MSW - Default Dummy");
        formatter.comment("Rough: MSW - Default Dummy");
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::generateCompsegTables(RifEclipseDataTableFormatter& formatter,
                                                                       const RicMswExportInfo&       exportInfo)
{
    /*
     * TODO: Creating the regular perforation COMPSEGS table should come in here, before the others
     * should take precedence by appearing later in the output. See #3230.
     */

    {
        std::set<RigCompletionData::CompletionType> fishbonesTypes = {RigCompletionData::ICD, RigCompletionData::FISHBONES};
        generateCompsegTable(formatter, exportInfo, false, fishbonesTypes);
        if (exportInfo.hasSubGridIntersections())
        {
            generateCompsegTable(formatter, exportInfo, true, fishbonesTypes);
        }
    }

    {
        std::set<RigCompletionData::CompletionType> fractureTypes = {RigCompletionData::FRACTURE};
        generateCompsegTable(formatter, exportInfo, false, fractureTypes);
        if (exportInfo.hasSubGridIntersections())
        {
            generateCompsegTable(formatter, exportInfo, true, fractureTypes);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::generateCompsegTable(
    RifEclipseDataTableFormatter&                      formatter,
    const RicMswExportInfo&                            exportInfo,
    bool                                               exportSubGridIntersections,
    const std::set<RigCompletionData::CompletionType>& exportCompletionTypes)
{
    bool generatedHeader = false;

    for (const RicMswSegment& location : exportInfo.wellSegmentLocations())
    {
        double startMD = location.startMD();

        for (const RicMswCompletion& completion : location.completions())
        {
            if (exportCompletionTypes.count(completion.completionType()))
            {
                if (!generatedHeader)
                {
                    generateCompsegHeader(formatter, exportInfo, completion.completionType(), exportSubGridIntersections);
                    generatedHeader = true;
                }

                for (const RicMswSubSegment& segment : completion.subSegments())
                {
                    if (completion.completionType() == RigCompletionData::ICD)
                    {
                        startMD = segment.startMD();
                    }

                    for (const RicMswSubSegmentCellIntersection& intersection : segment.intersections())
                    {
                        bool isSubGridIntersection = !intersection.gridName().isEmpty();
                        if (isSubGridIntersection == exportSubGridIntersections)
                        {
                            if (exportSubGridIntersections)
                            {
                                formatter.add(intersection.gridName());
                            }
                            cvf::Vec3st ijk = intersection.gridLocalCellIJK();
                            formatter.addOneBasedCellIndex(ijk.x()).addOneBasedCellIndex(ijk.y()).addOneBasedCellIndex(
                                ijk.z());
                            formatter.add(completion.branchNumber());

                            double startLength = segment.startMD();
                            if (exportInfo.lengthAndDepthText() == QString("INC"))
                            {
                                startLength -= startMD;
                            }
                            formatter.add(startLength);
                            formatter.add(startLength + segment.deltaMD());

                            formatter.rowCompleted();
                        }
                    }
                }
            }
        }
    }

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::generateCompsegHeader(RifEclipseDataTableFormatter&     formatter,
                                                                       const RicMswExportInfo&           exportInfo,
                                                                       RigCompletionData::CompletionType completionType,
                                                                       bool exportSubGridIntersections)
{
    if (exportSubGridIntersections)
    {
        formatter.keyword("COMPSEGL");
    }
    else
    {
        formatter.keyword("COMPSEGS");
    }

    if (completionType == RigCompletionData::ICD)
    {
        formatter.comment("Fishbones");
    }
    else if (completionType == RigCompletionData::FRACTURE)
    {
        formatter.comment("Fractures");
    }

    {
        std::vector<RifEclipseOutputTableColumn> header = {RifEclipseOutputTableColumn("Name")};
        formatter.header(header);
        formatter.add(exportInfo.wellPath()->name());
        formatter.rowCompleted();
    }

    {
        std::vector<RifEclipseOutputTableColumn> allHeaders;
        if (exportSubGridIntersections)
        {
            allHeaders.push_back(RifEclipseOutputTableColumn("Grid"));
        }

        std::vector<RifEclipseOutputTableColumn> commonHeaders = {RifEclipseOutputTableColumn("I"),
                                                                  RifEclipseOutputTableColumn("J"),
                                                                  RifEclipseOutputTableColumn("K"),
                                                                  RifEclipseOutputTableColumn("Branch no"),
                                                                  RifEclipseOutputTableColumn("Start Length"),
                                                                  RifEclipseOutputTableColumn("End Length"),
                                                                  RifEclipseOutputTableColumn("Dir Pen"),
                                                                  RifEclipseOutputTableColumn("End Range"),
                                                                  RifEclipseOutputTableColumn("Connection Depth")};
        allHeaders.insert(allHeaders.end(), commonHeaders.begin(), commonHeaders.end());
        formatter.header(allHeaders);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::generateWsegvalvTable(RifEclipseDataTableFormatter& formatter,
                                                                       const RicMswExportInfo&       exportInfo)
{
    {
        formatter.keyword("WSEGVALV");
        std::vector<RifEclipseOutputTableColumn> header = {
            RifEclipseOutputTableColumn("Well Name"),
            RifEclipseOutputTableColumn("Seg No"),
            RifEclipseOutputTableColumn("Cv"),
            RifEclipseOutputTableColumn("Ac"),
        };
        formatter.header(header);
    }
    for (const RicMswSegment& location : exportInfo.wellSegmentLocations())
    {
        for (const RicMswCompletion& completion : location.completions())
        {
            if (completion.completionType() == RigCompletionData::ICD)
            {
                CVF_ASSERT(completion.subSegments().size() == 1u);
                formatter.add(exportInfo.wellPath()->name());
                formatter.add(completion.subSegments().front().segmentNumber());
                formatter.add(location.icdFlowCoefficient());
                formatter.add(location.icdArea());
                formatter.rowCompleted();
            }
        }
    }
    formatter.tableCompleted();
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
        return firstCompletion;
    }

    // completion type, skin factor, well bore diameter and cell direction are taken from (first) main bore,
    // if no main bore they are taken from first completion
    double        skinfactor       = firstCompletion.skinFactor();
    double        wellBoreDiameter = firstCompletion.diameter();
    CellDirection cellDirection    = firstCompletion.direction();

    for (const RigCompletionData& completion : completions)
    {
        // Use data from the completion with largest diameter
        // This is more robust than checking for main bore flag
        // See also https://github.com/OPM/ResInsight/issues/2765
        if (completion.diameter() > wellBoreDiameter)
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
        resultCompletion.setCombinedValuesExplicitTrans(totalTrans, skinfactor, wellBoreDiameter, cellDirection, completionType);
    }
    else if (settings.compdatExport == RicExportCompletionDataSettingsUi::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS)
    {
        // calculate trans for main bore - but as Eclipse will do it!
        double transmissibilityEclipseCalculation =
            RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibilityAsEclipseDoes(
                settings.caseToApply(), skinfactor, wellBoreDiameter / 2, cellIndexIJK.globalCellIndex(), cellDirection);

        double wpimult = totalTrans / transmissibilityEclipseCalculation;
        resultCompletion.setCombinedValuesImplicitTransWPImult(
            wpimult, skinfactor, wellBoreDiameter, cellDirection, completionType);
    }

    return resultCompletion;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QFilePtr RicWellPathExportCompletionDataFeatureImpl::openFileForExport(const QString& fullFileName)
{
    std::pair<QString, QString> folderAndFileName = RiaFilePathTools::toFolderAndFileName(fullFileName);
    return openFileForExport(folderAndFileName.first, folderAndFileName.second);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QFilePtr RicWellPathExportCompletionDataFeatureImpl::openFileForExport(const QString& folderName, const QString& fileName)
{
    QDir exportFolder = QDir(folderName);
    if (!exportFolder.exists())
    {
        bool createdPath = exportFolder.mkpath(".");
        if (createdPath)
            RiaLogging::info("Created export folder " + folderName);
        else
        {
            auto errorMessage = QString("Selected output folder does not exist, and could not be created.");
            RiaLogging::error(errorMessage);
            throw OpenFileException(errorMessage);
        }
    }

    QString  filePath = exportFolder.filePath(fileName);
    QFilePtr exportFile(new QFile(filePath));
    if (!exportFile->open(QIODevice::WriteOnly))
    {
        auto errorMessage = QString("Export Completions Data: Could not open the file: %1").arg(filePath);
        RiaLogging::error(errorMessage);
        throw OpenFileException(errorMessage);
    }
    return exportFile;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
RicWellPathExportCompletionDataFeatureImpl::mainGridCompletions(std::vector<RigCompletionData>& allCompletions)
{
    std::vector<RigCompletionData> completions;

    for (const auto& completion : allCompletions)
    {
        QString gridName = completion.completionDataGridCell().lgrName();
        if (gridName.isEmpty())
        {
            completions.push_back(completion);
        }
    }
    return completions;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QString, std::vector<RigCompletionData>>
RicWellPathExportCompletionDataFeatureImpl::subGridsCompletions(std::vector<RigCompletionData>& allCompletions)
{
    std::map<QString, std::vector<RigCompletionData>> completions;

    for (const auto& completion : allCompletions)
    {
        QString gridName = completion.completionDataGridCell().lgrName();
        if (!gridName.isEmpty())
        {
            auto it = completions.find(gridName);
            if (it == completions.end())
            {
                completions.insert(
                    std::pair<QString, std::vector<RigCompletionData>>(gridName, {completion}));
            }
            else
            {
                it->second.push_back(completion);
            }
        }
    }
    return completions;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportWellPathFractureReport(RimEclipseCase* sourceCase,
                                                                              QFilePtr exportFile,
                                                                              const std::vector<RicWellPathFractureReportItem>& wellPathFractureReportItems)
{
    QTextStream stream(exportFile.get());

    if (!wellPathFractureReportItems.empty())
    {
        std::vector<RimWellPath*> wellPathsToReport;
        {
            std::set<RimWellPath*> wellPathsSet;

            auto allWellPaths = RicWellPathFractureTextReportFeatureImpl::wellPathsWithFractures();
            for (const auto& wellPath : allWellPaths)
            {
                for (const auto& reportItem : wellPathFractureReportItems)
                {
                    if (reportItem.wellPathName() == wellPath->name())
                    {
                        wellPathsSet.insert(wellPath);
                    }
                }
            }

            std::copy(wellPathsSet.begin(), wellPathsSet.end(), std::back_inserter(wellPathsToReport));

            RicWellPathFractureTextReportFeatureImpl reportGenerator;

            QString summaryText =
                reportGenerator.wellPathFractureReport(sourceCase, wellPathsToReport, wellPathFractureReportItems);

            stream << summaryText;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportWelspecsToFile(RimEclipseCase* gridCase,
                                                                      QFilePtr exportFile,
                                                                      const std::vector<RigCompletionData>& completions)
{
    QTextStream stream(exportFile.get());

    RifEclipseDataTableFormatter formatter(stream);
    formatter.setColumnSpacing(3);

    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn("Well"),
        RifEclipseOutputTableColumn("Grp"),
        RifEclipseOutputTableColumn("I"),
        RifEclipseOutputTableColumn("J"),
        RifEclipseOutputTableColumn("RefDpth"),
        RifEclipseOutputTableColumn("WellType")
    };

    formatter.keyword("WELSPEC");
    formatter.header(header);

    std::set<const RimWellPath*> wellPathSet;

    // Build list of unique RimWellPath
    for (const auto completion : completions)
    {
        const auto wellPath = findWellPathFromExportName(completion.wellName());
        if (wellPath)
        {
            wellPathSet.insert(wellPath);
        }
    }

    // Export
    for (const auto wellPath : wellPathSet)
    {
        auto completions = wellPath->completions();
        cvf::Vec2i ijIntersection = wellPathUpperGridIntersectionIJ(gridCase, wellPath);

        formatter
            .add(completions->wellNameForExport())
            .add(completions->wellGroupName())
            .addOneBasedCellIndex(ijIntersection.x())
            .addOneBasedCellIndex(ijIntersection.y())
            .add(completions->referenceDepth())
            .add(completions->wellTypeName())
            .rowCompleted();
    }

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportWelspeclToFile(RimEclipseCase* gridCase,
                                                                      QFilePtr exportFile,
                                                                      const std::map<QString, std::vector<RigCompletionData>>& completions)
{
    QTextStream stream(exportFile.get());

    RifEclipseDataTableFormatter formatter(stream);
    formatter.setColumnSpacing(3);

    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn("Well"),
        RifEclipseOutputTableColumn("Grp"),
        RifEclipseOutputTableColumn("LGR"),
        RifEclipseOutputTableColumn("I"),
        RifEclipseOutputTableColumn("J"),
        RifEclipseOutputTableColumn("RefDpth"),
        RifEclipseOutputTableColumn("WellType")
    };

    formatter.keyword("WELSPECL");
    formatter.header(header);

    std::map<QString, std::set<const RimWellPath*>> wellPathMap;

    // Build list of unique RimWellPath for each LGR
    for (const auto completionsForLgr : completions)
    {
        wellPathMap.insert(std::make_pair(completionsForLgr.first, std::set<const RimWellPath*>()));

        for (const auto completion : completionsForLgr.second)
        {
            const auto wellPath = findWellPathFromExportName(completion.wellName());
            if (wellPath)
            {
                wellPathMap[completionsForLgr.first].insert(wellPath);
            }
        }
    }

    for (const auto wellPathsForLgr : wellPathMap)
    {
        QString lgrName = wellPathsForLgr.first;

        // Export
        for (const auto wellPath : wellPathsForLgr.second)
        {
            auto completions = wellPath->completions();
            cvf::Vec2i ijIntersection = wellPathUpperGridIntersectionIJ(gridCase, wellPath, lgrName);

            formatter
                .add(completions->wellNameForExport())
                .add(completions->wellGroupName())
                .add(lgrName)
                .addOneBasedCellIndex(ijIntersection.x())
                .addOneBasedCellIndex(ijIntersection.y())
                .add(completions->referenceDepth())
                .add(completions->wellTypeName())
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
    std::vector<RigCompletionData>&                      completions,
    const std::vector<RicWellPathFractureReportItem>&    wellPathFractureReportItems,
    RicExportCompletionDataSettingsUi::CompdatExportType exportType)
{
    // Sort completions based on grid they belong to
    std::vector<RigCompletionData>                    completionsForMainGrid = mainGridCompletions(completions);
    std::map<QString, std::vector<RigCompletionData>> completionsForSubGrids = subGridsCompletions(completions);

    if (!completionsForMainGrid.empty())
    {
        try
        {
            QFilePtr exportFile = openFileForExport(folderName, fileName);

            std::map<QString, std::vector<RigCompletionData>> completionsForGrid;
            completionsForGrid.insert(std::pair<QString, std::vector<RigCompletionData>>("", completionsForMainGrid));

            exportWellPathFractureReport(eclipseCase, exportFile, wellPathFractureReportItems);
            exportWelspecsToFile(eclipseCase, exportFile, completionsForMainGrid);
            exportCompdatAndWpimultTables(eclipseCase, exportFile, completionsForGrid, exportType);
        }
        catch(OpenFileException)
        { }
    }

    if (!completionsForSubGrids.empty())
    {
        try
        {
            QString lgrFileName = fileName + "_LGR";
            QFilePtr exportFile = openFileForExport(folderName, lgrFileName);

            exportWellPathFractureReport(eclipseCase, exportFile, wellPathFractureReportItems);
            exportWelspeclToFile(eclipseCase, exportFile, completionsForSubGrids);
            exportCompdatAndWpimultTables(eclipseCase, exportFile, completionsForSubGrids, exportType);
        }
        catch(OpenFileException)
        { }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportCompdatAndWpimultTables(
    RimEclipseCase*                                          sourceCase,
    QFilePtr                                                 exportFile,
    const std::map<QString, std::vector<RigCompletionData>>& completionsPerGrid,
    RicExportCompletionDataSettingsUi::CompdatExportType     exportType)
{
    if (completionsPerGrid.empty()) return;

    QTextStream stream(exportFile.get());

    RifEclipseDataTableFormatter formatter(stream);
    formatter.setColumnSpacing(3);

    for (const auto& gridCompletions : completionsPerGrid)
    {
        std::vector<RigCompletionData> completions = gridCompletions.second;

        // Sort by well name / cell index
        std::sort(completions.begin(), completions.end());

        // Print completion data
        QString gridName = gridCompletions.first;
        exportCompdatTableUsingFormatter(formatter, gridName, completions);

        if (exportType == RicExportCompletionDataSettingsUi::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS)
        {
            exportWpimultTableUsingFormatter(formatter, gridName, completions);
        }
    }

    RiaLogging::info(QString("Successfully exported completion data to %1").arg(exportFile->fileName()));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportCompdatTableUsingFormatter(
    RifEclipseDataTableFormatter&         formatter,
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
                  RifEclipseOutputTableColumn(
                      "KH", RifEclipseOutputTableDoubleFormatting(RifEclipseOutputTableDoubleFormat::RIF_SCIENTIFIC)),
                  RifEclipseOutputTableColumn("S"),
                  RifEclipseOutputTableColumn(
                      "Df", RifEclipseOutputTableDoubleFormatting(RifEclipseOutputTableDoubleFormat::RIF_SCIENTIFIC)),
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
                  RifEclipseOutputTableColumn(
                      "KH", RifEclipseOutputTableDoubleFormatting(RifEclipseOutputTableDoubleFormat::RIF_SCIENTIFIC)),
                  RifEclipseOutputTableColumn("S"),
                  RifEclipseOutputTableColumn(
                      "Df", RifEclipseOutputTableDoubleFormatting(RifEclipseOutputTableDoubleFormat::RIF_SCIENTIFIC)),
                  RifEclipseOutputTableColumn("DIR"),
                  RifEclipseOutputTableColumn("r0")};

        formatter.keyword("COMPDATL");
    }
    formatter.header(header);

    RigCompletionData::CompletionType currentCompletionType = RigCompletionData::CT_UNDEFINED;

    for (const RigCompletionData& data : completionData)
    {
        if (data.transmissibility() == 0.0 || data.wpimult() == 0.0)
        {
            // Don't export completions without transmissibility
            continue;
        }

        if (currentCompletionType != data.completionType())
        {
            // The completions are sorted by completion type, write out a heading when completion type changes

            QString txt;
            if (data.completionType() == RigCompletionData::FISHBONES) txt = "Fishbones";
            if (data.completionType() == RigCompletionData::FRACTURE) txt = "Fracture";
            if (data.completionType() == RigCompletionData::PERFORATION) txt = "Perforation";

            formatter.comment("---- Completions for completion type " + txt + " ----");

            currentCompletionType = data.completionType();
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

        formatter.addOneBasedCellIndex(data.completionDataGridCell().localCellIndexI())
            .addOneBasedCellIndex(data.completionDataGridCell().localCellIndexJ())
            .addOneBasedCellIndex(data.completionDataGridCell().localCellIndexK())
            .addOneBasedCellIndex(data.completionDataGridCell().localCellIndexK());
        switch (data.connectionState())
        {
            case OPEN:
                formatter.add("OPEN");
                break;
            case SHUT:
                formatter.add("SHUT");
                break;
            case AUTO:
                formatter.add("AUTO");
                break;
        }

        formatter.addValueOrDefaultMarker(data.saturation(), RigCompletionData::defaultValue());

        if (data.isNonDarcyFlow() || RigCompletionData::isDefaultValue(data.transmissibility()))
        {
            formatter.addValueOrDefaultMarker(data.transmissibility(), RigCompletionData::defaultValue());
            formatter.addValueOrDefaultMarker(data.diameter(), RigCompletionData::defaultValue());
            formatter.addValueOrDefaultMarker(data.kh(), RigCompletionData::defaultValue());
            formatter.addValueOrDefaultMarker(data.skinFactor(), RigCompletionData::defaultValue());
            if (RigCompletionData::isDefaultValue(data.dFactor()))
                formatter.add("1*");
            else
                formatter.add(-data.dFactor());

            switch (data.direction())
            {
                case DIR_I:
                    formatter.add("'X'");
                    break;
                case DIR_J:
                    formatter.add("'Y'");
                    break;
                case DIR_K:
                    formatter.add("'Z'");
                    break;
                default:
                    formatter.add("'Z'");
                    break;
            }
        }
        else
        {
            formatter.add(data.transmissibility());

            // Based on feedback from Shunping for COMPDATL, hhgs required COMPDAT
            // Always include diameter
            // See https://github.com/OPM/ResInsight/issues/2517
            // See https://github.com/OPM/ResInsight/issues/2709
            formatter.addValueOrDefaultMarker(data.diameter(), RigCompletionData::defaultValue());
        }

        formatter.rowCompleted();
    }
    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportWpimultTableUsingFormatter(
    RifEclipseDataTableFormatter&         formatter,
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

        if (!gridName.isEmpty())
        {
            formatter.add(gridName);
        }

        formatter.add(completion.wpimult());

        formatter.addOneBasedCellIndex(completion.completionDataGridCell().localCellIndexI())
            .addOneBasedCellIndex(completion.completionDataGridCell().localCellIndexJ())
            .addOneBasedCellIndex(completion.completionDataGridCell().localCellIndexK());
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
                completion.addMetadata("Perforation Completion",
                                       QString("MD In: %1 - MD Out: %2").arg(cell.startMD).arg(cell.endMD) +
                                           QString(" Transmissibility: ") + QString::number(transmissibility));
                completionData.push_back(completion);
            }
        }
    }

    return completionData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswExportInfo RicWellPathExportCompletionDataFeatureImpl::generateFishbonesMswExportInfo(const RimEclipseCase* caseToApply,
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

    return generateFishbonesMswExportInfo(caseToApply, wellPath, fishbonesSubs);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswExportInfo RicWellPathExportCompletionDataFeatureImpl::generateFishbonesMswExportInfo(
    const RimEclipseCase*                         caseToApply,
    const RimWellPath*                            wellPath,
    const std::vector<RimFishbonesMultipleSubs*>& fishbonesSubs)
{
    RiaEclipseUnitTools::UnitSystem unitSystem = caseToApply->eclipseCaseData()->unitsType();

    RicMswExportInfo exportInfo(wellPath,
                                unitSystem,
                                wellPath->fishbonesCollection()->startMD(),
                                wellPath->fishbonesCollection()->mswParameters()->lengthAndDepth().text(),
                                wellPath->fishbonesCollection()->mswParameters()->pressureDrop().text());
    exportInfo.setLinerDiameter(wellPath->fishbonesCollection()->mswParameters()->linerDiameter(unitSystem));
    exportInfo.setRoughnessFactor(wellPath->fishbonesCollection()->mswParameters()->roughnessFactor(unitSystem));

    bool   foundSubGridIntersections = false;
    double startMD                   = wellPath->fishbonesCollection()->startMD();
    for (RimFishbonesMultipleSubs* subs : fishbonesSubs)
    {
        for (auto& sub : subs->installedLateralIndices())
        {
            double     endMD       = subs->measuredDepth(sub.subIndex);
            cvf::Vec3d position    = wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(startMD);
            cvf::Vec3d endPosition = wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(endMD);
            double     startTVD    = -position.z();
            double     endTVD      = -endPosition.z();

            RicMswSegment location = RicMswSegment(subs->generatedName(), startMD, endMD, startTVD, endTVD, sub.subIndex);
            location.setEffectiveDiameter(subs->effectiveDiameter(unitSystem));
            location.setHoleDiameter(subs->holeDiameter(unitSystem));
            location.setOpenHoleRoughnessFactor(subs->openHoleRoughnessFactor(unitSystem));
            location.setSkinFactor(subs->skinFactor());
            location.setIcdFlowCoefficient(subs->icdFlowCoefficient());
            double icdOrificeRadius = subs->icdOrificeDiameter(unitSystem) / 2;
            location.setIcdArea(icdOrificeRadius * icdOrificeRadius * cvf::PI_D * subs->icdCount());

            // Add completion for ICD
            RicMswCompletion icdCompletion(RigCompletionData::ICD, QString("ICD"));
            RicMswSubSegment icdSegment(endMD, 0.1, endTVD, 0.0);
            icdCompletion.addSubSegment(icdSegment);
            location.addCompletion(icdCompletion);

            for (size_t lateralIndex : sub.lateralIndices)
            {
                QString label = QString("Lateral %1").arg(lateralIndex);
                location.addCompletion(RicMswCompletion(RigCompletionData::FISHBONES, label, lateralIndex));
            }
            assignFishbonesLateralIntersections(caseToApply, subs, &location, &foundSubGridIntersections);

            exportInfo.addWellSegmentLocation(location);

            startMD = endMD;
        }
    }
    exportInfo.setHasSubGridIntersections(foundSubGridIntersections);
    exportInfo.sortLocations();

    assignBranchAndSegmentNumbers(caseToApply, &exportInfo);

    return exportInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswExportInfo RicWellPathExportCompletionDataFeatureImpl::generateFracturesMswExportInfo(RimEclipseCase*    caseToApply,
                                                                                            const RimWellPath* wellPath)
{
    std::vector<RimWellPathFracture*> fractures;

    if (wellPath->fractureCollection()->isChecked())
    {
        for (RimWellPathFracture* fracture : wellPath->fractureCollection()->fractures())
        {
            if (fracture->isChecked())
            {
                fractures.push_back(fracture);
            }
        }
    }

    return generateFracturesMswExportInfo(caseToApply, wellPath, fractures);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswExportInfo
    RicWellPathExportCompletionDataFeatureImpl::generateFracturesMswExportInfo(RimEclipseCase*    caseToApply,
                                                                               const RimWellPath* wellPath,
                                                                               const std::vector<RimWellPathFracture*>& fractures)
{
    const RigMainGrid* grid = caseToApply->eclipseCaseData()->mainGrid();
    const RigActiveCellInfo* activeCellInfo = caseToApply->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);
    RiaEclipseUnitTools::UnitSystem unitSystem = caseToApply->eclipseCaseData()->unitsType();

    const RigWellPath*             wellPathGeometry = wellPath->wellPathGeometry();
    const std::vector<cvf::Vec3d>& coords           = wellPathGeometry->wellPathPoints();
    const std::vector<double>&     mds              = wellPathGeometry->measureDepths();
    CVF_ASSERT(!coords.empty() && !mds.empty());

    std::vector<WellPathCellIntersectionInfo> intersections =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath(caseToApply->eclipseCaseData(), coords, mds);

    double initialMD = 0.0;
    if (wellPath->fractureCollection()->referenceMDType() == RimWellPathFractureCollection::MANUAL_REFERENCE_MD)
    {
        initialMD = wellPath->fractureCollection()->manualReferenceMD();
    }
    else
    {
        for (WellPathCellIntersectionInfo intersection : intersections)
        {
            if (activeCellInfo->isActive(intersection.globCellIndex))
            {
                initialMD = intersection.startMD;
                break;
            }
        }
    }

    RicMswExportInfo exportInfo(wellPath,
                                unitSystem,
                                initialMD,
                                wellPath->fractureCollection()->mswParameters()->lengthAndDepth().text(),
                                wellPath->fractureCollection()->mswParameters()->pressureDrop().text());

    exportInfo.setLinerDiameter(wellPath->fractureCollection()->mswParameters()->linerDiameter(unitSystem));
    exportInfo.setRoughnessFactor(wellPath->fractureCollection()->mswParameters()->roughnessFactor(unitSystem));

    bool foundSubGridIntersections = false;

    // Main bore
    int mainBoreSegment = 0;
    for (const auto& cellIntInfo : intersections)
    {
        double startTVD = -cellIntInfo.startPoint.z();
        double endTVD   = -cellIntInfo.endPoint.z();

        size_t             localGridIdx = 0u;
        const RigGridBase* localGrid    = grid->gridAndGridLocalIdxFromGlobalCellIdx(cellIntInfo.globCellIndex, &localGridIdx);
        QString            gridName;
        if (localGrid != grid)
        {
            gridName                  = QString::fromStdString(localGrid->gridName());
            foundSubGridIntersections = true;
        }

        size_t i = 0u, j = 0u, k = 0u;
        localGrid->ijkFromCellIndex(localGridIdx, &i, &j, &k);
        QString       label = QString("Main stem segment %1").arg(++mainBoreSegment);
        RicMswSegment location(label, cellIntInfo.startMD, cellIntInfo.endMD, startTVD, endTVD);

        // Check if fractures are to be assigned to current main bore segment
        for (RimWellPathFracture* fracture : fractures)
        {
            double fractureStartMD = fracture->fractureMD();
            if (fracture->fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH)
            {
                double perforationLength = fracture->fractureTemplate()->perforationLength();
                fractureStartMD -= 0.5 * perforationLength;
            }

            if (cvf::Math::valueInRange(fractureStartMD, cellIntInfo.startMD, cellIntInfo.endMD))
            {
                cvf::Vec3d position = wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(fractureStartMD);

                std::vector<const RimFracture*> fractureVector(1, fracture);
                std::vector<RigCompletionData>  completionData =
                    RicExportFractureCompletionsImpl::generateCompdatValues(caseToApply,
                                                                            wellPath->completions()->wellNameForExport(),
                                                                            wellPath->wellPathGeometry(),
                                                                            fractureVector,
                                                                            nullptr,
                                                                            nullptr);

                assignFractureIntersections(caseToApply, fracture, completionData, &location, &foundSubGridIntersections);
            }
        }

        exportInfo.addWellSegmentLocation(location);
    }
    exportInfo.setHasSubGridIntersections(foundSubGridIntersections);
    exportInfo.sortLocations();
    assignBranchAndSegmentNumbers(caseToApply, &exportInfo);

    return exportInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::assignFishbonesLateralIntersections(
    const RimEclipseCase*           caseToApply,
    const RimFishbonesMultipleSubs* fishbonesSubs,
    RicMswSegment*                  location,
    bool*                           foundSubGridIntersections)
{
    CVF_ASSERT(foundSubGridIntersections != nullptr);

    const RigMainGrid* grid = caseToApply->eclipseCaseData()->mainGrid();

    for (RicMswCompletion& completion : location->completions())
    {
        if (completion.completionType() != RigCompletionData::FISHBONES)
        {
            continue;
        }

        std::vector<std::pair<cvf::Vec3d, double>> lateralCoordMDPairs =
            fishbonesSubs->coordsAndMDForLateral(location->subIndex(), completion.index());

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

        for (const auto& cellIntInfo : intersections)
        {
            size_t             localGridIdx = 0u;
            const RigGridBase* localGrid = grid->gridAndGridLocalIdxFromGlobalCellIdx(cellIntInfo.globCellIndex, &localGridIdx);
            QString            gridName;
            if (localGrid != grid)
            {
                gridName                   = QString::fromStdString(localGrid->gridName());
                *foundSubGridIntersections = true;
            }

            size_t i = 0u, j = 0u, k = 0u;
            localGrid->ijkFromCellIndex(localGridIdx, &i, &j, &k);
            RicMswSubSegment subSegment(
                previousExitMD, cellIntInfo.endMD - previousExitMD, previousExitTVD, cellIntInfo.endPoint.z() - previousExitTVD);

            RicMswSubSegmentCellIntersection intersection(
                gridName, cellIntInfo.globCellIndex, cvf::Vec3st(i, j, k), cellIntInfo.intersectionLengthsInCellCS);
            subSegment.addIntersection(intersection);
            completion.addSubSegment(subSegment);

            previousExitMD  = cellIntInfo.endMD;
            previousExitTVD = cellIntInfo.endPoint.z();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::assignFractureIntersections(const RimEclipseCase*                 caseToApply,
                                                                             const RimWellPathFracture*            fracture,
                                                                             const std::vector<RigCompletionData>& completionData,
                                                                             RicMswSegment*                        location,
                                                                             bool* foundSubGridIntersections)
{
    CVF_ASSERT(foundSubGridIntersections != nullptr);
    const RigMainGrid* grid = caseToApply->eclipseCaseData()->mainGrid();

    double           fractureMD = fracture->fractureMD();
    RicMswCompletion fractureCompletion(RigCompletionData::FRACTURE, fracture->name());
    double           position = fracture->fractureMD();
    double           width    = fracture->fractureTemplate()->computeFractureWidth(fracture);

    if (fracture->fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH)
    {
        double perforationLength = fracture->fractureTemplate()->perforationLength();
        position -= 0.5 * perforationLength;
        width = perforationLength;
    }

    RicMswSubSegment subSegment(position, width, 0.0, 0.0);
    for (const RigCompletionData& compIntersection : completionData)
    {
        const RigCompletionDataGridCell& cell = compIntersection.completionDataGridCell();
        cvf::Vec3st                      localIJK(cell.localCellIndexI(), cell.localCellIndexJ(), cell.localCellIndexK());

        RicMswSubSegmentCellIntersection intersection(cell.lgrName(), cell.globalCellIndex(), localIJK, cvf::Vec3d::ZERO);
        subSegment.addIntersection(intersection);
    }
    fractureCompletion.addSubSegment(subSegment);
    location->addCompletion(fractureCompletion);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::assignBranchAndSegmentNumbers(const RimEclipseCase* caseToApply,
                                                                               RicMswSegment*        location,
                                                                               int*                  branchNum,
                                                                               int*                  segmentNum)
{
    int icdSegmentNumber = cvf::UNDEFINED_INT;
    for (RicMswCompletion& completion : location->completions())
    {
        if (completion.completionType() != RigCompletionData::ICD)
        {
            ++(*branchNum);
            completion.setBranchNumber(*branchNum);
        }

        int attachedSegmentNumber = location->segmentNumber();
        if (icdSegmentNumber != cvf::UNDEFINED_INT)
        {
            attachedSegmentNumber = icdSegmentNumber;
        }

        for (auto& subSegment : completion.subSegments())
        {
            if (completion.completionType() == RigCompletionData::ICD)
            {
                subSegment.setSegmentNumber(location->segmentNumber() + 1);
                icdSegmentNumber = subSegment.segmentNumber();
            }
            else
            {
                ++(*segmentNum);
                subSegment.setSegmentNumber(*segmentNum);
            }
            subSegment.setAttachedSegmentNumber(attachedSegmentNumber);
            attachedSegmentNumber = *segmentNum;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::assignBranchAndSegmentNumbers(const RimEclipseCase* caseToApply,
                                                                               RicMswExportInfo*     exportInfo)
{
    int segmentNumber = 1;
    int branchNumber  = 1;

    // First loop over the locations so that each segment on the main stem is an incremental number
    for (RicMswSegment& location : exportInfo->wellSegmentLocations())
    {
        location.setSegmentNumber(++segmentNumber);
        for (RicMswCompletion& completion : location.completions())
        {
            if (completion.completionType() == RigCompletionData::ICD)
            {
                ++segmentNumber; // Skip a segment number because we need one for the ICD
                completion.setBranchNumber(++branchNumber);
            }
        }
    }

    // Then assign branch and segment numbers to each completion sub segment
    for (RicMswSegment& location : exportInfo->wellSegmentLocations())
    {
        assignBranchAndSegmentNumbers(caseToApply, &location, &branchNumber, &segmentNumber);
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec2i RicWellPathExportCompletionDataFeatureImpl::wellPathUpperGridIntersectionIJ(const RimEclipseCase* gridCase,
                                                                                       const RimWellPath* wellPath,
                                                                                       const QString& gridName)
{
    const RigEclipseCaseData*       caseData = gridCase->eclipseCaseData();
    const RigMainGrid*              mainGrid = caseData->mainGrid();
    const RigActiveCellInfo*        activeCellInfo = caseData->activeCellInfo(RiaDefines::MATRIX_MODEL);
    const RigWellPath*              wellPathGeometry = wellPath->wellPathGeometry();
    const std::vector<cvf::Vec3d>&  coords = wellPathGeometry->wellPathPoints();
    const std::vector<double>&      mds = wellPathGeometry->measureDepths();
    CVF_ASSERT(!coords.empty() && !mds.empty());

    std::vector<WellPathCellIntersectionInfo> intersections =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath(caseData, coords, mds);

    size_t gridId = 0;

    if (!gridName.isEmpty())
    {
        const auto grid = caseData->grid(gridName);
        if (grid) gridId = grid->gridId();
    }

    for (WellPathCellIntersectionInfo intersection : intersections)
    {
        size_t             gridLocalCellIndex = 0;
        const RigGridBase* grid = mainGrid->gridAndGridLocalIdxFromGlobalCellIdx(intersection.globCellIndex, &gridLocalCellIndex);

        if (grid->gridId() == gridId && activeCellInfo->isActive(intersection.globCellIndex))
        {
            size_t i, j, k;
            if (grid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k))
            {
                return cvf::Vec2i((int)i, (int)j);
            }
        }
    }
    return cvf::Vec2i();
}


//--------------------------------------------------------------------------------------------------
/// Internal function
//--------------------------------------------------------------------------------------------------
const RimWellPath* findWellPathFromExportName(const QString& wellNameForExport)
{
    auto allWellPaths = RiaApplication::instance()->project()->allWellPaths();

    for (const auto wellPath : allWellPaths)
    {
        if (wellPath->completions()->wellNameForExport() == wellNameForExport) return wellPath;
    }
    return nullptr;
}

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

#include "../ExportCommands/RicExportLgrFeature.h"
#include "RicExportCompletionDataSettingsUi.h"
#include "RicExportFeatureImpl.h"
#include "RicExportFractureCompletionsImpl.h"
#include "RicFishbonesTransmissibilityCalculationFeatureImp.h"
#include "RicWellPathFractureReportItem.h"
#include "RicWellPathFractureTextReportFeatureImpl.h"

#include "RifEclipseDataTableFormatter.h"

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

#include <QDir>
#include <map>
#include <set>

//--------------------------------------------------------------------------------------------------
/// Internal definitions
//--------------------------------------------------------------------------------------------------
class SubSegmentIntersectionInfo
{
public:
    SubSegmentIntersectionInfo(size_t     globCellIndex,
                               double     startTVD,
                               double     endTVD,
                               double     startMD,
                               double     endMD,
                               cvf::Vec3d lengthsInCell)
        : globCellIndex(globCellIndex)
        , startTVD(startTVD)
        , endTVD(endTVD)
        , startMD(startMD)
        , endMD(endMD)
        , intersectionLengthsInCellCS(lengthsInCell)
    {
    }

    size_t     globCellIndex;
    double     startTVD;
    double     endTVD;
    double     startMD;
    double     endMD;
    cvf::Vec3d intersectionLengthsInCellCS;
};

const RimWellPath* findWellPathFromExportName(const QString& wellNameForExport);
std::vector<SubSegmentIntersectionInfo>
    spiltIntersectionSegmentsToMaxLength(const RigWellPath*                               pathGeometry,
                                         const std::vector<WellPathCellIntersectionInfo>& intersections,
                                         double                                           maxSegmentLength);
int numberOfSplittedSegments(double startMd, double endMd, double maxSegmentLength);

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class OpenFileException
{
public:
    OpenFileException(const QString& message)
        : message(message)
    {
    }
    QString message;
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

    exportCarfinForTemporaryLgrs(exportSettings.caseToApply(), exportSettings.folder);

    if (exportSettings.compdatExport == RicExportCompletionDataSettingsUi::TRANSMISSIBILITIES ||
        exportSettings.compdatExport == RicExportCompletionDataSettingsUi::WPIMULT_AND_DEFAULT_CONNECTION_FACTORS)
    {
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
                QString format =
                    QString("Unit systems for well path \"%1\" must match unit system of chosen eclipse case \"%2\"");
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
            if (!outputDir.mkpath("."))
            {
                QString errMsg = QString("Could not create export folder: %1").arg(exportSettings.folder);
                RiaLogging::error(errMsg);
                return;
            }

            QString fractureTransmisibillityExportInformationPath =
                QDir(exportSettings.folder).absoluteFilePath("FractureTransmissibilityExportInformation");

            fractureTransmissibilityExportInformationFile.setFileName(fractureTransmisibillityExportInformationPath);
            if (!fractureTransmissibilityExportInformationFile.open(QIODevice::WriteOnly | QIODevice::Text))
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

        size_t maxProgress =
            usedWellPaths.size() * 3 + simWells.size() +
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
            std::map<size_t, std::vector<RigCompletionData>> completionsPerEclipseCellAllCompletionTypes;
            std::map<size_t, std::vector<RigCompletionData>> completionsPerEclipseCellFishbones;
            std::map<size_t, std::vector<RigCompletionData>> completionsPerEclipseCellFracture;
            std::map<size_t, std::vector<RigCompletionData>> completionsPerEclipseCellPerforations;

            // Generate completion data

            if (exportSettings.includePerforations)
            {
                std::vector<RigCompletionData> perforationCompletionData = generatePerforationsCompdatValues(
                    wellPath, wellPath->perforationIntervalCollection()->perforations(), exportSettings);

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
                // If no report is wanted, set reportItems = nullptr
                std::vector<RicWellPathFractureReportItem>* reportItems = &fractureDataReportItems;

                std::vector<RigCompletionData> fractureCompletionData =
                    RicExportFractureCompletionsImpl::generateCompdatValuesForWellPath(
                        wellPath,
                        exportSettings.caseToApply(),
                        reportItems,
                        fractureTransmissibilityExportInformationStream.get(),
                        RicExportFractureCompletionsImpl::PressureDepletionParameters(exportSettings.performTransScaling(),
                                                                                      exportSettings.transScalingTimeStep(),
                                                                                      exportSettings.transScalingWBHPSource(),
                                                                                      exportSettings.transScalingWBHP()));

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
            std::map<size_t, std::vector<RigCompletionData>> completionsPerEclipseCell;

            std::vector<RigCompletionData> fractureCompletionData =
                RicExportFractureCompletionsImpl::generateCompdatValuesForSimWell(
                    exportSettings.caseToApply(),
                    simWell,
                    fractureTransmissibilityExportInformationStream.get(),
                    RicExportFractureCompletionsImpl::PressureDepletionParameters(exportSettings.performTransScaling(),
                                                                                  exportSettings.transScalingTimeStep(),
                                                                                  exportSettings.transScalingWBHPSource(),
                                                                                  exportSettings.transScalingWBHP()));

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
            QString fileName = QString("UnifiedCompletions_%1").arg(eclipseCaseName);
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
                std::vector<RigCompletionData> completionsForWell;
                for (const auto& completion : completions)
                {
                    if (RicWellPathExportCompletionDataFeatureImpl::isCompletionWellPathEqual(completion, wellPath))
                    {
                        completionsForWell.push_back(completion);
                    }
                }

                if (completionsForWell.empty()) continue;

                std::vector<RicWellPathFractureReportItem> reportItemsForWell;
                for (const auto& fracItem : fractureDataReportItems)
                {
                    if (fracItem.wellPathNameForExport() == wellPath->completions()->wellNameForExport())
                    {
                        reportItemsForWell.push_back(fracItem);
                    }
                }

                QString fileName = QString("%1_unifiedCompletions_%2").arg(wellPath->name()).arg(eclipseCaseName);
                sortAndExportCompletionsToFile(exportSettings.caseToApply,
                                               exportSettings.folder,
                                               fileName,
                                               completionsForWell,
                                               reportItemsForWell,
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
                    std::vector<RigCompletionData> completionsForWell;
                    for (const auto& completion : completions)
                    {
                        if (completionType == completion.completionType())
                        {
                            if (RicWellPathExportCompletionDataFeatureImpl::isCompletionWellPathEqual(completion, wellPath))
                            {
                                completionsForWell.push_back(completion);
                            }
                        }
                    }

                    if (completionsForWell.empty()) continue;

                    {
                        QString completionTypeText;
                        if (completionType == RigCompletionData::FISHBONES) completionTypeText = "Fishbones";
                        if (completionType == RigCompletionData::FRACTURE) completionTypeText = "Fracture";
                        if (completionType == RigCompletionData::PERFORATION) completionTypeText = "Perforation";

                        QString fileName = QString("%1_%2_%3").arg(wellPath->name()).arg(completionTypeText).arg(eclipseCaseName);
                        if (completionType == RigCompletionData::FRACTURE)
                        {
                            std::vector<RicWellPathFractureReportItem> reportItemsForWell;
                            for (const auto& fracItem : fractureDataReportItems)
                            {
                                if (fracItem.wellPathNameForExport() == wellPath->completions()->wellNameForExport())
                                {
                                    reportItemsForWell.push_back(fracItem);
                                }
                            }

                            sortAndExportCompletionsToFile(exportSettings.caseToApply,
                                                           exportSettings.folder,
                                                           fileName,
                                                           completionsForWell,
                                                           reportItemsForWell,
                                                           exportSettings.compdatExport);
                        }
                        else
                        {
                            std::vector<RicWellPathFractureReportItem> emptyReportItemVector;
                            sortAndExportCompletionsToFile(exportSettings.caseToApply,
                                                           exportSettings.folder,
                                                           fileName,
                                                           completionsForWell,
                                                           emptyReportItemVector,
                                                           exportSettings.compdatExport);
                        }
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

    if (exportSettings.includeMsw)
    {
        if (exportSettings.includeFractures())
        {
            bool anyActiveFractures = false;

            for (const auto& wellPath : wellPaths)
            {
                if (!wellPath->fractureCollection()->activeFractures().empty())
                {
                    anyActiveFractures = true;
                }
            }

            if (anyActiveFractures)
            {
                QString  fileName   = QString("%1-Fracture-Welsegs").arg(exportSettings.caseToApply->caseUserDescription());
                QFilePtr exportFile = openFileForExport(exportSettings.folder, fileName);

                for (const auto wellPath : wellPaths)
                {
                    auto fractures = wellPath->fractureCollection()->activeFractures();
                    if (!fractures.empty())
                    {
                        exportWellSegments(exportSettings.caseToApply, exportFile, wellPath, fractures);
                    }
                }
                exportFile->close();
            }
        }

        if (exportSettings.includeFishbones())
        {
            bool anyFishbones = false;

            for (const auto& wellPath : wellPaths)
            {
                if (!wellPath->fishbonesCollection()->activeFishbonesSubs().empty())
                {
                    anyFishbones = true;
                }
            }

            if (anyFishbones)
            {
                QString  fileName   = QString("%1-Fishbone-Welsegs").arg(exportSettings.caseToApply->caseUserDescription());
                QFilePtr exportFile = openFileForExport(exportSettings.folder, fileName);

                for (const auto wellPath : wellPaths)
                {
                    auto fishbones = wellPath->fishbonesCollection()->activeFishbonesSubs();
                    if (!fishbones.empty())
                    {
                        exportWellSegments(exportSettings.caseToApply, exportFile, wellPath, fishbones);
                    }
                }

                exportFile->close();
            }
        }

        if (exportSettings.includePerforations())
        {
            QString  fileName   = QString("%1-Perforation-Welsegs").arg(exportSettings.caseToApply->caseUserDescription());
            QFilePtr exportFile = openFileForExport(exportSettings.folder, fileName);

            for (const auto wellPath : wellPaths)
            {
                auto perforations = wellPath->perforationIntervalCollection()->perforations();
                exportWellSegments(exportSettings, exportFile, wellPath, perforations);
            }
            exportFile->close();
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

        completionsPerEclipseCell = generatePerforationsCompdatValues(
            wellPath, wellPath->perforationIntervalCollection()->perforations(), exportSettings);
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
        formatter.comment("Main Stem Segments");
        for (std::shared_ptr<RicMswSegment> location : exportInfo.wellSegmentLocations())
        {
            double depth  = 0;
            double length = 0;

            if (exportInfo.lengthAndDepthText() == QString("INC"))
            {
                depth  = location->endTVD() - prevTVD;
                length = location->endMD() - prevMD;
            }
            else
            {
                depth  = location->endTVD();
                length = location->endMD();
            }

            if (location->subIndex() != cvf::UNDEFINED_SIZE_T)
            {
                QString comment = location->label() + QString(", sub %1").arg(location->subIndex());
                formatter.comment(comment);
            }

            formatter.add(location->segmentNumber()).add(location->segmentNumber());
            formatter.add(1); // All segments on main stem are branch 1
            formatter.add(location->segmentNumber() - 1); // All main stem segments are connected to the segment below them
            formatter.add(length);
            formatter.add(depth);
            formatter.add(exportInfo.linerDiameter());
            formatter.add(exportInfo.roughnessFactor());
            formatter.rowCompleted();
            prevMD  = location->endMD();
            prevTVD = location->endTVD();
        }
    }

    {
        generateWelsegsSegments(formatter, exportInfo, {RigCompletionData::FISHBONES_ICD, RigCompletionData::FISHBONES});
        generateWelsegsSegments(formatter, exportInfo, {RigCompletionData::FRACTURE});
        generateWelsegsSegments(formatter, exportInfo, { RigCompletionData::PERFORATION_ICD });
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
    for (std::shared_ptr<RicMswSegment> segment : exportInfo.wellSegmentLocations())
    {
        for (std::shared_ptr<RicMswCompletion> completion : segment->completions())
        {
            if (exportCompletionTypes.count(completion->completionType()))
            {
                if (!generatedHeader)
                {
                    generateWelsegsCompletionCommentHeader(formatter, completion->completionType());
                    generatedHeader = true;
                }

                if (completion->completionType() == RigCompletionData::FISHBONES_ICD ||
                    completion->completionType() == RigCompletionData::PERFORATION_ICD) // Found ICD
                {
                    if (!completion->subSegments().empty())
                    {
                        formatter.comment(completion->label());

                        formatter.add(completion->subSegments().front()->segmentNumber());
                        formatter.add(completion->subSegments().front()->segmentNumber());
                        formatter.add(completion->branchNumber());
                        formatter.add(segment->segmentNumber());
                        formatter.add(0.1); // ICDs have 0.1 length
                        formatter.add(0); // Depth change
                        formatter.add(exportInfo.linerDiameter());
                        formatter.add(exportInfo.roughnessFactor());
                        formatter.rowCompleted();
                    }
                }
                else
                {
                    if (completion->completionType() == RigCompletionData::FISHBONES)
                    {
                        formatter.comment(QString("%1 : Sub index %2 - %3")
                                              .arg(segment->label())
                                              .arg(segment->subIndex())
                                              .arg(completion->label()));
                    }
                    else if (completion->completionType() == RigCompletionData::FRACTURE)
                    {
                        formatter.comment(QString("%1 connected to %2").arg(completion->label()).arg(segment->label()));
                    }

                    for (std::shared_ptr<RicMswSubSegment> subSegment : completion->subSegments())
                    {
                        double depth  = 0;
                        double length = 0;

                        if (exportInfo.lengthAndDepthText() == QString("INC"))
                        {
                            depth  = subSegment->deltaTVD();
                            length = subSegment->deltaMD();
                        }
                        else
                        {
                            depth  = subSegment->startTVD() + subSegment->deltaTVD();
                            length = subSegment->startMD() + subSegment->deltaMD();
                        }
                        double diameter = segment->effectiveDiameter();
                        formatter.add(subSegment->segmentNumber());
                        formatter.add(subSegment->segmentNumber());
                        formatter.add(completion->branchNumber());
                        formatter.add(subSegment->attachedSegmentNumber());
                        formatter.add(length);
                        formatter.add(depth);
                        formatter.add(diameter);
                        formatter.add(segment->openHoleRoughnessFactor());
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
    else if (completionType == RigCompletionData::FISHBONES_ICD)
    {
        formatter.comment("Fishbones segments");
        formatter.comment("Diam: MSW - Tubing Radius");
        formatter.comment("Rough: MSW - Open Hole Roughness Factor");
    }
    else if (completionType == RigCompletionData::PERFORATION_ICD)
    {
        formatter.comment("Perforation valve segments");
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
        std::set<RigCompletionData::CompletionType> fishbonesTypes = {RigCompletionData::FISHBONES_ICD,
                                                                      RigCompletionData::FISHBONES};
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

    {
        std::set<RigCompletionData::CompletionType> perforationTypes = {RigCompletionData::PERFORATION,
                                                                        RigCompletionData::PERFORATION_ICD};
        generateCompsegTable(formatter, exportInfo, false, perforationTypes);
        if (exportInfo.hasSubGridIntersections())
        {
            generateCompsegTable(formatter, exportInfo, true, perforationTypes);
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

    for (std::shared_ptr<RicMswSegment> location : exportInfo.wellSegmentLocations())
    {
        double startMD = location->startMD();

        for (std::shared_ptr<RicMswCompletion> completion : location->completions())
        {
            if (!completion->subSegments().empty() && exportCompletionTypes.count(completion->completionType()))
            {
                if (!generatedHeader)
                {
                    generateCompsegHeader(formatter, exportInfo, completion->completionType(), exportSubGridIntersections);
                    generatedHeader = true;
                }

                for (std::shared_ptr<RicMswSubSegment> segment : completion->subSegments())
                {
                    if (completion->completionType() == RigCompletionData::FISHBONES_ICD)
                    {
                        startMD = segment->startMD();
                    }

                    for (std::shared_ptr<RicMswSubSegmentCellIntersection> intersection : segment->intersections())
                    {
                        bool isSubGridIntersection = !intersection->gridName().isEmpty();
                        if (isSubGridIntersection == exportSubGridIntersections)
                        {
                            if (exportSubGridIntersections)
                            {
                                formatter.add(intersection->gridName());
                            }
                            cvf::Vec3st ijk = intersection->gridLocalCellIJK();
                            formatter.addOneBasedCellIndex(ijk.x()).addOneBasedCellIndex(ijk.y()).addOneBasedCellIndex(ijk.z());
                            formatter.add(completion->branchNumber());

                            double startLength = segment->startMD();
                            if (exportInfo.lengthAndDepthText() == QString("INC") &&
                                completion->completionType() != RigCompletionData::PERFORATION)
                            {
                                startLength -= startMD;
                            }
                            formatter.add(startLength);
                            formatter.add(startLength + segment->deltaMD());

                            formatter.rowCompleted();
                        }
                    }
                }
            }
        }
    }
    if (generatedHeader)
    {
        formatter.tableCompleted();
    }
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

    if (completionType == RigCompletionData::FISHBONES_ICD)
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
    bool foundValve = false;

    for (std::shared_ptr<RicMswSegment> location : exportInfo.wellSegmentLocations())
    {
        for (std::shared_ptr<RicMswCompletion> completion : location->completions())
        {
            if (completion->completionType() == RigCompletionData::FISHBONES_ICD ||
                completion->completionType() == RigCompletionData::PERFORATION_ICD)
            {
                if (!foundValve)
                {
                    formatter.keyword("WSEGVALV");
                    std::vector<RifEclipseOutputTableColumn> header = {
                        RifEclipseOutputTableColumn("Well Name"),
                        RifEclipseOutputTableColumn("Seg No"),
                        RifEclipseOutputTableColumn("Cv"),
                        RifEclipseOutputTableColumn("Ac"),
                    };
                    formatter.header(header);

                    foundValve = true;
                }
                if (completion->completionType() == RigCompletionData::FISHBONES_ICD ||
                    completion->completionType() == RigCompletionData::PERFORATION_ICD)
                {
                    std::shared_ptr<RicMswICD> icd = std::static_pointer_cast<RicMswICD>(completion);
                    if (!icd->subSegments().empty())
                    {
                        CVF_ASSERT(icd->subSegments().size() == 1u);
                        formatter.comment(icd->label());
                        formatter.add(exportInfo.wellPath()->name());
                        formatter.add(icd->subSegments().front()->segmentNumber());
                        formatter.add(icd->flowCoefficient());
                        formatter.add(QString("%1").arg(icd->area(), 8, 'g', 4));
                        formatter.rowCompleted();
                    }
                }
            }
        }
    }
    if (foundValve)
    {
        formatter.tableCompleted();
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

    if (completions.size() == 1)
    {
        return completions[0];
    }

    const RigCompletionData& firstCompletion = completions[0];

    const QString&                    wellName       = firstCompletion.wellName();
    const RigCompletionDataGridCell&  cellIndexIJK   = firstCompletion.completionDataGridCell();
    RigCompletionData::CompletionType completionType = firstCompletion.completionType();

    RigCompletionData resultCompletion(wellName, cellIndexIJK, firstCompletion.firstOrderingValue());
    resultCompletion.setSecondOrderingValue(firstCompletion.secondOrderingValue());
    resultCompletion.setSourcePdmObject(firstCompletion.sourcePdmObject());

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
    if (!exportFile->open(QIODevice::WriteOnly | QIODevice::Text))
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
                completions.insert(std::pair<QString, std::vector<RigCompletionData>>(gridName, {completion}));
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
void RicWellPathExportCompletionDataFeatureImpl::exportWellPathFractureReport(
    RimEclipseCase*                                   sourceCase,
    QFilePtr                                          exportFile,
    const std::vector<RicWellPathFractureReportItem>& wellPathFractureReportItems)
{
    QTextStream stream(exportFile.get());

    if (!wellPathFractureReportItems.empty())
    {
        std::vector<RicWellPathFractureReportItem> sortedReportItems;
        {
            std::set<RicWellPathFractureReportItem> fractureReportItemsSet;

            for (const auto& reportItem : wellPathFractureReportItems)
            {
                fractureReportItemsSet.insert(reportItem);
            }

            for (const auto& reportItem : fractureReportItemsSet)
            {
                sortedReportItems.emplace_back(reportItem);
            }
        }

        std::vector<RimWellPath*> wellPathsToReport;
        {
            std::set<RimWellPath*> wellPathsSet;

            auto allWellPaths = RicWellPathFractureTextReportFeatureImpl::wellPathsWithActiveFractures();
            for (const auto& wellPath : allWellPaths)
            {
                for (const auto& reportItem : sortedReportItems)
                {
                    if (reportItem.wellPathNameForExport() == wellPath->completions()->wellNameForExport())
                    {
                        wellPathsSet.insert(wellPath);
                    }
                }
            }

            std::copy(wellPathsSet.begin(), wellPathsSet.end(), std::back_inserter(wellPathsToReport));
        }

        RicWellPathFractureTextReportFeatureImpl reportGenerator;
        QString summaryText = reportGenerator.wellPathFractureReport(sourceCase, wellPathsToReport, sortedReportItems);

        stream << summaryText;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportWelspecsToFile(RimEclipseCase*                       gridCase,
                                                                      QFilePtr                              exportFile,
                                                                      const std::vector<RigCompletionData>& completions)
{
    QTextStream stream(exportFile.get());

    RifEclipseDataTableFormatter formatter(stream);
    formatter.setColumnSpacing(3);

    std::vector<RifEclipseOutputTableColumn> header = {RifEclipseOutputTableColumn("Well"),
                                                       RifEclipseOutputTableColumn("Grp"),
                                                       RifEclipseOutputTableColumn("I"),
                                                       RifEclipseOutputTableColumn("J"),
                                                       RifEclipseOutputTableColumn("RefDepth"),
                                                       RifEclipseOutputTableColumn("WellType")};

    formatter.keyword("WELSPECS");
    formatter.header(header);

    std::set<const RimWellPath*> wellPathSet;

    // Build list of unique RimWellPath
    for (const auto& completion : completions)
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
        auto rimCcompletions = wellPath->completions();
        auto ijIntersection  = wellPathUpperGridIntersectionIJ(gridCase, wellPath);

        formatter.add(rimCcompletions->wellNameForExport())
            .add(rimCcompletions->wellGroupNameForExport())
            .addOneBasedCellIndex(ijIntersection.second.x())
            .addOneBasedCellIndex(ijIntersection.second.y())
            .add(rimCcompletions->referenceDepthForExport())
            .add(rimCcompletions->wellTypeNameForExport())
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
    const std::map<QString, std::vector<RigCompletionData>>& completions)
{
    QTextStream stream(exportFile.get());

    RifEclipseDataTableFormatter formatter(stream);
    formatter.setColumnSpacing(3);

    std::vector<RifEclipseOutputTableColumn> header = {RifEclipseOutputTableColumn("Well"),
                                                       RifEclipseOutputTableColumn("Grp"),
                                                       RifEclipseOutputTableColumn("LGR"),
                                                       RifEclipseOutputTableColumn("I"),
                                                       RifEclipseOutputTableColumn("J"),
                                                       RifEclipseOutputTableColumn("RefDepth"),
                                                       RifEclipseOutputTableColumn("WellType")};

    formatter.keyword("WELSPECL");
    formatter.header(header);

    std::map<const RimWellPath*, std::set<QString>> wellPathToLgrNameMap;

    for (const auto& completionsForLgr : completions)
    {
        for (const auto& completion : completionsForLgr.second)
        {
            const auto wellPath = findWellPathFromExportName(completion.wellName());
            auto       item     = wellPathToLgrNameMap.find(wellPath);
            wellPathToLgrNameMap[wellPath].insert(completionsForLgr.first);
        }
    }

    for (const auto& wellPathsForLgr : wellPathToLgrNameMap)
    {
        const RimWellPath* wellPath = wellPathsForLgr.first;

        std::tuple<double, cvf::Vec2i, QString> itemWithLowestMD =
            std::make_tuple(std::numeric_limits<double>::max(), cvf::Vec2i(), "");

        // Find first LGR-intersection along the well path

        for (const auto& lgrName : wellPathsForLgr.second)
        {
            auto ijIntersection = wellPathUpperGridIntersectionIJ(gridCase, wellPath, lgrName);
            if (ijIntersection.first < std::get<0>(itemWithLowestMD))
            {
                itemWithLowestMD = std::make_tuple(ijIntersection.first, ijIntersection.second, lgrName);
            }
        }

        {
            double     measuredDepth = 0.0;
            cvf::Vec2i ijIntersection;
            QString    lgrName;

            std::tie(measuredDepth, ijIntersection, lgrName) = itemWithLowestMD;

            auto rimCompletions = wellPath->completions();

            formatter.add(rimCompletions->wellNameForExport())
                .add(rimCompletions->wellGroupNameForExport())
                .add(lgrName)
                .addOneBasedCellIndex(ijIntersection.x())
                .addOneBasedCellIndex(ijIntersection.y())
                .add(rimCompletions->referenceDepthForExport())
                .add(rimCompletions->wellTypeNameForExport())
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
        catch (OpenFileException)
        {
        }
    }

    if (!completionsForSubGrids.empty())
    {
        try
        {
            QString  lgrFileName = fileName + "_LGR";
            QFilePtr exportFile  = openFileForExport(folderName, lgrFileName);

            exportWellPathFractureReport(eclipseCase, exportFile, wellPathFractureReportItems);
            exportWelspeclToFile(eclipseCase, exportFile, completionsForSubGrids);
            exportCompdatAndWpimultTables(eclipseCase, exportFile, completionsForSubGrids, exportType);
        }
        catch (OpenFileException)
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
                  RifEclipseOutputTableColumn("DIR")};

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
                  RifEclipseOutputTableColumn("DIR")};

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
            default:
                formatter.add("'Z'");
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
    const RimWellPath*                                wellPath,
    const std::vector<const RimPerforationInterval*>& intervals,
    const RicExportCompletionDataSettingsUi&          settings)
{
    RiaEclipseUnitTools::UnitSystem unitSystem = settings.caseToApply->eclipseCaseData()->unitsType();

    std::vector<RigCompletionData> completionData;
    const RigActiveCellInfo* activeCellInfo = settings.caseToApply->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);

    if (wellPath->perforationIntervalCollection()->isChecked())
    {
        for (const RimPerforationInterval* interval : intervals)
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
                    calculateCellMainDirection(settings.caseToApply, cell.globCellIndex, cell.intersectionLengthsInCellCS);

                const RimNonDarcyPerforationParameters* nonDarcyParameters =
                    wellPath->perforationIntervalCollection()->nonDarcyParameters();

                double transmissibility = 0.0;
                double kh               = RigCompletionData::defaultValue();
                double dFactor          = RigCompletionData::defaultValue();

                {
                    auto transmissibilityData = calculateTransmissibilityData(settings.caseToApply,
                                                                              wellPath,
                                                                              cell.intersectionLengthsInCellCS,
                                                                              interval->skinFactor(),
                                                                              interval->diameter(unitSystem) / 2,
                                                                              cell.globCellIndex,
                                                                              settings.useLateralNTG);

                    transmissibility = transmissibilityData.connectionFactor();

                    if (nonDarcyParameters->nonDarcyFlowType() == RimNonDarcyPerforationParameters::NON_DARCY_USER_DEFINED)
                    {
                        kh      = transmissibilityData.kh();
                        dFactor = nonDarcyParameters->userDefinedDFactor();
                    }
                    else if (nonDarcyParameters->nonDarcyFlowType() == RimNonDarcyPerforationParameters::NON_DARCY_COMPUTED)
                    {
                        kh = transmissibilityData.kh();

                        const double effectiveH = transmissibilityData.effectiveH();

                        const double effectivePermeability =
                            nonDarcyParameters->gridPermeabilityScalingFactor() * transmissibilityData.effectiveK();

                        dFactor = calculateDFactor(settings.caseToApply,
                                                   effectiveH,
                                                   cell.globCellIndex,
                                                   wellPath->perforationIntervalCollection()->nonDarcyParameters(),
                                                   effectivePermeability);
                    }
                }

                completion.setTransAndWPImultBackgroundDataFromPerforation(
                    transmissibility, interval->skinFactor(), interval->diameter(unitSystem), dFactor, kh, direction);
                completion.addMetadata("Perforation Completion",
                                       QString("MD In: %1 - MD Out: %2").arg(cell.startMD).arg(cell.endMD) +
                                           QString(" Transmissibility: ") + QString::number(transmissibility));
                completion.setSourcePdmObject(interval);
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
                                                                                            const RimWellPath*    wellPath,
                                                                                            bool enableSegmentSplitting)
{
    std::vector<RimFishbonesMultipleSubs*> fishbonesSubs = wellPath->fishbonesCollection()->activeFishbonesSubs();

    return generateFishbonesMswExportInfo(caseToApply, wellPath, fishbonesSubs, enableSegmentSplitting);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswExportInfo RicWellPathExportCompletionDataFeatureImpl::generateFishbonesMswExportInfo(
    const RimEclipseCase*                         caseToApply,
    const RimWellPath*                            wellPath,
    const std::vector<RimFishbonesMultipleSubs*>& fishbonesSubs,
    bool                                          enableSegmentSplitting)
{
    RiaEclipseUnitTools::UnitSystem unitSystem = caseToApply->eclipseCaseData()->unitsType();

    RicMswExportInfo exportInfo(wellPath,
                                unitSystem,
                                wellPath->fishbonesCollection()->startMD(),
                                wellPath->fishbonesCollection()->mswParameters()->lengthAndDepth().text(),
                                wellPath->fishbonesCollection()->mswParameters()->pressureDrop().text());
    exportInfo.setLinerDiameter(wellPath->fishbonesCollection()->mswParameters()->linerDiameter(unitSystem));
    exportInfo.setRoughnessFactor(wellPath->fishbonesCollection()->mswParameters()->roughnessFactor(unitSystem));

    double maxSegmentLength = enableSegmentSplitting ? wellPath->fishbonesCollection()->mswParameters()->maxSegmentLength()
                                                     : std::numeric_limits<double>::infinity();
    bool   foundSubGridIntersections = false;
    double subStartMD                = wellPath->fishbonesCollection()->startMD();
    for (RimFishbonesMultipleSubs* subs : fishbonesSubs)
    {
        for (auto& sub : subs->installedLateralIndices())
        {
            double subEndMD    = subs->measuredDepth(sub.subIndex);
            double subEndTVD   = -wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(subEndMD).z();
            int    subSegCount = numberOfSplittedSegments(subStartMD, subEndMD, maxSegmentLength);
            double subSegLen   = (subEndMD - subStartMD) / subSegCount;

            double startMd  = subStartMD;
            double startTvd = -wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(startMd).z();
            for (int ssi = 0; ssi < subSegCount; ssi++)
            {
                double endMd  = startMd + subSegLen;
                double endTvd = -wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(endMd).z();

                std::shared_ptr<RicMswSegment> location (new RicMswSegment(subs->generatedName(), startMd, endMd, startTvd, endTvd, sub.subIndex));
                location->setEffectiveDiameter(subs->effectiveDiameter(unitSystem));
                location->setHoleDiameter(subs->holeDiameter(unitSystem));
                location->setOpenHoleRoughnessFactor(subs->openHoleRoughnessFactor(unitSystem));
                location->setSkinFactor(subs->skinFactor());
                location->setSourcePdmObject(subs);

                if (ssi == 0)
                {
                    // Add completion for ICD
                    std::shared_ptr<RicMswFishbonesICD> icdCompletion(new RicMswFishbonesICD(QString("ICD")));
                    std::shared_ptr<RicMswSubSegment> icdSegment(new RicMswSubSegment(subEndMD, 0.1, subEndTVD, 0.0));
                    icdCompletion->setFlowCoefficient(subs->icdFlowCoefficient());
                    double icdOrificeRadius = subs->icdOrificeDiameter(unitSystem) / 2;
                    icdCompletion->setArea(icdOrificeRadius * icdOrificeRadius * cvf::PI_D * subs->icdCount());

                    icdCompletion->addSubSegment(icdSegment);
                    location->addCompletion(icdCompletion);

                    for (size_t lateralIndex : sub.lateralIndices)
                    {
                        QString label = QString("Lateral %1").arg(lateralIndex);
                        location->addCompletion(std::make_shared<RicMswFishbones>(label, lateralIndex));
                    }
                    assignFishbonesLateralIntersections(
                        caseToApply, subs, location, &foundSubGridIntersections, maxSegmentLength);
                }

                exportInfo.addWellSegment(location);

                startMd  = endMd;
                startTvd = endTvd;
            }

            subStartMD = subEndMD;
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
    std::vector<RimWellPathFracture*> fractures = wellPath->fractureCollection()->activeFractures();

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
    const RigMainGrid*              grid           = caseToApply->eclipseCaseData()->mainGrid();
    const RigActiveCellInfo*        activeCellInfo = caseToApply->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);
    RiaEclipseUnitTools::UnitSystem unitSystem     = caseToApply->eclipseCaseData()->unitsType();

    const RigWellPath*             wellPathGeometry = wellPath->wellPathGeometry();
    const std::vector<cvf::Vec3d>& coords           = wellPathGeometry->wellPathPoints();
    const std::vector<double>&     mds              = wellPathGeometry->measureDepths();
    CVF_ASSERT(!coords.empty() && !mds.empty());

    std::vector<WellPathCellIntersectionInfo> intersections =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath(caseToApply->eclipseCaseData(), coords, mds);

    double maxSegmentLength = wellPath->fractureCollection()->mswParameters()->maxSegmentLength();
    std::vector<SubSegmentIntersectionInfo> subSegIntersections =
        spiltIntersectionSegmentsToMaxLength(wellPathGeometry, intersections, maxSegmentLength);

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
    int mainBoreSegment = 1;
    for (const auto& cellIntInfo : subSegIntersections)
    {
        double startTVD = cellIntInfo.startTVD;
        double endTVD   = cellIntInfo.endTVD;

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
        std::shared_ptr<RicMswSegment> location(new RicMswSegment(label, cellIntInfo.startMD, cellIntInfo.endMD, startTVD, endTVD));

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
                std::vector<RigCompletionData> completionData =
                    RicExportFractureCompletionsImpl::generateCompdatValues(caseToApply,
                                                                            wellPath->completions()->wellNameForExport(),
                                                                            wellPath->wellPathGeometry(),
                                                                            {fracture},
                                                                            nullptr,
                                                                            nullptr);

                assignFractureIntersections(caseToApply, fracture, completionData, location, &foundSubGridIntersections);
            }
        }

        exportInfo.addWellSegment(location);
    }
    exportInfo.setHasSubGridIntersections(foundSubGridIntersections);
    exportInfo.sortLocations();
    assignBranchAndSegmentNumbers(caseToApply, &exportInfo);

    return exportInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswExportInfo RicWellPathExportCompletionDataFeatureImpl::generatePerforationsMswExportInfo(
    const RicExportCompletionDataSettingsUi&          exportSettings,
    const RimWellPath*                                wellPath,
    const std::vector<const RimPerforationInterval*>& perforationIntervals)
{
    const RimEclipseCase*           caseToApply    = exportSettings.caseToApply;
    const RigActiveCellInfo*        activeCellInfo = caseToApply->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);
    RiaEclipseUnitTools::UnitSystem unitSystem     = caseToApply->eclipseCaseData()->unitsType();

    const RigWellPath*             wellPathGeometry = wellPath->wellPathGeometry();
    const std::vector<cvf::Vec3d>& coords           = wellPathGeometry->wellPathPoints();
    const std::vector<double>&     mds              = wellPathGeometry->measureDepths();
    CVF_ASSERT(!coords.empty() && !mds.empty());

    std::vector<WellPathCellIntersectionInfo> intersections =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath(caseToApply->eclipseCaseData(), coords, mds);

    double maxSegmentLength = wellPath->perforationIntervalCollection()->mswParameters()->maxSegmentLength();
    std::vector<SubSegmentIntersectionInfo> subSegIntersections =
        spiltIntersectionSegmentsToMaxLength(wellPathGeometry, intersections, maxSegmentLength);

    double initialMD = 0.0;
    for (WellPathCellIntersectionInfo intersection : intersections)
    {
        if (activeCellInfo->isActive(intersection.globCellIndex))
        {
            initialMD = intersection.startMD;
            break;
        }
    }

    RicMswExportInfo exportInfo(wellPath,
                                unitSystem,
                                initialMD,
                                wellPath->perforationIntervalCollection()->mswParameters()->lengthAndDepth().text(),
                                wellPath->perforationIntervalCollection()->mswParameters()->pressureDrop().text());

    exportInfo.setLinerDiameter(wellPath->perforationIntervalCollection()->mswParameters()->linerDiameter(unitSystem));
    exportInfo.setRoughnessFactor(wellPath->perforationIntervalCollection()->mswParameters()->roughnessFactor(unitSystem));

    bool foundSubGridIntersections = false;

    MainBoreSegments mainBoreSegments = createMainBoreSegments(subSegIntersections, perforationIntervals, wellPath, exportSettings, &foundSubGridIntersections);
    
    assignSuperValveCompletions(mainBoreSegments, perforationIntervals);
    assignValveContributionsToSuperValves(mainBoreSegments, perforationIntervals, unitSystem);    
    moveIntersectionsToSuperValves(mainBoreSegments);

    for (std::shared_ptr<RicMswSegment> segment : mainBoreSegments)
    {
        exportInfo.addWellSegment(segment);
    }

    exportInfo.setHasSubGridIntersections(foundSubGridIntersections);
    exportInfo.sortLocations();
    assignBranchAndSegmentNumbers(caseToApply, &exportInfo);

    return exportInfo;    
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellPathExportCompletionDataFeatureImpl::MainBoreSegments
RicWellPathExportCompletionDataFeatureImpl::createMainBoreSegments(
    const std::vector<SubSegmentIntersectionInfo>&    subSegIntersections,
    const std::vector<const RimPerforationInterval*>& perforationIntervals,
    const RimWellPath*                                wellPath,
    const RicExportCompletionDataSettingsUi&          exportSettings,
    bool*                                             foundSubGridIntersections)
{
    MainBoreSegments mainBoreSegments;

    for (const auto& cellIntInfo : subSegIntersections)
    {
        if (std::fabs(cellIntInfo.endMD - cellIntInfo.startMD) > 1.0e-8)
        {
            QString                        label = QString("Main stem segment %1").arg(mainBoreSegments.size() + 2);
            std::shared_ptr<RicMswSegment> segment(
                new RicMswSegment(label, cellIntInfo.startMD, cellIntInfo.endMD, cellIntInfo.startTVD, cellIntInfo.endTVD));

            for (const RimPerforationInterval* interval : perforationIntervals)
            {
                double overlapStart = std::max(interval->startMD(), segment->startMD());
                double overlapEnd   = std::min(interval->endMD(), segment->endMD());
                double overlap      = std::max(0.0, overlapEnd - overlapStart);
                if (overlap > 0.0)
                {
                    std::shared_ptr<RicMswCompletion> intervalCompletion(
                        new RicMswPerforation(interval->name()));
                    std::vector<RigCompletionData> completionData =
                        generatePerforationsCompdatValues(wellPath, {interval}, exportSettings);
                    assignPerforationIntervalIntersections(
                        completionData, intervalCompletion, cellIntInfo, foundSubGridIntersections);
                    segment->addCompletion(intervalCompletion);
                }
            }
            mainBoreSegments.push_back(segment);
        }
    }
    return mainBoreSegments;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::assignSuperValveCompletions(
    std::vector<std::shared_ptr<RicMswSegment>>&      mainBoreSegments,
    const std::vector<const RimPerforationInterval*>& perforationIntervals)
{
    for (size_t nMainSegment = 0u; nMainSegment < mainBoreSegments.size(); ++nMainSegment)
    {
        std::shared_ptr<RicMswSegment> segment = mainBoreSegments[nMainSegment];

        std::shared_ptr<RicMswPerforationICD> superValve;        
        for (const RimPerforationInterval* interval : perforationIntervals)
        {
            std::vector<const RimWellPathValve*> perforationValves;
            interval->descendantsIncludingThisOfType(perforationValves);

            for (const RimWellPathValve* valve : perforationValves)
            {
                for (size_t nSubValve = 0u; nSubValve < valve->valveLocations().size(); ++nSubValve)
                {
                    double valveMD = valve->valveLocations()[nSubValve];

                    std::pair<double, double> valveSegment       = valve->valveSegments()[nSubValve];
                    double                    overlapStart       = std::max(valveSegment.first, segment->startMD());
                    double                    overlapEnd         = std::min(valveSegment.second, segment->endMD());
                    double                    overlap            = std::max(0.0, overlapEnd - overlapStart);

                    if (segment->startMD() <= valveMD && valveMD < segment->endMD())
                    {
                        QString valveLabel = QString("%1 #%2").arg("Combined Valve for segment").arg(nMainSegment + 2);
                        superValve.reset(new RicMswPerforationICD(valveLabel));
                        std::shared_ptr<RicMswSubSegment> subSegment(new RicMswSubSegment(valveMD, 0.1, 0.0, 0.0));
                        superValve->addSubSegment(subSegment);
                    }
                    else if (overlap > 0.0 && !superValve)
                    {
                        QString valveLabel = QString("%1 #%2").arg("Combined Valve for segment").arg(nMainSegment + 2);
                        superValve.reset(new RicMswPerforationICD(valveLabel));
                        std::shared_ptr<RicMswSubSegment> subSegment(new RicMswSubSegment(overlapStart, 0.1, 0.0, 0.0));
                        superValve->addSubSegment(subSegment);
                    }
                }
            }
        }

        if (superValve)
        {
            segment->addCompletion(superValve);
        }

    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::assignValveContributionsToSuperValves(
    const std::vector<std::shared_ptr<RicMswSegment>>& mainBoreSegments,
    const std::vector<const RimPerforationInterval*>& perforationIntervals,
    RiaEclipseUnitTools::UnitSystem unitSystem)
{
    ValveContributionMap assignedRegularValves;
    for (std::shared_ptr<RicMswSegment> segment : mainBoreSegments)
    {
        std::shared_ptr<RicMswPerforationICD> superValve;
        for (auto completion : segment->completions())
        {
            std::shared_ptr<RicMswPerforationICD> valve = std::dynamic_pointer_cast<RicMswPerforationICD>(completion);
            if (valve)
            {
                superValve = valve;
                break;
            }
        }

        if (!superValve) continue;

        double                            totalIcdArea = 0.0;
        RiaWeightedMeanCalculator<double> coeffMeanCalc;

        for (const RimPerforationInterval* interval : perforationIntervals)
        {
            std::vector<const RimWellPathValve*> perforationValves;
            interval->descendantsIncludingThisOfType(perforationValves);

            for (const RimWellPathValve* valve : perforationValves)
            {
                for (size_t nSubValve = 0u; nSubValve < valve->valveSegments().size(); ++nSubValve)
                {
                    std::pair<double, double> valveSegment       = valve->valveSegments()[nSubValve];
                    double                    valveSegmentLength = valveSegment.second - valveSegment.first;
                    double                    overlapStart       = std::max(valveSegment.first, segment->startMD());
                    double                    overlapEnd         = std::min(valveSegment.second, segment->endMD());
                    double                    overlap            = std::max(0.0, overlapEnd - overlapStart);

                    if (overlap > 0.0)
                    {
                        assignedRegularValves[superValve].insert(std::make_pair(valve, nSubValve));
                        double icdOrificeRadius = valve->orificeDiameter(unitSystem) / 2;
                        double icdArea          = icdOrificeRadius * icdOrificeRadius * cvf::PI_D * overlap / valveSegmentLength;
                        totalIcdArea += icdArea;
                        coeffMeanCalc.addValueAndWeight(valve->flowCoefficient(), icdArea);
                    }
                }
            }
        }
        superValve->setArea(totalIcdArea);
        if (coeffMeanCalc.validAggregatedWeight())
        {
            superValve->setFlowCoefficient(coeffMeanCalc.weightedMean());
        }
    }
    
    for (auto regularValvePair : assignedRegularValves)
    {
        if (regularValvePair.second.size())
        {
            QStringList valveLabels;
            for (std::pair<const RimWellPathValve*, size_t> regularValve : regularValvePair.second)
            {
                QString valveLabel = QString("%1 #%2").arg(regularValve.first->name()).arg(regularValve.second + 1);
                valveLabels.push_back(valveLabel);
            }
            QString valveContribLabel = QString(" with contribution from: %1").arg(valveLabels.join(", "));
            regularValvePair.first->setLabel(regularValvePair.first->label() + valveContribLabel);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::moveIntersectionsToSuperValves(MainBoreSegments mainBoreSegments)
{
    for (auto segmentPtr : mainBoreSegments)
    {
        std::shared_ptr<RicMswCompletion> superValve;
        std::vector<std::shared_ptr<RicMswCompletion>> perforations;
        for (auto completionPtr : segmentPtr->completions())
        {
            if (completionPtr->completionType() == RigCompletionData::PERFORATION_ICD)
            {
                superValve = completionPtr;
            }
            else
            {
                CVF_ASSERT(completionPtr->completionType() == RigCompletionData::PERFORATION);
                perforations.push_back(completionPtr);
            }
        }

        if (superValve == nullptr) continue;

        CVF_ASSERT(superValve->subSegments().size() == 1u);
        segmentPtr->completions().clear();
        segmentPtr->addCompletion(superValve);
        for (auto perforationPtr : perforations)
        {
            for (auto subSegmentPtr : perforationPtr->subSegments())
            {
                for (auto intersectionPtr : subSegmentPtr->intersections())
                {
                    superValve->subSegments()[0]->addIntersection(intersectionPtr);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::assignFishbonesLateralIntersections(
    const RimEclipseCase*           caseToApply,
    const RimFishbonesMultipleSubs* fishbonesSubs,
    std::shared_ptr<RicMswSegment>  location,
    bool*                           foundSubGridIntersections,
    double                          maxSegmentLength)
{
    CVF_ASSERT(foundSubGridIntersections != nullptr);

    const RigMainGrid* grid = caseToApply->eclipseCaseData()->mainGrid();

    for (std::shared_ptr<RicMswCompletion> completion : location->completions())
    {
        if (completion->completionType() != RigCompletionData::FISHBONES)
        {
            continue;
        }

        std::vector<std::pair<cvf::Vec3d, double>> lateralCoordMDPairs =
            fishbonesSubs->coordsAndMDForLateral(location->subIndex(), completion->index());

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

        RigWellPath pathGeometry;
        pathGeometry.m_wellPathPoints = lateralCoords;
        pathGeometry.m_measuredDepths = lateralMDs;
        std::vector<SubSegmentIntersectionInfo> subSegIntersections =
            spiltIntersectionSegmentsToMaxLength(&pathGeometry, intersections, maxSegmentLength);

        double previousExitMD  = lateralMDs.front();
        double previousExitTVD = -lateralCoords.front().z();

        for (const auto& cellIntInfo : subSegIntersections)
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
            std::shared_ptr<RicMswSubSegment> subSegment(new RicMswSubSegment(
                previousExitMD, cellIntInfo.endMD - previousExitMD, previousExitTVD, cellIntInfo.endTVD - previousExitTVD));

            std::shared_ptr<RicMswSubSegmentCellIntersection> intersection(new RicMswSubSegmentCellIntersection(
                gridName, cellIntInfo.globCellIndex, cvf::Vec3st(i, j, k), cellIntInfo.intersectionLengthsInCellCS));
            subSegment->addIntersection(intersection);
            completion->addSubSegment(subSegment);

            previousExitMD  = cellIntInfo.endMD;
            previousExitTVD = cellIntInfo.endTVD;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::assignFractureIntersections(const RimEclipseCase*                 caseToApply,
                                                                             const RimWellPathFracture*            fracture,
                                                                             const std::vector<RigCompletionData>& completionData,
                                                                             std::shared_ptr<RicMswSegment>        location,
                                                                             bool* foundSubGridIntersections)
{
    CVF_ASSERT(foundSubGridIntersections != nullptr);

    std::shared_ptr<RicMswFracture> fractureCompletion(new RicMswFracture(fracture->name()));
    double           position = fracture->fractureMD();
    double           width    = fracture->fractureTemplate()->computeFractureWidth(fracture);

    if (fracture->fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH)
    {
        double perforationLength = fracture->fractureTemplate()->perforationLength();
        position -= 0.5 * perforationLength;
        width = perforationLength;
    }

    std::shared_ptr<RicMswSubSegment> subSegment(new RicMswSubSegment(position, width, 0.0, 0.0));
    for (const RigCompletionData& compIntersection : completionData)
    {
        const RigCompletionDataGridCell& cell = compIntersection.completionDataGridCell();
        cvf::Vec3st                      localIJK(cell.localCellIndexI(), cell.localCellIndexJ(), cell.localCellIndexK());

        std::shared_ptr<RicMswSubSegmentCellIntersection> intersection(new RicMswSubSegmentCellIntersection(cell.lgrName(), cell.globalCellIndex(), localIJK, cvf::Vec3d::ZERO));
        subSegment->addIntersection(intersection);
    }
    fractureCompletion->addSubSegment(subSegment);
    location->addCompletion(fractureCompletion);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::assignPerforationIntervalIntersections(
    const std::vector<RigCompletionData>& completionData,
    std::shared_ptr<RicMswCompletion>     perforationCompletion,
    const SubSegmentIntersectionInfo&     cellIntInfo,
    bool*                                 foundSubGridIntersections)
{
    size_t currCellId = cellIntInfo.globCellIndex;

    std::shared_ptr<RicMswSubSegment> subSegment(new RicMswSubSegment(cellIntInfo.startMD,
                                                                      cellIntInfo.endMD - cellIntInfo.startMD,
                                                                      cellIntInfo.startTVD,
                                                                      cellIntInfo.endTVD - cellIntInfo.startTVD));
    for (const RigCompletionData& compIntersection : completionData)
    {
        const RigCompletionDataGridCell& cell = compIntersection.completionDataGridCell();
        if (!cell.isMainGridCell())
        {
            *foundSubGridIntersections = true;
        }

        if (cell.globalCellIndex() != currCellId) continue;

        cvf::Vec3st localIJK(cell.localCellIndexI(), cell.localCellIndexJ(), cell.localCellIndexK());

        std::shared_ptr<RicMswSubSegmentCellIntersection> intersection(new RicMswSubSegmentCellIntersection(
            cell.lgrName(), cell.globalCellIndex(), localIJK, cellIntInfo.intersectionLengthsInCellCS));
        subSegment->addIntersection(intersection);
    }
    perforationCompletion->addSubSegment(subSegment);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::assignBranchAndSegmentNumbers(const RimEclipseCase*          caseToApply,
                                                                               std::shared_ptr<RicMswSegment> location,
                                                                               int*                           branchNum,
                                                                               int*                           segmentNum)
{
    int icdSegmentNumber = cvf::UNDEFINED_INT;
    for (std::shared_ptr<RicMswCompletion> completion : location->completions())
    {
        if (completion->completionType() == RigCompletionData::PERFORATION)
        {
            completion->setBranchNumber(1);
        }
        else if (completion->completionType() != RigCompletionData::FISHBONES_ICD)
        {
            ++(*branchNum);
            completion->setBranchNumber(*branchNum);
        }

        int attachedSegmentNumber = location->segmentNumber();
        if (icdSegmentNumber != cvf::UNDEFINED_INT)
        {
            attachedSegmentNumber = icdSegmentNumber;
        }

        for (auto subSegment : completion->subSegments())
        {
            if (completion->completionType() == RigCompletionData::FISHBONES_ICD)
            {
                subSegment->setSegmentNumber(location->segmentNumber() + 1);
                icdSegmentNumber = subSegment->segmentNumber();
            }
            else if (completion->completionType() != RigCompletionData::PERFORATION)
            {
                ++(*segmentNum);
                subSegment->setSegmentNumber(*segmentNum);
            }
            subSegment->setAttachedSegmentNumber(attachedSegmentNumber);
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
    for (auto location : exportInfo->wellSegmentLocations())
    {
        location->setSegmentNumber(++segmentNumber);
        for (auto completion : location->completions())
        {
            if (completion->completionType() == RigCompletionData::FISHBONES_ICD)
            {
                ++segmentNumber; // Skip a segment number because we need one for the ICD
                if (completion->completionType() == RigCompletionData::FISHBONES_ICD)
                {
                    completion->setBranchNumber(++branchNumber);
                }
            }
        }
    }

    // Then assign branch and segment numbers to each completion sub segment
    for (auto location : exportInfo->wellSegmentLocations())
    {
        assignBranchAndSegmentNumbers(caseToApply, location, &branchNumber, &segmentNumber);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::appendCompletionData(
    std::map<size_t, std::vector<RigCompletionData>>* completionData,
    const std::vector<RigCompletionData>&             completionsToAppend)
{
    for (const auto& completion : completionsToAppend)
    {
        auto it = completionData->find(completion.completionDataGridCell().globalCellIndex());
        if (it != completionData->end())
        {
            it->second.push_back(completion);
        }
        else
        {
            completionData->insert(std::pair<size_t, std::vector<RigCompletionData>>(
                completion.completionDataGridCell().globalCellIndex(), std::vector<RigCompletionData>{completion}));
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CellDirection RicWellPathExportCompletionDataFeatureImpl::calculateCellMainDirection(RimEclipseCase*   eclipseCase,
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
TransmissibilityData
    RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibilityData(RimEclipseCase*    eclipseCase,
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

    if (dxAccessObject.isNull() || dyAccessObject.isNull() || dzAccessObject.isNull() || permxAccessObject.isNull() ||
        permyAccessObject.isNull() || permzAccessObject.isNull())
    {
        return TransmissibilityData();
    }

    double ntg = 1.0;
    {
        // Trigger loading from file
        eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "NTG");

        cvf::ref<RigResultAccessor> ntgAccessObject =
            RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "NTG");

        if (ntgAccessObject.notNull())
        {
            ntg = ntgAccessObject->cellScalarGlobIdx(globalCellIndex);
        }
    }
    double latNtg = useLateralNTG ? ntg : 1.0;

    double dx    = dxAccessObject->cellScalarGlobIdx(globalCellIndex);
    double dy    = dyAccessObject->cellScalarGlobIdx(globalCellIndex);
    double dz    = dzAccessObject->cellScalarGlobIdx(globalCellIndex);
    double permx = permxAccessObject->cellScalarGlobIdx(globalCellIndex);
    double permy = permyAccessObject->cellScalarGlobIdx(globalCellIndex);
    double permz = permzAccessObject->cellScalarGlobIdx(globalCellIndex);

    const double totalKh = RigTransmissibilityEquations::totalKh(permx, permy, permz, internalCellLengths, latNtg, ntg);

    const double effectiveK = RigTransmissibilityEquations::effectiveK(permx, permy, permz, internalCellLengths, latNtg, ntg);
    const double effectiveH = RigTransmissibilityEquations::effectiveH(internalCellLengths, latNtg, ntg);

    double darcy = RiaEclipseUnitTools::darcysConstant(wellPath->unitSystem());

    if (volumeScaleConstant != 1)
    {
        if (directionForVolumeScaling == CellDirection::DIR_I) dx = dx / volumeScaleConstant;
        if (directionForVolumeScaling == CellDirection::DIR_J) dy = dy / volumeScaleConstant;
        if (directionForVolumeScaling == CellDirection::DIR_K) dz = dz / volumeScaleConstant;
    }

    const double transx = RigTransmissibilityEquations::wellBoreTransmissibilityComponent(
        internalCellLengths.x() * latNtg, permy, permz, dy, dz, wellRadius, skinFactor, darcy);
    const double transy = RigTransmissibilityEquations::wellBoreTransmissibilityComponent(
        internalCellLengths.y() * latNtg, permx, permz, dx, dz, wellRadius, skinFactor, darcy);
    const double transz = RigTransmissibilityEquations::wellBoreTransmissibilityComponent(
        internalCellLengths.z() * ntg, permy, permx, dy, dx, wellRadius, skinFactor, darcy);

    const double totalConnectionFactor = RigTransmissibilityEquations::totalConnectionFactor(transx, transy, transz);

    TransmissibilityData trData;
    trData.setData(effectiveH, effectiveK, totalConnectionFactor, totalKh);
    return trData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathExportCompletionDataFeatureImpl::calculateDFactor(RimEclipseCase*                         eclipseCase,
                                                                    double                                  effectiveH,
                                                                    size_t                                  globalCellIndex,
                                                                    const RimNonDarcyPerforationParameters* nonDarcyParameters,
                                                                    const double                            effectivePermeability)
{
    using EQ = RigPerforationTransmissibilityEquations;

    if (!eclipseCase || !eclipseCase->eclipseCaseData())
    {
        return std::numeric_limits<double>::infinity();
    }

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    double porosity = 0.0;
    {
        eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PORO");
        cvf::ref<RigResultAccessor> poroAccessObject =
            RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RiaDefines::MATRIX_MODEL, 0, "PORO");

        if (poroAccessObject.notNull())
        {
            porosity = poroAccessObject->cellScalar(globalCellIndex);
        }
    }

    const double betaFactor = EQ::betaFactor(nonDarcyParameters->inertialCoefficientBeta0(),
                                             effectivePermeability,
                                             nonDarcyParameters->permeabilityScalingFactor(),
                                             porosity,
                                             nonDarcyParameters->porosityScalingFactor());

    const double alpha = RiaDefines::nonDarcyFlowAlpha(eclipseCaseData->unitsType());

    return EQ::dFactor(alpha,
                       betaFactor,
                       effectivePermeability,
                       effectiveH,
                       nonDarcyParameters->wellRadius(),
                       nonDarcyParameters->relativeGasDensity(),
                       nonDarcyParameters->gasViscosity());
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
std::pair<double, cvf::Vec2i>
    RicWellPathExportCompletionDataFeatureImpl::wellPathUpperGridIntersectionIJ(const RimEclipseCase* gridCase,
                                                                                const RimWellPath*    wellPath,
                                                                                const QString&        gridName)
{
    const RigEclipseCaseData*      caseData         = gridCase->eclipseCaseData();
    const RigMainGrid*             mainGrid         = caseData->mainGrid();
    const RigActiveCellInfo*       activeCellInfo   = caseData->activeCellInfo(RiaDefines::MATRIX_MODEL);
    const RigWellPath*             wellPathGeometry = wellPath->wellPathGeometry();
    const std::vector<cvf::Vec3d>& coords           = wellPathGeometry->wellPathPoints();
    const std::vector<double>&     mds              = wellPathGeometry->measureDepths();
    CVF_ASSERT(!coords.empty() && !mds.empty());

    std::vector<WellPathCellIntersectionInfo> intersections =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath(caseData, coords, mds);

    int gridId = 0;

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
                return std::make_pair(intersection.startMD, cvf::Vec2i((int)i, (int)j));
            }
        }
    }
    return std::make_pair(cvf::UNDEFINED_DOUBLE, cvf::Vec2i());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportWellSegments(RimEclipseCase*                          eclipseCase,
                                                                    QFilePtr                                 exportFile,
                                                                    const RimWellPath*                       wellPath,
                                                                    const std::vector<RimWellPathFracture*>& fractures)
{
    if (eclipseCase == nullptr)
    {
        RiaLogging::error("Export Fracture Well Segments: Cannot export completions data without specified eclipse case");
        return;
    }

    RicMswExportInfo exportInfo =
        RicWellPathExportCompletionDataFeatureImpl::generateFracturesMswExportInfo(eclipseCase, wellPath, fractures);

    QTextStream                  stream(exportFile.get());
    RifEclipseDataTableFormatter formatter(stream);
    RicWellPathExportCompletionDataFeatureImpl::generateWelsegsTable(formatter, exportInfo);
    RicWellPathExportCompletionDataFeatureImpl::generateCompsegTables(formatter, exportInfo);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportWellSegments(RimEclipseCase*                               eclipseCase,
                                                                    QFilePtr                                      exportFile,
                                                                    const RimWellPath*                            wellPath,
                                                                    const std::vector<RimFishbonesMultipleSubs*>& fishbonesSubs)
{
    if (eclipseCase == nullptr)
    {
        RiaLogging::error("Export Well Segments: Cannot export completions data without specified eclipse case");
        return;
    }

    RicMswExportInfo exportInfo =
        RicWellPathExportCompletionDataFeatureImpl::generateFishbonesMswExportInfo(eclipseCase, wellPath, fishbonesSubs, true);

    QTextStream                  stream(exportFile.get());
    RifEclipseDataTableFormatter formatter(stream);
    RicWellPathExportCompletionDataFeatureImpl::generateWelsegsTable(formatter, exportInfo);
    RicWellPathExportCompletionDataFeatureImpl::generateCompsegTables(formatter, exportInfo);
    RicWellPathExportCompletionDataFeatureImpl::generateWsegvalvTable(formatter, exportInfo);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportWellSegments(
    const RicExportCompletionDataSettingsUi&          exportSettings,
    QFilePtr                                          exportFile,
    const RimWellPath*                                wellPath,
    const std::vector<const RimPerforationInterval*>& perforationIntervals)
{
    if (exportSettings.caseToApply == nullptr)
    {
        RiaLogging::error("Export Well Segments: Cannot export completions data without specified eclipse case");
        return;
    }

    RicMswExportInfo exportInfo = RicWellPathExportCompletionDataFeatureImpl::generatePerforationsMswExportInfo(
        exportSettings, wellPath, perforationIntervals);

    QTextStream                  stream(exportFile.get());
    RifEclipseDataTableFormatter formatter(stream);
    RicWellPathExportCompletionDataFeatureImpl::generateWelsegsTable(formatter, exportInfo);
    RicWellPathExportCompletionDataFeatureImpl::generateCompsegTables(formatter, exportInfo);
    RicWellPathExportCompletionDataFeatureImpl::generateWsegvalvTable(formatter, exportInfo);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeatureImpl::exportCarfinForTemporaryLgrs(const RimEclipseCase* sourceCase,
                                                                              const QString&        folder)
{
    if (!sourceCase || !sourceCase->mainGrid()) return;

    const auto  mainGrid         = sourceCase->mainGrid();
    const auto& lgrInfosForWells = RicExportLgrFeature::createLgrInfoListForTemporaryLgrs(mainGrid);

    for (const auto& lgrInfoForWell : lgrInfosForWells)
    {
        RicExportLgrFeature::exportLgrs(folder, lgrInfoForWell.first, lgrInfoForWell.second);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellPathExportCompletionDataFeatureImpl::isCompletionWellPathEqual(const RigCompletionData& completion,
                                                                           const RimWellPath*       wellPath)
{
    if (!wellPath) return false;

    RimWellPath* parentWellPath = nullptr;
    if (completion.sourcePdmObject())
    {
        completion.sourcePdmObject()->firstAncestorOrThisOfType(parentWellPath);
    }

    return (parentWellPath == wellPath);
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<SubSegmentIntersectionInfo>
    spiltIntersectionSegmentsToMaxLength(const RigWellPath*                               pathGeometry,
                                         const std::vector<WellPathCellIntersectionInfo>& intersections,
                                         double                                           maxSegmentLength)
{
    std::vector<SubSegmentIntersectionInfo> out;

    if (!pathGeometry) return out;

    for (size_t i = 0; i < intersections.size(); i++)
    {
        const auto& intersection = intersections[i];
        double      segLen       = intersection.endMD - intersection.startMD;
        int         segCount     = (int)std::trunc(segLen / maxSegmentLength) + 1;

        // Calc effective max length
        double effectiveMaxSegLen = segLen / segCount;

        if (segCount == 1)
        {
            out.push_back(SubSegmentIntersectionInfo(intersection.globCellIndex,
                                                     -intersection.startPoint.z(),
                                                     -intersection.endPoint.z(),
                                                     intersection.startMD,
                                                     intersection.endMD,
                                                     intersection.intersectionLengthsInCellCS));
        }
        else
        {
            double currStartMd = intersection.startMD;
            double currEndMd   = currStartMd;
            double lastTvd     = -intersection.startPoint.z();

            for (int segIndex = 0; segIndex < segCount; segIndex++)
            {
                bool lasti = segIndex == (segCount - 1);
                currEndMd  = currStartMd + effectiveMaxSegLen;

                cvf::Vec3d segEndPoint = pathGeometry->interpolatedPointAlongWellPath(currEndMd);
                out.push_back(SubSegmentIntersectionInfo(intersection.globCellIndex,
                                                         lastTvd,
                                                         lasti ? -intersection.endPoint.z() : -segEndPoint.z(),
                                                         currStartMd,
                                                         lasti ? intersection.endMD : currEndMd,
                                                         intersection.intersectionLengthsInCellCS / segCount));

                currStartMd = currEndMd;
                lastTvd     = -segEndPoint.z();
            }
        }
    }
    return out;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int numberOfSplittedSegments(double startMd, double endMd, double maxSegmentLength)
{
    return (int)(std::trunc((endMd - startMd) / maxSegmentLength) + 1);
}

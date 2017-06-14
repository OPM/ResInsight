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

#include "RicWellPathExportCompletionDataFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimFishbonesCollection.h"
#include "RimFishboneWellPath.h"
#include "RimFishboneWellPathCollection.h"
#include "RimPerforationInterval.h"
#include "RimPerforationCollection.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"

#include "RicExportCompletionDataSettingsUi.h"

#include "RiuMainWindow.h"

#include "RigWellLogExtractionTools.h"
#include "RigWellPathIntersectionTools.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"
#include "RigResultAccessorFactory.h"
#include "RigTransmissibilityEquations.h"

#include "cafSelectionManager.h"
#include "cafPdmUiPropertyViewDialog.h"

#include "cvfPlane.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include "RicExportFractureCompletionsImpl.h"

CAF_CMD_SOURCE_INIT(RicWellPathExportCompletionDataFeature, "RicWellPathExportCompletionDataFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellPathExportCompletionDataFeature::isCommandEnabled()
{
    std::vector<RimWellPath*> wellPaths = selectedWellPaths();
    std::vector<RimEclipseWell*> simWells = selectedSimWells();

    if (wellPaths.empty() && simWells.empty())
    {
        return false;
    }

    if (!wellPaths.empty() && !simWells.empty())
    {
        return false;
    }

    std::set<RimEclipseCase*> eclipseCases;
    for (auto simWell : simWells)
    {
        RimEclipseCase* eclipseCase;
        simWell->firstAncestorOrThisOfType(eclipseCase);
        eclipseCases.insert(eclipseCase);
    }
    if (eclipseCases.size() > 1)
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*> wellPaths = selectedWellPaths();
    std::vector<RimEclipseWell*> simWells = selectedSimWells();

    CVF_ASSERT(wellPaths.size() > 0 || simWells.size() > 0);

    RiaApplication* app = RiaApplication::instance();

    QString projectFolder = app->currentProjectPath();
    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("COMPLETIONS", projectFolder);

    RicExportCompletionDataSettingsUi exportSettings;

    if (wellPaths.empty())
    {
        exportSettings.showForSimWells();
    }
    else
    {
        exportSettings.showForWellPath();
    }

    std::vector<RimCase*> cases;
    app->project()->allCases(cases);
    for (auto c : cases)
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(c);
        if (eclipseCase != nullptr)
        {
            exportSettings.caseToApply = eclipseCase;
            break;
        }
    }

    exportSettings.fileName = QDir(defaultDir).filePath("Completions");

    caf::PdmUiPropertyViewDialog propertyDialog(RiuMainWindow::instance(), &exportSettings, "Export Completion Data", "");
    if (propertyDialog.exec() == QDialog::Accepted)
    {
        RiaApplication::instance()->setLastUsedDialogDirectory("COMPLETIONS", QFileInfo(exportSettings.fileName).absolutePath());

        exportCompletions(wellPaths, simWells, exportSettings);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Completion Data");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicWellPathExportCompletionDataFeature::selectedWellPaths()
{
    std::vector<RimWellPath*> wellPaths;
    caf::SelectionManager::instance()->objectsByType(&wellPaths);

    std::vector<RimWellPathCollection*> wellPathCollections;
    caf::SelectionManager::instance()->objectsByType(&wellPathCollections);

    for (auto wellPathCollection : wellPathCollections)
    {
        for (auto wellPath : wellPathCollection->wellPaths())
        {
            wellPaths.push_back(wellPath);
        }
    }

    std::set<RimWellPath*> uniqueWellPaths(wellPaths.begin(), wellPaths.end());
    wellPaths.assign(uniqueWellPaths.begin(), uniqueWellPaths.end());
    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseWell*> RicWellPathExportCompletionDataFeature::selectedSimWells()
{
    std::vector<RimEclipseWell*> simWells;
    caf::SelectionManager::instance()->objectsByType(&simWells);

    std::vector<RimEclipseWellCollection*> simWellCollections;
    caf::SelectionManager::instance()->objectsByType(&simWellCollections);

    for (auto simWellCollection : simWellCollections)
    {
        for (auto simWell : simWellCollection->wells())
        {
            simWells.push_back(simWell);
        }
    }

    std::set<RimEclipseWell*> uniqueSimWells(simWells.begin(), simWells.end());
    simWells.assign(uniqueSimWells.begin(), uniqueSimWells.end());

    return simWells;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::exportCompletions(const std::vector<RimWellPath*>& wellPaths, const std::vector<RimEclipseWell*>& simWells, const RicExportCompletionDataSettingsUi& exportSettings)
{
    QFile exportFile(exportSettings.fileName());

    if (exportSettings.caseToApply() == nullptr)
    {
        RiaLogging::error("Export Completions Data: Cannot export completions data without specified eclipse case");
        return;
    }

    if (!exportFile.open(QIODevice::WriteOnly))
    {
        RiaLogging::error(QString("Export Completions Data: Could not open the file: %1").arg(exportSettings.fileName()));
        return;
    }

    {
        bool unitSystemMismatch = false;
        for (const RimWellPath* wellPath : wellPaths)
        {
            if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD && exportSettings.caseToApply->eclipseCaseData()->unitsType() != RigEclipseCaseData::UNITS_FIELD)
            {
                unitSystemMismatch = true;
                break;
            }
            else if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC && exportSettings.caseToApply->eclipseCaseData()->unitsType() != RigEclipseCaseData::UNITS_METRIC)
            {
                unitSystemMismatch = true;
                break;
            }
        }
        for (const RimEclipseWell* simWell : simWells)
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

    QTextStream stream(&exportFile);
    RifEclipseDataTableFormatter formatter(stream);

    std::map<IJKCellIndex, RigCompletionData> completionData;

    for (auto wellPath : wellPaths)
    {
        // Generate completion data

        if (exportSettings.includePerforations)
        {
            std::vector<RigCompletionData> perforationCompletionData = generatePerforationsCompdatValues(wellPath, exportSettings);
            appendCompletionData(&completionData, perforationCompletionData);
        }
        if (exportSettings.includeFishbones)
        {
            std::vector<RigCompletionData> fishbonesCompletionData = generateFishboneLateralsCompdatValues(wellPath, exportSettings);
            appendCompletionData(&completionData, fishbonesCompletionData);
            std::vector<RigCompletionData> fishbonesWellPathCompletionData = generateFishbonesImportedLateralsCompdatValues(wellPath, exportSettings);
            appendCompletionData(&completionData, fishbonesWellPathCompletionData);
        }

        if (exportSettings.includeFractures())
        {
            std::vector<RigCompletionData> fractureCompletionData = RicExportFractureCompletionsImpl::generateCompdatValuesForWellPath(wellPath, exportSettings, &stream);
            appendCompletionData(&completionData, fractureCompletionData);
        }

    }

    for (auto simWell : simWells)
    {
        std::vector<RigCompletionData> fractureCompletionData = RicExportFractureCompletionsImpl::generateCompdatValuesForSimWell(exportSettings.caseToApply(), simWell, exportSettings.timeStep(), &stream);
        appendCompletionData(&completionData, fractureCompletionData);
    }

    // Merge map into a vector of values
    std::vector<RigCompletionData> completions;
    for (auto& data : completionData)
    {
        completions.push_back(data.second);
    }
    // Sort by well name / cell index
    std::sort(completions.begin(), completions.end());
    // Print completion data
    generateCompdatTable(formatter, completions);
    if (exportSettings.includeWpimult)
    {
        generateWpimultTable(formatter, completions);
    }

    RiaLogging::info(QString("Successfully exported completion data to %1").arg(exportSettings.fileName()));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::generateCompdatTable(RifEclipseDataTableFormatter& formatter, const std::vector<RigCompletionData>& completionData)
{
    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn("Well"),
        RifEclipseOutputTableColumn("I"),
        RifEclipseOutputTableColumn("J"),
        RifEclipseOutputTableColumn("K1"),
        RifEclipseOutputTableColumn("K2"),
        RifEclipseOutputTableColumn("Status"),
        RifEclipseOutputTableColumn("SAT"),
        RifEclipseOutputTableColumn("TR"),
        RifEclipseOutputTableColumn("DIAM"),
        RifEclipseOutputTableColumn("KH"),
        RifEclipseOutputTableColumn("S"),
        RifEclipseOutputTableColumn("Df"),
        RifEclipseOutputTableColumn("DIR"),
        RifEclipseOutputTableColumn("r0")
    };

    formatter.keyword("COMPDAT");
    formatter.header(header);

    for (const RigCompletionData& data : completionData)
    {
        for (const RigCompletionMetaData& metadata : data.metadata())
        {
            formatter.comment(QString("%1 : %2").arg(metadata.name).arg(metadata.comment));
        }
        formatter.add(data.wellName());
        formatter.addZeroBasedCellIndex(data.cellIndex().i).addZeroBasedCellIndex(data.cellIndex().j).addZeroBasedCellIndex(data.cellIndex().k).addZeroBasedCellIndex(data.cellIndex().k);
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
        if (RigCompletionData::isDefaultValue(data.saturation())) formatter.add("1*"); else formatter.add(data.saturation());
        if (RigCompletionData::isDefaultValue(data.transmissibility()))
        {
            formatter.add("1*"); // Transmissibility

            if (RigCompletionData::isDefaultValue(data.diameter())) formatter.add("1*"); else formatter.add(data.diameter());
            if (RigCompletionData::isDefaultValue(data.kh())) formatter.add("1*"); else formatter.add(data.kh());
            if (RigCompletionData::isDefaultValue(data.skinFactor())) formatter.add("1*"); else formatter.add(data.skinFactor());
            if (RigCompletionData::isDefaultValue(data.dFactor())) formatter.add("1*"); else formatter.add(data.dFactor());

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
        }

        formatter.rowCompleted();
    }
    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::generateWpimultTable(RifEclipseDataTableFormatter& formatter, const std::vector<RigCompletionData>& completionData)
{
    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn("Well"),
        RifEclipseOutputTableColumn("Mult"),
        RifEclipseOutputTableColumn("I"),
        RifEclipseOutputTableColumn("J"),
        RifEclipseOutputTableColumn("K"),
    };
    formatter.keyword("WPIMULT");
    formatter.header(header);

    for (auto& completion : completionData)
    {
        formatter.add(completion.wellName());
        formatter.add(completion.count());
        formatter.addZeroBasedCellIndex(completion.cellIndex().i).addZeroBasedCellIndex(completion.cellIndex().j).addZeroBasedCellIndex(completion.cellIndex().k);
        formatter.rowCompleted();
    }

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicWellPathExportCompletionDataFeature::generateFishboneLateralsCompdatValues(const RimWellPath* wellPath, const RicExportCompletionDataSettingsUi& settings)
{
    // Generate data
    const RigEclipseCaseData* caseData =  settings.caseToApply()->eclipseCaseData();
    std::vector<WellSegmentLocation> locations = findWellSegmentLocations(settings.caseToApply, wellPath);

    // Filter out cells where main bore is present
    if (settings.removeLateralsInMainBoreCells())
    {
        std::vector<size_t> wellPathCells = findIntersectingCells(caseData, wellPath->wellPathGeometry()->m_wellPathPoints);
        markWellPathCells(wellPathCells, &locations);
    }

    RigMainGrid* grid = settings.caseToApply->eclipseCaseData()->mainGrid();

    std::vector<RigCompletionData> completionData;

    for (const WellSegmentLocation& location : locations)
    {
        for (const WellSegmentLateral& lateral : location.laterals)
        {
            for (const WellSegmentLateralIntersection& intersection : lateral.intersections)
            {
                if (intersection.mainBoreCell && settings.removeLateralsInMainBoreCells()) continue;

                size_t i, j, k;
                grid->ijkFromCellIndex(intersection.cellIndex, &i, &j, &k);
                RigCompletionData completion(wellPath->name(), IJKCellIndex(i, j, k));
                completion.addMetadata(location.fishbonesSubs->name(), QString("Sub: %1 Lateral: %2").arg(location.subIndex).arg(lateral.lateralIndex));
                double diameter = location.fishbonesSubs->holeDiameter() / 1000;
                if (settings.computeTransmissibility())
                {
                    double transmissibility = calculateTransmissibility(settings.caseToApply,
                                                                        wellPath,
                                                                        intersection.lengthsInCell,
                                                                        location.fishbonesSubs->skinFactor(),
                                                                        diameter / 2,
                                                                        intersection.cellIndex);
                    completion.setFromFishbone(transmissibility, location.fishbonesSubs->skinFactor());
                }
                else {
                    CellDirection direction = calculateDirectionInCell(settings.caseToApply, intersection.cellIndex, intersection.lengthsInCell);
                    completion.setFromFishbone(diameter, direction);
                }
                completionData.push_back(completion);
            }
        }
    }

    return completionData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicWellPathExportCompletionDataFeature::generateFishbonesImportedLateralsCompdatValues(const RimWellPath* wellPath, const RicExportCompletionDataSettingsUi& settings)
{
    std::vector<RigCompletionData> completionData;

    std::vector<size_t> wellPathCells = findIntersectingCells(settings.caseToApply()->eclipseCaseData(), wellPath->wellPathGeometry()->m_wellPathPoints);

    double diameter = wellPath->fishbonesCollection()->wellPathCollection()->holeDiameter() / 1000;
    for (const RimFishboneWellPath* fishbonesPath : wellPath->fishbonesCollection()->wellPathCollection()->wellPaths())
    {
        std::vector<WellPathCellIntersectionInfo> intersectedCells = RigWellPathIntersectionTools::findCellsIntersectedByPath(settings.caseToApply->eclipseCaseData(), fishbonesPath->coordinates());
        for (auto& cell : intersectedCells)
        {
            if (std::find(wellPathCells.begin(), wellPathCells.end(), cell.cellIndex) != wellPathCells.end()) continue;

            size_t i, j, k;
            settings.caseToApply->eclipseCaseData()->mainGrid()->ijkFromCellIndex(cell.cellIndex, &i, &j, &k);
            RigCompletionData completion(wellPath->name(), IJKCellIndex(i, j, k));
            completion.addMetadata(fishbonesPath->name(), "");
            if (settings.computeTransmissibility())
            {
                double skinFactor = wellPath->fishbonesCollection()->wellPathCollection()->skinFactor();
                double transmissibility = calculateTransmissibility(settings.caseToApply(),
                                                                    wellPath,
                                                                    cell.internalCellLengths,
                                                                    skinFactor,
                                                                    diameter / 2,
                                                                    cell.cellIndex);
                completion.setFromFishbone(transmissibility, skinFactor);
            }
            else {
                CellDirection direction = calculateDirectionInCell(settings.caseToApply, cell.cellIndex, cell.internalCellLengths);
                completion.setFromFishbone(diameter, direction);
            }
            completionData.push_back(completion);
        }
    }

    return completionData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicWellPathExportCompletionDataFeature::generatePerforationsCompdatValues(const RimWellPath* wellPath, const RicExportCompletionDataSettingsUi& settings)
{
    std::vector<RigCompletionData> completionData;

    for (const RimPerforationInterval* interval : wellPath->perforationIntervalCollection()->perforations())
    {
        if (!interval->isActiveOnDate(settings.caseToApply->timeStepDates()[settings.timeStep])) continue;

        std::vector<cvf::Vec3d> perforationPoints = wellPath->wellPathGeometry()->clippedPointSubset(interval->startMD(), interval->endMD());
        std::vector<WellPathCellIntersectionInfo> intersectedCells = RigWellPathIntersectionTools::findCellsIntersectedByPath(settings.caseToApply->eclipseCaseData(), perforationPoints);
        for (auto& cell : intersectedCells)
        {
            size_t i, j, k;
            settings.caseToApply->eclipseCaseData()->mainGrid()->ijkFromCellIndex(cell.cellIndex, &i, &j, &k);
            RigCompletionData completion(wellPath->name(), IJKCellIndex(i, j, k));
            completion.addMetadata("Perforation", QString("StartMD: %1 - EndMD: %2").arg(interval->startMD()).arg(interval->endMD()));
            double diameter = interval->diameter();
            CellDirection direction = calculateDirectionInCell(settings.caseToApply, cell.cellIndex, cell.internalCellLengths);
            completion.setFromPerforation(diameter, direction);
            completionData.push_back(completion);
        }
    }

    return completionData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RicWellPathExportCompletionDataFeature::findIntersectingCells(const RigEclipseCaseData* caseData, const std::vector<cvf::Vec3d>& coords)
{
    std::set<size_t> cells;

    std::vector<HexIntersectionInfo> intersections = RigWellPathIntersectionTools::getIntersectedCells(caseData->mainGrid(), coords);
    for (auto intersection : intersections)
    {
        cells.insert(intersection.m_hexIndex);
    }

    // Ensure only unique cells are included
    std::vector<size_t> cellsVector;
    cellsVector.assign(cells.begin(), cells.end());
    // Sort cells
    std::sort(cellsVector.begin(), cellsVector.end());
    return cellsVector;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::markWellPathCells(const std::vector<size_t>& wellPathCells, std::vector<WellSegmentLocation>* locations)
{
    std::set<size_t> wellPathCellSet(wellPathCells.begin(), wellPathCells.end());
    for (WellSegmentLocation& location : *locations)
    {
        for (WellSegmentLateral& lateral : location.laterals)
        {
            for (WellSegmentLateralIntersection& intersection : lateral.intersections)
            {
                if (wellPathCellSet.find(intersection.cellIndex) != wellPathCellSet.end())
                {
                    intersection.mainBoreCell = true;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellPathExportCompletionDataFeature::wellSegmentLocationOrdering(const WellSegmentLocation& first, const WellSegmentLocation& second)
{
    return first.measuredDepth < second.measuredDepth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellPathExportCompletionDataFeature::isPointBetween(const cvf::Vec3d& pointA, const cvf::Vec3d& pointB, const cvf::Vec3d& needle)
{
    cvf::Plane plane;
    plane.setFromPointAndNormal(needle, pointB - pointA);
    return plane.side(pointA) != plane.side(pointB);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<WellSegmentLocation> RicWellPathExportCompletionDataFeature::findWellSegmentLocations(const RimEclipseCase* caseToApply, const RimWellPath* wellPath)
{
    std::vector<RimFishbonesMultipleSubs*> fishbonesSubs;
    for (RimFishbonesMultipleSubs* subs : wellPath->fishbonesCollection()->fishbonesSubs())
    {
        fishbonesSubs.push_back(subs);
    }
    return findWellSegmentLocations(caseToApply, wellPath, fishbonesSubs);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<WellSegmentLocation> RicWellPathExportCompletionDataFeature::findWellSegmentLocations(const RimEclipseCase* caseToApply, const RimWellPath* wellPath, const std::vector<RimFishbonesMultipleSubs*>& fishbonesSubs)
{
    std::vector<WellSegmentLocation> wellSegmentLocations;
    for (RimFishbonesMultipleSubs* subs : fishbonesSubs)
    {
        for (auto& sub : subs->installedLateralIndices())
        {
            double measuredDepth = subs->measuredDepth(sub.subIndex);
            cvf::Vec3d position = wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(measuredDepth);
            WellSegmentLocation location = WellSegmentLocation(subs, measuredDepth, -position.z(), sub.subIndex);
            for (size_t lateralIndex : sub.lateralIndices)
            {
                location.laterals.push_back(WellSegmentLateral(lateralIndex));
            }
            wellSegmentLocations.push_back(location);
        }
    }
    std::sort(wellSegmentLocations.begin(), wellSegmentLocations.end(), wellSegmentLocationOrdering);

    assignBranchAndSegmentNumbers(caseToApply, &wellSegmentLocations);

    return wellSegmentLocations;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::calculateLateralIntersections(const RimEclipseCase* caseToApply, WellSegmentLocation* location, int* branchNum, int* segmentNum)
{
    for (WellSegmentLateral& lateral : location->laterals)
    {
        lateral.branchNumber = ++(*branchNum);
        std::vector<cvf::Vec3d> coords = location->fishbonesSubs->coordsForLateral(location->subIndex, lateral.lateralIndex);
        std::vector<WellPathCellIntersectionInfo> intersections = RigWellPathIntersectionTools::findCellsIntersectedByPath(caseToApply->eclipseCaseData(), coords);

        auto intersection = intersections.cbegin();
        double length = 0;
        double depth = 0;
        cvf::Vec3d startPoint = coords[0];
        int attachedSegmentNumber = location->icdSegmentNumber;

        for (size_t i = 1; i < coords.size() && intersection != intersections.cend(); ++i)
        {
            if (isPointBetween(startPoint, coords[i], intersection->endPoint))
            {
                length += (intersection->endPoint - startPoint).length();
                depth += intersection->endPoint.z() - startPoint.z();

                WellSegmentLateralIntersection lateralIntersection(++(*segmentNum), attachedSegmentNumber, intersection->cellIndex, length, depth);
                lateralIntersection.lengthsInCell = intersection->internalCellLengths;
                lateral.intersections.push_back(lateralIntersection);

                length = 0;
                depth = 0;
                startPoint = intersection->endPoint;
                attachedSegmentNumber = *segmentNum;
                ++intersection;
            }
            else
            {
                length += (coords[i] - startPoint).length();
                depth += coords[i].z() - startPoint.z();
                startPoint = coords[i];
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::assignBranchAndSegmentNumbers(const RimEclipseCase* caseToApply, std::vector<WellSegmentLocation>* locations)
{
    int segmentNumber = 1;
    int branchNumber = 1;
    // First loop over the locations so that each segment on the main stem is an incremental number
    for (WellSegmentLocation& location : *locations)
    {
        location.segmentNumber = ++segmentNumber;
        location.icdBranchNumber = ++branchNumber;
        location.icdSegmentNumber = ++segmentNumber;
    }
    // Then assign branch and segment numbers to each lateral parts
    for (WellSegmentLocation& location : *locations)
    {
        calculateLateralIntersections(caseToApply, &location, &branchNumber, &segmentNumber);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::appendCompletionData(std::map<IJKCellIndex, RigCompletionData>* completionData, const std::vector<RigCompletionData>& data)
{
    for (auto& completion : data)
    {
        auto it = completionData->find(completion.cellIndex());
        if (it != completionData->end())
        {
            it->second = RigCompletionData::combine(it->second, completion);
        }
        else
        {
            completionData->insert(std::pair<IJKCellIndex, RigCompletionData>(completion.cellIndex(), completion));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CellDirection RicWellPathExportCompletionDataFeature::calculateDirectionInCell(RimEclipseCase* eclipseCase, size_t cellIndex, const cvf::Vec3d& lengthsInCell)
{
    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DX");
    cvf::ref<RigResultAccessor> dxAccessObject = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RifReaderInterface::MATRIX_RESULTS, 0, "DX");
    eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DY");
    cvf::ref<RigResultAccessor> dyAccessObject = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RifReaderInterface::MATRIX_RESULTS, 0, "DY");
    eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DZ");
    cvf::ref<RigResultAccessor> dzAccessObject = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RifReaderInterface::MATRIX_RESULTS, 0, "DZ");

    double xLengthFraction = abs(lengthsInCell.x() / dxAccessObject->cellScalarGlobIdx(cellIndex));
    double yLengthFraction = abs(lengthsInCell.y() / dyAccessObject->cellScalarGlobIdx(cellIndex));
    double zLengthFraction = abs(lengthsInCell.z() / dzAccessObject->cellScalarGlobIdx(cellIndex));

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
double RicWellPathExportCompletionDataFeature::calculateTransmissibility(RimEclipseCase* eclipseCase, const RimWellPath* wellPath, const cvf::Vec3d& internalCellLengths, double skinFactor, double wellRadius, size_t cellIndex)
{
    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DX");
    cvf::ref<RigResultAccessor> dxAccessObject = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RifReaderInterface::MATRIX_RESULTS, 0, "DX");
    eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DY");
    cvf::ref<RigResultAccessor> dyAccessObject = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RifReaderInterface::MATRIX_RESULTS, 0, "DY");
    eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "DZ");
    cvf::ref<RigResultAccessor> dzAccessObject = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RifReaderInterface::MATRIX_RESULTS, 0, "DZ");

    eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PERMX");
    cvf::ref<RigResultAccessor> permxAccessObject = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RifReaderInterface::MATRIX_RESULTS, 0, "PERMX");
    eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PERMY");
    cvf::ref<RigResultAccessor> permyAccessObject = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RifReaderInterface::MATRIX_RESULTS, 0, "PERMY");
    eclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PERMZ");
    cvf::ref<RigResultAccessor> permzAccessObject = RigResultAccessorFactory::createFromUiResultName(eclipseCaseData, 0, RifReaderInterface::MATRIX_RESULTS, 0, "PERMZ");

    double dx = dxAccessObject->cellScalarGlobIdx(cellIndex);
    double dy = dyAccessObject->cellScalarGlobIdx(cellIndex);
    double dz = dzAccessObject->cellScalarGlobIdx(cellIndex);
    double permx = permxAccessObject->cellScalarGlobIdx(cellIndex);
    double permy = permxAccessObject->cellScalarGlobIdx(cellIndex);
    double permz = permxAccessObject->cellScalarGlobIdx(cellIndex);

    double darcy = RiaEclipseUnitTools::darcysConstant(wellPath->unitSystem());

    double transx = RigTransmissibilityEquations::wellBoreTransmissibilityComponent(internalCellLengths.x(), permy, permz, dy, dz, wellRadius, skinFactor, darcy);
    double transy = RigTransmissibilityEquations::wellBoreTransmissibilityComponent(internalCellLengths.y(), permx, permz, dx, dz, wellRadius, skinFactor, darcy);
    double transz = RigTransmissibilityEquations::wellBoreTransmissibilityComponent(internalCellLengths.z(), permy, permx, dy, dx, wellRadius, skinFactor, darcy);

    return RigTransmissibilityEquations::totalConnectionFactor(transx, transy, transz);
}

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
#include "RimExportCompletionDataSettings.h"

#include "RiuMainWindow.h"

#include "RigWellLogExtractionTools.h"
#include "RigWellPathIntersectionTools.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"

#include "cafSelectionManager.h"
#include "cafPdmUiPropertyViewDialog.h"

#include "cvfPlane.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT(RicWellPathExportCompletionDataFeature, "RicWellPathExportCompletionDataFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellPathExportCompletionDataFeature::isCommandEnabled()
{
    return !selectedWellPaths().empty();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*> wellPaths = selectedWellPaths();

    CVF_ASSERT(wellPaths.size() > 0);

    RiaApplication* app = RiaApplication::instance();

    QString projectFolder = app->currentProjectPath();
    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("COMPLETIONS", projectFolder);

    RimExportCompletionDataSettings exportSettings;
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

        exportCompletions(wellPaths, exportSettings);
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
void RicWellPathExportCompletionDataFeature::exportCompletions(const std::vector<RimWellPath*>& wellPaths, const RimExportCompletionDataSettings& exportSettings)
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

    QTextStream stream(&exportFile);
    RifEclipseOutputTableFormatter formatter(stream);

    for (auto wellPath : wellPaths)
    {
        // Generate completion data
        std::map<IJKCellIndex, RigCompletionData> completionData;

        if (exportSettings.includePerforations)
        {
            std::vector<RigCompletionData> perforationCompletionData = generatePerforationsCompdatValues(wellPath, exportSettings);
            appendCompletionData(&completionData, perforationCompletionData);
        }
        if (exportSettings.includeFishbones)
        {
            std::vector<RigCompletionData> fishbonesCompletionData = generateFishbonesCompdatValues(wellPath, exportSettings);
            appendCompletionData(&completionData, fishbonesCompletionData);
            std::vector<RigCompletionData> fishbonesWellPathCompletionData = generateFishbonesWellPathCompdatValues(wellPath, exportSettings);
            appendCompletionData(&completionData, fishbonesWellPathCompletionData);
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
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::generateCompdatTable(RifEclipseOutputTableFormatter& formatter, const std::vector<RigCompletionData>& completionData)
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
void RicWellPathExportCompletionDataFeature::generateWpimultTable(RifEclipseOutputTableFormatter& formatter, const std::vector<RigCompletionData>& completionData)
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
std::vector<RigCompletionData> RicWellPathExportCompletionDataFeature::generateFishbonesCompdatValues(const RimWellPath* wellPath, const RimExportCompletionDataSettings& settings)
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
                double diameter = location.fishbonesSubs->holeRadius() / 1000 * 2;
                CellDirection direction = wellPathCellDirectionToCellDirection(completion.direction);
                completion.setFromFishbone(diameter, direction);
                completionData.push_back(completion);
            }
        }
    }

    return completionData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicWellPathExportCompletionDataFeature::generateFishbonesWellPathCompdatValues(const RimWellPath* wellPath, const RimExportCompletionDataSettings & settings)
{
    std::vector<RigCompletionData> completionData;

    double diameter = wellPath->fishbonesCollection()->wellPathCollection()->holeRadius() / 1000 * 2;
    for (const RimFishboneWellPath* fishbonesPath : wellPath->fishbonesCollection()->wellPathCollection()->wellPaths())
    {
        std::vector<WellPathCellIntersectionInfo> intersectedCells = RigWellPathIntersectionTools::findCellsIntersectedByPath(settings.caseToApply->eclipseCaseData(), fishbonesPath->coordinates());
        for (auto& cell : intersectedCells)
        {
            size_t i, j, k;
            settings.caseToApply->eclipseCaseData()->mainGrid()->ijkFromCellIndex(cell.cellIndex, &i, &j, &k);
            RigCompletionData completion(wellPath->name(), IJKCellIndex(i, j, k));
            completion.addMetadata(fishbonesPath->name(), "");
            CellDirection direction = wellPathCellDirectionToCellDirection(cell.direction);
            completion.setFromFishbone(diameter, direction);
            completionData.push_back(completion);
        }
    }

    return completionData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicWellPathExportCompletionDataFeature::generatePerforationsCompdatValues(const RimWellPath* wellPath, const RimExportCompletionDataSettings& settings)
{
    std::vector<RigCompletionData> completionData;

    for (const RimPerforationInterval* interval : wellPath->perforationIntervalCollection()->perforations())
    {
        std::vector<cvf::Vec3d> perforationPoints = wellPath->wellPathGeometry()->clippedPointSubset(interval->startMD(), interval->endMD());
        std::vector<WellPathCellIntersectionInfo> intersectedCells = RigWellPathIntersectionTools::findCellsIntersectedByPath(settings.caseToApply->eclipseCaseData(), perforationPoints);
        for (auto& cell : intersectedCells)
        {
            size_t i, j, k;
            settings.caseToApply->eclipseCaseData()->mainGrid()->ijkFromCellIndex(cell.cellIndex, &i, &j, &k);
            RigCompletionData completion(wellPath->name(), IJKCellIndex(i, j, k));
            completion.addMetadata("Perforation", QString("StartMD: %1 - EndMD: %2").arg(interval->startMD()).arg(interval->endMD()));
            double diameter = interval->radius() * 2;
            CellDirection direction = wellPathCellDirectionToCellDirection(cell.direction);
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
    const std::vector<cvf::Vec3d>& nodeCoords = caseData->mainGrid()->nodes();
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
std::map<size_t, double> RicWellPathExportCompletionDataFeature::computeLateralsPerCell(const std::vector<WellSegmentLocation>& segmentLocations, bool removeMainBoreCells)
{
    std::map<size_t, double> lateralsPerCell;
    for (const WellSegmentLocation& location : segmentLocations)
    {
        for (const WellSegmentLateral& lateral : location.laterals)
        {
            for (const WellSegmentLateralIntersection& intersection : lateral.intersections)
            {
                if (removeMainBoreCells && intersection.mainBoreCell) continue;

                auto match = lateralsPerCell.find(intersection.cellIndex);
                if (match == lateralsPerCell.end())
                {
                    lateralsPerCell[intersection.cellIndex] = 1;
                }
                else
                {
                    lateralsPerCell[intersection.cellIndex] = match->second + 1;
                }
            }
        }
    }
    return lateralsPerCell;
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
        for (size_t subIndex = 0; subIndex < subs->locationOfSubs().size(); ++subIndex)
        {
            double measuredDepth = subs->locationOfSubs()[subIndex];
            cvf::Vec3d position = wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(measuredDepth);
            WellSegmentLocation location = WellSegmentLocation(subs, measuredDepth, -position.z(), subIndex);
            for (size_t lateralIndex = 0; lateralIndex < subs->lateralLengths().size(); ++lateralIndex)
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
        int attachedSegmentNumber = location->segmentNumber;

        for (size_t i = 1; i < coords.size() && intersection != intersections.cend(); ++i)
        {
            if (isPointBetween(startPoint, coords[i], intersection->endPoint))
            {
                length += (intersection->endPoint - startPoint).length();
                depth += intersection->endPoint.z() - startPoint.z();

                WellSegmentLateralIntersection lateralIntersection(++(*segmentNum), attachedSegmentNumber, intersection->cellIndex, length, depth);
                lateralIntersection.direction = intersection->direction;
                lateral.intersections.push_back(lateralIntersection);

                length = 0;
                depth = 0;
                startPoint = intersection->startPoint;
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
            it->second = it->second.combine(completion);
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
CellDirection RicWellPathExportCompletionDataFeature::wellPathCellDirectionToCellDirection(WellPathCellDirection direction)
{
    switch (direction)
    {
    case POS_I:
    case NEG_I:
        CellDirection::DIR_I;
        break;
    case POS_J:
    case NEG_J:
        CellDirection::DIR_J;
        break;
    case POS_K:
    case NEG_K:
        CellDirection::DIR_K;
        break;
    default:
        CellDirection::DIR_UNDEF;
        break;
    }
}

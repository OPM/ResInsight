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
#include "RimFishbonesMultipleSubs.h"
#include "RimExportCompletionDataSettings.h"

#include "RiuMainWindow.h"

#include "RifEclipseOutputTableFormatter.h"

#include "RigWellLogExtractionTools.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"

#include "cafSelectionManager.h"
#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT(RicWellPathExportCompletionDataFeature, "RicWellPathExportCompletionDataFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellPathExportCompletionDataFeature::isCommandEnabled()
{
    std::vector<RimWellPath*> objects;
    caf::SelectionManager::instance()->objectsByType(&objects);

    if (objects.size() == 1) {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*> objects;
    caf::SelectionManager::instance()->objectsByType(&objects);

    CVF_ASSERT(objects.size() == 1);

    RiaApplication* app = RiaApplication::instance();

    QString projectFolder = app->currentProjectPath();
    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("COMPLETIONS", projectFolder);

    RimExportCompletionDataSettings exportSettings;

    exportSettings.fileName = QDir(defaultDir).filePath("Completions");

    RimEclipseCase* caseToApply;
    objects[0]->firstAncestorOrThisOfType(caseToApply);
    exportSettings.caseToApply = caseToApply;

    caf::PdmUiPropertyViewDialog propertyDialog(RiuMainWindow::instance(), &exportSettings, "Export Completion Data", "");
    if (propertyDialog.exec() == QDialog::Accepted)
    {
        RiaApplication::instance()->setLastUsedDialogDirectory("COMPLETIONS", QFileInfo(exportSettings.fileName).absolutePath());

        exportToFolder(objects[0], exportSettings.fileName, exportSettings.caseToApply, exportSettings.includeWpimult());
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
void RicWellPathExportCompletionDataFeature::exportToFolder(RimWellPath* wellPath, const QString& fileName, const RimEclipseCase* caseToApply, bool includeWpimult)
{
    QFile exportFile(fileName);

    if (caseToApply == nullptr)
    {
        RiaLogging::error("Export Completions Data: Cannot export completions data without specified eclipse case");
        return;
    }
    
    if (!exportFile.open(QIODevice::WriteOnly))
    {
        RiaLogging::error(QString("Export Completions Data: Could not open the file: %1").arg(fileName));
        return;
    }

    QTextStream stream(&exportFile);

    const RigEclipseCaseData* caseData = caseToApply->eclipseCaseData();
    std::vector<size_t> wellPathCells = findIntersectingCells(caseData, wellPath->wellPathGeometry()->m_wellPathPoints);
    std::map<size_t, double> lateralsPerCell;

    RifEclipseOutputTableFormatter formatter(stream);

    // COMPDAT
    {
        std::vector<RifEclipseOutputTableColumn> header = {
                   RifEclipseOutputTableColumn{"Well",   LEFT},
                   RifEclipseOutputTableColumn{"I",      LEFT},
                   RifEclipseOutputTableColumn{"J",      LEFT},
                   RifEclipseOutputTableColumn{"K1",     LEFT},
                   RifEclipseOutputTableColumn{"K2",     LEFT},
                   RifEclipseOutputTableColumn{"Status", LEFT},
                   RifEclipseOutputTableColumn{"SAT",    LEFT},
                   RifEclipseOutputTableColumn{"TR",     LEFT},
                   RifEclipseOutputTableColumn{"DIAM",   LEFT},
                   RifEclipseOutputTableColumn{"KH",     LEFT},
                   RifEclipseOutputTableColumn{"S",      LEFT},
                   RifEclipseOutputTableColumn{"Df",     LEFT},
                   RifEclipseOutputTableColumn{"DIR",    LEFT},
                   RifEclipseOutputTableColumn{"r0",     LEFT}
        };

        formatter.keyword("COMPDAT");
        formatter.header(header);

        for (RimFishbonesMultipleSubs* subs : wellPath->fishbonesSubs)
        {
            for (size_t subIndex = 0; subIndex < subs->locationOfSubs().size(); ++subIndex)
            {
                for (size_t lateralIndex = 0; lateralIndex < subs->lateralLengths().size(); ++lateralIndex)
                {
                    std::vector<cvf::Vec3d> lateralCoords = subs->coordsForLateral(subIndex, lateralIndex);

                    std::vector<size_t> lateralCells = findIntersectingCells(caseData, lateralCoords);

                    if (includeWpimult)
                    {
                        // Only need this data if WPIMULT should be included in file
                        addLateralToCells(&lateralsPerCell, lateralCells);
                    }

                    std::vector<size_t> cellsUniqueToLateral = filterWellPathCells(lateralCells, wellPathCells);

                    std::vector<EclipseCellIndexRange> cellRanges = getCellIndexRange(caseData->mainGrid(), cellsUniqueToLateral);

                    formatter.comment(QString("Fishbone %1 - Sub: %2 - Lateral: %3").arg(subs->name()).arg(subIndex).arg(lateralIndex));
                    for (auto cellRange : cellRanges)
                    {
                        // Add cell indices
                        formatter.add(wellPath->name()).addZeroBasedCellIndex(cellRange.i).addZeroBasedCellIndex(cellRange.j).addZeroBasedCellIndex(cellRange.k1).addZeroBasedCellIndex(cellRange.k2);
                        // Remaining data, to be computed
                        formatter.add("'OPEN'").add("1*").add("1*");
                        // Diameter (originally in mm) in m
                        formatter.add(subs->holeRadius() / 1000);
                        formatter.add("1*").add("1*").add("1*").add("'Z'").add("1*");
                        formatter.rowCompleted();
                    }
                }
            }
        }
        formatter.flush();
    }

    // WPIMULT
    if (includeWpimult)
    {
        std::vector<RifEclipseOutputTableColumn> header = {
            RifEclipseOutputTableColumn{"Well", LEFT},
            RifEclipseOutputTableColumn{"Mult", LEFT},
            RifEclipseOutputTableColumn{"I", LEFT},
            RifEclipseOutputTableColumn{"J", LEFT},
            RifEclipseOutputTableColumn{"K", LEFT},
        };
        formatter.keyword("WPIMULT");
        formatter.header(header);

        for (auto lateralsInCell : lateralsPerCell)
        {
            size_t i, j, k;
            caseData->mainGrid()->ijkFromCellIndex(lateralsInCell.first, &i, &j, &k);
            formatter.add(wellPath->name()).add(lateralsInCell.second).addZeroBasedCellIndex(i).addZeroBasedCellIndex(j).addZeroBasedCellIndex(k);
            formatter.rowCompleted();
        }

        formatter.flush();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RicWellPathExportCompletionDataFeature::findCloseCells(const RigEclipseCaseData* caseData, const cvf::BoundingBox& bb)
{
    std::vector<size_t> closeCells;
    caseData->mainGrid()->findIntersectingCells(bb, &closeCells);
    return closeCells;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<EclipseCellIndexRange> RicWellPathExportCompletionDataFeature::getCellIndexRange(const RigMainGrid* grid, const std::vector<size_t>& cellIndices)
{
    // Retrieve I, J, K indices
    std::vector<EclipseCellIndex> eclipseCellIndices;
    for (auto cellIndex : cellIndices)
    {
        size_t i, j, k;
        if (!grid->ijkFromCellIndex(cellIndex, &i, &j, &k)) continue;
        eclipseCellIndices.push_back(std::make_tuple(i, j, k));
    }

    // Group cell indices in K-ranges
    std::sort(eclipseCellIndices.begin(), eclipseCellIndices.end(), RicWellPathExportCompletionDataFeature::cellOrdering);
    std::vector<EclipseCellIndexRange> eclipseCellRanges;
    size_t lastI = std::numeric_limits<size_t>::max();
    size_t lastJ = std::numeric_limits<size_t>::max();
    size_t lastK = std::numeric_limits<size_t>::max();
    size_t startK = std::numeric_limits<size_t>::max();
    for (EclipseCellIndex cell : eclipseCellIndices)
    {
        size_t i, j, k;
        std::tie(i, j, k) = cell;
        if (i != lastI || j != lastJ || k != lastK + 1)
        {
            if (startK != std::numeric_limits<size_t>::max())
            {
                EclipseCellIndexRange cellRange = {lastI, lastJ, startK, lastK};
                eclipseCellRanges.push_back(cellRange);
            }
            lastI = i;
            lastJ = j;
            lastK = k;
            startK = k;
        }
        else
        {
            lastK = k;
        }
    }
    // Append last cell range
    if (startK != std::numeric_limits<size_t>::max())
    {
        EclipseCellIndexRange cellRange = {lastI, lastJ, startK, lastK};
        eclipseCellRanges.push_back(cellRange);
    }
    return eclipseCellRanges;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellPathExportCompletionDataFeature::cellOrdering(const EclipseCellIndex& cell1, const EclipseCellIndex& cell2)
{
    size_t i1, i2, j1, j2, k1, k2;
    std::tie(i1, j1, k1) = cell1;
    std::tie(i2, j2, k2) = cell2;
    if (i1 == i2)
    {
        if (j1 == j2)
        {
            return k1 < k2;
        }
        else
        {
            return j1 < j2;
        }
    }
    else
    {
        return i1 < i2;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RicWellPathExportCompletionDataFeature::findIntersectingCells(const RigEclipseCaseData* caseData, const std::vector<cvf::Vec3d>& coords)
{
    const std::vector<cvf::Vec3d>& nodeCoords = caseData->mainGrid()->nodes();
    std::set<size_t> cells;

    // Find starting cell
    if (coords.size() > 0)
    {
        cvf::BoundingBox bb;
        bb.add(coords[0]);
        std::vector<size_t> closeCells = findCloseCells(caseData, bb);
        cvf::Vec3d hexCorners[8];

        for (size_t closeCell : closeCells)
        {
            const RigCell& cell = caseData->mainGrid()->globalCellArray()[closeCell];
            if (cell.isInvalid()) continue;

            setHexCorners(cell, nodeCoords, hexCorners);

            if (RigHexIntersector::isPointInCell(coords[0], hexCorners, closeCell))
            {
                cells.insert(closeCell);
                break;
            }
        }
    }

    for (size_t i = 0; i < coords.size() - 1; ++i)
    {
        cvf::BoundingBox bb;
        bb.add(coords[i]);
        bb.add(coords[i + 1]);

        std::vector<size_t> closeCells = findCloseCells(caseData, bb);

        cvf::Vec3d hexCorners[8];
        std::vector<HexIntersectionInfo> intersections;

        for (size_t closeCell : closeCells)
        {
            const RigCell& cell = caseData->mainGrid()->globalCellArray()[closeCell];
            if (cell.isInvalid()) continue;

            setHexCorners(cell, nodeCoords, hexCorners);

            RigHexIntersector::lineHexCellIntersection(coords[i], coords[i + 1], hexCorners, closeCell, &intersections);
        }

        for (auto intersection : intersections)
        {
            cells.insert(intersection.m_hexIndex);
        }
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
void RicWellPathExportCompletionDataFeature::setHexCorners(const RigCell& cell, const std::vector<cvf::Vec3d>& nodeCoords, cvf::Vec3d* hexCorners)
{
    const caf::SizeTArray8& cornerIndices = cell.cornerIndices();

    hexCorners[0] = nodeCoords[cornerIndices[0]];
    hexCorners[1] = nodeCoords[cornerIndices[1]];
    hexCorners[2] = nodeCoords[cornerIndices[2]];
    hexCorners[3] = nodeCoords[cornerIndices[3]];
    hexCorners[4] = nodeCoords[cornerIndices[4]];
    hexCorners[5] = nodeCoords[cornerIndices[5]];
    hexCorners[6] = nodeCoords[cornerIndices[6]];
    hexCorners[7] = nodeCoords[cornerIndices[7]];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RicWellPathExportCompletionDataFeature::filterWellPathCells(const std::vector<size_t>& completionCells, const std::vector<size_t>& wellPathCells)
{
    std::vector<size_t> filteredCells;
    std::set_difference(completionCells.begin(), completionCells.end(), wellPathCells.begin(), wellPathCells.end(), std::back_inserter(filteredCells));
    return filteredCells;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::addLateralToCells(std::map<size_t, double>* lateralsPerCell, const std::vector<size_t>& lateralCells)
{
    for (size_t cell : lateralCells)
    {
        std::map<size_t, double>::iterator it = lateralsPerCell->find(cell);
        if (it == lateralsPerCell->end())
        {
            (*lateralsPerCell)[cell] = 1;
        }
        else
        {
            (*lateralsPerCell)[cell] = it->second + 1;
        }
    }
}

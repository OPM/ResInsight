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

#include "RicCreateTemporaryLgrFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "CompletionExportCommands/RicWellPathExportCompletionDataFeature.h"
#include "ExportCommands/RicExportLgrUi.h"
#include "ExportCommands/RicExportLgrFeature.h"

#include "RifEclipseDataTableFormatter.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigVirtualPerforationTransmissibilities.h"
#include "RigCellGeometryTools.h"

#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimWellPath.h"
#include "RimProject.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"
#include "RimMainPlotCollection.h"
#include "RimWellLogPlotCollection.h"

#include "RiuPlotMainWindow.h"

#include <QAction>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

#include <cafPdmUiPropertyViewDialog.h>
#include <cafSelectionManager.h>
#include <cafSelectionManagerTools.h>
#include <cafVecIjk.h>
#include <cafUtils.h>

#include <limits>
#include <algorithm>

CAF_CMD_SOURCE_INIT(RicCreateTemporaryLgrFeature, "RicCreateTemporaryLgrFeature");


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateTemporaryLgrFeature::isCommandEnabled()
{
    std::vector<RimWellPathCompletions*> completions = caf::selectedObjectsByTypeStrict<RimWellPathCompletions*>();

    return !completions.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateTemporaryLgrFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*> wellPaths = selectedWellPaths();
    CVF_ASSERT(wellPaths.size() > 0);

    std::vector<RimSimWellInView*> simWells;
    QString                        dialogTitle = "Create Temporary LGR";

    RimEclipseCase* defaultEclipseCase = nullptr;
    int defaultTimeStep = 0;
    auto activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeGridView());
    if (activeView)
    {
        defaultEclipseCase = activeView->eclipseCase();
        defaultTimeStep = activeView->currentTimeStep();
    }

    auto dialogData = RicExportLgrFeature::openDialog(dialogTitle, defaultEclipseCase, defaultTimeStep);
    if (dialogData)
    {
        auto eclipseCase = dialogData->caseToApply();
        auto lgrCellCounts = dialogData->lgrCellCount();
        size_t timeStep = dialogData->timeStep();
        auto splitType = dialogData->splitType();

        auto eclipseCaseData = eclipseCase->eclipseCaseData();
        RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo(RiaDefines::MATRIX_MODEL);
        RigActiveCellInfo* fractureActiveCellInfo = eclipseCaseData->activeCellInfo(RiaDefines::FRACTURE_MODEL);

        bool intersectsExistingLgr = false;
        for (const auto& wellPath : wellPaths)
        {
            std::vector<LgrInfo> lgrs;

            try
            {
                lgrs = RicExportLgrFeature::buildLgrsForWellPath(wellPath,
                                                                 eclipseCase,
                                                                 timeStep,
                                                                 lgrCellCounts,
                                                                 splitType);

                auto mainGrid = eclipseCase->eclipseCaseData()->mainGrid();

                for (auto lgr : lgrs)
                {
                    createLgr(lgr, eclipseCase->eclipseCaseData()->mainGrid());

                    size_t lgrCellCount = lgr.cellCount();

                    activeCellInfo->addLgr(lgrCellCount);
                    fractureActiveCellInfo->addLgr(lgrCellCount);
                }

                mainGrid->calculateFaults(activeCellInfo, true);
            }
            catch (CreateLgrException e)
            {
                intersectsExistingLgr = true;
            }
        }

        deleteAllCachedData(eclipseCase);
        RiaApplication::instance()->project()->mainPlotCollection()->deleteAllCachedData();
        computeCachedData(eclipseCase);
        RiaApplication::instance()->project()->mainPlotCollection()->wellLogPlotCollection()->reloadAllPlots();

        activeView->loadDataAndUpdate();

        if (intersectsExistingLgr)
        {
            QMessageBox::warning(nullptr,
                                 "LGR cells intersected",
                                 "At least one completion intersects with an LGR. No output for those completions produced");
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateTemporaryLgrFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Create Temporary LGR");
}

//--------------------------------------------------------------------------------------------------
/// Todo: Guarding, caching LGR corner nodes calculations
//--------------------------------------------------------------------------------------------------
void RicCreateTemporaryLgrFeature::createLgr(LgrInfo& lgrInfo, RigMainGrid* mainGrid)
{
    auto app = RiaApplication::instance();
    auto eclipseView = dynamic_cast<RimEclipseView*>(app->activeReservoirView());
    if (!eclipseView) return;

    int lgrId = lgrInfo.id;
    size_t totalCellCount = mainGrid->globalCellArray().size();
    size_t lgrCellCount = lgrInfo.cellCount();

    // Create local grid and set properties
    RigLocalGrid* localGrid = new RigLocalGrid(mainGrid);
    localGrid->setAsTempGrid(true);
    localGrid->setGridId(lgrId);
    localGrid->setIndexToStartOfCells(totalCellCount);
    localGrid->setGridName(lgrInfo.name.toStdString());
    localGrid->setGridPointDimensions(cvf::Vec3st(lgrInfo.sizes.i() + 1, lgrInfo.sizes.j() + 1, lgrInfo.sizes.k() + 1));
    mainGrid->addLocalGrid(localGrid);

    size_t cellStartIndex = mainGrid->globalCellArray().size();
    size_t nodeStartIndex = mainGrid->nodes().size();

    // Resize global cell and node arrays
    {
        RigCell defaultCell;
        defaultCell.setHostGrid(localGrid);
        mainGrid->globalCellArray().resize(cellStartIndex + lgrCellCount, defaultCell);
        mainGrid->nodes().resize(nodeStartIndex + lgrCellCount * 8, cvf::Vec3d(0, 0, 0));
    }

    auto lgrSizePerMainCell = lgrInfo.sizesPerMainGridCell();
    size_t gridLocalCellIndex = 0;

    // Loop through all new LGR cells
    for (size_t lgrK = 0; lgrK < lgrInfo.sizes.k(); lgrK++)
    {
        size_t mainK = lgrInfo.mainGridStartCell.k() + lgrK / lgrSizePerMainCell.k();

        for (size_t lgrJ = 0; lgrJ < lgrInfo.sizes.j(); lgrJ++)
        {
            size_t mainJ = lgrInfo.mainGridStartCell.j() + lgrJ / lgrSizePerMainCell.j();

            for (size_t lgrI = 0; lgrI < lgrInfo.sizes.i(); lgrI++, gridLocalCellIndex++)
            {
                size_t mainI = lgrInfo.mainGridStartCell.i() + lgrI / lgrSizePerMainCell.i();

                size_t mainCellIndex = mainGrid->cellIndexFromIJK(mainI, mainJ, mainK);
                mainGrid->globalCellArray()[mainCellIndex].setSubGrid(localGrid);

                RigCell& cell = mainGrid->globalCellArray()[cellStartIndex + gridLocalCellIndex];
                cell.setGridLocalCellIndex(gridLocalCellIndex);
                cell.setParentCellIndex(mainCellIndex);

                // Corner coordinates
                {
                    size_t                    cIdx;
                    std::array<cvf::Vec3d, 8> vertices;
                    mainGrid->cellCornerVertices(mainCellIndex, vertices.data());

                    auto cellCounts = lgrInfo.sizesPerMainGridCell();
                    auto lgrCoords = RigCellGeometryTools::createHexCornerCoords(vertices,
                                                                                 cellCounts.i(),
                                                                                 cellCounts.j(),
                                                                                 cellCounts.k());

                    size_t subI = lgrI % lgrSizePerMainCell.i();
                    size_t subJ = lgrJ % lgrSizePerMainCell.j();
                    size_t subK = lgrK % lgrSizePerMainCell.k();

                    size_t subIndex = subI + subJ * lgrSizePerMainCell.i() + subK * lgrSizePerMainCell.i() * lgrSizePerMainCell.j();

                    for (cIdx = 0; cIdx < 8; ++cIdx)
                    {
                        auto& node = mainGrid->nodes()[nodeStartIndex + gridLocalCellIndex * 8 + cIdx];
                        node.set(lgrCoords[subIndex * 8 + cIdx]);
                        cell.cornerIndices()[cIdx] = nodeStartIndex + gridLocalCellIndex * 8 + cIdx;
                    }
                }
            }
        }
    }

    localGrid->setParentGrid(mainGrid);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCreateTemporaryLgrFeature::deleteAllCachedData(RimEclipseCase* eclipseCase)
{
    if (eclipseCase)
    {
        RigCaseCellResultsData* cellResultsDataMatrix = eclipseCase->results(RiaDefines::MATRIX_MODEL);
        if (cellResultsDataMatrix)
        {
            cellResultsDataMatrix->freeAllocatedResultsData();
        }

        RigCaseCellResultsData* cellResultsDataFracture = eclipseCase->results(RiaDefines::FRACTURE_MODEL);
        if (cellResultsDataFracture)
        {
            cellResultsDataFracture->freeAllocatedResultsData();
        }

        RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
        if (eclipseCaseData)
        {
            eclipseCaseData->clearWellCellsInGridCache();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCreateTemporaryLgrFeature::computeCachedData(RimEclipseCase* eclipseCase)
{
    if (eclipseCase)
    {
        RigCaseCellResultsData* cellResultsDataMatrix = eclipseCase->results(RiaDefines::MATRIX_MODEL);
        RigCaseCellResultsData* cellResultsDataFracture = eclipseCase->results(RiaDefines::FRACTURE_MODEL);

        RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
        if (eclipseCaseData)
        {
            eclipseCaseData->mainGrid()->computeCachedData();
            eclipseCaseData->computeActiveCellBoundingBoxes();
        }

        if (cellResultsDataMatrix)
        {
            cellResultsDataMatrix->computeDepthRelatedResults();
        }

        if (cellResultsDataFracture)
        {
            cellResultsDataFracture->computeDepthRelatedResults();
        }
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicCreateTemporaryLgrFeature::selectedWellPaths()
{
    std::vector<RimWellPathCompletions*> selectedCompletions = caf::selectedObjectsByTypeStrict<RimWellPathCompletions*>();
    std::vector<RimWellPath*> wellPaths;

    for (auto completion : selectedCompletions)
    {
        RimWellPath* parentWellPath;
        completion->firstAncestorOrThisOfType(parentWellPath);

        if (parentWellPath) wellPaths.push_back(parentWellPath);
    }
    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCreateTemporaryLgrFeature::containsAnyNonMainGridCells(const std::vector<RigCompletionDataGridCell>& cells)
{
    return std::find_if(cells.begin(), cells.end(), [](const RigCompletionDataGridCell& cell)
    {
        return !cell.isMainGridCell();
    }) != cells.end();
}

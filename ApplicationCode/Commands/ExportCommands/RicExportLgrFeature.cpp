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

#include "RicExportLgrFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "CompletionExportCommands/RicWellPathExportCompletionDataFeature.h"
#include "RicExportLgrUi.h"

#include "RifEclipseDataTableFormatter.h"

#include "RigCaseCellResultsData.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigVirtualPerforationTransmissibilities.h"

#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimWellPath.h"
#include "RimProject.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"

#include "RiuPlotMainWindow.h"

#include "RimPerforationInterval.h"
#include "RimFracture.h"
#include "RimFishbonesMultipleSubs.h"

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

#include <QDebug>

CAF_CMD_SOURCE_INIT(RicExportLgrFeature, "RicExportLgrFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportLgrUi* RicExportLgrFeature::openDialog(const QString& dialogTitle, RimEclipseCase* defaultCase, int defaultTimeStep)
{
    RiaApplication* app = RiaApplication::instance();
    RimProject* proj = app->project();

    QString startPath = app->lastUsedDialogDirectory("LGR_EXPORT_DIR");
    if (startPath.isEmpty())
    {
        QFileInfo fi(proj->fileName());
        startPath = fi.absolutePath();
    }

    RicExportLgrUi* featureUi = app->project()->dialogData()->exportLgrData();
    if (featureUi->exportFolder().isEmpty())
    {
        featureUi->setExportFolder(startPath);
    }

    if (!featureUi->caseToApply() && !defaultCase)
    {
        std::vector<RimCase*> cases;
        app->project()->allCases(cases);
        for (auto c : cases)
        {
            RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(c);
            if (eclipseCase != nullptr)
            {
                featureUi->setCase(eclipseCase);
                break;
            }
        }
    }
    if (defaultCase) featureUi->setCase(defaultCase);
    featureUi->setTimeStep(defaultTimeStep);

    caf::PdmUiPropertyViewDialog propertyDialog(nullptr, featureUi, dialogTitle, "", QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    propertyDialog.resize(QSize(600, 230));

    if (propertyDialog.exec() == QDialog::Accepted && !featureUi->exportFolder().isEmpty())
    {
        app->setLastUsedDialogDirectory("LGR_EXPORT_DIR", featureUi->exportFolder());
        return featureUi;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportLgrFeature::openFileForExport(const QString& folderName, const QString& fileName, QFile* exportFile)
{
    QDir exportFolder = QDir(folderName);
    if (!exportFolder.exists())
    {
        bool createdPath = exportFolder.mkpath(".");
        if (createdPath)
            RiaLogging::info("Created export folder " + folderName);
    }

    QString  filePath = exportFolder.filePath(fileName);
    exportFile->setFileName(filePath);
    if (!exportFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        auto errorMessage = QString("Export Well Path: Could not open the file: %1").arg(filePath);
        RiaLogging::error(errorMessage);
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportLgrFeature::exportLgrs(QTextStream& stream, const std::vector<LgrInfo>& lgrInfos)
{
    int count = 0;
    for (auto lgrInfo : lgrInfos)
    {
        auto lgrName = QString("LGR_%1").arg(++count);

        {
            RifEclipseDataTableFormatter formatter(stream);
            formatter.keyword("CARFIN");
            formatter.header(
                {
                    RifEclipseOutputTableColumn("Name"),
                    RifEclipseOutputTableColumn("I1"),
                    RifEclipseOutputTableColumn("I2"),
                    RifEclipseOutputTableColumn("J1"),
                    RifEclipseOutputTableColumn("J2"),
                    RifEclipseOutputTableColumn("K1"),
                    RifEclipseOutputTableColumn("K2"),
                    RifEclipseOutputTableColumn("NX"),
                    RifEclipseOutputTableColumn("NY"),
                    RifEclipseOutputTableColumn("NZ")
                }
            );

            formatter.add(lgrInfo.name);
            formatter.addOneBasedCellIndex(lgrInfo.mainGridStartCell.i());
            formatter.addOneBasedCellIndex(lgrInfo.mainGridEndCell.i());
            formatter.addOneBasedCellIndex(lgrInfo.mainGridStartCell.j());
            formatter.addOneBasedCellIndex(lgrInfo.mainGridEndCell.j());
            formatter.addOneBasedCellIndex(lgrInfo.mainGridStartCell.k());
            formatter.addOneBasedCellIndex(lgrInfo.mainGridEndCell.k());
            formatter.add(lgrInfo.sizesPerMainGridCell().i());
            formatter.add(lgrInfo.sizesPerMainGridCell().j());
            formatter.add(lgrInfo.sizesPerMainGridCell().k());
            formatter.rowCompleted();
            formatter.tableCompleted("", false);
        }

        {
            RifEclipseDataTableFormatter formatter(stream);
            formatter.keyword("ENDFIN");
            formatter.tableCompleted("", true);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrFeature::exportLgrsForWellPath(const QString& exportFolder,
                                                RimWellPath* wellPath,
                                                RimEclipseCase* eclipseCase,
                                                size_t timeStep,
                                                caf::VecIjk lgrCellCounts,
                                                RicExportLgrUi::SplitType splitType)
{
    std::vector<LgrInfo> lgrs;
    
    try
    {
        lgrs = buildLgrsForWellPath(wellPath,
                                    eclipseCase,
                                    timeStep,
                                    lgrCellCounts,
                                    splitType);

        // Export
        QFile   file;
        QString fileName = caf::Utils::makeValidFileBasename(QString("LGR_%1").arg(wellPath->name())) + ".dat";
        openFileForExport(exportFolder, fileName, &file);
        QTextStream stream(&file);
        stream.setRealNumberNotation(QTextStream::FixedNotation);
        stream.setRealNumberPrecision(2);
        exportLgrs(stream, lgrs);
        file.close();
    }
    catch (CreateLgrException e)
    {
        throw;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<LgrInfo> RicExportLgrFeature::buildLgrsForWellPath(RimWellPath*                 wellPath,
                                                               RimEclipseCase*              eclipseCase,
                                                               size_t                       timeStep,
                                                               caf::VecIjk                  lgrCellCounts,
                                                               RicExportLgrUi::SplitType    splitType)
{
    std::vector<LgrInfo> lgrs;
    bool intersectsWithExistingLgr = false;

    if (splitType == RicExportLgrUi::LGR_PER_CELL)
    {
        auto intersectingCells = cellsIntersectingCompletions(eclipseCase, wellPath, timeStep);

        if (containsAnyNonMainGridCells(intersectingCells))
        {
            intersectsWithExistingLgr = true;
        }
        else
        {
            lgrs = buildLgrsPerMainCell(eclipseCase, intersectingCells, lgrCellCounts);
        }
    }
    else if (splitType == RicExportLgrUi::LGR_PER_COMPLETION)
    {
        auto intersectingCells = cellsIntersectingCompletions_PerCompletion(eclipseCase, wellPath, timeStep);

        if (containsAnyNonMainGridCells(intersectingCells))
        {
            intersectsWithExistingLgr = true;
        }
        else
        {
            lgrs = buildLgrsPerCompletion(eclipseCase, intersectingCells, lgrCellCounts);
        }
    }
    else if (splitType == RicExportLgrUi::LGR_PER_WELL)
    {
        auto intersectingCells = cellsIntersectingCompletions(eclipseCase, wellPath, timeStep);

        if (containsAnyNonMainGridCells(intersectingCells))
        {
            intersectsWithExistingLgr = true;
        }
        else
        {
            int lgrId = firstAvailableLgrId(eclipseCase->mainGrid());
            lgrs.push_back(buildLgr(lgrId, eclipseCase, intersectingCells, lgrCellCounts));
        }
    }

    if (intersectsWithExistingLgr)
    {
        throw CreateLgrException("At least one completion intersects with an existing LGR");
    }
    return lgrs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<LgrInfo> RicExportLgrFeature::buildLgrsPerMainCell(RimEclipseCase* eclipseCase,
                                                               const std::vector<RigCompletionDataGridCell>& intersectingCells,
                                                               const caf::VecIjk& lgrSizes)
{
    std::vector<LgrInfo> lgrs;

    int lgrId = firstAvailableLgrId(eclipseCase->mainGrid());
    for (auto intersectionCell : intersectingCells)
    {
        lgrs.push_back(buildLgr(lgrId++, eclipseCase, { intersectionCell }, lgrSizes));
    }
    return lgrs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<LgrInfo> RicExportLgrFeature::buildLgrsPerCompletion(RimEclipseCase* eclipseCase,
                                                                 const std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>>& intersectingCells,
                                                                 const caf::VecIjk& lgrSizesPerMainGridCell)
{
    std::vector<LgrInfo> lgrs;
   
    int lgrId = firstAvailableLgrId(eclipseCase->mainGrid());
    for (auto intersectionInfo : intersectingCells)
    {
        lgrs.push_back(buildLgr(lgrId++, eclipseCase, intersectionInfo.second, lgrSizesPerMainGridCell));
    }
    return lgrs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LgrInfo RicExportLgrFeature::buildLgr(int lgrId,
                                                   RimEclipseCase* eclipseCase,
                                                   const std::vector<RigCompletionDataGridCell>& intersectingCells,
                                                   const caf::VecIjk& lgrSizesPerMainGridCell)
{
    std::vector<LgrInfo> lgrs;

    // Find min and max IJK
    auto iRange = initRange();
    auto jRange = initRange();
    auto kRange = initRange();

    for (auto cell : intersectingCells)
    {
        iRange.first = std::min(cell.localCellIndexI(), iRange.first);
        iRange.second = std::max(cell.localCellIndexI(), iRange.second);
        jRange.first = std::min(cell.localCellIndexJ(), jRange.first);
        jRange.second = std::max(cell.localCellIndexJ(), jRange.second);
        kRange.first = std::min(cell.localCellIndexK(), kRange.first);
        kRange.second = std::max(cell.localCellIndexK(), kRange.second);
    }

    caf::VecIjk lgrSizes((iRange.second - iRange.first + 1) * lgrSizesPerMainGridCell.i(),
                         (jRange.second - jRange.first + 1) * lgrSizesPerMainGridCell.j(),
                         (kRange.second - kRange.first + 1) * lgrSizesPerMainGridCell.k());
    caf::VecIjk mainGridStartCell(iRange.first, jRange.first, kRange.first);
    caf::VecIjk mainGridEndCell(iRange.second, jRange.second, kRange.second);

    return LgrInfo(lgrId, QString("LGR_%1").arg(lgrId), lgrSizes, mainGridStartCell, mainGridEndCell);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionDataGridCell>
RicExportLgrFeature::cellsIntersectingCompletions(RimEclipseCase* eclipseCase,
                                                  const RimWellPath* wellPath,
                                                  size_t timeStep)
{
    std::vector<RigCompletionDataGridCell> cells;

    auto completions = eclipseCase->computeAndGetVirtualPerforationTransmissibilities();
    if (completions)
    {
        auto intCells = completions->multipleCompletionsPerEclipseCell(wellPath, timeStep);

        for (auto intCell : intCells)
        {
            cells.push_back(intCell.first);
        }
    }
    return cells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>>
    RicExportLgrFeature::cellsIntersectingCompletions_PerCompletion(RimEclipseCase*    eclipseCase,
                                                                    const RimWellPath* wellPath,
                                                                    size_t             timeStep)
{
    std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>> completionToCells;

    auto completions = eclipseCase->computeAndGetVirtualPerforationTransmissibilities();
    if (completions)
    {
        auto intCells = completions->multipleCompletionsPerEclipseCell(wellPath, timeStep);

        for (auto intCell : intCells)
        {
            auto pdmSrcObj = intCell.second.front().sourcePdmObject();
            auto perf      = dynamic_cast<const RimPerforationInterval*>(pdmSrcObj);
            auto frac      = dynamic_cast<const RimFracture*>(pdmSrcObj);
            auto fish      = dynamic_cast<const RimFishbonesMultipleSubs*>(pdmSrcObj);

            QString name;
            if (perf)
                name = perf->name();
            else if (frac)
                name = frac->name();
            else if (fish)
                name = fish->generatedName();

            if (name.isEmpty()) continue;

            for (auto compl : intCell.second)
            {
                completionToCells[CompletionInfo(compl.completionType(), name)].push_back(intCell.first);
            }
        }
    }
    return completionToCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportLgrFeature::isCommandEnabled()
{
    std::vector<RimWellPathCompletions*> completions = caf::selectedObjectsByTypeStrict<RimWellPathCompletions*>();

    return !completions.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*> wellPaths = selectedWellPaths();
    CVF_ASSERT(wellPaths.size() > 0);

    QString                        dialogTitle = "LGR Export";

    RimEclipseCase* defaultEclipseCase = nullptr;
    int defaultTimeStep = 0;
    auto activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeGridView());
    if (activeView)
    {
        defaultEclipseCase = activeView->eclipseCase();
        defaultTimeStep = activeView->currentTimeStep();
    }

    auto dialogData = openDialog("LGR Export", defaultEclipseCase, defaultTimeStep);
    if (dialogData)
    {
        auto eclipseCase = dialogData->caseToApply();
        auto lgrCellCounts = dialogData->lgrCellCount();
        size_t timeStep = dialogData->timeStep();

        bool intersectsExistingLgr = false;
        for (const auto& wellPath : wellPaths)
        {
            try
            {
                exportLgrsForWellPath(dialogData->exportFolder(), wellPath, eclipseCase, timeStep, lgrCellCounts, dialogData->splitType());
            }
            catch(CreateLgrException e)
            {
                intersectsExistingLgr = true;
            }
        }

        if (intersectsExistingLgr)
        {
            QMessageBox::warning(nullptr, "LGR cells intersected", "At least one completion intersects with an LGR. No output for those completions produced");
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("LGR Export");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicExportLgrFeature::selectedWellPaths()
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
bool RicExportLgrFeature::containsAnyNonMainGridCells(
    const std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>>& cellsPerCompletion)
{
    for (auto cells : cellsPerCompletion)
    {
        if (containsAnyNonMainGridCells(cells.second)) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportLgrFeature::containsAnyNonMainGridCells(const std::vector<RigCompletionDataGridCell>& cells)
{
    return std::find_if(cells.begin(), cells.end(), [](const RigCompletionDataGridCell& cell)
    {
        return !cell.isMainGridCell();
    }) != cells.end();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RicExportLgrFeature::firstAvailableLgrId(const RigMainGrid* mainGrid)
{
    int gridCount = (int)mainGrid->gridCount();
    int lastUsedId = 0;
    for (int i = 0; i < gridCount; i++)
    {
        lastUsedId = std::max(lastUsedId, mainGrid->gridByIndex(i)->gridId());
    }
    return lastUsedId + 1;
}

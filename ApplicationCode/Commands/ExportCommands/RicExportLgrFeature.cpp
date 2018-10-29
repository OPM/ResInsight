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
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"

#include "RiuPlotMainWindow.h"

#include "RimFishbonesMultipleSubs.h"
#include "RimFracture.h"
#include "RimPerforationInterval.h"

#include <QAction>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>

#include <cafPdmUiPropertyViewDialog.h>
#include <cafSelectionManager.h>
#include <cafSelectionManagerTools.h>
#include <cafUtils.h>
#include <cafVecIjk.h>

#include <limits>
#include <array>
#include <set>

CAF_CMD_SOURCE_INIT(RicExportLgrFeature, "RicExportLgrFeature");

//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
int completionPriority(const RigCompletionData& completion)
{
    return completion.completionType() == RigCompletionData::FRACTURE ? 1 :
        completion.completionType() == RigCompletionData::FISHBONES ? 2 :
        completion.completionType() == RigCompletionData::PERFORATION ? 3 : 4;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> filterCompletionsOnType(const std::vector<RigCompletionData>& completions,
                                                       const std::set<RigCompletionData::CompletionType>& includedCompletionTypes)
{
    std::vector<RigCompletionData> filtered;
    for (auto completion : completions)
    {
        if (includedCompletionTypes.count(completion.completionType()) > 0) filtered.push_back(completion);
    }
    return filtered;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString completionName(const caf::PdmObject* object)
{
    auto perf = dynamic_cast<const RimPerforationInterval*>(object);
    auto frac = dynamic_cast<const RimFracture*>(object);
    auto fish = dynamic_cast<const RimFishbonesMultipleSubs*>(object);

    QString name;
    if (perf) name = perf->name();
    else if (frac) name = frac->name();
    else if (fish) name = fish->generatedName();
    return name;
}

//--------------------------------------------------------------------------------------------------
/// Returns the completion having highest priority.
/// Pri: 1. Fractures, 2. Fishbones, 3. Perforation intervals
//--------------------------------------------------------------------------------------------------
RigCompletionData findCompletionByPriority(const std::vector<RigCompletionData>& completions)
{
    std::vector<RigCompletionData> sorted = completions;

    std::sort(sorted.begin(), sorted.end(),
              [](const RigCompletionData& c1, const RigCompletionData& c2 )
    { 
        if (completionPriority(c1) == completionPriority(c2))
        {
            return completionName(c1.sourcePdmObject()) < completionName(c2.sourcePdmObject());
        }
        return completionPriority(c1) < completionPriority(c2);
    });
    return sorted.front();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportLgrUi* RicExportLgrFeature::openDialog(const QString& dialogTitle,
                                                RimEclipseCase* defaultCase,
                                                int defaultTimeStep,
                                                bool hideExportFolderField)
{
    RiaApplication* app  = RiaApplication::instance();
    RimProject*     proj = app->project();

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
    featureUi->hideExportFolderField(hideExportFolderField);

    caf::PdmUiPropertyViewDialog propertyDialog(
        nullptr, featureUi, dialogTitle, "", QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    propertyDialog.resize(QSize(300, 285 - (hideExportFolderField ? 25 : 0)));

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
        if (createdPath) RiaLogging::info("Created export folder " + folderName);
    }

    QString filePath = exportFolder.filePath(fileName);
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
            formatter.header({RifEclipseOutputTableColumn("Name"),
                              RifEclipseOutputTableColumn("I1"),
                              RifEclipseOutputTableColumn("I2"),
                              RifEclipseOutputTableColumn("J1"),
                              RifEclipseOutputTableColumn("J2"),
                              RifEclipseOutputTableColumn("K1"),
                              RifEclipseOutputTableColumn("K2"),
                              RifEclipseOutputTableColumn("NX"),
                              RifEclipseOutputTableColumn("NY"),
                              RifEclipseOutputTableColumn("NZ")});

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
void RicExportLgrFeature::exportLgrsForWellPath(const QString&            exportFolder,
                                                RimWellPath*              wellPath,
                                                RimEclipseCase*           eclipseCase,
                                                size_t                    timeStep,
                                                caf::VecIjk               lgrCellCounts,
                                                RicExportLgrUi::SplitType splitType,
                                                const std::set<RigCompletionData::CompletionType>& completionTypes,
                                                bool* intersectingOtherLgrs)
{
    std::vector<LgrInfo> lgrs;

    lgrs = buildLgrsForWellPath(wellPath,
                                eclipseCase,
                                timeStep,
                                lgrCellCounts,
                                splitType,
                                completionTypes,
                                intersectingOtherLgrs);

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<LgrInfo> RicExportLgrFeature::buildLgrsForWellPath(RimWellPath*                 wellPath,
                                                               RimEclipseCase*              eclipseCase,
                                                               size_t                       timeStep,
                                                               caf::VecIjk                  lgrCellCounts,
                                                               RicExportLgrUi::SplitType    splitType,
                                                               const std::set<RigCompletionData::CompletionType>& completionTypes,
                                                               bool* intersectingOtherLgrs)
{
    std::vector<LgrInfo> lgrs;

    if (splitType == RicExportLgrUi::LGR_PER_CELL)
    {
        auto intersectingCells = cellsIntersectingCompletions(eclipseCase, wellPath, timeStep, completionTypes);

        *intersectingOtherLgrs = containsAnyNonMainGridCells(intersectingCells);
        lgrs = buildLgrsPerMainCell(eclipseCase, intersectingCells, lgrCellCounts);
    }
    else if (splitType == RicExportLgrUi::LGR_PER_COMPLETION)
    {
        auto intersectingCells = cellsIntersectingCompletions_PerCompletion(eclipseCase, wellPath, timeStep, completionTypes);

        *intersectingOtherLgrs = containsAnyNonMainGridCells(intersectingCells);
        lgrs = buildLgrsPerCompletion(eclipseCase, intersectingCells, lgrCellCounts);
    }
    else if (splitType == RicExportLgrUi::LGR_PER_WELL)
    {
        auto intersectingCells = cellsIntersectingCompletions(eclipseCase, wellPath, timeStep, completionTypes);

        *intersectingOtherLgrs = containsAnyNonMainGridCells(intersectingCells);

        int lgrId = firstAvailableLgrId(eclipseCase->mainGrid());
        lgrs.push_back(buildLgr(lgrId, eclipseCase, intersectingCells, lgrCellCounts));
    }
    return lgrs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<LgrInfo> RicExportLgrFeature::buildLgrsPerMainCell(RimEclipseCase*                               eclipseCase,
                                                               const std::vector<RigCompletionDataGridCell>& intersectingCells,
                                                               const caf::VecIjk&                            lgrSizes)
{
    std::vector<LgrInfo> lgrs;

    int lgrId = firstAvailableLgrId(eclipseCase->mainGrid());
    for (auto intersectionCell : intersectingCells)
    {
        lgrs.push_back(buildLgr(lgrId++, eclipseCase, {intersectionCell}, lgrSizes));
    }
    return lgrs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<LgrInfo> RicExportLgrFeature::buildLgrsPerCompletion(
    RimEclipseCase*                                                         eclipseCase,
    const std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>>& intersectingCells,
    const caf::VecIjk&                                                      lgrSizesPerMainGridCell)
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
LgrInfo RicExportLgrFeature::buildLgr(int                                           lgrId,
                                      RimEclipseCase*                               eclipseCase,
                                      const std::vector<RigCompletionDataGridCell>& intersectingCells,
                                      const caf::VecIjk&                            lgrSizesPerMainGridCell)
{
    std::vector<LgrInfo> lgrs;

    // Find min and max IJK
    auto iRange = initRange();
    auto jRange = initRange();
    auto kRange = initRange();

    for (auto cell : intersectingCells)
    {
        iRange.first  = std::min(cell.localCellIndexI(), iRange.first);
        iRange.second = std::max(cell.localCellIndexI(), iRange.second);
        jRange.first  = std::min(cell.localCellIndexJ(), jRange.first);
        jRange.second = std::max(cell.localCellIndexJ(), jRange.second);
        kRange.first  = std::min(cell.localCellIndexK(), kRange.first);
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
                                                  size_t timeStep,
                                                  const std::set<RigCompletionData::CompletionType>& completionTypes)
{
    std::vector<RigCompletionDataGridCell> cells;

    auto completions = eclipseCase->computeAndGetVirtualPerforationTransmissibilities();
    if (completions)
    {
        auto intCells = completions->multipleCompletionsPerEclipseCell(wellPath, timeStep);

        for (auto intCell : intCells)
        {
            auto filteredCompletions = filterCompletionsOnType(intCell.second, completionTypes);

            if (filteredCompletions.empty()) continue;

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
                                                                    size_t             timeStep,
                                                                    const std::set<RigCompletionData::CompletionType>& completionTypes)
{
    std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>> completionToCells;

    auto completions = eclipseCase->computeAndGetVirtualPerforationTransmissibilities();
    if (completions)
    {
        auto intCells = completions->multipleCompletionsPerEclipseCell(wellPath, timeStep);
        CompletionInfo lastCompletionInfo;

        // This loop assumes that cells are ordered downwards along well path
        for (auto intCell : intCells)
        {
            if (!intCell.first.isMainGridCell()) continue;

            auto filteredCompletions = filterCompletionsOnType(intCell.second, completionTypes);
            if (filteredCompletions.empty()) continue;

            auto completion = findCompletionByPriority(filteredCompletions);

            QString        name = completionName(completion.sourcePdmObject());
            CompletionInfo completionInfo(completion.completionType(), name, 0);

            if (!lastCompletionInfo.isValid()) lastCompletionInfo = completionInfo;

            if (completionInfo != lastCompletionInfo && completionToCells.count(completionInfo) > 0)
            {
                completionInfo.number++;
            }
            completionToCells[completionInfo].push_back(intCell.first);
            lastCompletionInfo = completionInfo;
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
    std::vector<RimWellPath*> wellPaths = caf::selectedObjectsByTypeStrict<RimWellPath*>();

    return !completions.empty() || !wellPaths.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*> wellPaths = selectedWellPaths();
    if(wellPaths.size() == 0) return;

    QString dialogTitle = "LGR Export";

    RimEclipseCase* defaultEclipseCase = nullptr;
    int             defaultTimeStep    = 0;
    auto            activeView         = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeGridView());
    if (activeView)
    {
        defaultEclipseCase = activeView->eclipseCase();
        defaultTimeStep    = activeView->currentTimeStep();
    }

    auto dialogData = openDialog("LGR Export", defaultEclipseCase, defaultTimeStep);
    if (dialogData)
    {
        auto   eclipseCase   = dialogData->caseToApply();
        auto   lgrCellCounts = dialogData->lgrCellCount();
        size_t timeStep      = dialogData->timeStep();

        bool intersectingOtherLgrs = false;
        for (const auto& wellPath : wellPaths)
        {
            bool intersectingLgrs = false;
            exportLgrsForWellPath(dialogData->exportFolder(),
                                  wellPath,
                                  eclipseCase,
                                  timeStep,
                                  lgrCellCounts,
                                  dialogData->splitType(),
                                  dialogData->completionTypes(),
                                  &intersectingLgrs);

            if (intersectingLgrs) intersectingOtherLgrs = true;
        }

        if (intersectingOtherLgrs)
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
    std::vector<RimWellPath*>            wellPaths = caf::selectedObjectsByTypeStrict<RimWellPath*>();

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
    return std::find_if(cells.begin(), cells.end(), [](const RigCompletionDataGridCell& cell) {
               return !cell.isMainGridCell();
           }) != cells.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicExportLgrFeature::firstAvailableLgrId(const RigMainGrid* mainGrid)
{
    int gridCount  = (int)mainGrid->gridCount();
    int lastUsedId = 0;
    for (int i = 0; i < gridCount; i++)
    {
        lastUsedId = std::max(lastUsedId, mainGrid->gridByIndex(i)->gridId());
    }
    return lastUsedId + 1;
}


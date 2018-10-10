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

CAF_CMD_SOURCE_INIT(RicExportLgrFeature, "RicExportLgrFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportLgrUi* RicExportLgrFeature::openDialog()
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

    if (!featureUi->caseToApply())
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

    caf::PdmUiPropertyViewDialog propertyDialog(nullptr, featureUi, "LGR Export", "", QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    propertyDialog.resize(QSize(600, 250));

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
            formatter.add(lgrInfo.sizes.i());
            formatter.add(lgrInfo.sizes.j());
            formatter.add(lgrInfo.sizes.k());
            formatter.rowCompleted();
            formatter.tableCompleted("", false);
        }

        if (!lgrInfo.values.empty())
        {
            RifEclipseDataTableFormatter::addValueTable(stream, "PORO", 8, lgrInfo.values);
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
std::vector<LgrInfo> RicExportLgrFeature::buildOneLgrPerMainCell(RimEclipseCase* eclipseCase,
                                                                const std::vector<RigCompletionDataGridCell>& intersectingCells,
                                                                const caf::VecIjk& lgrSizes)
{
    int lgrCount = 0;
    std::vector<LgrInfo> lgrs;

    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PORO");
    cvf::ref<RigResultAccessor> poroAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCase->eclipseCaseData(), 0, RiaDefines::MATRIX_MODEL, 0, "PORO");

    bool poroExists = !poroAccessObject.isNull();
    for (const auto& intersectingCell : intersectingCells)
    {
        size_t globCellIndex = intersectingCell.globalCellIndex();
        double poro = poroExists ? poroAccessObject->cellScalarGlobIdx(globCellIndex) : std::numeric_limits<double>::infinity();

        std::vector<double> lgrValues;

        for (size_t k = 0; k < lgrSizes.k(); k++)
        {
            for (size_t j = 0; j < lgrSizes.j(); j++)
            {
                for (size_t i = 0; i < lgrSizes.i(); i++)
                {
                    lgrValues.push_back(poro);
                }
            }
        }

        caf::VecIjk mainGridFirstCell(intersectingCell.localCellIndexI(),
                                      intersectingCell.localCellIndexJ(),
                                      intersectingCell.localCellIndexK());
        caf::VecIjk mainGridEndCell(intersectingCell.localCellIndexI(),
                                    intersectingCell.localCellIndexJ(),
                                    intersectingCell.localCellIndexK());

        LgrInfo lgrInfo(QString("LGR_%1").arg(++lgrCount), lgrSizes, mainGridFirstCell, mainGridEndCell);
        if(poroExists) lgrInfo.values = lgrValues;
        lgrs.push_back(lgrInfo);
    }

    return lgrs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<LgrInfo> RicExportLgrFeature::buildSingleLgr(RimEclipseCase* eclipseCase,
                                                         const std::vector<RigCompletionDataGridCell>& intersectingCells,
                                                         const caf::VecIjk& lgrSizes)
{
    std::vector<LgrInfo> lgrs;

    eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PORO");
    cvf::ref<RigResultAccessor> poroAccessObject =
        RigResultAccessorFactory::createFromUiResultName(eclipseCase->eclipseCaseData(), 0, RiaDefines::MATRIX_MODEL, 0, "PORO");

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

    bool poroExists = !poroAccessObject.isNull();
    auto mainGrid = eclipseCase->mainGrid();
    std::vector<double> lgrValues;
    for (size_t mainK = kRange.first; mainK <= kRange.second; mainK++)
    {
        for (size_t mainJ = jRange.first; mainJ <= jRange.second; mainJ++)
        {
            for (size_t mainI = iRange.first; mainI <= iRange.second; mainI++)
            {
                size_t globCellIndex = mainGrid->cellIndexFromIJK(mainI, mainJ, mainK);

                double poro = poroExists ? poroAccessObject->cellScalarGlobIdx(globCellIndex) : std::numeric_limits<double>::infinity();

                for (size_t k = 0; k < lgrSizes.k(); k++)
                {
                    for (size_t j = 0; j < lgrSizes.j(); j++)
                    {
                        for (size_t i = 0; i < lgrSizes.i(); i++)
                        {
                            lgrValues.push_back(poro);
                        }
                    }
                }

            }
        }
    }

    caf::VecIjk mainGridStartCell(iRange.first, jRange.first, kRange.first);
    caf::VecIjk mainGridEndCell(iRange.second, jRange.second, kRange.second);

    LgrInfo lgrInfo(QString("LGR_1"), lgrSizes, mainGridStartCell, mainGridEndCell);
    if(poroExists) lgrInfo.values = lgrValues;
    lgrs.push_back(lgrInfo);

    return lgrs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionDataGridCell> RicExportLgrFeature::cellsIntersectingCompletions(RimEclipseCase* eclipseCase,
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

    std::vector<RimSimWellInView*> simWells;
    QString                        dialogTitle = "LGR Export";

    auto dialogData = openDialog();
    if (dialogData)
    {
        auto activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeGridView());
        if (!activeView) return;

        auto eclipseCase = dialogData->caseToApply();
        auto lgrCellCounts = dialogData->lgrCellCount();
        size_t timeStep = activeView->currentTimeStep();

        bool lgrIntersected = false;
        for (const auto& wellPath : wellPaths)
        {
            auto intersectingCells = cellsIntersectingCompletions(eclipseCase, wellPath, timeStep);
            if (containsAnyNonMainGridCells(intersectingCells))
            {
                lgrIntersected = true;
                continue;
            }

            std::vector<LgrInfo> lgrs;
            if(dialogData->singleLgrSplit())
                lgrs = buildSingleLgr(eclipseCase, intersectingCells, lgrCellCounts);
            else
                lgrs = buildOneLgrPerMainCell(eclipseCase, intersectingCells, lgrCellCounts);

            // Export
            QFile file;
            QString fileName = caf::Utils::makeValidFileBasename(QString("LGR_%1").arg(wellPath->name())) + ".dat";
            openFileForExport(dialogData->exportFolder(), fileName, &file);
            QTextStream stream(&file);
            stream.setRealNumberNotation(QTextStream::FixedNotation);
            stream.setRealNumberPrecision(2);
            exportLgrs(stream, lgrs);
            file.close();
        }

        if (lgrIntersected)
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
bool RicExportLgrFeature::containsAnyNonMainGridCells(const std::vector<RigCompletionDataGridCell>& cells)
{
    return std::find_if(cells.begin(), cells.end(), [](const RigCompletionDataGridCell& cell)
    {
        return !cell.isMainGridCell();
    }) != cells.end();
}

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

#include <cafPdmUiPropertyViewDialog.h>
#include <cafSelectionManager.h>
#include <cafSelectionManagerTools.h>
#include <cafVecIjk.h>
#include <cafUtils.h>


CAF_CMD_SOURCE_INIT(RicExportLgrFeature, "RicExportLgrFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportLgrUi* RicExportLgrFeature::openDialog()
{
    RiaApplication* app = RiaApplication::instance();
    RimProject* proj = app->project();

    QString startPath = app->lastUsedDialogDirectory("CARFIN_DIR");
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

    caf::PdmUiPropertyViewDialog propertyDialog(nullptr, featureUi, "Export Carfin", "", QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    propertyDialog.resize(QSize(600, 230));

    if (propertyDialog.exec() == QDialog::Accepted && !featureUi->exportFolder().isEmpty())
    {
        app->setLastUsedDialogDirectory("CARFIN_DIR", featureUi->exportFolder());
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
void RicExportLgrFeature::exportCarfin(QTextStream& stream, const std::map<RigCompletionDataGridCell, LgrInfo>& lgrInfos)
{
    int count = 0;
    for (auto lgr : lgrInfos)
    {
        auto lgrName = QString("LGR_%1").arg(++count);
        auto dataGridCell = lgr.first;
        auto lgrInfo = lgr.second;

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
            formatter.add(dataGridCell.localCellIndexI());
            formatter.add(dataGridCell.localCellIndexI());
            formatter.add(dataGridCell.localCellIndexJ());
            formatter.add(dataGridCell.localCellIndexJ());
            formatter.add(dataGridCell.localCellIndexK());
            formatter.add(dataGridCell.localCellIndexK());
            formatter.add(lgrInfo.sizes.i());
            formatter.add(lgrInfo.sizes.j());
            formatter.add(lgrInfo.sizes.k());
            formatter.rowCompleted();
            formatter.tableCompleted("", false);
        }

        RifEclipseDataTableFormatter::addValueTable(stream, "PORO", lgrInfo.sizes.i(), lgrInfo.values);

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
    std::vector<RimWellPath*> wellPaths = visibleWellPaths();
    CVF_ASSERT(wellPaths.size() > 0);

    std::vector<RimSimWellInView*> simWells;
    QString                        dialogTitle = "LGR Export";

    auto dialogData = openDialog();
    if (dialogData)
    {
        // Per cell LGR
        std::map<RigCompletionDataGridCell, LgrInfo> lgrs;

        auto activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeGridView());
        if (!activeView) return;

        auto eclipseCase = dialogData->caseToApply();

        eclipseCase->results(RiaDefines::MATRIX_MODEL)->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PORO");
        cvf::ref<RigResultAccessor> poroAccessObject =
            RigResultAccessorFactory::createFromUiResultName(eclipseCase->eclipseCaseData(), 0, RiaDefines::MATRIX_MODEL, 0, "PORO");

        auto lgrCellCounts = dialogData->lgrCellCount();

        auto completions = eclipseCase->computeAndGetVirtualPerforationTransmissibilities();
        if (completions)
        {
            size_t timeStep = activeView->currentTimeStep();

            for (const auto& wellPath : wellPaths)
            {
                int lgrCount = 0;

                for (const auto& completionsForWell : completions->multipleCompletionsPerEclipseCell(wellPath, timeStep))
                {
                    size_t globCellIndex = completionsForWell.first.globalCellIndex();
                    double poro = poroAccessObject->cellScalarGlobIdx(globCellIndex);

                    std::vector<double> lgrValues;

                    for (size_t k = 0; k < lgrCellCounts.k(); k++)
                    {
                        for (size_t j = 0; j < lgrCellCounts.j(); j++)
                        {
                            for (size_t i = 0; i < lgrCellCounts.i(); i++)
                            {
                                lgrValues.push_back(poro);
                            }
                        }
                    }

                    LgrInfo lgrInfo(QString("LGR_%1").arg(++lgrCount), lgrCellCounts);
                    lgrInfo.values = lgrValues;
                    lgrs.insert(std::make_pair(completionsForWell.first, lgrInfo));
                }

                // Export
                QFile file;
                QString fileName = caf::Utils::makeValidFileBasename(QString("LGR_%1.dat").arg(wellPath->name()));
                openFileForExport(dialogData->exportFolder(), fileName, &file);
                QTextStream stream(&file);
                stream.setRealNumberNotation(QTextStream::FixedNotation);
                stream.setRealNumberPrecision(2);
                exportCarfin(stream, lgrs);
                file.close();
            }
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
std::vector<RimWellPath*> RicExportLgrFeature::visibleWellPaths()
{
    std::vector<RimWellPath*> wellPaths;

    {
        std::vector<RimWellPathCollection*> wellPathCollections;
        caf::SelectionManager::instance()->objectsByType(&wellPathCollections);

        if (wellPathCollections.empty())
        {
            std::vector<RimWellPath*> selectedWellPaths;
            caf::SelectionManager::instance()->objectsByType(&selectedWellPaths);

            if (!selectedWellPaths.empty())
            {
                RimWellPathCollection* parent = nullptr;
                selectedWellPaths[0]->firstAncestorOrThisOfType(parent);
                if (parent)
                {
                    wellPathCollections.push_back(parent);
                }
            }
        }

        if (!wellPathCollections.empty())
        {
            for (auto wellPathCollection : wellPathCollections)
            {
                for (const auto& wellPath : wellPathCollection->wellPaths())
                {
                    if (wellPath->showWellPath())
                    {
                        wellPaths.push_back(wellPath);
                    }
                }
            }
        }
        else
        {
            // No well path or well path collection selected 

            auto allWellPaths = RiaApplication::instance()->project()->allWellPaths();
            for (const auto& w : allWellPaths)
            {
                if (w->showWellPath())
                {
                    wellPaths.push_back(w);
                }
            }

        }
    }

    std::set<RimWellPath*> uniqueWellPaths(wellPaths.begin(), wellPaths.end());
    wellPaths.assign(uniqueWellPaths.begin(), uniqueWellPaths.end());

    return wellPaths;
}

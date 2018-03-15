/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicImportSummaryCasesFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RicFileHierarchyDialog.h"
#include "RicSummaryCaseRestartDialog.h"

#include "RimGridSummaryCase.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlotCollection.h"

#include "RiuMainPlotWindow.h"
#include "RiuMainWindow.h"

#include "SummaryPlotCommands/RicNewSummaryPlotFeature.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicImportSummaryCasesFeature, "RicImportSummaryCasesFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportSummaryCasesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportSummaryCasesFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("INPUT_FILES");

    RicFileHierarchyDialogResult result = RicFileHierarchyDialog::getOpenFileNames(nullptr, "Import Summary Cases", defaultDir, m_pathFilter, m_fileNameFilter, QStringList(".SMSPEC"));

    // Remember filters
    m_pathFilter = result.pathFilter;
    m_fileNameFilter = result.fileNameFilter;

    if (!result.ok) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory("INPUT_FILES", QFileInfo(result.rootDir).absoluteFilePath());

    QStringList fileNames = result.files;
    if (fileNames.isEmpty()) return;

    RimProject*                     proj = app->project();
    RimSummaryCaseMainCollection*   sumCaseColl = proj->activeOilField() ? proj->activeOilField()->summaryCaseMainCollection() : nullptr;
    if (!sumCaseColl) return;

    for (auto f : fileNames)
    {
        RicImportSummaryCasesFeature::createAndAddSummaryCaseFromFile(f);
    }

    std::vector<RimCase*> cases;
    app->project()->allCases(cases);

    if (cases.size() == 0)
    {
        RiuMainWindow::instance()->close();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportSummaryCasesFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/SummaryCase48x48.png"));
    actionToSetup->setText("Import Summary Cases Recursively");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportSummaryCasesFeature::createAndAddSummaryCaseFromFile(const QString& fileName)
{
    RiaApplication* app = RiaApplication::instance();
    RimProject* proj = app->project();
    RimSummaryCaseMainCollection* sumCaseColl = proj->activeOilField() ? proj->activeOilField()->summaryCaseMainCollection() : nullptr;
    if (!sumCaseColl) return false;

    RicSummaryCaseRestartDialogResult result = RicSummaryCaseRestartDialog::openDialog(fileName);

    if (result.ok)
    {
        RimSummaryCase* sumCase = sumCaseColl->createAndAddSummaryCaseFromFileName(fileName, result.option == RicSummaryCaseRestartDialog::READ_ALL);
        sumCaseColl->updateAllRequiredEditors();

        RiuMainPlotWindow* mainPlotWindow = app->getOrCreateAndShowMainPlotWindow();
        if (mainPlotWindow)
        {
            mainPlotWindow->selectAsCurrentItem(sumCase);

            mainPlotWindow->updateSummaryPlotToolBar();
        }

        app->addToRecentFiles(fileName);
    }
    return true;
}


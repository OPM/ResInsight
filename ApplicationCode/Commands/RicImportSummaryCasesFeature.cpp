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

#include "RifSummaryCaseRestartSelector.h"

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
QString RicImportSummaryCasesFeature::m_pathFilter = "*";
QString RicImportSummaryCasesFeature::m_fileNameFilter = "*";

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
    RiaApplication*                 app = RiaApplication::instance();
    std::vector<RimSummaryCase*>    cases = importSummaryCases("Import Summary Cases");
    
    for (const auto& rimCase : cases) RiaApplication::instance()->addToRecentFiles(rimCase->summaryHeaderFilename());

    std::vector<RimCase*> allCases;
    app->project()->allCases(allCases);

    if (allCases.size() == 0)
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
bool RicImportSummaryCasesFeature::createAndAddSummaryCasesFromFiles(const QStringList& fileNames, std::vector<RimSummaryCase*>* newCases)
{
    RiaApplication* app = RiaApplication::instance();
    RimProject* proj = app->project();
    RimSummaryCaseMainCollection* sumCaseColl = proj->activeOilField() ? proj->activeOilField()->summaryCaseMainCollection() : nullptr;

    if (newCases) newCases->clear();
    if (!sumCaseColl) return false;

    RifSummaryCaseRestartSelector       fileSelector;
    std::vector<RifSummaryCaseFileInfo> importFileInfos = fileSelector.getFilesToImportFromSummaryFiles(fileNames);

    std::vector<RimSummaryCase*> sumCases = sumCaseColl->createAndAddSummaryCasesFromFileInfos(importFileInfos);
    sumCaseColl->updateAllRequiredEditors();

    RiuMainPlotWindow* mainPlotWindow = app->getOrCreateAndShowMainPlotWindow();
    if (mainPlotWindow && !sumCases.empty())
    {
        mainPlotWindow->selectAsCurrentItem(sumCases.back());

        mainPlotWindow->updateSummaryPlotToolBar();
    }

    if (newCases) newCases->insert(newCases->end(), sumCases.begin(), sumCases.end());
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RicImportSummaryCasesFeature::importSummaryCases(const QString& dialogTitle)
{
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("INPUT_FILES");

    RicFileHierarchyDialogResult result = RicFileHierarchyDialog::getOpenFileNames(nullptr, dialogTitle, defaultDir, m_pathFilter, m_fileNameFilter, QStringList(".SMSPEC"));

    // Remember filters
    m_pathFilter = result.pathFilter;
    m_fileNameFilter = result.fileNameFilter;

    if (!result.ok) return std::vector<RimSummaryCase*>();

    // Remember the path to next time
    app->setLastUsedDialogDirectory("INPUT_FILES", QFileInfo(result.rootDir).absoluteFilePath());

    QStringList fileNames = result.files;
    if (fileNames.isEmpty()) return std::vector<RimSummaryCase*>();

    std::vector<RimSummaryCase*> newCases;
    createAndAddSummaryCasesFromFiles(fileNames, &newCases);
    return newCases;
}

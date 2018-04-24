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
#include "RiaLogging.h"
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
#include <QMessageBox>

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
    
    addSummaryCases(cases);

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
    std::vector<RimSummaryCase*> temp;
    std::vector<RimSummaryCase*>* cases = newCases ? newCases : &temp;
    if (createSummaryCasesFromFiles(fileNames, cases))
    {
        addSummaryCases(*cases);
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportSummaryCasesFeature::createSummaryCasesFromFiles(const QStringList& fileNames, std::vector<RimSummaryCase*>* newCases)
{
    RiaApplication* app = RiaApplication::instance();
    RimProject* proj = app->project();
    RimSummaryCaseMainCollection* sumCaseColl = proj->activeOilField() ? proj->activeOilField()->summaryCaseMainCollection() : nullptr;

    if (newCases) newCases->clear();
    if (!sumCaseColl) return false;

    RifSummaryCaseRestartSelector       fileSelector;
    fileSelector.determineFilesToImportFromSummaryFiles(fileNames);

    std::vector<RifSummaryCaseFileInfo> importFileInfos = fileSelector.summaryFileInfos();

    if (!importFileInfos.empty())
    {
        std::vector<RimSummaryCase*> sumCases = sumCaseColl->createSummaryCasesFromFileInfos(importFileInfos);
        if (newCases) newCases->insert(newCases->end(), sumCases.begin(), sumCases.end());
    }

    if (fileSelector.foundErrors())
    {
        QString errorMessage = fileSelector.createCombinedErrorMessage();
        RiaLogging::error(errorMessage);
        QMessageBox::warning(NULL, QString("Problem Importing Summary Case File(s)"), errorMessage);
    }

    return !importFileInfos.empty();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportSummaryCasesFeature::addSummaryCases(const std::vector<RimSummaryCase*> cases)
{
    RiaApplication* app = RiaApplication::instance();
    RimProject* proj = app->project();
    RimSummaryCaseMainCollection* sumCaseColl = proj->activeOilField() ? proj->activeOilField()->summaryCaseMainCollection() : nullptr;
    sumCaseColl->addCases(cases);
    sumCaseColl->updateAllRequiredEditors();

    RiuMainPlotWindow* mainPlotWindow = app->getOrCreateAndShowMainPlotWindow();
    if (mainPlotWindow && !cases.empty())
    {
        mainPlotWindow->selectAsCurrentItem(cases.back());

        mainPlotWindow->updateSummaryPlotToolBar();
    }
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
    createSummaryCasesFromFiles(fileNames, &newCases);
    return newCases;
}

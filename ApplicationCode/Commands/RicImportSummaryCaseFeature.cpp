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

#include "RicImportSummaryCaseFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RimGridSummaryCase.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlotCollection.h"

#include "RiuMainWindow.h"

#include "SummaryPlotCommands/RicNewSummaryPlotFeature.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicImportSummaryCaseFeature, "RicImportSummaryCaseFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportSummaryCaseFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportSummaryCaseFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("INPUT_FILES");
    QStringList fileNames = QFileDialog::getOpenFileNames(NULL, "Import Summary Case", defaultDir, "Eclipse Summary File (*.SMSPEC);;All Files (*.*)");

    if (fileNames.isEmpty()) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory("INPUT_FILES", QFileInfo(fileNames.last()).absolutePath());

    RimProject* proj = app->project();

    RimSummaryCaseMainCollection* sumCaseColl = proj->activeOilField() ? proj->activeOilField()->summaryCaseMainCollection() : nullptr;
    if (!sumCaseColl) return;

    for (auto f : fileNames)
    {
        RicImportSummaryCaseFeature::createAndAddSummaryCaseFromFile(f);
    }

    app->getOrCreateAndShowMainPlotWindow();

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
void RicImportSummaryCaseFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/SummaryCase48x48.png"));
    actionToSetup->setText("Import Summary Case");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportSummaryCaseFeature::createAndAddSummaryCaseFromFile(const QString& fileName)
{
    RiaApplication* app = RiaApplication::instance();
    RimProject* proj = app->project();
    RimSummaryCaseMainCollection* sumCaseColl = proj->activeOilField() ? proj->activeOilField()->summaryCaseMainCollection() : nullptr;
    if (!sumCaseColl) return false;

    RimSummaryCase* newSumCase = sumCaseColl->createAndAddSummaryCaseFromFileName(fileName);
    newSumCase->loadCase();


    if (app->preferences()->autoCreatePlotsOnImport())
    {
        RimMainPlotCollection* mainPlotColl = proj->mainPlotCollection();
        RimSummaryPlotCollection* summaryPlotColl = mainPlotColl->summaryPlotCollection();

        RicNewSummaryPlotFeature::createNewSummaryPlot(summaryPlotColl, newSumCase);
    }

    sumCaseColl->updateConnectedEditors();
    app->addToRecentFiles(fileName);

    return true;
}


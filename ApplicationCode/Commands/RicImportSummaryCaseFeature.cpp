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
#include "RiaFilePathTools.h"

#include "RicImportSummaryCasesFeature.h"

#include "RimGridSummaryCase.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlotCollection.h"

#include "RiuPlotMainWindow.h"
#include "RiuMainWindow.h"

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
    RiaPreferences* prefs = app->preferences();
    QString defaultDir = app->lastUsedDialogDirectory("INPUT_FILES");
    QStringList fileNames_ = QFileDialog::getOpenFileNames(nullptr, "Import Summary Case", defaultDir, "Eclipse Summary File (*.SMSPEC);;All Files (*.*)");

    if (fileNames_.isEmpty()) return;

    QStringList fileNames;

    // Convert to internal path separator
    for (QString s : fileNames_)
    {
        fileNames.push_back(RiaFilePathTools::toInternalSeparator(s));
    }

    // Remember the path to next time
    app->setLastUsedDialogDirectory("INPUT_FILES", QFileInfo(fileNames.last()).absolutePath());

    if (fileNames.isEmpty()) return;

    std::vector<RimSummaryCase*> newCases;
    if (RicImportSummaryCasesFeature::createAndAddSummaryCasesFromFiles(fileNames, &newCases))
    {
        RicImportSummaryCasesFeature::addCasesToGroupIfRelevant(newCases);
        for (const RimSummaryCase* newCase : newCases)
        {
            RiaApplication::instance()->addToRecentFiles(newCase->summaryHeaderFilename());
        }

        RiuPlotMainWindow* mainPlotWindow = app->getOrCreateAndShowMainPlotWindow();
        if (mainPlotWindow && !newCases.empty())
        {
            mainPlotWindow->selectAsCurrentItem(newCases.back());

            mainPlotWindow->updateSummaryPlotToolBar();
        }
    }

    std::vector<RimCase*> cases;
    app->project()->allCases(cases);
    if (cases.size() == 0 && !newCases.empty())
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

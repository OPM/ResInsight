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

#include "RicImportEnsembleFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RicImportSummaryCasesFeature.h"
#include "RicCreateSummaryCaseCollectionFeature.h"

#include "RifSummaryCaseRestartSelector.h"

#include "RimGridSummaryCase.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlotCollection.h"

#include "RiuPlotMainWindow.h"
#include "RiuMainWindow.h"

#include "SummaryPlotCommands/RicNewSummaryPlotFeature.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>


CAF_CMD_SOURCE_INIT(RicImportEnsembleFeature, "RicImportEnsembleFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportEnsembleFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app   = RiaApplication::instance();
    QStringList fileNames = RicImportSummaryCasesFeature::runRecursiveSummaryCaseFileSearchDialog("Import Ensemble");
    
    if (fileNames.isEmpty()) return;

    QString ensembleName = askForEnsembleName();
    if (ensembleName.isEmpty()) return;

    std::vector<RimSummaryCase*> cases;
    RicImportSummaryCasesFeature::createSummaryCasesFromFiles(fileNames, &cases, true);

    RicImportSummaryCasesFeature::addSummaryCases(cases);
    auto newGroup = RicCreateSummaryCaseCollectionFeature::groupSummaryCases(cases, ensembleName, true);

    RiuPlotMainWindow* mainPlotWindow = app->getOrCreateAndShowMainPlotWindow();
    if (mainPlotWindow && newGroup)
    {
        mainPlotWindow->selectAsCurrentItem(newGroup);
        mainPlotWindow->updateSummaryPlotToolBar();
    }

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
void RicImportEnsembleFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/SummaryEnsemble16x16.png"));
    actionToSetup->setText("Import Ensemble");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicImportEnsembleFeature::askForEnsembleName()
{
    RimProject* project = RiaApplication::instance()->project();
    std::vector<RimSummaryCaseCollection*> groups = project->summaryGroups();
    int ensembleCount = std::count_if(groups.begin(), groups.end(), [](RimSummaryCaseCollection* group) { return group->isEnsemble(); });
    ensembleCount += 1;

    QInputDialog dialog;
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setWindowTitle("Ensemble Name");
    dialog.setLabelText("Ensemble Name");
    dialog.setTextValue(QString("Ensemble %1").arg(ensembleCount));
    dialog.resize(300, 50);
    dialog.exec();
    return dialog.result() == QDialog::Accepted ? dialog.textValue() : QString("");
}

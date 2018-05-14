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

#include "RicImportSummaryGroupFeature.h"

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
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlotCollection.h"

#include "RiuPlotMainWindow.h"
#include "RiuMainWindow.h"

#include "SummaryPlotCommands/RicNewSummaryPlotFeature.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>


CAF_CMD_SOURCE_INIT(RicImportSummaryGroupFeature, "RicImportSummaryGroupFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportSummaryGroupFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportSummaryGroupFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app   = RiaApplication::instance();
    QStringList fileNames = RicImportSummaryCasesFeature::runRecursiveSummaryCaseFileSearchDialog("Import Summary Case Group");
    
    if (fileNames.isEmpty()) return;

    std::vector<RimSummaryCase*> cases;
    RicImportSummaryCasesFeature::createSummaryCasesFromFiles(fileNames, &cases);

    RicImportSummaryCasesFeature::addSummaryCases(cases);
    RicCreateSummaryCaseCollectionFeature::groupSummaryCases(cases, "", false);

    RiuPlotMainWindow* mainPlotWindow = app->getOrCreateAndShowMainPlotWindow();
    if (mainPlotWindow && !cases.empty())
    {
        mainPlotWindow->selectAsCurrentItem(cases.back());

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
void RicImportSummaryGroupFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/SummaryGroup16x16.png"));
    actionToSetup->setText("Import Summary Case Group");
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicDeleteSummaryCaseCollectionFeature.h"

#include "RiaApplication.h"

#include "RiaSummaryTools.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT(RicDeleteSummaryCaseCollectionFeature, "RicDeleteSummaryCaseCollectionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteSummaryCaseCollectionFeature::deleteSummaryCaseCollection(RimSummaryCaseCollection* caseCollection)
{
    RimSummaryPlotCollection* summaryPlotColl = RiaSummaryTools::summaryPlotCollection();

    for (RimSummaryCase* summaryCase : caseCollection->allSummaryCases())
    {
        for (RimSummaryPlot* summaryPlot : summaryPlotColl->summaryPlots)
        {
            summaryPlot->deleteCurvesAssosiatedWithCase(summaryCase);
        }
    }
    
    summaryPlotColl->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicDeleteSummaryCaseCollectionFeature::isCommandEnabled()
{
    std::vector<RimSummaryCaseCollection*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    return (selection.size() > 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteSummaryCaseCollectionFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSummaryCaseCollection*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    if (selection.size() == 0) return;

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);

    QString questionText;
    questionText = QString("Do you also want to close the summary cases in the group?");

    msgBox.setText(questionText);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    int ret = msgBox.exec();

    if (ret == QMessageBox::Cancel)
    {
        return;
    }
    
    if (ret == QMessageBox::Yes)
    {
        for (RimSummaryCaseCollection* summaryCaseCollection : selection)
        {
            RicDeleteSummaryCaseCollectionFeature::deleteSummaryCaseCollection(summaryCaseCollection);
        }
    }
    else if (ret == QMessageBox::No)
    {
        for (RimSummaryCaseCollection* summaryCaseCollection : selection)
        {
            RicDeleteSummaryCaseCollectionFeature::moveAllCasesToMainSummaryCollection(summaryCaseCollection);
        }
    }

    RimSummaryCaseMainCollection* summaryCaseMainCollection = nullptr;
    selection[0]->firstAncestorOrThisOfTypeAsserted(summaryCaseMainCollection);

    for (RimSummaryCaseCollection* caseCollection : selection)
    {
        summaryCaseMainCollection->removeCaseCollection(caseCollection);
        delete caseCollection;
    }

    summaryCaseMainCollection->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteSummaryCaseCollectionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Delete Summary Case Group/Ensemble");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicDeleteSummaryCaseCollectionFeature::moveAllCasesToMainSummaryCollection(RimSummaryCaseCollection* summaryCaseCollection)
{
    std::vector<RimSummaryCase*> summaryCases = summaryCaseCollection->allSummaryCases();
    if (summaryCases.size() == 0) return;

    RimSummaryCaseMainCollection* summaryCaseMainCollection = nullptr;
    summaryCaseCollection->firstAncestorOrThisOfType(summaryCaseMainCollection);

    for (RimSummaryCase* summaryCase : summaryCases)
    {
        summaryCaseCollection->removeCase(summaryCase);
        summaryCaseMainCollection->addCase(summaryCase);
    }
}

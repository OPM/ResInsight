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

#include "RicNewDerivedEnsembleFeature.h"

#include "RiaApplication.h"

#include "RimDerivedEnsembleCaseCollection.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QMessageBox>

#include <memory>


CAF_CMD_SOURCE_INIT(RicNewDerivedEnsembleFeature, "RicNewDerivedEnsembleFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewDerivedEnsembleFeature::showWarningDialog()
{
    QMessageBox::warning(nullptr, "Ensemble Matching", "None of the cases in the ensembles match");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewDerivedEnsembleFeature::showWarningDialogWithQuestion()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle("Ensemble Matching");
    msgBox.setText("None of the cases in the ensembles match");
    msgBox.setInformativeText("Do you want to keep the derived ensemble?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    int ret = msgBox.exec();
    return ret == QMessageBox::Yes;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewDerivedEnsembleFeature::isCommandEnabled()
{
    std::vector<RimSummaryCaseMainCollection*> mainColls = caf::selectedObjectsByTypeStrict<RimSummaryCaseMainCollection*>();
    std::vector<RimSummaryCaseCollection*> ensembles = caf::selectedObjectsByTypeStrict<RimSummaryCaseCollection*>();

    return mainColls.size() == 1 || ensembles.size() == 1 || ensembles.size() == 2;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewDerivedEnsembleFeature::onActionTriggered(bool isChecked)
{
    if (isCommandEnabled())
    {
        auto project = RiaApplication::instance()->project();
        auto mainColl = project->firstSummaryCaseMainCollection();

        auto newColl = mainColl->addCaseCollection({}, "", true, []() {return new RimDerivedEnsembleCaseCollection(); });
        auto newEnsemble = dynamic_cast<RimDerivedEnsembleCaseCollection*>(newColl);

        {
            std::vector<RimSummaryCaseCollection*> ensembles = caf::selectedObjectsByType<RimSummaryCaseCollection*>();

            if (ensembles.size() >= 1) newEnsemble->setEnsemble1(ensembles[0]);
            if (ensembles.size() == 2)
            {
                newEnsemble->setEnsemble2(ensembles[1]);
                newEnsemble->updateDerivedEnsembleCases();

                if (newEnsemble->allSummaryCases().empty())
                {
                    if(!showWarningDialogWithQuestion())
                    {
                        mainColl->removeCaseCollection(newEnsemble);
                    }
                }
            }
        }
        
        mainColl->updateConnectedEditors();
        RiuPlotMainWindowTools::selectAsCurrentItem(newEnsemble);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewDerivedEnsembleFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Derived Ensemble");
    actionToSetup->setIcon(QIcon(":/SummaryEnsemble16x16.png"));
}


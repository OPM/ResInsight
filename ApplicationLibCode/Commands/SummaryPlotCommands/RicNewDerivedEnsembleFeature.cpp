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

#include "RimDeltaSummaryEnsemble.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QMessageBox>

#include <memory>

CAF_CMD_SOURCE_INIT( RicNewDerivedEnsembleFeature, "RicNewDerivedEnsembleFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewDerivedEnsembleFeature::showWarningDialog()
{
    QMessageBox::warning( nullptr, "Ensemble Matching", "None of the cases in the ensembles match" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewDerivedEnsembleFeature::showWarningDialogWithQuestion()
{
    QMessageBox msgBox;
    msgBox.setIcon( QMessageBox::Question );
    msgBox.setWindowTitle( "Ensemble Matching" );
    msgBox.setText( "None of the cases in the ensembles match" );
    msgBox.setInformativeText( "Do you want to keep the delta ensemble?" );
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );

    int ret = msgBox.exec();
    return ret == QMessageBox::Yes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewDerivedEnsembleFeature::isCommandEnabled() const
{
    std::vector<RimSummaryCaseMainCollection*> mainColls = caf::selectedObjectsByTypeStrict<RimSummaryCaseMainCollection*>();
    std::vector<RimSummaryEnsemble*>           ensembles = caf::selectedObjectsByTypeStrict<RimSummaryEnsemble*>();

    return mainColls.size() == 1 || ensembles.size() == 1 || ensembles.size() == 2;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewDerivedEnsembleFeature::onActionTriggered( bool isChecked )
{
    if ( isCommandEnabled() )
    {
        auto project  = RimProject::current();
        auto mainColl = project->firstSummaryCaseMainCollection();

        auto newColl     = mainColl->addEnsemble( {}, "", true, []() { return new RimDeltaSummaryEnsemble(); } );
        auto newEnsemble = dynamic_cast<RimDeltaSummaryEnsemble*>( newColl );

        {
            std::vector<RimSummaryEnsemble*> ensembles = caf::selectedObjectsByType<RimSummaryEnsemble*>();

            if ( !ensembles.empty() ) newEnsemble->setEnsemble1( ensembles[0] );
            if ( ensembles.size() == 2 )
            {
                newEnsemble->setEnsemble2( ensembles[1] );
                newEnsemble->createDerivedEnsembleCases();

                if ( newEnsemble->allSummaryCases().empty() )
                {
                    if ( !showWarningDialogWithQuestion() )
                    {
                        mainColl->removeEnsemble( newEnsemble );
                    }
                }
            }
        }

        mainColl->updateConnectedEditors();
        RiuPlotMainWindowTools::selectAsCurrentItem( newEnsemble );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewDerivedEnsembleFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Delta Ensemble" );
    actionToSetup->setIcon( QIcon( ":/SummaryEnsemble.svg" ) );
}

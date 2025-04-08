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

#include "Summary/RiaSummaryTools.h"

#include "RimDeltaSummaryEnsemble.h"
#include "RimDepthTrackPlot.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"
#include "RimWellPlotTools.h"

#include "cafPdmObject.h"
#include "cafPdmObjectHandleTools.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicDeleteSummaryCaseCollectionFeature, "RicDeleteSummaryCaseCollectionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSummaryCaseCollectionFeature::deleteSummaryCaseCollection( RimSummaryEnsemble* caseCollection )
{
    RimSummaryMultiPlotCollection* summaryPlotColl = RiaSummaryTools::summaryMultiPlotCollection();

    for ( RimSummaryCase* summaryCase : caseCollection->allSummaryCases() )
    {
        for ( RimSummaryMultiPlot* multiPlot : summaryPlotColl->multiPlots() )
        {
            for ( RimSummaryPlot* summaryPlot : multiPlot->summaryPlots() )
            {
                summaryPlot->deleteCurvesAssosiatedWithCase( summaryCase );
            }
            multiPlot->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteSummaryCaseCollectionFeature::isCommandEnabled() const
{
    auto selection = caf::SelectionManager::instance()->objectsByType<RimSummaryEnsemble>();

    selection.erase( std::remove_if( selection.begin(),
                                     selection.end(),
                                     []( RimSummaryEnsemble* coll ) { return dynamic_cast<RimDeltaSummaryEnsemble*>( coll ) != nullptr; } ),
                     selection.end() );
    return ( !selection.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSummaryCaseCollectionFeature::onActionTriggered( bool isChecked )
{
    const auto ensembles = caf::SelectionManager::instance()->objectsByType<RimSummaryEnsemble>();
    if ( ensembles.empty() ) return;

    QMessageBox msgBox;
    msgBox.setIcon( QMessageBox::Question );

    QString questionText;
    questionText = QString( "Do you also want to close the summary cases in the group?" );

    msgBox.setText( questionText );
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );

    int ret = msgBox.exec();

    if ( ret == QMessageBox::Cancel )
    {
        return;
    }

    auto depthTrackPlots = caf::PdmObjectHandleTools::referringAncestorOfType<RimDepthTrackPlot, RimSummaryEnsemble>( ensembles );

    for ( RimSummaryEnsemble* ensemble : ensembles )
    {
        if ( ret == QMessageBox::Yes )
        {
            RicDeleteSummaryCaseCollectionFeature::deleteSummaryCaseCollection( ensemble );
        }
        else if ( ret == QMessageBox::No )
        {
            RicDeleteSummaryCaseCollectionFeature::moveAllCasesToMainSummaryCollection( ensemble );
        }
    }

    RimSummaryCaseMainCollection* summaryCaseMainCollection = ensembles[0]->firstAncestorOrThisOfTypeAsserted<RimSummaryCaseMainCollection>();
    for ( RimSummaryEnsemble* ensemble : ensembles )
    {
        summaryCaseMainCollection->removeEnsemble( ensemble );
        delete ensemble;
    }

    RimWellPlotTools::loadDataAndUpdateDepthTrackPlots( depthTrackPlots );

    summaryCaseMainCollection->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSummaryCaseCollectionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete Summary Case Group/Ensemble" );
    actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteSummaryCaseCollectionFeature::moveAllCasesToMainSummaryCollection( RimSummaryEnsemble* summaryCaseCollection )
{
    std::vector<RimSummaryCase*> summaryCases = summaryCaseCollection->allSummaryCases();
    if ( summaryCases.empty() ) return;

    RimSummaryCaseMainCollection* summaryCaseMainCollection = summaryCaseCollection->firstAncestorOrThisOfType<RimSummaryCaseMainCollection>();

    for ( RimSummaryCase* summaryCase : summaryCases )
    {
        summaryCaseCollection->removeCase( summaryCase );
        summaryCaseMainCollection->addCase( summaryCase );
    }
}

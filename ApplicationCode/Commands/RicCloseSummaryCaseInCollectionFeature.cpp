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

#include "RicCloseSummaryCaseInCollectionFeature.h"

#include "RiaGuiApplication.h"

#include "RicCloseSummaryCaseFeature.h"

#include "RimDerivedEnsembleCaseCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuPlotMainWindow.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include <vector>

CAF_CMD_SOURCE_INIT( RicCloseSummaryCaseInCollectionFeature, "RicCloseSummaryCaseInCollectionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCloseSummaryCaseInCollectionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Close Summary Cases" );
    actionToSetup->setIcon( QIcon( ":/Erase.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCloseSummaryCaseInCollectionFeature::isCommandEnabled()
{
    std::vector<RimSummaryCaseMainCollection*> summaryCaseMainCollections;
    caf::SelectionManager::instance()->objectsByType( &summaryCaseMainCollections );

    std::vector<RimSummaryCaseCollection*> summaryCaseCollections;
    caf::SelectionManager::instance()->objectsByType( &summaryCaseCollections );

    summaryCaseCollections.erase( std::remove_if( summaryCaseCollections.begin(),
                                                  summaryCaseCollections.end(),
                                                  []( RimSummaryCaseCollection* coll ) {
                                                      return dynamic_cast<RimDerivedEnsembleCaseCollection*>( coll ) !=
                                                             nullptr;
                                                  } ),
                                  summaryCaseCollections.end() );

    return ( summaryCaseMainCollections.size() > 0 || summaryCaseCollections.size() > 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCloseSummaryCaseInCollectionFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimSummaryCaseMainCollection*> summaryCaseMainCollections;
    caf::SelectionManager::instance()->objectsByType( &summaryCaseMainCollections );

    if ( summaryCaseMainCollections.size() > 0 )
    {
        std::vector<RimSummaryCase*> allSummaryCases = summaryCaseMainCollections[0]->allSummaryCases();
        RicCloseSummaryCaseFeature::deleteSummaryCases( allSummaryCases );
    }

    std::vector<RimSummaryCaseCollection*> summaryCaseCollections;
    caf::SelectionManager::instance()->objectsByType( &summaryCaseCollections );

    for ( RimSummaryCaseCollection* summaryCaseCollection : summaryCaseCollections )
    {
        std::vector<RimSummaryCase*> collectionSummaryCases = summaryCaseCollection->allSummaryCases();
        RicCloseSummaryCaseFeature::deleteSummaryCases( collectionSummaryCases );
    }

    RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
    mainPlotWindow->updateSummaryPlotToolBar();
}

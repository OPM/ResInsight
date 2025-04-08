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

#include "RimDeltaSummaryEnsemble.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlot.h"

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
    actionToSetup->setIcon( QIcon( ":/Close.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCloseSummaryCaseInCollectionFeature::isCommandEnabled() const
{
    auto summaryCaseMainCollections = caf::SelectionManager::instance()->objectsByType<RimSummaryCaseMainCollection>();
    auto summaryCaseCollections     = caf::SelectionManager::instance()->objectsByType<RimSummaryEnsemble>();

    summaryCaseCollections.erase( std::remove_if( summaryCaseCollections.begin(),
                                                  summaryCaseCollections.end(),
                                                  []( RimSummaryEnsemble* coll )
                                                  { return dynamic_cast<RimDeltaSummaryEnsemble*>( coll ) != nullptr; } ),
                                  summaryCaseCollections.end() );

    return ( !summaryCaseMainCollections.empty() || !summaryCaseCollections.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCloseSummaryCaseInCollectionFeature::onActionTriggered( bool isChecked )
{
    auto summaryCaseMainCollections = caf::SelectionManager::instance()->objectsByType<RimSummaryCaseMainCollection>();
    if ( !summaryCaseMainCollections.empty() )
    {
        std::vector<RimSummaryCase*> allSummaryCases = summaryCaseMainCollections[0]->allSummaryCases();
        RicCloseSummaryCaseFeature::deleteSummaryCases( allSummaryCases );
    }

    auto summaryCaseCollections = caf::SelectionManager::instance()->objectsByType<RimSummaryEnsemble>();
    for ( RimSummaryEnsemble* summaryCaseCollection : summaryCaseCollections )
    {
        std::vector<RimSummaryCase*> collectionSummaryCases = summaryCaseCollection->allSummaryCases();
        RicCloseSummaryCaseFeature::deleteSummaryCases( collectionSummaryCases );
    }

    RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
    mainPlotWindow->updateMultiPlotToolBar();
}

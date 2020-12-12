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

#include "RicNewEnsembleCurveFilterFeature.h"

#include "RimEnsembleCurveFilter.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimEnsembleCurveSet.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewEnsembleCurveFilterFeature, "RicNewEnsembleCurveFilterFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewEnsembleCurveFilterFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEnsembleCurveFilterFeature::onActionTriggered( bool isChecked )
{
    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( !selObj ) return;

    RimEnsembleCurveFilterCollection* filterCollection = nullptr;
    {
        std::vector<RimEnsembleCurveFilterCollection*> filterColls;
        selObj->descendantsIncludingThisOfType( filterColls );
        if ( filterColls.size() == 1 )
        {
            filterCollection = filterColls.front();
        }
    }

    if ( filterCollection )
    {
        RimEnsembleCurveFilter* newFilter = filterCollection->addFilter();
        if ( !filterCollection->filters().empty() )
        {
            auto existingFilter = filterCollection->filters().front();
            newFilter->setSummaryAddresses( existingFilter->summaryAddresses() );
        }
        else
        {
            std::vector<RifEclipseSummaryAddress> addresses;
            addresses.push_back( newFilter->parentCurveSet()->summaryAddress() );
            newFilter->setSummaryAddresses( addresses );
        }
        newFilter->loadDataAndUpdate();
        filterCollection->updateConnectedEditors();

        selObj->updateConnectedEditors();

        RiuPlotMainWindowTools::selectAsCurrentItem( newFilter );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEnsembleCurveFilterFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Ensemble Curve Filter" );
    actionToSetup->setIcon( QIcon( ":/Filter.svg" ) );
}

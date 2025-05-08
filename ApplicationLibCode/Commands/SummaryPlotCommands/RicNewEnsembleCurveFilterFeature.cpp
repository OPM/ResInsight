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
#include "RimObjectiveFunctionTools.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewEnsembleCurveFilterFeature, "RicNewEnsembleCurveFilterFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEnsembleCurveFilterFeature::onActionTriggered( bool isChecked )
{
    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( !selObj ) return;

    RimEnsembleCurveFilterCollection* filterCollection = nullptr;
    {
        std::vector<RimEnsembleCurveFilterCollection*> filterColls =
            selObj->descendantsIncludingThisOfType<RimEnsembleCurveFilterCollection>();
        if ( filterColls.size() == 1 )
        {
            filterCollection = filterColls.front();
        }
    }

    if ( filterCollection )
    {
        bool                    firstFilter = filterCollection->filters().empty();
        RimEnsembleCurveFilter* newFilter   = filterCollection->addFilter();
        if ( !firstFilter )
        {
            auto existingFilter = filterCollection->filters().front();
            newFilter->setSummaryAddresses( existingFilter->summaryAddresses() );
        }
        else
        {
            std::vector<RifEclipseSummaryAddress> addresses;

            auto curveSet           = newFilter->parentCurveSet();
            auto candidateAdr       = curveSet->summaryAddressY();
            auto nativeQuantityName = RimObjectiveFunctionTools::nativeQuantityName( candidateAdr.vectorName() );
            candidateAdr.setVectorName( nativeQuantityName );
            addresses.push_back( candidateAdr );
            newFilter->setSummaryAddresses( addresses );

            curveSet->setDefaultTimeRange();
            curveSet->loadDataAndUpdate( true );
        }
        newFilter->updateMaxMinAndDefaultValuesFromParent();
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

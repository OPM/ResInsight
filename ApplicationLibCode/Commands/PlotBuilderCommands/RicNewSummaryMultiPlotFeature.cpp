/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RicNewSummaryMultiPlotFeature.h"

#include "RimMultiPlotCollection.h"
#include "RimPlot.h"
#include "RimSummaryMultiPlot.h"

#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>

RICF_SOURCE_INIT( RicNewSummaryMultiPlotFeature, "RicNewSummaryMultiPlotFeature", "createSummaryMultiPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicNewSummaryMultiPlotFeature::RicNewSummaryMultiPlotFeature()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicNewSummaryMultiPlotFeature::execute()
{
    std::vector<RimSummaryPlot*> plots;
    RimSummaryMultiPlot::createAndAppendMultiPlot( plots );

    return caf::PdmScriptResponse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryMultiPlotFeature::isCommandEnabled()
{
    RimMultiPlotCollection* objToFind = nullptr;

    auto pdmUiItem = caf::SelectionManager::instance()->selectedItem();
    auto objHandle = dynamic_cast<caf::PdmObjectHandle*>( pdmUiItem );
    if ( objHandle )
    {
        objHandle->firstAncestorOrThisOfType( objToFind );
    }

    return ( objToFind != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryMultiPlotFeature::onActionTriggered( bool isChecked )
{
    execute();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryMultiPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Summary Multi Plot" );
    actionToSetup->setIcon( QIcon( ":/MultiPlot16x16.png" ) );
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RicNewDefaultHistogramPlotFeature.h"

#include "Histogram/RimHistogramMultiPlot.h"
#include "Histogram/RimHistogramPlot.h"

#include "RicHistogramPlotTools.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewDefaultHistogramPlotFeature, "RicNewDefaultHistogramPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewDefaultHistogramPlotFeature::isCommandEnabled() const
{
    RimHistogramMultiPlot* multiPlot = dynamic_cast<RimHistogramMultiPlot*>( caf::SelectionManager::instance()->selectedItem() );
    return multiPlot != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewDefaultHistogramPlotFeature::onActionTriggered( bool isChecked )
{
    RimHistogramMultiPlot* multiPlot = dynamic_cast<RimHistogramMultiPlot*>( caf::SelectionManager::instance()->selectedItem() );
    if ( multiPlot )
    {
        auto dataSourceType = caf::AppEnum<RicHistogramPlotTools::DataSourceType>::fromText( userData().toString() );
        auto plot           = RicHistogramPlotTools::addNewHistogramPlot( multiPlot );
        RicHistogramPlotTools::createDefaultHistogramCurve( plot, dataSourceType );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewDefaultHistogramPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Add Histogram Plot" );
    actionToSetup->setIcon( QIcon( ":/SummaryPlotLight16x16.png" ) );
}

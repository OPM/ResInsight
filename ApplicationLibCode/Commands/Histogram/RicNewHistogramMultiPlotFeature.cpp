/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RicNewHistogramMultiPlotFeature.h"

#include "Histogram/RimHistogramMultiPlotCollection.h"

#include "RicHistogramPlotTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewHistogramMultiPlotFeature, "RicNewHistogramMultiPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewHistogramMultiPlotFeature::isCommandEnabled() const
{
    std::vector<RimHistogramMultiPlotCollection*> selectedItems =
        caf::SelectionManager::instance()->objectsByType<RimHistogramMultiPlotCollection>();

    return ( selectedItems.size() == 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewHistogramMultiPlotFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimHistogramMultiPlotCollection*> selectedItems =
        caf::SelectionManager::instance()->objectsByType<RimHistogramMultiPlotCollection>();
    if ( selectedItems.size() != 1 ) return;

    if ( RimHistogramMultiPlotCollection* coll = selectedItems[0] )
    {
        auto dataSourceType = caf::AppEnum<RicHistogramPlotTools::DataSourceType>::fromText( userData().toString() );
        auto multiplot      = RicHistogramPlotTools::addNewHistogramMultiplot( coll );
        auto plot           = RicHistogramPlotTools::addNewHistogramPlot( multiplot );
        RicHistogramPlotTools::createDefaultHistogramCurve( plot, dataSourceType );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewHistogramMultiPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Histogram Plot" );
    actionToSetup->setIcon( QIcon( ":/MultiPlot16x16.png" ) );
}

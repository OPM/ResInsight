/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RicNewPlotDataFilterFeature.h"

#include "RimAnalysisPlot.h"
#include "RimAnalysisPlotCollection.h"
#include "RimPlotDataFilterCollection.h"
#include "RimPlotDataFilterItem.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewPlotDataFilterFeature, "RicNewPlotDataFilterFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewPlotDataFilterFeature::isCommandEnabled()
{
    RimAnalysisPlot* analysisPlot = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimAnalysisPlot>();

    if ( analysisPlot ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPlotDataFilterFeature::onActionTriggered( bool isChecked )
{
    RimAnalysisPlot* analysisPlot = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimAnalysisPlot>();

    if ( !analysisPlot ) return;

    RimPlotDataFilterItem* newFilter = analysisPlot->plotDataFilterCollection()->addFilter();

    analysisPlot->updateConnectedEditors();

    RiuPlotMainWindowTools::setExpanded( newFilter );
    RiuPlotMainWindowTools::selectAsCurrentItem( newFilter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPlotDataFilterFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Plot Data Filter" );
    actionToSetup->setIcon( QIcon( ":/AnalysisPlotFilter16x16.png" ) );
}

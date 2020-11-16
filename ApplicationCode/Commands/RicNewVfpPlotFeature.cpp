/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020  Equinor ASA
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

#include "RicNewVfpPlotFeature.h"

#include "RiaApplication.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimVfpPlot.h"
#include "RimVfpPlotCollection.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

#include <vector>

CAF_CMD_SOURCE_INIT( RicNewVfpPlotFeature, "RicNewVfpPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewVfpPlotFeature::isCommandEnabled()
{
    RimVfpPlotCollection* plotColl = caf::firstAncestorOfTypeFromSelectedObject<RimVfpPlotCollection*>();
    return ( plotColl != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewVfpPlotFeature::onActionTriggered( bool isChecked )
{
    RimProject* proj = RiaApplication::instance()->project();

    RimVfpPlotCollection* vfpPlotColl = proj->mainPlotCollection()->vfpPlotCollection();
    if ( vfpPlotColl )
    {
        RimVfpPlot* vfpPlot = new RimVfpPlot();
        vfpPlotColl->addPlot( vfpPlot );
        vfpPlotColl->updateConnectedEditors();
        vfpPlot->loadDataAndUpdate();

        RiuPlotMainWindowTools::showPlotMainWindow();
        RiuPlotMainWindowTools::selectAsCurrentItem( vfpPlot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewVfpPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New VFP Plot" );
    // TODO: add icon
    // actionToSetup->setIcon( QIcon( ":/VerticalFlowPerformancePlot16x16.png" ) );
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicNewEnsembleWellLogCurveSetFeature.h"

#include "RimEnsembleWellLogCurveSet.h"
#include "RimWellLogTrack.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

#include <vector>

CAF_CMD_SOURCE_INIT( RicNewEnsembleWellLogCurveSetFeature, "RicNewEnsembleWellLogCurveSetFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewEnsembleWellLogCurveSetFeature::isCommandEnabled()
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimWellLogTrack>() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEnsembleWellLogCurveSetFeature::onActionTriggered( bool isChecked )
{
    RimWellLogTrack* wellLogPlotTrack = caf::SelectionManager::instance()->selectedItemOfType<RimWellLogTrack>();
    if ( wellLogPlotTrack )
    {
        RimEnsembleWellLogCurveSet* curveSet = new RimEnsembleWellLogCurveSet();
        wellLogPlotTrack->setEnsembleWellLogCurveSet( curveSet );
        wellLogPlotTrack->updateEditors();
        RiuPlotMainWindowTools::selectAsCurrentItem( curveSet );
    }
    RiuPlotMainWindowTools::refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEnsembleWellLogCurveSetFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Ensemble Well Log Curve Set" );
    actionToSetup->setIcon( QIcon( ":/EnsembleCurveSet16x16.png" ) );
}

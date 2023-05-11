/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RicNewWellLogDiffCurveFeature.h"

#include "RicWellLogTools.h"

#include "RimWellLogCurve.h"
#include "RimWellLogDiffCurve.h"

#include "cafSelectionManager.h"

#include <vector>

CAF_CMD_SOURCE_INIT( RicNewWellLogDiffCurveFeature, "RicNewWellLogDiffCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogDiffCurveFeature::isCommandEnabled()
{
    std::vector<RimWellLogCurve*> wellLogCurves;
    caf::SelectionManager::instance()->objectsByType( &wellLogCurves );

    return ( caf::SelectionManager::instance()->selectedItemOfType<RimWellLogTrack>() != nullptr || wellLogCurves.size() == 2 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellLogDiffCurveFeature::onActionTriggered( bool isChecked )
{
    RimWellLogTrack* wellLogTrack = caf::SelectionManager::instance()->selectedItemOfType<RimWellLogTrack>();
    if ( wellLogTrack )
    {
        RicWellLogTools::addWellLogDiffCurve( wellLogTrack );
    }
    else
    {
        std::vector<RimWellLogCurve*> wellLogCurves;
        caf::SelectionManager::instance()->objectsByType( &wellLogCurves );
        if ( wellLogCurves.size() != 2 ) return;

        RimWellLogTrack* wellLogTrack = nullptr;
        wellLogCurves[0]->firstAncestorOrThisOfType( wellLogTrack );
        if ( !wellLogTrack ) return;

        RimWellLogDiffCurve* newCurve = RicWellLogTools::addWellLogDiffCurve( wellLogTrack );
        newCurve->setWellLogCurves( wellLogCurves[0], wellLogCurves[1] );
        newCurve->updateConnectedEditors();
    }
    RiuPlotMainWindowTools::refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellLogDiffCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Well Log Diff Curve" );
    actionToSetup->setIcon( QIcon( ":/WellLogCurve16x16.png" ) );
}

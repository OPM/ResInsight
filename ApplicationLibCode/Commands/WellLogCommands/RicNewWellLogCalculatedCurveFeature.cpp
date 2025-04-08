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

#include "RicNewWellLogCalculatedCurveFeature.h"

#include "RicWellLogTools.h"

#include "RimWellLogCalculatedCurve.h"
#include "RimWellLogCurve.h"
#include "RimWellLogTrack.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QIcon>

#include <vector>

CAF_CMD_SOURCE_INIT( RicNewWellLogCalculatedCurveFeature, "RicNewWellLogCalculatedCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogCalculatedCurveFeature::isCommandEnabled() const
{
    const auto wellLogCurves = caf::SelectionManager::instance()->objectsByType<RimWellLogCurve>();

    return ( caf::SelectionManager::instance()->selectedItemOfType<RimWellLogTrack>() != nullptr || wellLogCurves.size() == 2 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellLogCalculatedCurveFeature::onActionTriggered( bool isChecked )
{
    RimWellLogTrack* wellLogTrack = caf::SelectionManager::instance()->selectedItemOfType<RimWellLogTrack>();
    if ( wellLogTrack )
    {
        RicWellLogTools::addWellLogCalculatedCurve( wellLogTrack );
    }
    else
    {
        const auto wellLogCurves = caf::SelectionManager::instance()->objectsByType<RimWellLogCurve>();
        if ( wellLogCurves.size() != 2 ) return;

        RimWellLogTrack* wellLogTrack = wellLogCurves[0]->firstAncestorOrThisOfType<RimWellLogTrack>();
        if ( !wellLogTrack ) return;

        RimWellLogCalculatedCurve* newCurve = RicWellLogTools::addWellLogCalculatedCurve( wellLogTrack );
        newCurve->setWellLogCurves( wellLogCurves[0], wellLogCurves[1] );
        newCurve->updateConnectedEditors();
    }
    RiuPlotMainWindowTools::refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellLogCalculatedCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Well Log Calculated Curve" );
    actionToSetup->setIcon( QIcon( ":/WellLogCurve16x16.png" ) );
}

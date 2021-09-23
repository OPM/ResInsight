/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RicCreateSurfaceIntersectionCurveFeature.h"

#include "RimExtrudedCurveIntersection.h"
#include "RimSurfaceIntersectionCurve.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateSurfaceIntersectionCurveFeature, "RicCreateSurfaceIntersectionCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateSurfaceIntersectionCurveFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSurfaceIntersectionCurveFeature::onActionTriggered( bool isChecked )
{
    auto* intersection = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimExtrudedCurveIntersection>();
    if ( intersection )
    {
        auto curve = intersection->addIntersectionCurve();
        intersection->updateAllRequiredEditors();

        Riu3DMainWindowTools::selectAsCurrentItem( curve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSurfaceIntersectionCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Surface Intersection Curve" );
    actionToSetup->setIcon( QIcon( ":/ReservoirSurface16x16.png" ) );
}

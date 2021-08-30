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

#include "RicDeleteIntersectionBandFeature.h"

#include "RimCurveIntersectionBand.h"
#include "RimExtrudedCurveIntersection.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeleteIntersectionBandFeature, "RicDeleteIntersectionBandFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteIntersectionBandFeature::isCommandEnabled()
{
    std::vector<RimCurveIntersectionBand*> objects;
    caf::SelectionManager::instance()->objectsByType( &objects, caf::SelectionManager::FIRST_LEVEL );

    if ( !objects.empty() )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteIntersectionBandFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimCurveIntersectionBand*> objectsToDelete;
    caf::SelectionManager::instance()->objectsByType( &objectsToDelete, caf::SelectionManager::FIRST_LEVEL );

    if ( !objectsToDelete.empty() )
    {
        RimExtrudedCurveIntersection* extrudedCurveIntersection = nullptr;
        objectsToDelete[0]->firstAncestorOrThisOfTypeAsserted( extrudedCurveIntersection );

        for ( auto target : objectsToDelete )
        {
            extrudedCurveIntersection->deleteIntersectionBand( target );
        }

        extrudedCurveIntersection->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteIntersectionBandFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/IntersectionBox16x16.png" ) );
    actionToSetup->setText( "Delete Intersection Band" );
}

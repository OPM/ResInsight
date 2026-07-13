/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RicAppendIjkIntersectionFeature.h"

#include "RimEclipseView.h"
#include "RimGridView.h"
#include "RimIjkIntersection.h"
#include "RimIntersectionCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicAppendIjkIntersectionFeature, "RicAppendIjkIntersectionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAppendIjkIntersectionFeature::isCommandEnabled() const
{
    RimIntersectionCollection* coll = RicAppendIjkIntersectionFeature::intersectionCollection();

    // I/J/K intersections rely on a structured Eclipse grid
    return coll != nullptr && coll->firstAncestorOrThisOfType<RimEclipseView>() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendIjkIntersectionFeature::onActionTriggered( bool isChecked )
{
    RimIntersectionCollection* coll = RicAppendIjkIntersectionFeature::intersectionCollection();

    if ( coll )
    {
        RimIjkIntersection* intersection = new RimIjkIntersection();
        intersection->setName( "Intersection I/J/K" );

        // The default values are computed from the grid, which is resolved through the parent
        // view, so the intersection must be added to the collection first
        coll->appendIjkIntersectionNoUpdate( intersection );
        intersection->setToDefaultValues();

        coll->updateConnectedEditors();
        Riu3DMainWindowTools::selectAsCurrentItem( intersection );

        RimGridView* rimView = coll->firstAncestorOrThisOfTypeAsserted<RimGridView>();
        rimView->scheduleCreateDisplayModelAndRedraw();
        rimView->showGridCells( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendIjkIntersectionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/IntersectionBox16x16.png" ) );
    actionToSetup->setText( "New Intersection I/J/K" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionCollection* RicAppendIjkIntersectionFeature::intersectionCollection()
{
    if ( auto selectedObject = caf::SelectionManager::instance()->selectedItemOfType<caf::PdmObjectHandle>() )
    {
        return selectedObject->firstAncestorOrThisOfType<RimIntersectionCollection>();
    }

    return nullptr;
}

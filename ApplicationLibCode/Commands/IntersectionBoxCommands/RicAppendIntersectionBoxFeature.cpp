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

#include "RicAppendIntersectionBoxFeature.h"

#include "RimBoxIntersection.h"
#include "RimCase.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"
#include "Riu3DMainWindowTools.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicAppendIntersectionBoxFeature, "RicAppendIntersectionBoxFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAppendIntersectionBoxFeature::isCommandEnabled() const
{
    RimIntersectionCollection* coll = RicAppendIntersectionBoxFeature::intersectionCollection();
    return coll != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendIntersectionBoxFeature::onActionTriggered( bool isChecked )
{
    RimIntersectionCollection* coll = RicAppendIntersectionBoxFeature::intersectionCollection();

    if ( coll )
    {
        RimBoxIntersection* intersectionBox = new RimBoxIntersection();
        intersectionBox->setName( "Intersection Box" );

        coll->appendIntersectionBoxAndUpdate( intersectionBox );

        intersectionBox->setToDefaultSizeBox();
        intersectionBox->updateConnectedEditors();

        coll->updateConnectedEditors();
        Riu3DMainWindowTools::selectAsCurrentItem( intersectionBox );

        RimGridView* rimView = coll->firstAncestorOrThisOfTypeAsserted<RimGridView>();
        rimView->showGridCells( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendIntersectionBoxFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/IntersectionBox16x16.png" ) );
    actionToSetup->setText( "New Intersection Box" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionCollection* RicAppendIntersectionBoxFeature::intersectionCollection()
{
    if ( auto selectedObject = caf::SelectionManager::instance()->selectedItemOfType<caf::PdmObjectHandle>() )
    {
        return selectedObject->firstAncestorOrThisOfType<RimIntersectionCollection>();
    }

    return nullptr;
}

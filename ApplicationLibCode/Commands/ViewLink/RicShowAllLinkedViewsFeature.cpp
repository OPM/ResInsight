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

#include "RicShowAllLinkedViewsFeature.h"

#include "RimGridView.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicShowAllLinkedViewsFeature, "RicShowAllLinkedViewsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowAllLinkedViewsFeature::isCommandEnabled() const
{
    return caf::SelectionManager::instance()->selectedItemAncestorOfType<RimViewLinkerCollection>() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowAllLinkedViewsFeature::onActionTriggered( bool isChecked )
{
    auto linkedViews  = caf::SelectionManager::instance()->objectsByType<RimViewLinker>();
    auto managedViews = caf::SelectionManager::instance()->objectsByType<RimViewController>();

    for ( auto& managedView : managedViews )
    {
        RimViewLinker* rimLinked = managedView->firstAncestorOrThisOfTypeAsserted<RimViewLinker>();

        linkedViews.push_back( rimLinked );
    }

    for ( auto& linkedView : linkedViews )
    {
        auto views = linkedView->allViews();
        for ( auto& view : views )
        {
            view->forceShowWindowOn();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowAllLinkedViewsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Open All Linked Views" );
}

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

#include "RicDeleteAllLinkedViewsFeature.h"

#include "RimEclipseContourMapView.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeleteAllLinkedViewsFeature, "RicDeleteAllLinkedViewsFeature" );

class DeleteAllLinkedViewsImpl
{
public:
    static void execute()
    {
        RimProject* proj = RimProject::current();

        RimViewLinker* viewLinker = proj->viewLinkerCollection()->viewLinker();
        if ( viewLinker )
        {
            // Remove the view linker object from the view linker collection
            // viewLinkerCollection->viewLinker is a PdmChildField containing one RimViewLinker child object
            proj->viewLinkerCollection->viewLinker.removeChild( viewLinker );

            viewLinker->applyCellFilterCollectionByUserChoice();

            delete viewLinker;

            proj->uiCapability()->updateConnectedEditors();
        }
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteAllLinkedViewsFeature::isCommandEnabled()
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimViewLinker>() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteAllLinkedViewsFeature::onActionTriggered( bool isChecked )
{
    RimProject* proj = RimProject::current();

    RimViewLinker* viewLinker = proj->viewLinkerCollection()->viewLinker();
    if ( viewLinker )
    {
        // Remove the view linker object from the view linker collection
        // viewLinkerCollection->viewLinker is a PdmChildField containing one RimViewLinker child object
        proj->viewLinkerCollection->viewLinker.removeChild( viewLinker );

        auto views = viewLinker->allViews();

        viewLinker->applyCellFilterCollectionByUserChoice();
        delete viewLinker;

        for ( auto v : views )
        {
            if ( dynamic_cast<RimEclipseContourMapView*>( v ) ) v->zoomAll();
        }

        proj->uiCapability()->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteAllLinkedViewsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Unlink All Views" );
    actionToSetup->setIcon( QIcon( ":/UnLinkView.svg" ) );
    actionToSetup->setShortcut( Qt::Key_Delete );
}

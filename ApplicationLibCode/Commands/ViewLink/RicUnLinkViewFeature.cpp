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

#include "RicUnLinkViewFeature.h"

#include "RiaApplication.h"

#include "ContourMap/RimEclipseContourMapView.h"
#include "Rim3dView.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimViewController.h"
#include "RimViewLinker.h"
#include "RimViewLinkerCollection.h"

#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicUnLinkViewFeature, "RicUnLinkViewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicUnLinkViewFeature::isCommandEnabled() const
{
    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    if ( !activeView ) return false;

    if ( activeView->assosiatedViewLinker() )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUnLinkViewFeature::onActionTriggered( bool isChecked )
{
    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    if ( !activeView ) return;

    RimViewController* viewController = activeView->viewController();
    RimViewLinker*     viewLinker     = activeView->assosiatedViewLinker();

    if ( viewController )
    {
        viewController->applyCellFilterCollectionByUserChoice();
        delete viewController;

        // Remove the slots in the vector that was set to nullptr by the destructor
        viewLinker->removeViewController( nullptr );
    }
    else if ( viewLinker )
    {
        viewLinker->applyCellFilterCollectionByUserChoice();

        Rim3dView* firstControlledView = viewLinker->firstControlledView();

        if ( firstControlledView )
        {
            viewLinker->setMasterView( firstControlledView );

            viewLinker->updateDependentViews();
        }
        else
        {
            // Remove the view linker object from the view linker collection
            // viewLinkerCollection->viewLinker is a PdmChildField containing one RimViewLinker child object
            RimProject::current()->viewLinkerCollection->viewLinker.removeChild( viewLinker );

            delete viewLinker;
        }
        activeView->updateAutoName();
    }

    if ( dynamic_cast<RimEclipseContourMapView*>( activeView ) ) activeView->zoomAll();

    RimProject::current()->viewLinkerCollection.uiCapability()->updateConnectedEditors();
    RimProject::current()->uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUnLinkViewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Unlink View" );
    actionToSetup->setIcon( QIcon( ":/UnLinkView.svg" ) );
}

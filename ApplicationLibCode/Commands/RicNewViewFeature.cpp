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

#include "RicNewViewFeature.h"

#include "RiaLogging.h"

#include "ContourMap/RimEclipseContourMapView.h"
#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimEclipseCaseTools.h"
#include "RimEclipseView.h"
#include "RimEclipseViewCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewViewFeature, "RicNewViewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView* RicNewViewFeature::addReservoirView( RimEclipseCase* eclipseCase, RimGeoMechCase* geomCase, RimEclipseViewCollection* viewColl )
{
    Rim3dView* newView = createReservoirView( eclipseCase, geomCase, viewColl );

    if ( newView )
    {
        Riu3DMainWindowTools::setExpanded( newView );

        // Select the new view to make sure RiaApplication::setActiveReservoirView() is called
        Riu3DMainWindowTools::selectAsCurrentItem( newView );
    }

    return newView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewViewFeature::isCommandEnabled() const
{
    return selectedEclipseCase() != nullptr || selectedEclipseView() != nullptr || selectedGeoMechCase() != nullptr ||
           selectedGeoMechView() != nullptr || selectedEclipseViewCollection() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewViewFeature::onActionTriggered( bool isChecked )
{
    // Establish type of selected object
    RimEclipseCase*           eclipseCase     = selectedEclipseCase();
    RimGeoMechCase*           geomCase        = selectedGeoMechCase();
    RimGeoMechView*           geoMechView     = selectedGeoMechView();
    RimEclipseView*           reservoirView   = selectedEclipseView();
    RimEclipseViewCollection* viewCollection  = selectedEclipseViewCollection();
    RimEclipseCaseEnsemble*   eclipseEnsemble = selectedEclipseCaseEnsemble();

    // Find case to insert into
    if ( geoMechView ) geomCase = geoMechView->geoMechCase();
    if ( reservoirView ) eclipseCase = reservoirView->eclipseCase();

    if ( eclipseCase )
    {
        viewCollection = eclipseCase->viewCollection();
    }
    else if ( eclipseEnsemble )
    {
        viewCollection    = eclipseEnsemble->viewCollection();
        auto eclipseCases = eclipseEnsemble->cases();
        eclipseCase       = !eclipseCases.empty() ? eclipseCases[0] : nullptr;
    }
    else if ( viewCollection )
    {
        // Use global view collection if view collection is not descendant of Eclipse case.
        eclipseCase = viewCollection->firstAncestorOrThisOfType<RimEclipseCase>();
        if ( !eclipseCase )
        {
            // Use cases from grid ensemble if applicable
            auto gridEnsemble = viewCollection->firstAncestorOfType<RimEclipseCaseEnsemble>();
            auto eclipseCases = gridEnsemble ? gridEnsemble->cases() : RimEclipseCaseTools::nativeEclipseGridCases();

            if ( !eclipseCases.empty() )
            {
                eclipseCase = eclipseCases[0];
            }
        }
    }

    addReservoirView( eclipseCase, geomCase, viewCollection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewViewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New View" );
    actionToSetup->setIcon( QIcon( ":/3DView16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView* RicNewViewFeature::createReservoirView( RimEclipseCase* eclipseCase, RimGeoMechCase* geomCase, RimEclipseViewCollection* viewColl )
{
    RimGridView* insertedView = nullptr;

    if ( eclipseCase )
    {
        insertedView = eclipseCase->createAndAddReservoirView( viewColl );
    }
    else if ( geomCase )
    {
        insertedView = geomCase->createAndAddReservoirView();
    }

    // Must be run before buildViewItems, as wells are created in this function
    if ( insertedView ) insertedView->loadDataAndUpdate();

    if ( eclipseCase )
    {
        eclipseCase->updateConnectedEditors();
    }

    if ( geomCase )
    {
        geomCase->updateConnectedEditors();
    }

    return insertedView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicNewViewFeature::selectedEclipseCase()
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimEclipseCase>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RicNewViewFeature::selectedGeoMechCase()
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimGeoMechCase>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RicNewViewFeature::selectedEclipseView()
{
    const auto selection = caf::SelectionManager::instance()->objectsByType<RimEclipseView>();
    for ( RimEclipseView* view : selection )
    {
        if ( dynamic_cast<RimEclipseContourMapView*>( view ) == nullptr )
        {
            return view;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseViewCollection* RicNewViewFeature::selectedEclipseViewCollection()
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimEclipseViewCollection>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCaseEnsemble* RicNewViewFeature::selectedEclipseCaseEnsemble()
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimEclipseCaseEnsemble>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechView* RicNewViewFeature::selectedGeoMechView()
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimGeoMechView>();
}

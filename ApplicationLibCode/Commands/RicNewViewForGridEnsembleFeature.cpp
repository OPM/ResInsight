/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RicNewViewForGridEnsembleFeature.h"

#include "RiaLogging.h"

#include "ContourMap/RimEclipseContourMapView.h"
#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseViewCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimReservoirGridEnsembleBase.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewViewForGridEnsembleFeature, "RicNewViewForGridEnsembleFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewViewForGridEnsembleFeature::addView( RimReservoirGridEnsembleBase* gridEnsemble )
{
    if ( !gridEnsemble ) return;

    auto viewCollection = gridEnsemble->viewCollection();
    if ( !viewCollection ) return;

    std::vector<RimEclipseCase*> cases = gridEnsemble->sourceCases();
    if ( cases.empty() ) return;

    auto newView = viewCollection->addView( cases[0] );

    // The ensemble types are not PdmObject in the shared base, so get there by cast.
    if ( auto pdmObject = dynamic_cast<caf::PdmObject*>( gridEnsemble ) ) pdmObject->updateConnectedEditors();

    Riu3DMainWindowTools::setExpanded( newView );

    // Select the new view to make sure RiaApplication::setActiveReservoirView() is called
    Riu3DMainWindowTools::selectAsCurrentItem( newView );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewViewForGridEnsembleFeature::isCommandEnabled() const
{
    return selectedGridEnsemble() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewViewForGridEnsembleFeature::onActionTriggered( bool isChecked )
{
    addView( selectedGridEnsemble() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewViewForGridEnsembleFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New View" );
    actionToSetup->setIcon( QIcon( ":/3DView16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReservoirGridEnsembleBase* RicNewViewForGridEnsembleFeature::selectedGridEnsemble()
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimReservoirGridEnsembleBase>();
}

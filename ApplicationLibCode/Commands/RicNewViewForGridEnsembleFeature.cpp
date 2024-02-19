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

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewViewForGridEnsembleFeature, "RicNewViewForGridEnsembleFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewViewForGridEnsembleFeature::addView( RimEclipseCaseEnsemble* eclipseCaseEnsemble )
{
    std::vector<RimEclipseCase*> cases = eclipseCaseEnsemble->cases();
    if ( cases.empty() ) return;

    auto newView = eclipseCaseEnsemble->addViewForCase( cases[0] );
    eclipseCaseEnsemble->updateConnectedEditors();

    Riu3DMainWindowTools::setExpanded( newView );

    // Select the new view to make sure RiaApplication::setActiveReservoirView() is called
    Riu3DMainWindowTools::selectAsCurrentItem( newView );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewViewForGridEnsembleFeature::isCommandEnabled() const
{
    return selectedEclipseCaseEnsemble() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewViewForGridEnsembleFeature::onActionTriggered( bool isChecked )
{
    RimEclipseCaseEnsemble* eclipseCaseEnsemble = selectedEclipseCaseEnsemble();
    addView( eclipseCaseEnsemble );
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
RimEclipseCaseEnsemble* RicNewViewForGridEnsembleFeature::selectedEclipseCaseEnsemble()
{
    std::vector<RimEclipseCaseEnsemble*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );

    if ( !selection.empty() )
    {
        return selection[0];
    }

    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RicNewWellPathStimPlanModelAtPosFeature.h"

#include "RiaApplication.h"

#include "RicNewStimPlanModelFeature.h"

#include "RimEclipseView.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "Riu3dSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewWellPathStimPlanModelAtPosFeature, "RicNewWellPathStimPlanModelAtPosFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellPathStimPlanModelAtPosFeature::onActionTriggered( bool isChecked )
{
    Riu3dSelectionManager* riuSelManager = Riu3dSelectionManager::instance();
    RiuSelectionItem*      selItem       = riuSelManager->selectedItem( Riu3dSelectionManager::RUI_TEMPORARY );

    RiuWellPathSelectionItem* wellPathItem = dynamic_cast<RiuWellPathSelectionItem*>( selItem );
    if ( !wellPathItem ) return;

    RimWellPath* wellPath = wellPathItem->m_wellpath;
    if ( !wellPath ) return;

    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
    if ( !wellPathCollection ) return;

    RimEclipseView* activeView  = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeGridView() );
    RimEclipseCase* eclipseCase = nullptr;
    int             timeStep    = 0;
    if ( activeView )
    {
        eclipseCase = activeView->eclipseCase();
        timeStep    = activeView->currentTimeStep();
    }

    RicNewStimPlanModelFeature::addStimPlanModel( wellPath,
                                                  wellPathCollection,
                                                  eclipseCase,
                                                  timeStep,
                                                  wellPathItem->m_measuredDepth );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellPathStimPlanModelAtPosFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/FractureSymbol16x16.png" ) );
    actionToSetup->setText( "Create StimPlan Model at this Depth" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathStimPlanModelAtPosFeature::isCommandEnabled()
{
    return true;
}

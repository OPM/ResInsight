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

#include "RicNewStatisticsContourMapFeature.h"

#include "RimEclipseCaseEnsemble.h"
#include "RimStatisticsContourMap.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewStatisticsContourMapFeature, "RicNewStatisticsContourMapFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStatisticsContourMapFeature::addStatisticsContourMap( RimEclipseCaseEnsemble* eclipseCaseEnsemble )
{
    if ( !eclipseCaseEnsemble ) return;

    std::vector<RimEclipseCase*> cases = eclipseCaseEnsemble->cases();
    if ( cases.empty() ) return;

    RimStatisticsContourMap* statisticsContourMap = new RimStatisticsContourMap;
    statisticsContourMap->setEclipseCase( cases[0] );
    eclipseCaseEnsemble->addStatisticsContourMap( statisticsContourMap );
    eclipseCaseEnsemble->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewStatisticsContourMapFeature::isCommandEnabled() const
{
    return selectedEclipseCaseEnsemble() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStatisticsContourMapFeature::onActionTriggered( bool isChecked )
{
    RimEclipseCaseEnsemble* eclipseCaseEnsemble = selectedEclipseCaseEnsemble();
    addStatisticsContourMap( eclipseCaseEnsemble );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStatisticsContourMapFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Statistics Contour Map" );
    actionToSetup->setIcon( QIcon( ":/3DView16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCaseEnsemble* RicNewStatisticsContourMapFeature::selectedEclipseCaseEnsemble()
{
    std::vector<RimEclipseCaseEnsemble*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );

    if ( !selection.empty() )
    {
        return selection[0];
    }

    return nullptr;
}

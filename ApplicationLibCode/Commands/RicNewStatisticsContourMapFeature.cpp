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

#include "ContourMap/RimStatisticsContourMap.h"
#include "RimReservoirGridEnsembleBase.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewStatisticsContourMapFeature, "RicNewStatisticsContourMapFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStatisticsContourMapFeature::addStatisticsContourMap( RimReservoirGridEnsembleBase* ensemble )
{
    if ( !ensemble ) return;

    auto cases = ensemble->sourceCases();
    if ( cases.empty() ) return;

    auto statisticsContourMap = new RimStatisticsContourMap();
    statisticsContourMap->setEclipseCase( cases[0] );
    ensemble->addStatisticsContourMap( statisticsContourMap );

    if ( auto* pdmObject = dynamic_cast<caf::PdmObject*>( ensemble ) ) pdmObject->updateConnectedEditors();

    Riu3DMainWindowTools::selectAsCurrentItem( statisticsContourMap );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewStatisticsContourMapFeature::isCommandEnabled() const
{
    return selectedEnsemble() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStatisticsContourMapFeature::onActionTriggered( bool isChecked )
{
    addStatisticsContourMap( selectedEnsemble() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStatisticsContourMapFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Ensemble Contour Map" );
    actionToSetup->setIcon( QIcon( ":/Histogram16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReservoirGridEnsembleBase* RicNewStatisticsContourMapFeature::selectedEnsemble()
{
    if ( auto* ens = caf::SelectionManager::instance()->selectedItemOfType<RimReservoirGridEnsembleBase>() ) return ens;
    return nullptr;
}

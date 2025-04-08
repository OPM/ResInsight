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

#include "RicReloadFormationNamesFeature.h"

#include "RiaLogging.h"

#include "Formations/RimFormationNames.h"
#include "Formations/RimFormationNamesCollection.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicReloadFormationNamesFeature, "RicReloadFormationNamesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReloadFormationNamesFeature::isCommandEnabled() const
{
    const auto selectedFormationNamesObjs     = caf::SelectionManager::instance()->objectsByType<RimFormationNames>();
    const auto selectedFormationNamesCollObjs = caf::SelectionManager::instance()->objectsByType<RimFormationNamesCollection>();

    return ( !selectedFormationNamesObjs.empty() || !selectedFormationNamesCollObjs.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadFormationNamesFeature::onActionTriggered( bool isChecked )
{
    const auto selectedFormationNamesCollObjs = caf::SelectionManager::instance()->objectsByType<RimFormationNamesCollection>();
    if ( !selectedFormationNamesCollObjs.empty() )
    {
        selectedFormationNamesCollObjs[0]->readAllFormationNames();
        for ( RimFormationNames* fnames : selectedFormationNamesCollObjs[0]->formationNamesList() )
        {
            fnames->updateConnectedViews();
        }

        return;
    }

    const auto selectedFormationNamesObjs = caf::SelectionManager::instance()->objectsByType<RimFormationNames>();
    for ( RimFormationNames* fnames : selectedFormationNamesObjs )
    {
        QString errorMessage;
        fnames->readFormationNamesFile( &errorMessage );
        if ( !errorMessage.isEmpty() )
        {
            RiaLogging::errorInMessageBox( nullptr, "Reload Formation Names", errorMessage );
        }

        fnames->updateConnectedViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadFormationNamesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Reload" );
    actionToSetup->setIcon( QIcon( ":/Refresh.svg" ) );
}

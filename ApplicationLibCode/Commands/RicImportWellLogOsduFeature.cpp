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

#include "RicImportWellLogOsduFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RimOilField.h"
#include "RimOsduWellLog.h"
#include "RimOsduWellPath.h"
#include "RimProject.h"
#include "RimWellPathCollection.h"

#include "RiuMainWindow.h"

#include "OsduImportCommands/RiaOsduConnector.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicImportWellLogOsduFeature, "RicImportWellLogOsduFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportWellLogOsduFeature::isCommandEnabled() const
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimOsduWellPath>() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportWellLogOsduFeature::onActionTriggered( bool isChecked )
{
    auto makeOsduConnector = []( auto app )
    {
        RiaPreferencesOsdu* osduPreferences = app->preferences()->osduPreferences();
        const QString       server          = osduPreferences->server();
        const QString       dataPartitionId = osduPreferences->dataPartitionId();
        const QString       authority       = osduPreferences->authority();
        const QString       scopes          = osduPreferences->scopes();
        const QString       clientId        = osduPreferences->clientId();
        return std::make_unique<RiaOsduConnector>( RiuMainWindow::instance(), server, dataPartitionId, authority, scopes, clientId );
    };

    if ( auto wellPath = caf::SelectionManager::instance()->selectedItemOfType<RimOsduWellPath>() )
    {
        RiaGuiApplication* app = RiaGuiApplication::instance();

        RimOilField* oilField = RimProject::current()->activeOilField();
        if ( oilField == nullptr ) return;

        if ( !oilField->wellPathCollection ) oilField->wellPathCollection = std::make_unique<RimWellPathCollection>();

        RimOsduWellLog* osduWellLog = new RimOsduWellLog;
        // TODO: get from OSDU...
        osduWellLog->setWellLogId( "npequinor-dev:work-product-component--WellLog:aeb5bd8b1de14138afe9f23cacbc7fe7" );
        oilField->wellPathCollection->addWellLog( osduWellLog, wellPath );

        auto osduConnector = makeOsduConnector( app );

        auto [wellLogData, errorMessage] = RimWellPathCollection::loadWellLogFromOsdu( osduConnector.get(), osduWellLog->wellLogId() );
        if ( wellLogData.notNull() )
        {
            osduWellLog->setWellLogData( wellLogData.p() );
        }
        else
        {
            RiaLogging::error( "Importing OSDU well log failed: " + errorMessage );
        }

        osduWellLog->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportWellLogOsduFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/LasFile16x16.png" ) );
    actionToSetup->setText( "Import Well Log From OSDU" );
}

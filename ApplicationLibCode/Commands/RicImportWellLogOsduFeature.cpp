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
    if ( auto wellPath = caf::SelectionManager::instance()->selectedItemOfType<RimOsduWellPath>() )
    {
        RiaGuiApplication* app = RiaGuiApplication::instance();

        RimOilField* oilField = RimProject::current()->activeOilField();
        if ( oilField == nullptr ) return;

        if ( !oilField->wellPathCollection ) oilField->wellPathCollection = std::make_unique<RimWellPathCollection>();

        auto                     osduConnector = app->makeOsduConnector();
        std::vector<OsduWellLog> wellLogs      = osduConnector->requestWellLogsByWellboreIdBlocking( wellPath->wellboreId() );

        for ( OsduWellLog wellLog : wellLogs )
        {
            auto [wellLogData, errorMessage] = RimWellPathCollection::loadWellLogFromOsdu( osduConnector, wellLog.id );
            if ( wellLogData.notNull() )
            {
                RimOsduWellLog* osduWellLog = new RimOsduWellLog;
                osduWellLog->setName( wellLog.id );
                osduWellLog->setWellLogId( wellLog.id );
                oilField->wellPathCollection->addWellLog( osduWellLog, wellPath );

                osduWellLog->setWellLogData( wellLogData.p() );
                osduWellLog->updateConnectedEditors();
            }
            else
            {
                RiaLogging::error( "Importing OSDU well log failed: " + errorMessage );
            }
        }
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

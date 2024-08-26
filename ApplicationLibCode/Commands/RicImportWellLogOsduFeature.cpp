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

#include "Cloud/RiaOsduConnector.h"
#include "RiaDefines.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RimOilField.h"
#include "RimOsduWellLog.h"
#include "RimOsduWellLogChannel.h"
#include "RimOsduWellPath.h"
#include "RimProject.h"
#include "RimWellPathCollection.h"

#include "OsduImportCommands/RiuWellLogImportWizard.h"
#include "RiuMainWindow.h"

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

        auto osduConnector = app->makeOsduConnector();
        if ( !osduConnector )
        {
            RiaLogging::error( "Failed to create OSDU connector" );
            return;
        }

        RiuWellLogImportWizard wellLogImportWizard( osduConnector, wellPath->wellboreId(), RiuMainWindow::instance() );

        if ( QDialog::Accepted == wellLogImportWizard.exec() )
        {
            std::vector<OsduWellLog> wellLogs = wellLogImportWizard.importedWellLogs();

            for ( OsduWellLog wellLog : wellLogs )
            {
                auto [wellLogData, errorMessage] = RimWellPathCollection::loadWellLogFromOsdu( osduConnector, wellLog.id );
                if ( wellLogData.notNull() )
                {
                    RimOsduWellLog* osduWellLog = new RimOsduWellLog;
                    osduWellLog->setName( wellLog.name );
                    osduWellLog->setWellLogId( wellLog.id );
                    for ( OsduWellLogChannel c : wellLog.channels )
                    {
                        RimOsduWellLogChannel* osduWellLogChannel = new RimOsduWellLogChannel;
                        osduWellLogChannel->setId( c.id );
                        osduWellLogChannel->setName( c.mnemonic );
                        osduWellLogChannel->setDescription( c.description );
                        osduWellLogChannel->setTopDepth( c.topDepth );
                        osduWellLogChannel->setBaseDepth( c.baseDepth );
                        osduWellLogChannel->setInterpreterName( c.interpreterName );
                        osduWellLogChannel->setQuality( c.quality );
                        osduWellLogChannel->setUnit( c.unit );
                        osduWellLogChannel->setDepthUnit( c.depthUnit );

                        osduWellLog->addWellLogChannel( osduWellLogChannel );
                    }

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportWellLogOsduFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/LasFile16x16.png" ) );
    actionToSetup->setText( "Import Well Log From OSDU" + RiaDefines::betaFeaturePostfix() );
}

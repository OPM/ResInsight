/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RicStopAllJobsFeature.h"

#include "RiaGuiApplication.h"

#include "Jobs/RimGenericJob.h"
#include "Jobs/RimJobCollection.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicStopAllJobsFeature, "RicStopAllJobsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicStopAllJobsFeature::onActionTriggered( bool isChecked )
{
    if ( auto coll = dynamic_cast<RimJobCollection*>( caf::SelectionManager::instance()->selectedItem() ) )
    {
        stopAllJobs( coll );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicStopAllJobsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/stop.svg" ) );
    actionToSetup->setText( "Stop..." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicStopAllJobsFeature::stopAllJobs( RimJobCollection* coll )
{
    if ( coll != nullptr )
    {
        if ( QMessageBox::question( RiaGuiApplication::widgetToUseAsParent(),
                                    "Stop All Jobs",
                                    "Do you want to stop all running and queued jobs?",
                                    QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
        {
            for ( auto job : coll->jobs() )
            {
                if ( job->isRunning() ) job->stop();
            }
        }
    }
}

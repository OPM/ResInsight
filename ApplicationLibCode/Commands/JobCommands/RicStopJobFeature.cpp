/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RicStopJobFeature.h"

#include "Jobs/RimGenericJob.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicStopJobFeature, "RicStopJobFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicStopJobFeature::onActionTriggered( bool isChecked )
{
    stopJob( dynamic_cast<RimGenericJob*>( caf::SelectionManager::instance()->selectedItem() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicStopJobFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/stop.svg" ) );
    actionToSetup->setText( "Stop..." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicStopJobFeature::stopJob( RimGenericJob* job )
{
    if ( job == nullptr ) return false;

    if ( QMessageBox::question( nullptr, job->name(), "Do you want to stop this job?", QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
    {
        return job->stop();
    }

    return false;
}

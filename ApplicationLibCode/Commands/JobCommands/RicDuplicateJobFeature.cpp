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

#include "RicDuplicateJobFeature.h"

#include "RicNewOpmFlowJobFeature.h"

#include "Jobs/RimJobCollection.h"
#include "Jobs/RimOpmFlowJob.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QDir>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicDuplicateJobFeature, "RicDuplicateJobFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDuplicateJobFeature::onActionTriggered( bool isChecked )
{
    if ( auto job = dynamic_cast<RimOpmFlowJob*>( caf::SelectionManager::instance()->selectedItem() ) )
    {
        QString   defaultDir = job->mainWorkingDirectory();
        QFileInfo fi( defaultDir );
        defaultDir = fi.dir().absolutePath();

        QString workDir = RicNewOpmFlowJobFeature::workingFolder( defaultDir );
        if ( workDir.isEmpty() ) return;

        if ( auto copiedJob = job->copyObject<RimOpmFlowJob>() )
        {
            copiedJob->setWorkingDirectory( workDir );
            copiedJob->setName( job->name() + " (copy)" );
            copiedJob->initAfterCopy();

            auto jobColl = RimTools::jobCollection();
            jobColl->addNewJob( copiedJob );

            copiedJob->resolveReferencesRecursively();

            Riu3DMainWindowTools::selectAsCurrentItem( copiedJob );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDuplicateJobFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Copy.svg" ) );
    actionToSetup->setText( "Duplicate..." );
}

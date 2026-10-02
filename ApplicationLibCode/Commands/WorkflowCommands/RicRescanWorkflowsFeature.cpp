/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RicRescanWorkflowsFeature.h"

#include "RicWorkflowFeatureTools.h"
#include "RimProject.h"
#include "Workflow/RimWorkflow.h"
#include "Workflow/RimWorkflowCollection.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicRescanWorkflowsFeature, "RicRescanWorkflowsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRescanWorkflowsFeature::isCommandEnabled() const
{
    RimProject* project = RimProject::current();
    if ( !project || !project->workflowCollection() ) return false;

    // Rescanning deletes the workflows, including running jobs
    for ( RimWorkflow* workflow : project->workflowCollection()->allWorkflows() )
    {
        if ( workflow->isLocked() ) return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRescanWorkflowsFeature::onActionTriggered( bool isChecked )
{
    RicWorkflowFeatureTools::rescanWorkflows();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRescanWorkflowsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Rescan Workflows" );
    actionToSetup->setIcon( QIcon( ":/Refresh.svg" ) );
}

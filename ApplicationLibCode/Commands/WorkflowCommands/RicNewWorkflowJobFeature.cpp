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

#include "RicNewWorkflowJobFeature.h"

#include "Workflow/RimWorkflow.h"
#include "Workflow/RimWorkflowJob.h"

#include "RiaLogging.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewWorkflowJobFeature, "RicNewWorkflowJobFeature" );

void RicNewWorkflowJobFeature::onActionTriggered( bool isChecked )
{
    auto workflows = caf::selectedObjectsByType<RimWorkflow*>();
    if ( workflows.size() != 1 ) return;

    RimWorkflow* workflow = workflows.front();
    auto         jobs     = workflow->jobs();
    if ( jobs.empty() )
    {
        RiaLogging::warning( "Cannot create a new job: workflow has no template job to clone." );
        return;
    }

    auto* clone = dynamic_cast<RimWorkflowJob*>(
        jobs.front()->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );
    if ( !clone ) return;

    clone->setJobName( QString( "Job %1" ).arg( jobs.size() + 1 ) );
    workflow->addJob( clone );
    clone->resolveReferencesRecursively();
    workflow->uiCapability()->updateAllRequiredEditors();
}

void RicNewWorkflowJobFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Job" );
    actionToSetup->setIcon( QIcon( ":/caf/duplicate.svg" ) );
}

bool RicNewWorkflowJobFeature::isCommandEnabled() const
{
    return caf::selectedObjectsByType<RimWorkflow*>().size() == 1;
}

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

#include "RicRunWorkflowJobFeature.h"

#include "Workflow/RimWorkflowJob.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicRunWorkflowJobFeature, "RicRunWorkflowJobFeature" );

void RicRunWorkflowJobFeature::onActionTriggered( bool isChecked )
{
    auto jobs = caf::selectedObjectsByType<RimWorkflowJob*>();
    if ( jobs.size() != 1 ) return;
    jobs.front()->runJob();
}

void RicRunWorkflowJobFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Run" );
    actionToSetup->setIcon( QIcon( ":/Play.svg" ) );
}

bool RicRunWorkflowJobFeature::isCommandEnabled() const
{
    return caf::selectedObjectsByType<RimWorkflowJob*>().size() == 1;
}

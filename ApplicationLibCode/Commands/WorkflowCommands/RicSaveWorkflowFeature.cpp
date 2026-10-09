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

#include "RicSaveWorkflowFeature.h"

#include "RicWorkflowFeatureTools.h"
#include "Workflow/RimWorkflow.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicSaveWorkflowFeature, "RicSaveWorkflowFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSaveWorkflowFeature::isCommandEnabled() const
{
    auto* workflow = RicWorkflowFeatureTools::selectedWorkflow();
    return workflow && workflow->isEditable() && !workflow->isLocked() && workflow->isDirty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveWorkflowFeature::onActionTriggered( bool isChecked )
{
    RicWorkflowFeatureTools::saveWorkflow( RicWorkflowFeatureTools::selectedWorkflow() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveWorkflowFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Save Workflow" );
    actionToSetup->setIcon( QIcon( ":/Save.svg" ) );
}

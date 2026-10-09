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

#include "RicDuplicateWorkflowAsEditableFeature.h"

#include "RicWorkflowFeatureTools.h"
#include "Workflow/RimWorkflow.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDuplicateWorkflowAsEditableFeature, "RicDuplicateWorkflowAsEditableFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDuplicateWorkflowAsEditableFeature::isCommandEnabled() const
{
    auto* workflow = RicWorkflowFeatureTools::selectedWorkflow();
    return workflow && workflow->loadError().isEmpty() && workflow->definition().editable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDuplicateWorkflowAsEditableFeature::onActionTriggered( bool isChecked )
{
    RicWorkflowFeatureTools::duplicateAsEditable( RicWorkflowFeatureTools::selectedWorkflow() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDuplicateWorkflowAsEditableFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Duplicate as Editable..." );
    actionToSetup->setIcon( QIcon( ":/Copy.svg" ) );
}

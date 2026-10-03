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

#include "RicRenameWorkflowCollectKeyFeature.h"

#include "RicWorkflowFeatureTools.h"
#include "RiuWorkflowEditorWidget.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicRenameWorkflowCollectKeyFeature, "RicRenameWorkflowCollectKeyFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRenameWorkflowCollectKeyFeature::isCommandEnabled() const
{
    return RicWorkflowFeatureTools::editableWorkflowEditor() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRenameWorkflowCollectKeyFeature::onActionTriggered( bool isChecked )
{
    const QVariantMap data = userData().toMap();
    if ( auto* editor = RicWorkflowFeatureTools::editableWorkflowEditor() )
    {
        editor->renameCollectKey( RiuWorkflowGraphView::ItemRef::forEdge( data.value( "from" ).toString(),
                                                                          data.value( "output" ).toString(),
                                                                          data.value( "to" ).toString(),
                                                                          data.value( "input" ).toString() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRenameWorkflowCollectKeyFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Rename Key..." );
}

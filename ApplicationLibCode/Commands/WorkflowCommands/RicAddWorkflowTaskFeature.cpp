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

#include "RicAddWorkflowTaskFeature.h"

#include "RicWorkflowFeatureTools.h"
#include "RiuWorkflowEditorWidget.h"

#include <QAction>
#include <QPointF>

CAF_CMD_SOURCE_INIT( RicAddWorkflowTaskFeature, "RicAddWorkflowTaskFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAddWorkflowTaskFeature::isCommandEnabled() const
{
    return RicWorkflowFeatureTools::editableWorkflowEditor() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAddWorkflowTaskFeature::onActionTriggered( bool isChecked )
{
    const QVariantMap data = userData().toMap();
    if ( auto* editor = RicWorkflowFeatureTools::editableWorkflowEditor() )
    {
        std::optional<QPointF> position;
        if ( data.contains( "x" ) && data.contains( "y" ) )
            position = QPointF( data.value( "x" ).toDouble(), data.value( "y" ).toDouble() );
        editor->addTask( data.value( "taskId" ).toString(), position );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAddWorkflowTaskFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Add Task" );
    actionToSetup->setIcon( QIcon( ":/Plus.png" ) );
}

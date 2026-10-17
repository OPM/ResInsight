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

#include "RicToggleWorkflowOptionalInputFeature.h"

#include "RicWorkflowFeatureTools.h"
#include "RiuWorkflowEditorWidget.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicToggleWorkflowOptionalInputFeature, "RicToggleWorkflowOptionalInputFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicToggleWorkflowOptionalInputFeature::isCommandEnabled() const
{
    return RicWorkflowFeatureTools::editableWorkflowEditor() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleWorkflowOptionalInputFeature::onActionTriggered( bool isChecked )
{
    const QVariantMap data = userData().toMap();
    if ( auto* editor = RicWorkflowFeatureTools::editableWorkflowEditor() )
        editor->setOptionalInputConfigured( data.value( "task" ).toString(),
                                            data.value( "field" ).toString(),
                                            data.value( "configured" ).toBool() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleWorkflowOptionalInputFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Configure Optional Input" );
}

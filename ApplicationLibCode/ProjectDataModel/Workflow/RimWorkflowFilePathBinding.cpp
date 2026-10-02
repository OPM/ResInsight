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

#include "RimWorkflowFilePathBinding.h"

#include "cafPdmUiFilePathEditor.h"

#include <QJsonObject>

CAF_PDM_SOURCE_INIT( RimWorkflowFilePathBinding, "WorkflowFilePathBinding" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowFilePathBinding::RimWorkflowFilePathBinding()
{
    CAF_PDM_InitFieldNoDefault( &m_value, "Value", "Value" );
    m_value.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowFilePathBinding::applySchema( const QJsonObject& fieldSchema )
{
    RimWorkflowFieldBinding::applySchema( fieldSchema );

    const QString fmt = fieldSchema.value( "format" ).toString();
    m_value.uiCapability()->setAttribute( caf::PdmUiFilePathEditor::Keys::SELECT_DIRECTORY, fmt == "directory-path" );

    if ( fieldSchema.contains( "default" ) )
    {
        m_value = fieldSchema.value( "default" ).toString();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflowFilePathBinding::displayValue() const
{
    if ( !hasValue() ) return "(not set)";

    const QString path = m_value().path();
    return path.isEmpty() ? "(not set)" : path;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonValue RimWorkflowFilePathBinding::toJsonValue() const
{
    return hasValue() ? QJsonValue( m_value().path() ) : QJsonValue( QJsonValue::Null );
}

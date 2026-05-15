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

RimWorkflowFilePathBinding::RimWorkflowFilePathBinding()
    : m_selectDirectory( false )
{
    CAF_PDM_InitFieldNoDefault( &m_value, "Value", "Value" );
    m_value.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
}

void RimWorkflowFilePathBinding::applySchema( const QJsonObject& fieldSchema )
{
    RimWorkflowFieldBinding::applySchema( fieldSchema );

    const QString fmt = fieldSchema.value( "format" ).toString();
    m_selectDirectory = ( fmt == "directory-path" );

    if ( fieldSchema.contains( "default" ) )
    {
        m_value = fieldSchema.value( "default" ).toString();
    }
}

QString RimWorkflowFilePathBinding::toYamlValue() const
{
    QString p = m_value().path();
    p.replace( "\\", "\\\\" );
    p.replace( "\"", "\\\"" );
    return QString( "\"%1\"" ).arg( p );
}

void RimWorkflowFilePathBinding::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                        QString                    uiConfigName,
                                                        caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_value )
    {
        if ( auto* attr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute ) )
        {
            attr->m_selectDirectory = m_selectDirectory;
        }
    }
}

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

#include "RimWorkflowStringBinding.h"

#include <QJsonObject>

CAF_PDM_SOURCE_INIT( RimWorkflowStringBinding, "WorkflowStringBinding" );

RimWorkflowStringBinding::RimWorkflowStringBinding()
{
    CAF_PDM_InitFieldNoDefault( &m_value, "Value", "Value" );
}

void RimWorkflowStringBinding::applySchema( const QJsonObject& fieldSchema )
{
    RimWorkflowFieldBinding::applySchema( fieldSchema );
    if ( fieldSchema.contains( "default" ) )
    {
        m_value = fieldSchema.value( "default" ).toString();
    }
}

QString RimWorkflowStringBinding::toYamlValue() const
{
    QString v = m_value();
    v.replace( "\\", "\\\\" );
    v.replace( "\"", "\\\"" );
    return QString( "\"%1\"" ).arg( v );
}

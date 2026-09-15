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

#include "RimWorkflowIntBinding.h"

#include <QJsonObject>

CAF_PDM_SOURCE_INIT( RimWorkflowIntBinding, "WorkflowIntBinding" );

RimWorkflowIntBinding::RimWorkflowIntBinding()
{
    CAF_PDM_InitField( &m_value, "Value", 0, "Value" );
}

void RimWorkflowIntBinding::applySchema( const QJsonObject& fieldSchema )
{
    RimWorkflowFieldBinding::applySchema( fieldSchema );
    if ( fieldSchema.contains( "default" ) )
    {
        m_value = fieldSchema.value( "default" ).toInt();
    }
}

QString RimWorkflowIntBinding::toYamlValue() const
{
    return QString::number( m_value() );
}

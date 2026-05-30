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

#include "RimWorkflowNumberBinding.h"

#include <QJsonObject>

CAF_PDM_SOURCE_INIT( RimWorkflowNumberBinding, "WorkflowNumberBinding" );

RimWorkflowNumberBinding::RimWorkflowNumberBinding()
    : m_isInteger( false )
{
    CAF_PDM_InitField( &m_value, "Value", 0.0, "Value" );
}

void RimWorkflowNumberBinding::setIsInteger( bool isInteger )
{
    m_isInteger = isInteger;
}

void RimWorkflowNumberBinding::applySchema( const QJsonObject& fieldSchema )
{
    RimWorkflowFieldBinding::applySchema( fieldSchema );
    setIsInteger( fieldSchema.value( "type" ).toString() == "integer" );
    if ( fieldSchema.contains( "default" ) )
    {
        m_value = fieldSchema.value( "default" ).toDouble();
    }
}

QString RimWorkflowNumberBinding::toYamlValue() const
{
    if ( m_isInteger ) return QString::number( static_cast<long long>( m_value() ) );
    return QString::number( m_value(), 'g', 17 );
}

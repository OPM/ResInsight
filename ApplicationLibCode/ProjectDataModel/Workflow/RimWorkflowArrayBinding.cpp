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

#include "RimWorkflowArrayBinding.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

CAF_PDM_SOURCE_INIT( RimWorkflowArrayBinding, "WorkflowArrayBinding" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowArrayBinding::RimWorkflowArrayBinding()
{
    CAF_PDM_InitFieldNoDefault( &m_value, "Value", "Value" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowArrayBinding::applySchema( const QJsonObject& fieldSchema )
{
    RimWorkflowFieldBinding::applySchema( fieldSchema );
    if ( fieldSchema.value( "default" ).isArray() )
    {
        m_value = QString::fromUtf8( QJsonDocument( fieldSchema.value( "default" ).toArray() ).toJson( QJsonDocument::Compact ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonValue RimWorkflowArrayBinding::toJsonValue() const
{
    if ( !hasValue() ) return QJsonValue::Null;
    const QJsonDocument document = QJsonDocument::fromJson( m_value().toUtf8() );
    return document.isArray() ? QJsonValue( document.array() ) : QJsonValue( m_value() );
}

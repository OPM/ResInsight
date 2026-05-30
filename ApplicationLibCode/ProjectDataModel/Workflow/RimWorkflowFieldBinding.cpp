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

#include "RimWorkflowFieldBinding.h"

#include <QJsonObject>

CAF_PDM_ABSTRACT_SOURCE_INIT( RimWorkflowFieldBinding, "WorkflowFieldBinding" );

RimWorkflowFieldBinding::RimWorkflowFieldBinding()
{
    CAF_PDM_InitObject( "Field", ":/Bullet.png" );

    CAF_PDM_InitFieldNoDefault( &m_fieldName, "FieldName", "Field" );
    m_fieldName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_description, "Description", "Description" );
    m_description.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_required, "Required", false, "Required" );
    m_required.uiCapability()->setUiReadOnly( true );
}

QString RimWorkflowFieldBinding::fieldName() const
{
    return m_fieldName();
}

void RimWorkflowFieldBinding::setFieldName( const QString& name )
{
    m_fieldName = name;
    setUiName( name );
}

void RimWorkflowFieldBinding::setDescription( const QString& description )
{
    m_description = description;
}

void RimWorkflowFieldBinding::setRequired( bool required )
{
    m_required = required;
}

void RimWorkflowFieldBinding::applySchema( const QJsonObject& fieldSchema )
{
    setFieldName( fieldSchema.value( "name" ).toString() );
    setDescription( fieldSchema.value( "description" ).toString() );
    setRequired( fieldSchema.value( "required" ).toBool( false ) );
}

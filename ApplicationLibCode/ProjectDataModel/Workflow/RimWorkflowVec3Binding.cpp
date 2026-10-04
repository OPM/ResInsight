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

#include "RimWorkflowVec3Binding.h"

#include <QJsonObject>

CAF_PDM_SOURCE_INIT( RimWorkflowVec3Binding, "WorkflowVec3Binding" );

RimWorkflowVec3Binding::RimWorkflowVec3Binding()
{
    CAF_PDM_InitField( &m_value, "Value", cvf::Vec3d::ZERO, "Value" );
}

void RimWorkflowVec3Binding::applySchema( const QJsonObject& fieldSchema )
{
    RimWorkflowFieldBinding::applySchema( fieldSchema );

    // taskmaestro_resinsight.models.Vec3 is a plain pydantic model with x/y/z number fields, so its
    // JSON representation (both the schema default and config_values) is an object, not an array.
    const QJsonObject defaultObject = fieldSchema.value( "default" ).toObject();
    if ( defaultObject.contains( "x" ) && defaultObject.contains( "y" ) && defaultObject.contains( "z" ) )
    {
        m_value =
            cvf::Vec3d( defaultObject.value( "x" ).toDouble(), defaultObject.value( "y" ).toDouble(), defaultObject.value( "z" ).toDouble() );
    }
}

QString RimWorkflowVec3Binding::toYamlValue() const
{
    if ( !hasValue() ) return "null";

    const cvf::Vec3d& v = m_value();
    return QString( "{x: %1, y: %2, z: %3}" )
        .arg( QString::number( v.x(), 'g', 17 ) )
        .arg( QString::number( v.y(), 'g', 17 ) )
        .arg( QString::number( v.z(), 'g', 17 ) );
}

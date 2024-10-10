/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RimFieldReference.h"

#include "cafPdmUiToolButtonEditor.h"

CAF_PDM_SOURCE_INIT( RimFieldReference, "RimFieldReference" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFieldReference::RimFieldReference()
{
    CAF_PDM_InitFieldNoDefault( &m_object, "Object", "Object" );
    CAF_PDM_InitFieldNoDefault( &m_fieldName, "FieldName", "FieldName" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFieldReference::setField( caf::PdmFieldHandle* field )
{
    if ( !field ) return;

    auto ownerObject = field->ownerObject();
    if ( !ownerObject ) return;

    setField( field->ownerObject(), field->keyword() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFieldReference::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( field() )
    {
        uiOrdering.add( field() );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFieldReference::field() const
{
    if ( !m_object() ) return nullptr;

    return m_object->findField( m_fieldName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimFieldReference::object() const
{
    return m_object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFieldReference::setField( caf::PdmObjectHandle* object, const QString& fieldName )
{
    m_object    = object;
    m_fieldName = fieldName;
}

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
void RimFieldReference::setObject( caf::PdmObject* object )
{
    m_object = object;

    std::vector<QString> fieldNames = RimFieldReference::fieldNames( object );
    if ( !fieldNames.empty() )
    {
        m_fieldName = fieldNames[0];
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFieldReference::setField( caf::PdmFieldHandle* field )
{
    if ( !field ) return;

    auto ownerObject = dynamic_cast<caf::PdmObject*>( field->ownerObject() );
    if ( !ownerObject ) return;

    setField( ownerObject, field->keyword() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimFieldReference::fieldNames( caf::PdmObject* object )
{
    std::vector<QString> names;

    if ( object )
    {
        auto allFields = object->fields();
        for ( auto field : allFields )
        {
            names.push_back( field->keyword() );
        }
    }

    return names;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFieldReference::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimFieldReference::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_fieldName )
    {
        auto fieldNames = RimFieldReference::fieldNames( m_object );
        for ( const auto& name : fieldNames )
        {
            options.push_back( caf::PdmOptionItemInfo( name, name ) );
        }
    }
    else if ( fieldNeedingOptions == &m_object )
    {
        if ( m_objectsForSelection.empty() )
        {
            if ( m_object )
            {
                QString text = m_object()->uiName();
                options.push_back( caf::PdmOptionItemInfo( text, m_object ) );
            }
        }
        else
        {
            for ( auto obj : m_objectsForSelection )
            {
                QString text = obj->uiName();
                options.push_back( caf::PdmOptionItemInfo( text, obj ) );
            }
        }
    }

    return options;
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
caf::PdmObject* RimFieldReference::object() const
{
    return m_object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFieldReference::setObjectsForSelection( const std::vector<caf::PdmObject*>& objectsForSelection )
{
    m_objectsForSelection = objectsForSelection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFieldReference::setField( caf::PdmObject* object, const QString& fieldName )
{
    m_object    = object;
    m_fieldName = fieldName;
}

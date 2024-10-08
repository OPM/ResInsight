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

#include "RimFieldReferenceCollection.h"

CAF_PDM_SOURCE_INIT( RimFieldReferenceCollection, "RimFieldReferenceCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFieldReferenceCollection::RimFieldReferenceCollection()
{
    CAF_PDM_InitObject( "Field Reference Collection" );

    CAF_PDM_InitFieldNoDefault( &m_objects, "Objects", "Objects" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFieldReferenceCollection::addFieldReference( caf::PdmFieldHandle* fieldHandle )
{
    if ( !fieldHandle ) return;

    for ( auto obj : m_objects )
    {
        auto field = obj->field();
        if ( field == fieldHandle )
        {
            return;
        }
    }

    auto fieldReference = new RimFieldReference();
    fieldReference->setField( fieldHandle );

    m_objects.push_back( fieldReference );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFieldReferenceCollection::removeFieldReference( caf::PdmFieldHandle* fieldHandle )
{
    if ( !fieldHandle ) return;

    for ( auto obj : m_objects )
    {
        auto field = obj->field();
        if ( field == fieldHandle )
        {
            m_objects.removeChild( obj );
            delete obj;
            return;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFieldReferenceCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    for ( auto obj : m_objects.childrenByType() )
    {
        auto field = obj->field();
        if ( field )
        {
            uiOrdering.add( field );
        }
    }
}

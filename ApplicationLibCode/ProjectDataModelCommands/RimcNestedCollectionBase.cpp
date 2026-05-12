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

#include "RimcNestedCollectionBase.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmNestedCollectionBase.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( caf::PdmNestedCollectionBase, RimcNestedCollectionBase, "AddFolder" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcNestedCollectionBase::RimcNestedCollectionBase( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Folder", "", "", "Add a new folder" );

    CAF_PDM_InitScriptableField( &m_folderName, "FolderName", QString( "Folder" ), "", "", "", "New folder name" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcNestedCollectionBase::execute()
{
    auto* coll = self<caf::PdmNestedCollectionBase>();
    if ( !coll || !coll->canAddSubCollection() )
    {
        return std::unexpected<QString>( QString( "Cannot add subfolder" ) );
    }

    caf::PdmObject* added = coll->addNewSubCollection();
    if ( !added )
    {
        return std::unexpected<QString>( QString( "Failed to add subfolder" ) );
    }

    if ( auto* asNested = dynamic_cast<caf::PdmNestedCollectionBase*>( added ) )
    {
        asNested->setCollectionName( m_folderName() );
    }
    coll->uiCapability()->updateConnectedEditors();
    return added;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcNestedCollectionBase::classKeywordReturnedType() const
{
    // Returned at codegen time. The Python generator emits AddFolder on the
    // PdmNestedCollectionBase Python class (since that is where the method is registered),
    // so the declared return type must be stable across all concrete nested collections.
    // The actual runtime instance is whatever the concrete addNewSubCollection() returns;
    // callers can use descendants() / class_from_keyword to recover the precise type.
    return caf::PdmNestedCollectionBase::classKeywordStatic();
}

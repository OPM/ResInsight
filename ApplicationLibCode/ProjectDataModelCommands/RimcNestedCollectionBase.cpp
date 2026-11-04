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

#include "RiaNameUniquenessTools.h"

#include "RimcDataContainerString.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmNestedCollectionBase.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( caf::PdmNestedCollectionBase, RimcNestedCollectionBase, "AddFolder" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcNestedCollectionBase::RimcNestedCollectionBase( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Folder",
                        "",
                        "",
                        "Add a new folder. Raises an error on a folder that cannot hold folders, such as an imported "
                        "polygon file or a cloud polygon folder" );

    CAF_PDM_InitScriptableField( &m_folderName, "FolderName", QString( "Folder" ), "", "", "", "New folder name" );
    CAF_PDM_InitScriptableField( &m_onNameConflict,
                                 "OnNameConflict",
                                 caf::AppEnum<RiaDefines::NameConflictPolicy>( RiaDefines::NameConflictPolicy::FAIL ),
                                 "",
                                 "",
                                 "",
                                 "How to handle a folder name already used in this folder" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcNestedCollectionBase::execute()
{
    auto* coll = self<caf::PdmNestedCollectionBase>();
    // Checked before the name conflict policy runs, since OVERWRITE deletes an existing folder.
    if ( !coll || !coll->canAddSubCollection() )
    {
        return std::unexpected( "Cannot add subfolder" );
    }

    auto* subCollections = coll->subCollectionsField();
    auto  resolution     = RiaNameUniquenessTools::applyConflictPolicy( subCollections, m_folderName(), m_onNameConflict().value() );
    if ( !resolution.errorMessage.isEmpty() ) return std::unexpected( resolution.errorMessage );

    if ( resolution.objectToReplace && subCollections )
    {
        size_t index = subCollections->indexOf( resolution.objectToReplace );
        if ( index < subCollections->size() )
        {
            subCollections->erase( index );
            delete resolution.objectToReplace;
        }
    }

    caf::PdmNestedCollectionBase* added = coll->addNewSubCollection();
    if ( !added )
    {
        return std::unexpected( "Failed to add subfolder" );
    }

    added->setCollectionName( resolution.nameToUse );
    coll->uiCapability()->updateConnectedEditors();
    return added;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcNestedCollectionBase::classKeywordReturnedType() const
{
    // The method is registered on PdmNestedCollectionBase, so this is the type the Python generator
    // declares for PdmNestedCollectionBase.add_folder(). It must not depend on self(): the generator
    // creates the method once per concrete collection and would otherwise keep the type of whichever
    // collection it visited last. The concrete type is reported by classKeywordReturnedTypeForSelf()
    // and emitted as a typed override on the concrete class.
    return caf::PdmNestedCollectionBase::classKeywordStatic();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcNestedCollectionBase::classKeywordReturnedTypeForSelf() const
{
    auto* coll = self<caf::PdmNestedCollectionBase>();
    if ( !coll ) return classKeywordReturnedType();

    QString keyword = coll->subCollectionClassKeyword();
    return keyword.isEmpty() ? classKeywordReturnedType() : keyword;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( caf::PdmNestedCollectionBase, RimcNestedCollectionBase_collectionName, "CollectionNameInternal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcNestedCollectionBase_collectionName::RimcNestedCollectionBase_collectionName( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_FALSE )
{
    CAF_PDM_InitObject( "Collection Name", "", "", "Name of the folder" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcNestedCollectionBase_collectionName::execute()
{
    auto* coll = self<caf::PdmNestedCollectionBase>();
    if ( !coll ) return std::unexpected( "Not a folder" );

    auto* result           = new RimcDataContainerString();
    result->m_stringValues = { coll->displayName() };
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcNestedCollectionBase_collectionName::classKeywordReturnedType() const
{
    return RimcDataContainerString::classKeywordStatic();
}

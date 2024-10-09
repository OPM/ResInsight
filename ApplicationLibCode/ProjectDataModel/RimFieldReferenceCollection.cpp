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
#include "RimProject.h"

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
RimFieldReferenceCollection* RimFieldReferenceCollection::instance()
{
    auto proj = RimProject::current();
    if ( !proj ) return nullptr;

    return proj->fieldReferenceCollection();
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
    std::map<caf::PdmObjectHandle*, std::vector<caf::PdmFieldHandle*>> fieldMap;

    // group fields by object
    for ( auto obj : m_objects )
    {
        if ( !obj ) continue;

        if ( auto field = obj->field() )
        {
            auto ownerObject = field->ownerObject();
            if ( ownerObject )
            {
                fieldMap[ownerObject].push_back( field );
            }
        }
    }

    int groupId = 1;

    // create ui ordering with a group for each object
    for ( auto& pair : fieldMap )
    {
        auto object = pair.first;
        auto fields = pair.second;

        QString groupName;
        auto    uiCapability = object->uiCapability();
        if ( uiCapability->userDescriptionField() )
        {
            groupName = uiCapability->userDescriptionField()->uiCapability()->uiValue().toString();
        }
        else
        {
            groupName = "Group " + QString::number( groupId );
        }

        auto group = uiOrdering.addNewGroup( groupName );
        groupId++;

        for ( auto field : fields )
        {
            group->add( field );
        }
    }
}

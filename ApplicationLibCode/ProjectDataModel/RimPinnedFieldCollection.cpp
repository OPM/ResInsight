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

#include "RimPinnedFieldCollection.h"

#include "RiaApplication.h"

#include "RimFieldQuickAccess.h"
#include "RimProject.h"

#include "cafAssert.h"

CAF_PDM_SOURCE_INIT( RimPinnedFieldCollection, "RimFieldReferenceCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPinnedFieldCollection::RimPinnedFieldCollection()
{
    CAF_PDM_InitObject( "Field Reference Collection" );

    CAF_PDM_InitFieldNoDefault( &m_fieldReferences, "Objects", "Objects" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPinnedFieldCollection* RimPinnedFieldCollection::instance()
{
    auto proj = RimProject::current();
    CAF_ASSERT( proj && "RimProject is nullptr when trying to access RimFieldReferenceCollection::instance()" );

    return proj->pinnedFieldCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPinnedFieldCollection::addField( caf::PdmFieldHandle* field )
{
    if ( !field ) return;

    for ( auto fieldRef : m_fieldReferences )
    {
        if ( field == fieldRef->field() )
        {
            return;
        }
    }

    auto fieldReference = new RimFieldQuickAccess();
    fieldReference->setField( field );

    m_fieldReferences.push_back( fieldReference );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPinnedFieldCollection::removeField( caf::PdmFieldHandle* field )
{
    if ( !field ) return;

    for ( auto fieldRef : m_fieldReferences )
    {
        if ( field == fieldRef->field() )
        {
            m_toBeDeleted.insert( fieldRef );
            return;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPinnedFieldCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto activeView = RiaApplication::instance()->activeGridView();
    if ( !activeView ) return;

    deleteMarkedObjects();

    std::vector<RimFieldQuickAccess*> fieldsForView;

    for ( auto fieldRef : m_fieldReferences )
    {
        if ( !fieldRef ) continue;

        if ( auto field = fieldRef->field() )
        {
            if ( auto ownerObject = field->ownerObject() )
            {
                auto view = ownerObject->firstAncestorOrThisOfType<RimGridView>();
                if ( view != activeView )
                {
                    continue;
                }

                fieldsForView.push_back( fieldRef );
            }
        }
    }

    if ( fieldsForView.empty() ) return;

    QString groupName;
    auto    uiCapability = activeView->uiCapability();
    if ( uiCapability->userDescriptionField() && uiCapability->userDescriptionField()->uiCapability() )
    {
        groupName = uiCapability->userDescriptionField()->uiCapability()->uiValue().toString();
    }
    else
    {
        groupName = "Group ";
    }

    auto group = uiOrdering.addNewGroup( groupName );

    for ( auto fieldRef : fieldsForView )
    {
        fieldRef->uiOrdering( uiConfigName, *group );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPinnedFieldCollection::deleteMarkedObjects()
{
    for ( auto fieldRef : m_fieldReferences )
    {
        if ( fieldRef->toBeDeleted() )
        {
            m_toBeDeleted.insert( fieldRef );
        }
    }

    for ( auto fieldRef : m_toBeDeleted )
    {
        m_fieldReferences.removeChild( fieldRef );
        delete fieldRef;
    }
    m_toBeDeleted.clear();
}

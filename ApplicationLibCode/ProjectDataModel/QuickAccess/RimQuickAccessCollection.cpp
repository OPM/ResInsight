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

#include "RimQuickAccessCollection.h"

#include "RiaApplication.h"

#include "RimFieldQuickAccess.h"
#include "RimFieldQuickAccessInterface.h"
#include "RimFieldReference.h"
#include "RimGridView.h"
#include "RimProject.h"

#include "cafAssert.h"

CAF_PDM_SOURCE_INIT( RimQuickAccessCollection, "RimQuickAccessCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimQuickAccessCollection::RimQuickAccessCollection()
{
    CAF_PDM_InitObject( "Field Reference Collection" );

    CAF_PDM_InitFieldNoDefault( &m_fieldQuickAccesGroups, "FieldReferencesGroup", "Field References Group" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimQuickAccessCollection* RimQuickAccessCollection::instance()
{
    auto proj = RimProject::current();
    CAF_ASSERT( proj && "RimProject is nullptr when trying to access RimQuickAccessCollection::instance()" );

    return proj->pinnedFieldCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQuickAccessCollection::addQuickAccessFieldsRecursively( caf::PdmObjectHandle* object )
{
    if ( object == nullptr ) return;
    addQuickAccessFields( object );

    for ( auto field : object->fields() )
    {
        if ( !field ) continue;

        for ( auto childObject : field->children() )
        {
            addQuickAccessFieldsRecursively( childObject );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQuickAccessCollection::addQuickAccessFields( caf::PdmObjectHandle* object )
{
    if ( !object ) return;

    if ( auto quickInterface = dynamic_cast<RimFieldQuickAccessInterface*>( object ) )
    {
        for ( const auto& [groupName, fields] : quickInterface->quickAccessFields() )
        {
            if ( auto group = findOrCreateGroup( object, groupName ) )
            {
                group->addFields( fields );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQuickAccessCollection::addQuickAccessField( const RimFieldReference& fieldReference )
{
    auto object = fieldReference.object();
    auto field  = fieldReference.field();
    if ( object && field )
    {
        if ( auto group = findOrCreateGroup( object, "" ) )
        {
            group->addField( field );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQuickAccessCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto activeView = RiaApplication::instance()->activeGridView();
    if ( !activeView ) return;

    deleteMarkedObjects();

    std::vector<RimFieldQuickAccessGroup*> groupsForView;

    for ( auto group : m_fieldQuickAccesGroups )
    {
        if ( group->ownerView() == activeView )
        {
            updateGroupName( group );
            groupsForView.push_back( group );
        }
    }

    for ( auto group : groupsForView )
    {
        auto name = group->name();
        if ( name.isEmpty() ) name = defaultGroupName();

        // Make sure the name of the group is unique in the UI ordering. This is a requirement for the UI ordering, as the groups are
        // identified by name.
        auto             uiName  = name;
        caf::PdmUiGroup* uiGroup = uiOrdering.findGroup( uiName );
        int              index   = 1;
        while ( uiGroup )
        {
            uiName  = "(" + QString::number( index++ ) + ") " + name;
            uiGroup = uiOrdering.findGroup( uiName );
        }

        uiGroup = uiOrdering.addNewGroup( uiName );

        for ( auto quickAccess : group->fieldQuickAccesses() )
        {
            quickAccess->uiOrdering( uiConfigName, *uiGroup );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQuickAccessCollection::deleteMarkedObjects()
{
    // Delete marked objects and objects pointing to fields that are no longer valid
    {
        std::set<RimFieldQuickAccess*> toBeDeleted;

        for ( auto group : m_fieldQuickAccesGroups.childrenByType() )
        {
            for ( auto quickAccess : group->fieldQuickAccesses() )
            {
                if ( quickAccess->markedForRemoval() || !quickAccess->field() )
                {
                    toBeDeleted.insert( quickAccess );
                }
            }
        }

        for ( auto quickAccess : toBeDeleted )
        {
            for ( auto group : m_fieldQuickAccesGroups )
            {
                group->removeFieldQuickAccess( quickAccess );
            }

            delete quickAccess;
        }
    }

    // Delete groups with no quick access fields
    {
        std::set<RimFieldQuickAccessGroup*> toBeDeleted;
        for ( auto group : m_fieldQuickAccesGroups.childrenByType() )
        {
            if ( group->fieldQuickAccesses().empty() )
            {
                toBeDeleted.insert( group );
            }
        }

        for ( auto group : toBeDeleted )
        {
            m_fieldQuickAccesGroups.removeChild( group );
            delete group;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFieldQuickAccessGroup* RimQuickAccessCollection::findOrCreateGroup( caf::PdmObjectHandle* object, const QString& groupName )
{
    if ( !object ) return nullptr;

    auto parentView = object->firstAncestorOrThisOfType<RimGridView>();
    if ( !parentView ) return nullptr;

    for ( auto group : m_fieldQuickAccesGroups )
    {
        if ( !group ) continue;

        if ( groupName.isEmpty() && group->name().isEmpty() && group->ownerView() == parentView )
        {
            // If group name is empty, we assume that this is the default group for the view
            return group;
        }

        if ( group->groupOwner() == object ) return group;
    }

    auto group = new RimFieldQuickAccessGroup();
    group->setName( groupName );
    group->setOwnerView( parentView );
    m_fieldQuickAccesGroups.push_back( group );

    return group;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimQuickAccessCollection::updateGroupName( RimFieldQuickAccessGroup* quickAccessGroup )
{
    if ( !quickAccessGroup ) return;

    caf::PdmObjectHandle* commonOwnerObject       = nullptr;
    caf::PdmFieldHandle*  firstFieldInQuickAccess = nullptr;

    for ( auto quickAccess : quickAccessGroup->fieldQuickAccesses() )
    {
        if ( !quickAccess || !quickAccess->field() || !quickAccess->field()->ownerObject() ) continue;

        if ( !firstFieldInQuickAccess ) firstFieldInQuickAccess = quickAccess->field();

        auto ownerToField = quickAccess->field()->ownerObject();
        if ( !commonOwnerObject )
        {
            commonOwnerObject = ownerToField;
        }
        else
        {
            if ( commonOwnerObject != ownerToField ) return;
        }
    }

    if ( auto fieldInterface = dynamic_cast<RimFieldQuickAccessInterface*>( commonOwnerObject ) )
    {
        auto ownerFields = fieldInterface->quickAccessFields();
        for ( const auto& [groupName, fields] : ownerFields )
        {
            for ( auto field : fields )
            {
                if ( field == firstFieldInQuickAccess )
                {
                    quickAccessGroup->setName( groupName );
                    return;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimQuickAccessCollection::defaultGroupName()
{
    return "Quick Access for View";
}

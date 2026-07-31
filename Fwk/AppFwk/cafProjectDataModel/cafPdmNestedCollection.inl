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

#include "cafPdmUiTableViewEditor.h"

namespace caf
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
PdmNestedCollection<SelfT, ItemT>::PdmNestedCollection()
{
    static_assert( DerivedFromPdmObject<SelfT>, "SelfT must inherit from caf::PdmObject" );
    static_assert( DerivedFromPdmObject<ItemT>, "ItemT must inherit from caf::PdmObject" );
    // m_items, m_collectionName and m_subCollections must be initialized by the derived class
    // using CAF_PDM_InitFieldNoDefault, so that the XML keywords are stable for that class.
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
PdmNestedCollection<SelfT, ItemT>::~PdmNestedCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
std::vector<ItemT*> PdmNestedCollection<SelfT, ItemT>::items() const
{
    return m_items.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
size_t PdmNestedCollection<SelfT, ItemT>::count() const
{
    return m_items.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
bool PdmNestedCollection<SelfT, ItemT>::isEmpty() const
{
    return m_items.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
PdmChildArrayField<ItemT*>& PdmNestedCollection<SelfT, ItemT>::itemsField()
{
    return m_items;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
const PdmChildArrayField<ItemT*>& PdmNestedCollection<SelfT, ItemT>::itemsField() const
{
    return m_items;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
void PdmNestedCollection<SelfT, ItemT>::addItem( ItemT* item )
{
    if ( item )
    {
        m_items.push_back( item );
        onItemsChanged();
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
void PdmNestedCollection<SelfT, ItemT>::insertItem( ItemT* insertBefore, ItemT* item )
{
    if ( !item ) return;

    size_t index = m_items.indexOf( insertBefore );
    if ( index < m_items.size() )
        m_items.insert( index, item );
    else
        m_items.push_back( item );

    onItemsChanged();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
void PdmNestedCollection<SelfT, ItemT>::deleteItem( ItemT* item )
{
    if ( item )
    {
        m_items.removeChild( item );
        delete item;
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
void PdmNestedCollection<SelfT, ItemT>::deleteAllItems()
{
    m_items.deleteChildren();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
ItemT* PdmNestedCollection<SelfT, ItemT>::createDefaultItem()
{
    auto* item = new ItemT();
    addItem( item );
    return item;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
void PdmNestedCollection<SelfT, ItemT>::updateConnectedEditors()
{
    this->uiCapability()->updateConnectedEditors();
    m_items.uiCapability()->updateConnectedEditors();
    m_subCollections.uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
std::vector<SelfT*> PdmNestedCollection<SelfT, ItemT>::subCollections() const
{
    return m_subCollections.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
void PdmNestedCollection<SelfT, ItemT>::addSubCollection( SelfT* sub )
{
    if ( sub )
    {
        m_subCollections.push_back( sub );
        this->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
PdmObject* PdmNestedCollection<SelfT, ItemT>::addNewSubCollection()
{
    auto* sub = new SelfT();
    addSubCollection( sub );
    return sub;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
PdmChildArrayFieldHandle* PdmNestedCollection<SelfT, ItemT>::subCollectionsField()
{
    return &m_subCollections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
SelfT* PdmNestedCollection<SelfT, ItemT>::findSubCollectionByName( const QString& name ) const
{
    for ( auto coll : m_subCollections )
    {
        if ( coll && coll->collectionName() == name ) return coll;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
std::vector<ItemT*> PdmNestedCollection<SelfT, ItemT>::allItems() const
{
    std::vector<ItemT*> result = items();
    for ( auto* sub : subCollections() )
    {
        if ( !sub ) continue;
        auto subItems = sub->allItems();
        result.insert( result.end(), subItems.begin(), subItems.end() );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
void PdmNestedCollection<SelfT, ItemT>::onItemsChanged()
{
    // Default implementation does nothing; derived classes may override.
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
void PdmNestedCollection<SelfT, ItemT>::fieldChangedByUi( const PdmFieldHandle* changedField,
                                                          const QVariant&       oldValue,
                                                          const QVariant&       newValue )
{
    if ( changedField == &m_items )
    {
        onItemsChanged();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
void PdmNestedCollection<SelfT, ItemT>::defineUiOrdering( QString uiConfigName, PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_collectionName );
    uiOrdering.add( &m_subCollections );
    uiOrdering.add( &m_items );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
void PdmNestedCollection<SelfT, ItemT>::defineEditorAttribute( const PdmFieldHandle* field,
                                                               QString               uiConfigName,
                                                               PdmUiEditorAttribute* attribute )
{
    if ( field == &m_items )
    {
        auto tvAttribute = dynamic_cast<PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttribute )
        {
            tvAttribute->resizePolicy              = PdmUiTableViewEditorAttribute::RESIZE_TO_FILL_CONTAINER;
            tvAttribute->alwaysEnforceResizePolicy = true;
            tvAttribute->minimumHeight             = 300;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename ItemT>
void PdmNestedCollection<SelfT, ItemT>::onChildDeleted( PdmChildArrayFieldHandle*      childArray,
                                                        std::vector<PdmObjectHandle*>& referringObjects )
{
    updateConnectedEditors();
}

} // namespace caf

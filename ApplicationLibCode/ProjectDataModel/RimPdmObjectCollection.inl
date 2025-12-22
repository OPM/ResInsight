/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
RimPdmObjectCollection<T>::RimPdmObjectCollection()
{
    // m_items field must be initialized by derived class using CAF_PDM_InitFieldNoDefault
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
RimPdmObjectCollection<T>::~RimPdmObjectCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
std::vector<T*> RimPdmObjectCollection<T>::items() const
{
    return m_items.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
size_t RimPdmObjectCollection<T>::count() const
{
    return m_items.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
bool RimPdmObjectCollection<T>::isEmpty() const
{
    return m_items.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
caf::PdmChildArrayField<T*>& RimPdmObjectCollection<T>::itemsField()
{
    return m_items;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
const caf::PdmChildArrayField<T*>& RimPdmObjectCollection<T>::itemsField() const
{
    return m_items;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void RimPdmObjectCollection<T>::addItem( T* item )
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
template <typename T>
void RimPdmObjectCollection<T>::insertItem( T* insertBefore, T* item )
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
template <typename T>
void RimPdmObjectCollection<T>::removeItem( T* item )
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
template <typename T>
void RimPdmObjectCollection<T>::removeAllItems()
{
    m_items.deleteChildren();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
T* RimPdmObjectCollection<T>::createDefaultItem()
{
    auto* item = new T();
    addItem( item );
    return item;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void RimPdmObjectCollection<T>::updateConnectedEditors()
{
    m_items.uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void RimPdmObjectCollection<T>::onItemsChanged()
{
    // Default implementation does nothing
    // Derived classes can override to add custom behavior
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void RimPdmObjectCollection<T>::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
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
template <typename T>
void RimPdmObjectCollection<T>::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_items );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void RimPdmObjectCollection<T>::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_items )
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttribute )
        {
            tvAttribute->resizePolicy              = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FILL_CONTAINER;
            tvAttribute->alwaysEnforceResizePolicy = true;
            tvAttribute->minimumHeight             = 300;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void RimPdmObjectCollection<T>::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    updateConnectedEditors();
}

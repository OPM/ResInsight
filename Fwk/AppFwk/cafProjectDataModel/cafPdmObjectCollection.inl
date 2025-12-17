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

namespace caf
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
PdmObjectCollection<T>::PdmObjectCollection()
{
    // m_items field must be initialized by derived class using CAF_PDM_InitFieldNoDefault
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
PdmObjectCollection<T>::~PdmObjectCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
std::vector<T*> PdmObjectCollection<T>::items() const
{
    return m_items.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
size_t PdmObjectCollection<T>::count() const
{
    return m_items.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
bool PdmObjectCollection<T>::isEmpty() const
{
    return m_items.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
PdmChildArrayField<T*>& PdmObjectCollection<T>::itemsField()
{
    return m_items;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
const PdmChildArrayField<T*>& PdmObjectCollection<T>::itemsField() const
{
    return m_items;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void PdmObjectCollection<T>::addItem( T* item )
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
void PdmObjectCollection<T>::insertItem( T* insertBefore, T* item )
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
void PdmObjectCollection<T>::deleteItem( T* item )
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
void PdmObjectCollection<T>::deleteAllItems()
{
    m_items.deleteChildren();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
T* PdmObjectCollection<T>::createDefaultItem()
{
    auto* item = new T();
    addItem( item );
    return item;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void PdmObjectCollection<T>::updateConnectedEditors()
{
    uiCapability()->updateConnectedEditors();
    m_items.uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void PdmObjectCollection<T>::onItemsChanged()
{
    // Default implementation does nothing
    // Derived classes can override to add custom behavior
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void PdmObjectCollection<T>::fieldChangedByUi( const PdmFieldHandle* changedField,
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
template <typename T>
void PdmObjectCollection<T>::defineUiOrdering( QString uiConfigName, PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_items );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
void PdmObjectCollection<T>::defineEditorAttribute( const PdmFieldHandle* field,
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
template <typename T>
void PdmObjectCollection<T>::onChildDeleted( PdmChildArrayFieldHandle*      childArray,
                                             std::vector<PdmObjectHandle*>& referringObjects )
{
    updateConnectedEditors();
}

} // namespace caf

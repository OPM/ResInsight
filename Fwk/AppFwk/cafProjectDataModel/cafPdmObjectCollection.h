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

#pragma once

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

#include <vector>

namespace caf
{

//==================================================================================================
///
/// Generic templated collection base class for managing collections of PDM objects
///
/// This template provides common collection operations (add, remove, insert, etc.) and UI
/// integration for any type of PDM object. Derived classes can add domain-specific functionality.
///
/// Template parameter:
///   T - Item type, must inherit from caf::PdmObject
///
/// Derived classes must call the base constructor with field name and display name.
///
//==================================================================================================
template <typename T>
class PdmObjectCollection : public PdmObject
{
    static_assert( std::is_base_of<caf::PdmObject, T>::value, "T must inherit from caf::PdmObject" );

public:
    // Collection access
    std::vector<T*>               items() const;
    size_t                        count() const;
    bool                          isEmpty() const;
    PdmChildArrayField<T*>&       itemsField();
    const PdmChildArrayField<T*>& itemsField() const;

    // CRUD operations
    void addItem( T* item );
    void insertItem( T* insertBefore, T* item );
    void deleteItem( T* item );
    void deleteAllItems();

    // Creation helper
    T* createDefaultItem();

    // UI integration
    void updateConnectedEditors();

protected:
    // Constructor (protected - only derived classes can instantiate)
    // Derived classes MUST initialize m_items field using CAF_PDM_InitFieldNoDefault
    PdmObjectCollection();
    ~PdmObjectCollection() override;

    // Virtual hook for derived class customization
    // Called when items are added, removed, or the collection is modified
    virtual void onItemsChanged();

    // PdmObject overrides
    void fieldChangedByUi( const PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const PdmFieldHandle* field, QString uiConfigName, PdmUiEditorAttribute* attribute ) override;

    // Protected member - derived classes can access for advanced operations
    PdmChildArrayField<T*> m_items;

private:
    void onChildDeleted( PdmChildArrayFieldHandle* childArray, std::vector<PdmObjectHandle*>& referringObjects ) override;
};

} // namespace caf

// Include template implementation
#include "cafPdmObjectCollection.inl"

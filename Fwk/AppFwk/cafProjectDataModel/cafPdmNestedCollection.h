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

#pragma once

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmNestedCollectionBase.h"
#include "cafPdmObjectCollection.h" // for DerivedFromPdmObject concept

#include <QString>

#include <vector>

namespace caf
{

//==================================================================================================
///
/// Generic templated nested collection base class
///
/// CRTP template for tree-shaped PDM containers (folder of items + folder of folders). Derives
/// from caf::PdmNestedCollectionBase, which provides the non-templated PdmObject anchor (so
/// generic script methods like AddFolder can be registered once on the base and inherit through
/// every concrete derivation).
///
/// Template parameters:
///   SelfT - The derived class (CRTP). Must inherit from caf::PdmObject.
///   ItemT - Item type held in the flat items vector. Must inherit from caf::PdmObject.
///
/// Derived classes must call CAF_PDM_InitFieldNoDefault for the inherited fields
/// m_items, m_collectionName and m_subCollections so the XML keywords stay derived-class
/// specific (matching the existing PdmObjectCollection<T> convention).
///
//==================================================================================================
template <typename SelfT, typename ItemT>
class PdmNestedCollection : public PdmNestedCollectionBase
{
public:
    // Item access
    std::vector<ItemT*>               items() const;
    size_t                            count() const;
    bool                              isEmpty() const;
    PdmChildArrayField<ItemT*>&       itemsField();
    const PdmChildArrayField<ItemT*>& itemsField() const;

    // Item CRUD
    void   addItem( ItemT* item );
    void   insertItem( ItemT* insertBefore, ItemT* item );
    void   deleteItem( ItemT* item );
    void   deleteAllItems();
    ItemT* createDefaultItem();

    void updateConnectedEditors();

    // Subcollection access
    std::vector<SelfT*> subCollections() const;

    // Subcollection CRUD
    void                      addSubCollection( SelfT* sub );
    PdmObject*                addNewSubCollection() override;
    PdmChildArrayFieldHandle* subCollectionsField() override;
    SelfT*                    findSubCollectionByName( const QString& name ) const;

    // Returns items held by this collection and recursively by all subcollections.
    // For items at this level only, use items().
    std::vector<ItemT*> allItems() const;

protected:
    PdmNestedCollection();
    ~PdmNestedCollection() override;

    // Hook for derived classes; called when m_items changes.
    virtual void onItemsChanged();

    void fieldChangedByUi( const PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const PdmFieldHandle* field, QString uiConfigName, PdmUiEditorAttribute* attribute ) override;
    void onChildDeleted( PdmChildArrayFieldHandle* childArray, std::vector<PdmObjectHandle*>& referringObjects ) override;

    PdmChildArrayField<SelfT*> m_subCollections;
    PdmChildArrayField<ItemT*> m_items;
};

} // namespace caf

#include "cafPdmNestedCollection.inl"

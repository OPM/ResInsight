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

#include "RimCheckableNamedObject.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmPtrField.h"

#include <vector>

//==================================================================================================
///
/// View-side analogue of caf::PdmNestedCollection.
///
/// Generic CRTP template for in-view collections that mirror a nested source collection
/// (a caf::PdmNestedCollection-derived type). Provides a flat items vector, a vector of
/// sub-collections of the same (derived) type, a source-collection pointer, and a sync
/// engine that mirrors the source tree by orphan-sweeping, find-or-creating wrappers,
/// and reordering to match the source.
///
/// Template parameters:
///   SelfT     - The derived class (CRTP).
///   SourceT   - The source collection type. Must expose collectionName(), and the data
///               needed by the sourceSubCollections()/sourceItems() hooks.
///   ItemViewT - View-side item wrapper type. Must declare a SourceItemT typedef and a
///               sourceItem() accessor returning SourceItemT*.
///
/// Derived classes must call CAF_PDM_InitFieldNoDefault for the inherited fields
/// m_itemsInView, m_collectionsInView and m_sourceCollection, so that the XML keywords
/// stay derived-class-specific.
//==================================================================================================
template <typename SelfT, typename SourceT, typename ItemViewT>
class RimNestedMirrorCollectionInView : public RimCheckableNamedObject
{
public:
    using SourceItemT = typename ItemViewT::SourceItemT;

    SourceT* sourceCollection() const;
    void     setSourceCollection( SourceT* source );

    // Public sync entry. Refreshes the view tree from the current source.
    void updateFromSource();

protected:
    RimNestedMirrorCollectionInView();
    ~RimNestedMirrorCollectionInView() override;

    // Pure virtual hooks - derived class adapts to its source type.
    virtual std::vector<SourceT*>     sourceSubCollections() const         = 0;
    virtual std::vector<SourceItemT*> sourceItems() const                  = 0;
    virtual ItemViewT*                createItemInView( SourceItemT* src ) = 0;

    // Default impl: new SelfT() + setSourceCollection(src). Override for extra wiring.
    virtual SelfT* createSubCollectionInView( SourceT* src );

    // Recursive sync: sub-collections, then items, then editor refresh.
    void updateAllViewItems();

    SelfT*     findCollectionInViewForSource( const SourceT* src ) const;
    ItemViewT* findItemInViewForSource( const SourceItemT* src ) const;

    caf::PdmChildArrayField<ItemViewT*> m_itemsInView;
    caf::PdmChildArrayField<SelfT*>     m_collectionsInView;
    caf::PdmPtrField<SourceT*>          m_sourceCollection;
};

#include "RimNestedMirrorCollectionInView.inl"

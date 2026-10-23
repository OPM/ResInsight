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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename SourceT, typename ItemViewT>
RimNestedMirrorCollectionInView<SelfT, SourceT, ItemViewT>::RimNestedMirrorCollectionInView()
{
    // m_itemsInView, m_collectionsInView and m_sourceCollection must be initialized by the
    // derived class using CAF_PDM_InitFieldNoDefault, so that the XML keywords are stable
    // for that class.
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename SourceT, typename ItemViewT>
RimNestedMirrorCollectionInView<SelfT, SourceT, ItemViewT>::~RimNestedMirrorCollectionInView()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename SourceT, typename ItemViewT>
SourceT* RimNestedMirrorCollectionInView<SelfT, SourceT, ItemViewT>::sourceCollection() const
{
    return m_sourceCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename SourceT, typename ItemViewT>
void RimNestedMirrorCollectionInView<SelfT, SourceT, ItemViewT>::setSourceCollection( SourceT* source )
{
    m_sourceCollection = source;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename SourceT, typename ItemViewT>
void RimNestedMirrorCollectionInView<SelfT, SourceT, ItemViewT>::updateFromSource()
{
    updateAllViewItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename SourceT, typename ItemViewT>
SelfT* RimNestedMirrorCollectionInView<SelfT, SourceT, ItemViewT>::createSubCollectionInView( SourceT* src )
{
    auto* sub = new SelfT();
    sub->setSourceCollection( src );
    return sub;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename SourceT, typename ItemViewT>
QString RimNestedMirrorCollectionInView<SelfT, SourceT, ItemViewT>::computeDisplayName() const
{
    return m_sourceCollection ? m_sourceCollection->collectionName() : QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename SourceT, typename ItemViewT>
void RimNestedMirrorCollectionInView<SelfT, SourceT, ItemViewT>::prepareForSync()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename SourceT, typename ItemViewT>
void RimNestedMirrorCollectionInView<SelfT, SourceT, ItemViewT>::onSynced()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename SourceT, typename ItemViewT>
SelfT* RimNestedMirrorCollectionInView<SelfT, SourceT, ItemViewT>::findCollectionInViewForSource( const SourceT* src ) const
{
    for ( auto coll : m_collectionsInView )
    {
        if ( coll && coll->sourceCollection() == src ) return coll;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename SourceT, typename ItemViewT>
ItemViewT* RimNestedMirrorCollectionInView<SelfT, SourceT, ItemViewT>::findItemInViewForSource( const SourceItemT* src ) const
{
    for ( auto item : m_itemsInView )
    {
        if ( item && item->sourceItem() == src ) return item;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Recursive sync mirroring the source tree: drops orphaned wrappers, then find-or-creates one
/// wrapper per source sub-collection/item, reordering to match, recursing into children.
//--------------------------------------------------------------------------------------------------
template <typename SelfT, typename SourceT, typename ItemViewT>
void RimNestedMirrorCollectionInView<SelfT, SourceT, ItemViewT>::updateAllViewItems()
{
    // Let the derived class lazily trigger a fetch before sourceItems() is read below.
    this->prepareForSync();

    // Refresh display name from source.
    if ( m_sourceCollection )
    {
        this->setName( this->computeDisplayName() );
    }

    // --- Sub-collections ---
    {
        std::vector<SelfT*> existing = m_collectionsInView.childrenByType();
        for ( auto* coll : existing )
        {
            if ( !coll->sourceCollection() )
            {
                m_collectionsInView.removeChild( coll );
                delete coll;
            }
        }

        std::vector<SelfT*> ordered;
        if ( m_sourceCollection )
        {
            for ( auto* srcSub : sourceSubCollections() )
            {
                if ( !srcSub ) continue;

                SelfT* viewSub = findCollectionInViewForSource( srcSub );
                if ( viewSub == nullptr )
                {
                    viewSub = createSubCollectionInView( srcSub );
                }
                ordered.push_back( viewSub );
            }
        }

        m_collectionsInView.clearWithoutDelete();
        for ( auto* viewSub : ordered )
        {
            m_collectionsInView.push_back( viewSub );
            viewSub->updateAllViewItems();
        }
    }

    // --- Items ---
    {
        std::vector<ItemViewT*> existing = m_itemsInView.childrenByType();
        for ( auto* item : existing )
        {
            if ( !item->sourceItem() )
            {
                m_itemsInView.removeChild( item );
                delete item;
            }
        }

        std::vector<ItemViewT*> ordered;
        if ( m_sourceCollection )
        {
            for ( auto* srcItem : sourceItems() )
            {
                if ( !srcItem ) continue;

                ItemViewT* viewItem = findItemInViewForSource( srcItem );
                if ( viewItem == nullptr )
                {
                    viewItem = createItemInView( srcItem );
                }
                ordered.push_back( viewItem );
            }
        }

        m_itemsInView.clearWithoutDelete();
        for ( auto* viewItem : ordered )
        {
            m_itemsInView.push_back( viewItem );
        }
    }

    this->onSynced();
    this->updateConnectedEditors();
}

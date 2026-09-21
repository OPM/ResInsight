/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimPolygonInViewCollection.h"

#include "RiaDefines.h"

#include "ContourMap/RimEclipseContourMapView.h"
#include "Rim3dView.h"
#include "RimGridView.h"
#include "RimPolygon.h"
#include "RimPolygonCollection.h"
#include "RimPolygonInView.h"
#include "RimProject.h"
#include "RimTools.h"

#include "Polygons/Cloud/RimPolygonCloudAddress.h"
#include "Polygons/Cloud/RimPolygonCloudRealizationGroup.h"
#include "Polygons/Cloud/RimPolygonCloudSource.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiOrdering.h"

CAF_PDM_SOURCE_INIT( RimPolygonInViewCollection, "RimPolygonInViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonInViewCollection::RimPolygonInViewCollection()
{
    CAF_PDM_InitObject( "Polygons", ":/Folder.png" );

    CAF_PDM_InitFieldNoDefault( &m_itemsInView, "Polygons", "Polygons" );
    CAF_PDM_InitFieldNoDefault( &m_collectionsInView, "Collections", "Collections" );
    CAF_PDM_InitFieldNoDefault( &m_sourceCollection, "SourceCollection", "Source Collection" );
    m_sourceCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_useAutoRealization, "UseAutoRealization", true, "Auto-Follow View Realization" );

    CAF_PDM_InitField( &m_didApplyDefaultAutoRealization, "DidApplyDefaultAutoRealization", false, "" );
    m_didApplyDefaultAutoRealization.uiCapability()->setUiHidden( true );

    nameField()->uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::updateFromPolygonCollection()
{
    if ( !sourceCollection() )
    {
        setSourceCollection( RimTools::polygonCollection() );
    }

    updateFromSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygonInView*> RimPolygonInViewCollection::visiblePolygonsInView() const
{
    if ( !m_isChecked ) return {};

    std::vector<RimPolygonInView*> polys = m_itemsInView.childrenByType();

    for ( auto coll : m_collectionsInView )
    {
        if ( !coll->isChecked() ) continue;

        auto other = coll->visiblePolygonsInView();
        polys.insert( polys.end(), other.begin(), other.end() );
    }

    return polys;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygonInView*> RimPolygonInViewCollection::allPolygonsInView() const
{
    std::vector<RimPolygonInView*> polys = m_itemsInView.childrenByType();

    for ( auto coll : m_collectionsInView )
    {
        auto other = coll->visiblePolygonsInView();
        polys.insert( polys.end(), other.begin(), other.end() );
    }

    return polys;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonInViewCollection::setPolygonVisible( RimPolygon* polygon, bool visible )
{
    updateFromPolygonCollection();

    auto* polygonInView = findPolygonInView( polygon );
    if ( !polygonInView ) return false;

    polygonInView->setCheckState( visible );
    polygonInView->updateConnectedEditors();
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonInView* RimPolygonInViewCollection::findPolygonInView( const RimPolygon* polygon ) const
{
    for ( auto polygonInView : m_itemsInView )
    {
        if ( polygonInView && polygonInView->polygon() == polygon ) return polygonInView;
    }

    for ( auto collection : m_collectionsInView )
    {
        if ( auto* polygonInView = collection->findPolygonInView( polygon ) ) return polygonInView;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimCheckableNamedObject::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_isChecked )
    {
        for ( auto poly : visiblePolygonsInView() )
        {
            poly->updateConnectedEditors();
        }

        if ( auto view = firstAncestorOfType<Rim3dView>() )
        {
            view->scheduleCreateDisplayModelAndRedraw();
        }

        if ( !m_isChecked() )
        {
            // Unchecked: this data is no longer needed *by this view*. Evict it if no other open
            // view still shows the same source container checked.
            if ( auto* src = sourceCollection() )
            {
                if ( auto* address = dynamic_cast<RimPolygonCloudAddress*>( src ) )
                {
                    if ( address->hasBaseData() && !isSourceCheckedInAnyView( address ) ) address->evictBaseData();
                }
                else if ( auto* group = dynamic_cast<RimPolygonCloudRealizationGroup*>( src ) )
                {
                    if ( auto* owningAddress = group->firstAncestorOfType<RimPolygonCloudAddress>() )
                    {
                        if ( !isSourceCheckedInAnyView( group ) ) owningAddress->evictRealizationGroup( group->realization() );
                    }
                }
            }
        }
    }
    else if ( changedField == &m_useAutoRealization )
    {
        updateFromPolygonCollection();

        if ( !m_useAutoRealization() )
        {
            if ( auto* source = dynamic_cast<RimPolygonCloudSource*>( sourceCollection() ) )
            {
                source->evictUnusedRealizationData();
            }
        }

        if ( auto view = firstAncestorOfType<Rim3dView>() )
        {
            view->updateViewTreeItems( RiaDefines::ItemIn3dView::POLYGON );
            view->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    if ( firstAncestorOfType<RimEclipseContourMapView>() )
    {
        menuBuilder << "RicCreateContourMapPolygonFeature";
    }
    RimPolygonCollection::appendPolygonMenuItems( menuBuilder );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( auto* src = sourceCollection(); src && src->supportsRealizationOverride() )
    {
        if ( viewMatchingRealizationOrMinusOne() == -1 )
        {
            m_useAutoRealization.uiCapability()->setUiReadOnly( true );
            m_useAutoRealization.uiCapability()->setUiToolTip(
                "This view's case belongs to a different Sumo case/ensemble than this polygon "
                "address -- realization cannot be followed automatically. Using the address's own "
                "Applied realization instead." );
        }
        else
        {
            m_useAutoRealization.uiCapability()->setUiReadOnly( false );
            m_useAutoRealization.uiCapability()->setUiToolTip( "" );
        }

        uiOrdering.add( &m_useAutoRealization );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimPolygonInViewCollection::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    // No option-driven fields left: m_useAutoRealization is a plain checkbox, and there is
    // deliberately no per-view realization override dropdown (see its declaration comment).
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygonContainer*> RimPolygonInViewCollection::sourceSubCollections() const
{
    if ( auto* src = sourceCollection() ) return src->subCollections();
    return {};
}

//--------------------------------------------------------------------------------------------------
/// Resolves the realization the owning 3D view's own case matches for the current source
/// container (see RimPolygonContainer::resolveViewMatchingRealization), or -1 if there is no view
/// ancestor or the view's case belongs to a different case/ensemble than the source's own data
/// source.
//--------------------------------------------------------------------------------------------------
int RimPolygonInViewCollection::viewMatchingRealizationOrMinusOne() const
{
    auto* src = sourceCollection();
    if ( !src ) return -1;

    auto* view = firstAncestorOfType<Rim3dView>();
    if ( !view ) return -1;

    return src->resolveViewMatchingRealization( view );
}

//--------------------------------------------------------------------------------------------------
/// Just this mirror's own source items -- no realization threading. A RimPolygonCloudAddress's
/// items() are always its base-realization data (fetched lazily, see onSynced()); a comparison
/// realization is a genuine sibling RimPolygonCloudRealizationGroup sub-collection instead, walked
/// via sourceSubCollections() like any other nested container.
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonInViewCollection::sourceItems() const
{
    auto* src = sourceCollection();
    if ( !src ) return {};

    return src->items();
}

//--------------------------------------------------------------------------------------------------
/// Per-sync hook (called once for every mirror node in the tree, after its own name/items/sub-
/// collections have been refreshed):
/// - One-shot default for the Auto-Follow checkbox: if this node's source does not match the
///   owning view's own case (checkbox would be disabled/not meaningful), default it to unchecked,
///   once, so it does not misleadingly stay checked while doing nothing.
/// - Lazy fetch: if this node's source is a RimPolygonCloudAddress and this node is checked
///   visible but its base data has not been fetched yet, fetch it now. Idempotent -- safe on every
///   sync, and this is what makes a saved project's previously-checked leaves "self-heal" back to
///   populated on the next load.
/// - Auto-follow realization-group materialization: if this node's source is a
///   RimPolygonCloudSource, Auto-Follow is checked and resolves to a realization other than the
///   source's own base realization, ensure every checked leaf address beneath this node (in this
///   view) has a RimPolygonCloudRealizationGroup for that realization, and default this view's
///   mirror for that new group to checked (other views default to unchecked, since a fresh group
///   mirror node is unchecked by default).
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::onSynced()
{
    auto* src = sourceCollection();

    if ( !m_didApplyDefaultAutoRealization() )
    {
        m_didApplyDefaultAutoRealization = true;

        if ( src && src->supportsRealizationOverride() && viewMatchingRealizationOrMinusOne() == -1 )
        {
            m_useAutoRealization = false;
        }
    }

    if ( auto* address = dynamic_cast<RimPolygonCloudAddress*>( src ) )
    {
        if ( m_isChecked() && !address->hasBaseData() )
        {
            address->ensureBaseFetched();
        }
    }
    else if ( auto* source = dynamic_cast<RimPolygonCloudSource*>( src ) )
    {
        if ( m_useAutoRealization() )
        {
            const int realization = viewMatchingRealizationOrMinusOne();
            if ( realization != -1 && realization != source->baseRealization() )
            {
                for ( auto* address : allCheckedAddressesRecursively() )
                {
                    if ( address->hasRealizationGroup( realization ) ) continue;

                    auto* group = address->ensureRealizationGroupFetched( realization );

                    // Re-sync so a mirror node for the freshly-added group appears somewhere in
                    // this subtree, then explicitly check it on *this* view only (other views'
                    // own mirror for the same group default to unchecked, see
                    // createSubCollectionInView()).
                    updateFromSource();
                    if ( auto* groupMirror = findMirrorForSource( group ) )
                    {
                        const_cast<RimPolygonInViewCollection*>( groupMirror )->setCheckState( true );
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// New mirror nodes default to checked (see RimCheckableNamedObject) -- correct for ordinary
/// file-based/user-drawn polygon folders, which have nothing to fetch. Two cloud-backed exceptions
/// default to unchecked instead, since checking is the explicit fetch-trigger signal for them and
/// they must never be visualized (and therefore fetched) just because a view happens to sync them
/// for the first time:
/// - RimPolygonCloudRealizationGroup: additionally, unchecked in every view except the one that
///   requested the comparison realization (see onSynced(), which explicitly checks it right after
///   creation in that view only).
/// - RimPolygonCloudAddress: a freshly-materialized leaf mirror must stay unchecked until the user
///   deliberately opts in to visualizing (and thereby fetching) that particular polygon result.
//--------------------------------------------------------------------------------------------------
RimPolygonInViewCollection* RimPolygonInViewCollection::createSubCollectionInView( RimPolygonContainer* src )
{
    auto* sub = new RimPolygonInViewCollection();
    sub->setSourceCollection( src );

    if ( dynamic_cast<RimPolygonCloudRealizationGroup*>( src ) || dynamic_cast<RimPolygonCloudAddress*>( src ) )
    {
        sub->setCheckState( false );
    }

    return sub;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygonCloudAddress*> RimPolygonInViewCollection::allCheckedAddressesRecursively() const
{
    std::vector<RimPolygonCloudAddress*> result;

    if ( auto* address = dynamic_cast<RimPolygonCloudAddress*>( sourceCollection() ) )
    {
        if ( m_isChecked() ) result.push_back( address );
    }

    for ( auto subMirror : m_collectionsInView )
    {
        if ( !subMirror ) continue;
        auto sub = subMirror->allCheckedAddressesRecursively();
        result.insert( result.end(), sub.begin(), sub.end() );
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/// Recursively searches this mirror node (and mirrored sub-collections) for the one whose
/// sourceCollection() is exactly the given container.
//--------------------------------------------------------------------------------------------------
const RimPolygonInViewCollection* RimPolygonInViewCollection::findMirrorForSource( const RimPolygonContainer* source ) const
{
    if ( sourceCollection() == source ) return this;

    for ( auto subMirror : m_collectionsInView )
    {
        if ( !subMirror ) continue;
        if ( auto* found = subMirror->findMirrorForSource( source ) ) return found;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Walks every open view's own RimGridView::polygonInViewCollection() mirror tree, looking for a
/// mirror whose sourceCollection() is the given container and whose checkbox (m_isChecked) is on.
//--------------------------------------------------------------------------------------------------
bool RimPolygonInViewCollection::isSourceCheckedInAnyView( const RimPolygonContainer* source )
{
    auto* project = RimProject::current();
    if ( !project ) return false;

    for ( auto* view : project->allViews() )
    {
        auto* gridView = dynamic_cast<RimGridView*>( view );
        if ( !gridView ) continue;

        auto* rootMirror = gridView->polygonInViewCollection();
        if ( !rootMirror ) continue;

        if ( auto* mirror = rootMirror->findMirrorForSource( source ) )
        {
            if ( mirror->isChecked() ) return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonInView* RimPolygonInViewCollection::createItemInView( RimPolygon* source )
{
    auto* viewItem = new RimPolygonInView();
    viewItem->setPolygon( source );
    return viewItem;
}

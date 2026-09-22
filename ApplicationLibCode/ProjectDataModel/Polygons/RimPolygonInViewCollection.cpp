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

    CAF_PDM_InitField( &m_useAutoRealizationUiState, "UseAutoRealizationUiState", true, "Auto-Follow View Realization" );
    m_useAutoRealizationUiState.xmlCapability()->setIOWritable( false );
    m_useAutoRealizationUiState.xmlCapability()->setIOReadable( false );

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
        if ( m_isChecked() )
        {
            // Checking this leaf on is the fetch-trigger signal. updateFromSource() re-runs
            // prepareForSync(), which fetches (if needed) the data this leaf's effective
            // realization currently resolves to, so the freshly-populated RimCloudPolygon
            // children are mirrored into m_itemsInView this same pass.
            updateFromSource();
        }

        for ( auto poly : visiblePolygonsInView() )
        {
            poly->updateConnectedEditors();
        }

        if ( auto view = firstAncestorOfType<Rim3dView>() )
        {
            view->scheduleCreateDisplayModelAndRedraw();
        }

        // Unchecking this leaf only hides it (visiblePolygonsInView() already excludes anything
        // beneath an unchecked node from 3D rendering) -- it must never evict fetched data, base
        // or comparison-realization alike, exactly like any other polygon container's visibility
        // checkbox. Cached data is only ever discarded via an explicit user action (the "Reload"
        // command) or a project reload.
    }
    else if ( changedField == &m_useAutoRealizationUiState )
    {
        // Only reachable while the checkbox is actually editable (matching case, see
        // defineUiOrdering()), so the shadow's value is always a genuine user edit here -- write
        // it back to the real, persisted field.
        m_useAutoRealization = m_useAutoRealizationUiState();

        updateFromPolygonCollection();

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
    // m_useAutoRealization (and its UI-facing shadow, m_useAutoRealizationUiState) is only
    // meaningful (and only ever added below) on the mirror node representing a whole
    // RimPolygonCloudSource -- every other mirror node (folders, name-folders, address leaves)
    // must not show it at all. Since it is a plain, non-hidden field, skipRemainingFields() is
    // required so caf's generic "auto-append any field not explicitly ordered" behavior does not
    // still surface it on those other nodes.
    if ( auto* src = sourceCollection(); src && src->supportsRealizationOverride() )
    {
        if ( viewMatchingRealizationOrMinusOne() == -1 )
        {
            // Not meaningful here: the checkbox must visually read as unchecked (never "checked
            // but disabled/no-op"), regardless of the real, persisted m_useAutoRealization value --
            // which is left untouched so it takes effect again automatically once the view's case
            // starts matching the source's data source.
            m_useAutoRealizationUiState = false;
            m_useAutoRealizationUiState.uiCapability()->setUiReadOnly( true );
            m_useAutoRealizationUiState.uiCapability()->setUiToolTip(
                "This view's case belongs to a different Sumo case/ensemble than this polygon "
                "address -- realization cannot be followed automatically. Using the address's own "
                "Applied realization instead." );
        }
        else
        {
            m_useAutoRealizationUiState = m_useAutoRealization();
            m_useAutoRealizationUiState.uiCapability()->setUiReadOnly( false );
            m_useAutoRealizationUiState.uiCapability()->setUiToolTip( "" );
        }

        uiOrdering.add( &m_useAutoRealizationUiState );
    }

    uiOrdering.skipRemainingFields();
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
/// This mirror's own effectively-shown source items: for anything except a RimPolygonCloudAddress
/// leaf, just the source's own items(). For a RimPolygonCloudAddress leaf, substitutes the cached
/// data for whichever realization this view effectively resolves to (see effectiveRealization()),
/// directly in place of the address's own base items -- with no separate visible tree node. This
/// is also the "compare with base realization" mechanism: two views auto-following different
/// realizations of the same address each see only their own resolved realization here.
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonInViewCollection::sourceItems() const
{
    auto* src = sourceCollection();
    if ( !src ) return {};

    if ( auto* address = dynamic_cast<RimPolygonCloudAddress*>( src ) )
    {
        const int realization = effectiveRealization();
        const int baseRealization = address->owningSource() ? address->owningSource()->baseRealization() : -1;

        if ( realization == -1 || realization == baseRealization ) return address->items();

        return address->cachedItemsForRealization( realization );
    }

    return src->items();
}

//--------------------------------------------------------------------------------------------------
/// Shows the effective realization in the mirror's own tree name, when it differs from the
/// address's own base/Applied realization (e.g. this view is auto-following a different one).
/// The RimPolygonCloudSource's own top mirror node gets the same treatment -- its name embeds the
/// realization (see RimPolygonCloudSource::composeName()), so an auto-followed view substitutes
/// the effective realization directly in that name rather than appending a second "Real" suffix.
//--------------------------------------------------------------------------------------------------
QString RimPolygonInViewCollection::computeDisplayName() const
{
    auto* src = sourceCollection();
    if ( !src ) return {};

    if ( auto* address = dynamic_cast<RimPolygonCloudAddress*>( src ) )
    {
        const int realization = effectiveRealization();
        const int baseRealization = address->owningSource() ? address->owningSource()->baseRealization() : -1;

        if ( realization != -1 && realization != baseRealization )
        {
            return QString( "%1 (Real %2)" ).arg( address->name() ).arg( realization );
        }
    }
    else if ( auto* source = dynamic_cast<RimPolygonCloudSource*>( src ) )
    {
        const int realization = effectiveRealization();

        if ( realization != -1 && realization != source->baseRealization() )
        {
            return source->nameForRealization( realization );
        }
    }

    return src->collectionName();
}

//--------------------------------------------------------------------------------------------------
/// Lazy-fetch trigger, called before sourceItems() is read this same sync pass: if this node's
/// source is a RimPolygonCloudAddress and this node is checked visible, ensure this view's
/// effective realization has been fetched. Also always ensures the address's own base realization
/// is fetched, even when a different (auto-followed) realization is what this view actually
/// displays -- per UX request, enabling a leaf's checkbox should always populate the address's
/// base data in the project tree, not only whichever realization happens to be shown in this
/// particular view, so the base is available for comparison/other views without a separate
/// action. Idempotent -- safe on every sync, and this is what makes a saved project's
/// previously-checked leaves "self-heal" back to populated on the next load.
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::prepareForSync()
{
    auto* address = dynamic_cast<RimPolygonCloudAddress*>( sourceCollection() );
    if ( !address || !m_isChecked() ) return;

    const int realization     = effectiveRealization();
    const int baseRealization = address->owningSource() ? address->owningSource()->baseRealization() : -1;

    address->ensureBaseFetched();

    if ( realization != -1 && realization != baseRealization )
    {
        address->ensureRealizationFetched( realization );
    }
}

//--------------------------------------------------------------------------------------------------
/// New mirror nodes default to checked (see RimCheckableNamedObject) -- correct for ordinary
/// file-based/user-drawn polygon folders, which have nothing to fetch. A RimPolygonCloudAddress
/// leaf defaults to unchecked instead, since checking is the explicit fetch-trigger signal for it
/// and it must never be visualized (and therefore fetched) just because a view happens to sync it
/// for the first time.
//--------------------------------------------------------------------------------------------------
RimPolygonInViewCollection* RimPolygonInViewCollection::createSubCollectionInView( RimPolygonContainer* src )
{
    auto* sub = new RimPolygonInViewCollection();
    sub->setSourceCollection( src );

    if ( dynamic_cast<RimPolygonCloudAddress*>( src ) )
    {
        sub->setCheckState( false );
    }

    return sub;
}

//--------------------------------------------------------------------------------------------------
/// Walks up this mirror node's own ancestor chain (this node included) for the one whose
/// sourceCollection() is a RimPolygonCloudSource -- the node that owns the single Auto-Follow
/// checkbox governing every leaf beneath it. Returns nullptr if this mirror is not nested under a
/// RimPolygonCloudSource mirror at all.
//--------------------------------------------------------------------------------------------------
const RimPolygonInViewCollection* RimPolygonInViewCollection::sourceMirrorAncestorOrThis() const
{
    const RimPolygonInViewCollection* node = this;
    while ( node )
    {
        if ( dynamic_cast<RimPolygonCloudSource*>( node->sourceCollection() ) ) return node;
        node = node->firstAncestorOfType<RimPolygonInViewCollection>();
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// The realization this mirror node's own items should effectively show: the ancestor
/// RimPolygonCloudSource mirror's Auto-Follow-resolved realization, or -1 (meaning: the address's
/// own base/Applied realization) if Auto-Follow is off, doesn't match, or there is no such
/// ancestor.
//--------------------------------------------------------------------------------------------------
int RimPolygonInViewCollection::effectiveRealization() const
{
    auto* sourceMirror = sourceMirrorAncestorOrThis();
    if ( !sourceMirror || !sourceMirror->m_useAutoRealization() ) return -1;

    return sourceMirror->viewMatchingRealizationOrMinusOne();
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

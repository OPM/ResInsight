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
            // Checking is the fetch trigger: re-run sync so prepareForSync() fetches (if needed)
            // and sourceItems() mirrors the data this same pass.
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

        // Unchecking only hides the node; it never evicts fetched data (that only happens via an
        // explicit Reload or project reload).
    }
    else if ( changedField == &m_useAutoRealizationUiState )
    {
        // Only reachable while editable (matching case), so this is always a genuine user edit.
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
    // m_useAutoRealizationUiState is only meaningful on the mirror node for a whole
    // RimPolygonCloudSource -- skipRemainingFields() below keeps it off every other node.
    if ( auto* src = sourceCollection(); src && src->supportsRealizationOverride() )
    {
        if ( viewMatchingRealizationOrMinusOne() == -1 )
        {
            // Not meaningful here: force the checkbox to read unchecked, but leave the real,
            // persisted m_useAutoRealization untouched so it re-applies once the case matches.
            m_useAutoRealizationUiState = false;
            m_useAutoRealizationUiState.uiCapability()->setUiReadOnly( true );
            m_useAutoRealizationUiState.uiCapability()->setUiToolTip(
                "This view's case belongs to a different Sumo case/ensemble than this polygon "
                "cloud source -- realization cannot be followed automatically. Using the polygon "
                "source's own Applied realization instead." );
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
    // No option-driven fields: m_useAutoRealization is a plain checkbox.
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
/// Realization the owning view's case matches for the current source, or -1 if no match/no view.
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
/// Items shown by this mirror: source's own items(), except a RimPolygonCloudAddress leaf
/// substitutes the cached data for this view's effective realization (see effectiveRealization())
/// -- this is also the base-realization comparison mechanism.
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonInViewCollection::sourceItems() const
{
    auto* src = sourceCollection();
    if ( !src ) return {};

    if ( auto* address = dynamic_cast<RimPolygonCloudAddress*>( src ) )
    {
        const int realization     = effectiveRealization();
        const int baseRealization = address->owningSource() ? address->owningSource()->baseRealization() : -1;

        if ( realization == -1 || realization == baseRealization ) return address->items();

        return address->cachedItemsForRealization( realization );
    }

    return src->items();
}

//--------------------------------------------------------------------------------------------------
/// Shows the effective realization in the tree name when it differs from the base realization
/// (e.g. this view is auto-following a different one).
//--------------------------------------------------------------------------------------------------
QString RimPolygonInViewCollection::computeDisplayName() const
{
    auto* src = sourceCollection();
    if ( !src ) return {};

    if ( auto* address = dynamic_cast<RimPolygonCloudAddress*>( src ) )
    {
        const int realization     = effectiveRealization();
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
/// Lazy-fetch trigger, run before sourceItems() this sync pass: if checked, ensures the address's
/// base realization is fetched (so it's always available in the project tree), plus the view's
/// effective (auto-followed) realization if different. Idempotent.
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
/// New mirror nodes default to checked, except a RimPolygonCloudAddress leaf defaults to
/// unchecked since checking is its explicit fetch trigger.
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
/// Walks up to the ancestor mirror node (this included) whose sourceCollection() is a
/// RimPolygonCloudSource -- owner of the Auto-Follow checkbox. Nullptr if none.
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
/// Realization this node's items should show: the ancestor source's Auto-Follow-resolved
/// realization, or -1 (base/Applied) if off, mismatched, or no such ancestor.
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

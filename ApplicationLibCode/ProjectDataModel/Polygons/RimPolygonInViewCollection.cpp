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
#include "RimPolygon.h"
#include "RimPolygonCollection.h"
#include "RimPolygonInView.h"
#include "RimTools.h"

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
    }
    else if ( changedField == &m_useAutoRealization )
    {
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
/// ancestor or the view's case belongs to a different case/ensemble than the source's Applied data
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
/// Resolves the realization to show in this view: when m_useAutoRealization is checked and the
/// source container can safely match this view's own case (see
/// RimPolygonContainer::resolveViewMatchingRealization -- only true for a cloud-backed address
/// whose Applied data source matches the view's own Sumo case), follow that; otherwise (unchecked,
/// or the view's case does not match) fall back to the source's own Applied/default items (-1) --
/// there is no separate per-view realization override.
//--------------------------------------------------------------------------------------------------
int RimPolygonInViewCollection::effectiveRealization() const
{
    auto* src = sourceCollection();
    if ( !src ) return -1;

    if ( src->supportsRealizationOverride() && m_useAutoRealization() )
    {
        return viewMatchingRealizationOrMinusOne();
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonInViewCollection::sourceItems() const
{
    auto* src = sourceCollection();
    if ( !src ) return {};

    return src->itemsForRealization( effectiveRealization() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPolygonInViewCollection::computeDisplayName() const
{
    auto* src = sourceCollection();
    if ( !src ) return {};

    return src->displayNameForRealization( effectiveRealization() );
}

//--------------------------------------------------------------------------------------------------
/// One-shot default for the Auto-Follow checkbox: the first time this mirror node is synced, if
/// its source's data source does not match the owning view's own case (i.e. the checkbox is
/// disabled/not meaningful here), default it to unchecked so it does not misleadingly stay
/// checked while doing nothing. This does NOT affect the node's own visibility checkbox
/// (m_isChecked) -- only the realization-follow checkbox. Never repeats once applied, so a user's
/// later manual toggle of the checkbox always sticks (even across a project save/reload, since
/// m_didApplyDefaultAutoRealization is persisted).
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::onSynced()
{
    if ( m_didApplyDefaultAutoRealization() ) return;

    m_didApplyDefaultAutoRealization = true;

    auto* src = sourceCollection();
    if ( src && src->supportsRealizationOverride() && viewMatchingRealizationOrMinusOne() == -1 )
    {
        m_useAutoRealization = false;
    }
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

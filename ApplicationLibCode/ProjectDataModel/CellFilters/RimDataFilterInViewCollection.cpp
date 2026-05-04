/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimDataFilterInViewCollection.h"

#include "Rim3dView.h"
#include "RimCellFilter.h"
#include "RimDataFilterCollection.h"
#include "RimDataFilterInView.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimDataFilterInViewCollection, "DataFilterInViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataFilterInViewCollection::RimDataFilterInViewCollection()
{
    CAF_PDM_InitObject( "Data Filters", ":/CellFilter.png" );

    CAF_PDM_InitFieldNoDefault( &m_wrappers, "Wrappers", "Filters" );

    CAF_PDM_InitFieldNoDefault( &m_sourceCollection, "SourceCollection", "Source Collection" );
    m_sourceCollection.uiCapability()->setUiHidden( true );

    setName( "Data Filters" );
    nameField()->uiCapability()->setUiHidden( true );

    setCheckState( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataFilterInViewCollection::~RimDataFilterInViewCollection() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterInViewCollection::setSourceCollection( RimDataFilterCollection* sourceCollection )
{
    if ( m_sourceCollection() == sourceCollection ) return;

    m_sourceCollection = sourceCollection;
    connectSourceSignal();
    syncWithSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataFilterCollection* RimDataFilterInViewCollection::sourceCollection() const
{
    return m_sourceCollection();
}

//--------------------------------------------------------------------------------------------------
/// Reconcile wrappers with the source. Mirrors RimSurfaceInViewCollection::syncSurfacesWithView.
/// Step 1: drop wrappers whose source pointer no longer resolves (source filter was deleted).
/// Step 2: rebuild ordered wrapper list following the source order; reuse existing wrappers
/// (preserves their isChecked state); create new ones for previously-unseen sources.
//--------------------------------------------------------------------------------------------------
void RimDataFilterInViewCollection::syncWithSource()
{
    std::vector<RimDataFilterInView*> existing = m_wrappers.childrenByType();
    for ( auto* w : existing )
    {
        if ( !w->sourceFilter() )
        {
            m_wrappers.removeChild( w );
            delete w;
        }
    }

    if ( !m_sourceCollection() )
    {
        m_wrappers.clearWithoutDelete();
        for ( auto* w : existing ) delete w;
        updateConnectedEditors();
        return;
    }

    std::vector<RimDataFilterInView*> ordered;
    for ( RimCellFilter* src : m_sourceCollection()->filters() )
    {
        if ( !src ) continue;

        RimDataFilterInView* wrapper = findWrapperFor( src );
        if ( !wrapper )
        {
            wrapper = new RimDataFilterInView();
            wrapper->setSourceFilter( src );
        }
        ordered.push_back( wrapper );
    }

    m_wrappers.clearWithoutDelete();
    for ( auto* w : ordered )
    {
        m_wrappers.push_back( w );
    }
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimDataFilterInView*> RimDataFilterInViewCollection::wrappers() const
{
    return m_wrappers.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimDataFilterInView*> RimDataFilterInViewCollection::activeWrappers() const
{
    std::vector<RimDataFilterInView*> active;
    if ( !isChecked() ) return active;

    for ( RimDataFilterInView* w : m_wrappers )
    {
        if ( w && w->isEvaluatable() ) active.push_back( w );
    }
    return active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDataFilterInViewCollection::hasActiveFilters() const
{
    if ( !isChecked() ) return false;
    for ( RimDataFilterInView* w : m_wrappers )
    {
        if ( w && w->isEvaluatable() ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterInViewCollection::initAfterRead()
{
    connectSourceSignal();
    syncWithSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterInViewCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    for ( RimDataFilterInView* w : m_wrappers )
    {
        if ( w ) uiTreeOrdering.add( w );
    }
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterInViewCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == objectToggleField() )
    {
        scheduleViewRegen();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataFilterInView* RimDataFilterInViewCollection::findWrapperFor( RimCellFilter* sourceFilter ) const
{
    if ( !sourceFilter ) return nullptr;
    for ( RimDataFilterInView* w : m_wrappers )
    {
        if ( w && w->sourceFilter() == sourceFilter ) return w;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterInViewCollection::connectSourceSignal()
{
    if ( !m_sourceCollection() ) return;
    m_sourceCollection()->filtersChanged.connect( this, &RimDataFilterInViewCollection::onSourceFiltersChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterInViewCollection::onSourceFiltersChanged( const caf::SignalEmitter* /*emitter*/ )
{
    syncWithSource();
    scheduleViewRegen();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterInViewCollection::scheduleViewRegen()
{
    if ( auto* view = firstAncestorOrThisOfType<Rim3dView>() )
    {
        view->scheduleGeometryRegen( PROPERTY_FILTERED );
        view->scheduleCreateDisplayModelAndRedraw();
    }
}

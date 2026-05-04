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

#include "RimDataFilterInView.h"

#include "Rim3dView.h"
#include "RimCellFilter.h"
#include "RimGridView.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimDataFilterInView, "DataFilterInView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataFilterInView::RimDataFilterInView()
{
    CAF_PDM_InitObject( "Data Filter", ":/CellFilter.png" );

    CAF_PDM_InitFieldNoDefault( &m_sourceFilter, "SourceFilter", "Source Filter" );
    m_sourceFilter.uiCapability()->setUiHidden( true );

    setCheckState( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimDataFilterInView::~RimDataFilterInView() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilter* RimDataFilterInView::sourceFilter() const
{
    return m_sourceFilter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterInView::setSourceFilter( RimCellFilter* sourceFilter )
{
    m_sourceFilter = sourceFilter;
    syncNameFromSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimDataFilterInView::isEvaluatable() const
{
    if ( !isChecked() ) return false;
    if ( !m_sourceFilter() ) return false;
    return m_sourceFilter()->isFilterEnabled();
}

//--------------------------------------------------------------------------------------------------
/// Forward to the wrapped source filter. The unified RimCellFilter::applyToCellVisibility virtual
/// already handles property (uses timeStep), range (legacy bridge), and combined (recurses)
/// uniformly, so no per-type branching is needed here.
//--------------------------------------------------------------------------------------------------
void RimDataFilterInView::applyToCellVisibility( cvf::UByteArray* cellVisibility, const RigGridBase* grid, size_t timeStepIndex )
{
    if ( !isEvaluatable() ) return;
    m_sourceFilter()->applyToCellVisibility( cellVisibility, grid, timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
/// Toggling the wrapper's checkbox affects only the owning view; trigger regen there.
//--------------------------------------------------------------------------------------------------
void RimDataFilterInView::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == objectToggleField() )
    {
        if ( auto* view = firstAncestorOrThisOfType<Rim3dView>() )
        {
            view->scheduleGeometryRegen( PROPERTY_FILTERED );
            view->scheduleCreateDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterInView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( nameField() );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterInView::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterInView::initAfterRead()
{
    syncNameFromSource();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimDataFilterInView::syncNameFromSource()
{
    if ( m_sourceFilter() ) setName( m_sourceFilter()->name() );
}

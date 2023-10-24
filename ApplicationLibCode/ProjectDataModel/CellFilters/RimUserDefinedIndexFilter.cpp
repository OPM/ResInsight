/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "RimUserDefinedIndexFilter.h"

#include "cafPdmUiListEditor.h"

#include "cvfStructGridGeometryGenerator.h"

CAF_PDM_SOURCE_INIT( RimUserDefinedIndexFilter, "RimUserDefinedIndexFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedIndexFilter::RimUserDefinedIndexFilter()
    : RimCellFilter( RimCellFilter::INDEX )
{
    CAF_PDM_InitObject( "User Defined Index Filter", ":/CellFilter_UserDefined.png" );
    CAF_PDM_InitFieldNoDefault( &m_individualCellIndexes, "IndividualCellIndexes", "Cells", "", "Use Ctrl-C for copy and Ctrl-V for paste", "" );
    m_individualCellIndexes.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    m_propagateToSubGrids = true;

    updateIconState();
    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedIndexFilter::~RimUserDefinedIndexFilter()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedIndexFilter::setCellIndexes( std::vector<size_t> cellIndexes )
{
    std::vector<int> cIdxs;

    for ( auto cIdx : cellIndexes )
    {
        cIdxs.push_back( (int)cIdx );
    }
    m_individualCellIndexes.setValue( cIdxs );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimUserDefinedIndexFilter::fullName() const
{
    return QString( "%1  [%2 cells]" ).arg( RimCellFilter::fullName(), QString::number( m_individualCellIndexes().size() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedIndexFilter::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimCellFilter::defineUiOrdering( uiConfigName, uiOrdering );

    auto group = uiOrdering.addNewGroup( QString( "Cell Indexes to " ) + modeString() );
    group->add( &m_individualCellIndexes );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedIndexFilter::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField != &m_name )
    {
        filterChanged.send();
        updateIconState();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedIndexFilter::updateCellIndexFilter( cvf::UByteArray* includeVisibility, cvf::UByteArray* excludeVisibility, int gridIndex )
{
    if ( gridIndex != m_gridIndex() ) return;

    if ( m_filterMode == FilterModeType::INCLUDE )
    {
        for ( auto cellIdx : m_individualCellIndexes() )
        {
            ( *includeVisibility )[cellIdx] = true;
        }
    }
    else
    {
        for ( auto cellIdx : m_individualCellIndexes() )
        {
            ( *excludeVisibility )[cellIdx] = false;
        }
    }
}

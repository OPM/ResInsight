/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimCellIndexFilter.h"

#include "cvfStructGridGeometryGenerator.h"

CAF_PDM_SOURCE_INIT( RimCellIndexFilter, "CellIndexFilter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellIndexFilter::RimCellIndexFilter()
    : RimCellFilter( RimCellFilter::INDEX )
{
    CAF_PDM_InitObject( "Cell Index Filter", ":/CellFilter_UserDefined.png" );

    m_propagateToSubGrids = true;

    updateIconState();
    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellIndexFilter::~RimCellIndexFilter()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCellIndexFilter::fullName() const
{
    int cells = 0;
    for ( const auto& item : m_cells )
    {
        cells += (int)item.size();
    }
    return QString( "%1  [%2 cells]" ).arg( RimCellFilter::fullName(), QString::number( cells ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCellIndexFilter::updateCellIndexFilter( cvf::UByteArray* includeVisibility, cvf::UByteArray* excludeVisibility, int gridIndex )
{
    if ( gridIndex >= static_cast<int>( m_cells.size() ) ) return;

    if ( m_filterMode == FilterModeType::INCLUDE )
    {
        for ( auto cellIdx : m_cells[gridIndex] )
        {
            ( *includeVisibility )[cellIdx] = true;
        }
    }
    else
    {
        for ( auto cellIdx : m_cells[gridIndex] )
        {
            ( *excludeVisibility )[cellIdx] = false;
        }
    }
}

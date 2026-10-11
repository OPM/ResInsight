/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RifOpmCommonFlowDiagGridSource.h"

#include "RiaLogging.h"

#include "opm/io/eclipse/EGrid.hpp"

#include <algorithm>
#include <tuple>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifOpmCommonFlowDiagGridSource::RifOpmCommonFlowDiagGridSource( const std::string& gridFileName )
{
    m_grids.push_back( std::make_unique<Opm::EclIO::EGrid>( gridFileName ) );

    m_lgrNames   = m_grids.front()->list_of_lgrs();
    m_lgrParents = m_grids.front()->list_of_lgr_parents();

    for ( const auto& lgrName : m_lgrNames )
    {
        m_grids.push_back( std::make_unique<Opm::EclIO::EGrid>( gridFileName, lgrName ) );
    }

    if ( m_grids.front()->hasKey( "NNA1" ) )
    {
        RiaLogging::warning( "Flow diagnostics: NNCs between local grids are not supported for opm-common grid import, and are ignored." );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifOpmCommonFlowDiagGridSource::~RifOpmCommonFlowDiagGridSource() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifOpmCommonFlowDiagGridSource::numGrids() const
{
    return static_cast<int>( m_grids.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifOpmCommonFlowDiagGridSource::gridName( int gridID ) const
{
    if ( gridID == 0 ) return "";

    return m_lgrNames[gridID - 1];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<int, 3> RifOpmCommonFlowDiagGridSource::dimensions( int gridID ) const
{
    return m_grids[gridID]->dimension();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RifOpmCommonFlowDiagGridSource::activeToGlobal( int gridID ) const
{
    // EGrid marks matrix-inactive cells with -1, also for dual porosity, matching resdata's matrix active cells
    const auto& activeIndexes = m_grids[gridID]->active_indexes();

    std::vector<int> globalIndexes;
    for ( int globalIndex = 0; globalIndex < static_cast<int>( activeIndexes.size() ); globalIndex++ )
    {
        if ( activeIndexes[globalIndex] >= 0 ) globalIndexes.push_back( globalIndex );
    }

    return globalIndexes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<bool> RifOpmCommonFlowDiagGridSource::activeIsSubdivided( int gridID ) const
{
    const auto dims = dimensions( gridID );

    std::vector<bool> isGlobalCellSubdivided( static_cast<size_t>( dims[0] ) * dims[1] * dims[2], false );
    for ( int lgrIndex = 0; lgrIndex < static_cast<int>( m_lgrNames.size() ); lgrIndex++ )
    {
        if ( parentGridId( lgrIndex ) != gridID ) continue;

        for ( int hostCell : m_grids[lgrIndex + 1]->hostCellsGlobalIndex() )
        {
            if ( hostCell >= 0 && hostCell < static_cast<int>( isGlobalCellSubdivided.size() ) ) isGlobalCellSubdivided[hostCell] = true;
        }
    }

    const auto        globalIndexes = activeToGlobal( gridID );
    std::vector<bool> isSubdivided( globalIndexes.size() );
    for ( size_t activeIndex = 0; activeIndex < globalIndexes.size(); activeIndex++ )
    {
        isSubdivided[activeIndex] = isGlobalCellSubdivided[globalIndexes[activeIndex]];
    }

    return isSubdivided;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Opm::ECLGraphNNC> RifOpmCommonFlowDiagGridSource::nncs() const
{
    std::vector<Opm::ECLGraphNNC> nncs;

    for ( int gridID = 0; gridID < numGrids(); gridID++ )
    {
        // nnc_connections() lists NNC1/NNC2 (TRANNNC) followed by NNCG/NNCL (TRANGL). The input index is the position
        // within each of these arrays, as used by resdata to map NNC flux vectors in restart files.
        int nncIndex       = 0;
        int globalNncIndex = 0;

        for ( const auto& c : m_grids[gridID]->nnc_connections( gridID ) )
        {
            const bool isGlobalLocal = c.grid1_Id != gridID;
            const int  inputIndex    = isGlobalLocal ? globalNncIndex++ : nncIndex++;

            nncs.push_back( { c.grid1_Id, c.grid2_Id, c.grid1_CellIdx - 1, c.grid2_CellIdx - 1, c.transValue, inputIndex } );
        }
    }

    // Same ordering as resdata's ecl_nnc_export()
    std::sort( nncs.begin(),
               nncs.end(),
               []( const Opm::ECLGraphNNC& a, const Opm::ECLGraphNNC& b )
               { return std::tie( a.grid1, a.grid2, a.glob1, a.glob2 ) < std::tie( b.grid1, b.grid2, b.glob1, b.glob2 ); } );

    return nncs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifOpmCommonFlowDiagGridSource::parentGridId( int lgrIndex ) const
{
    if ( lgrIndex >= static_cast<int>( m_lgrParents.size() ) ) return 0;

    auto it = std::find( m_lgrNames.begin(), m_lgrNames.end(), m_lgrParents[lgrIndex] );
    if ( it == m_lgrNames.end() ) return 0;

    return static_cast<int>( std::distance( m_lgrNames.begin(), it ) ) + 1;
}

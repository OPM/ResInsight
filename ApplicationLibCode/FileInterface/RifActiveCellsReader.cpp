////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RifActiveCellsReader.h"

#include "ert/ecl/ecl_file.hpp"
#include "ert/ecl/ecl_grid.hpp"
#include "ert/ecl/ecl_kw_magic.hpp"

#include "cafAssert.h"

#include "RiaStringEncodingTools.h"

#include "RifEclipseOutputFileTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<int>> RifActiveCellsReader::activeCellsFromActnumKeyword( const ecl_file_type* ecl_file )
{
    CAF_ASSERT( ecl_file );

    std::vector<std::vector<int>> activeCellsAllGrids;

    int actnumKeywordCount = ecl_file_get_num_named_kw( ecl_file, ACTNUM_KW );
    for ( size_t gridIdx = 0; gridIdx < static_cast<size_t>( actnumKeywordCount ); gridIdx++ )
    {
        std::vector<int> nativeActnumvValues;
        RifEclipseOutputFileTools::keywordData( ecl_file, ACTNUM_KW, gridIdx, &nativeActnumvValues );

        activeCellsAllGrids.push_back( nativeActnumvValues );
    }

    return activeCellsAllGrids;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<int>>
    RifActiveCellsReader::activeCellsFromPorvKeyword( const ecl_file_type* ecl_file, bool dualPorosity, const int cellCountMainGrid )
{
    CAF_ASSERT( ecl_file );

    std::vector<std::vector<int>> activeCellsAllGrids;

    // Active cell count is always the same size as the number of cells in the grid
    // If we have dual porosity, we have to divide by 2
    //
    // See documentation of active cells in top of ecl_grid.cpp

    bool divideCellCountByTwo = dualPorosity;

    int porvKeywordCount = ecl_file_get_num_named_kw( ecl_file, PORV_KW );
    for ( size_t gridIdx = 0; gridIdx < static_cast<size_t>( porvKeywordCount ); gridIdx++ )
    {
        std::vector<double> porvValues;
        RifEclipseOutputFileTools::keywordData( ecl_file, PORV_KW, gridIdx, &porvValues );

        int activeCellCount = static_cast<int>( porvValues.size() );

        // For some cases, the dual porosity flag is not interpreted correctly. Add a fallback by checking the number of
        // cells in the main grid
        // https://github.com/OPM/ResInsight/issues/9833
        if ( ( gridIdx == 0 ) && ( activeCellCount == cellCountMainGrid * 2 ) ) divideCellCountByTwo = true;

        if ( divideCellCountByTwo )
        {
            activeCellCount /= 2;
        }

        std::vector<int> activeCellsOneGrid;
        activeCellsOneGrid.resize( activeCellCount, 0 );

        for ( int poreValueIndex = 0; poreValueIndex < static_cast<int>( porvValues.size() ); poreValueIndex++ )
        {
            int indexToCell = poreValueIndex;
            if ( indexToCell >= activeCellCount )
            {
                indexToCell = poreValueIndex - activeCellCount;
            }

            if ( porvValues[poreValueIndex] > 0.0 )
            {
                if ( dualPorosity )
                {
                    if ( poreValueIndex < activeCellCount )
                    {
                        activeCellsOneGrid[indexToCell] += CELL_ACTIVE_MATRIX;
                    }
                    else
                    {
                        activeCellsOneGrid[indexToCell] += CELL_ACTIVE_FRACTURE;
                    }
                }
                else
                {
                    activeCellsOneGrid[indexToCell] = CELL_ACTIVE_MATRIX;
                }
            }
        }

        activeCellsAllGrids.push_back( activeCellsOneGrid );
    }

    return activeCellsAllGrids;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifActiveCellsReader::applyActiveCellsToAllGrids( ecl_grid_type* ecl_main_grid, const std::vector<std::vector<int>>& activeCellsForAllGrids )
{
    CAF_ASSERT( ecl_main_grid );

    for ( int gridIndex = 0; gridIndex < static_cast<int>( activeCellsForAllGrids.size() ); gridIndex++ )
    {
        ecl_grid_type* currentGrid = ecl_main_grid;
        if ( gridIndex > 0 )
        {
            currentGrid = ecl_grid_iget_lgr( ecl_main_grid, gridIndex - 1 );
        }

        const auto& activeCellsForGrid = activeCellsForAllGrids[gridIndex];
        CAF_ASSERT( ecl_grid_get_global_size( currentGrid ) == static_cast<int>( activeCellsForGrid.size() ) );

        auto actnum_values = activeCellsForGrid.data();

        ecl_grid_reset_actnum( currentGrid, actnum_values );
    }
}

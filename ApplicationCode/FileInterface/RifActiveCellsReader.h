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

#pragma once

#include <vector>

typedef struct ecl_grid_struct ecl_grid_type;
typedef struct ecl_file_struct ecl_file_type;

//==================================================================================================
//
//
//==================================================================================================
class RifActiveCellsReader
{
public:
    static std::vector<std::vector<int>> activeCellsFromActnumKeyword( const ecl_file_type* ecl_file );

    static std::vector<std::vector<int>> activeCellsFromPorvKeyword( const ecl_file_type* ecl_file, bool dualPorosity );

    static void applyActiveCellsToAllGrids( ecl_grid_type*                       ecl_main_grid,
                                            const std::vector<std::vector<int>>& activeCellsForAllGrids );
};

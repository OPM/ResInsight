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
std::vector<std::vector<int>> RifActiveCellsReader::activeCellsFromActnumKeyword(const ecl_grid_type* mainEclGrid,
                                                                                 const ecl_file_type* ecl_file)
{
    std::vector<std::vector<int>> activeCellsAllGrids;

    int actnumKeywordCount = ecl_file_get_num_named_kw(ecl_file, ACTNUM_KW);
    for (size_t gridIdx = 0; gridIdx < static_cast<size_t>(actnumKeywordCount); gridIdx++)
    {
        std::vector<int> nativeActnumvValues;
        RifEclipseOutputFileTools::keywordData(ecl_file, ACTNUM_KW, gridIdx, &nativeActnumvValues);

        activeCellsAllGrids.push_back(nativeActnumvValues);
    }

    return activeCellsAllGrids;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<int>> RifActiveCellsReader::activeCellsFromPorvKeyword(const ecl_grid_type* mainEclGrid,
                                                                               const ecl_file_type* ecl_file)
{
    CAF_ASSERT(mainEclGrid && ecl_file);

    std::vector<std::vector<int>> activeCellsAllGrids;

    // When PORV is used as criteria, make sure all active cells are assigned both
    // active matrix state and active fracture state. This will make sure that
    // both single porosity models and dual porosity models are initialized with
    // the correct bit mask. See documentation in top of ecl_grid.cpp
    //
    const int combinedActnumValueForMatrixAndFracture = CELL_ACTIVE_MATRIX + CELL_ACTIVE_FRACTURE;

    int porvKeywordCount = ecl_file_get_num_named_kw(ecl_file, PORV_KW);
    for (size_t gridIdx = 0; gridIdx < static_cast<size_t>(porvKeywordCount); gridIdx++)
    {
        std::vector<double> porvValues;
        RifEclipseOutputFileTools::keywordData(ecl_file, PORV_KW, gridIdx, &porvValues);

        std::vector<int> activeCellsOneGrid;
        activeCellsOneGrid.resize(porvValues.size());

        for (size_t i = 0; i < porvValues.size(); i++)
        {
            if (porvValues[i] > 0.0)
            {
                activeCellsOneGrid[i] = combinedActnumValueForMatrixAndFracture;
            }
            else
            {
                activeCellsOneGrid[i] = 0;
            }
        }

        activeCellsAllGrids.push_back(activeCellsOneGrid);
    }

    return activeCellsAllGrids;
}

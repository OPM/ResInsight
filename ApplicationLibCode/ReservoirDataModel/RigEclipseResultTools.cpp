/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025  Equinor ASA
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

#include "RigEclipseResultTools.h"

#include "RiaDefines.h"
#include "RiaPorosityModel.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"

namespace RigEclipseResultTools
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void createResultVector( RimEclipseCase& eclipseCase, const QString& resultName, const std::vector<int>& intValues )
{
    RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::GENERATED, RiaDefines::ResultDataType::INTEGER, resultName );

    auto resultsData = eclipseCase.results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    resultsData->addStaticScalarResult( RiaDefines::ResultCatType::GENERATED, resultName, false, intValues.size() );

    std::vector<double>* resultVector = resultsData->modifiableCellScalarResult( resultAddress, 0 );
    resultVector->resize( intValues.size() );

    for ( size_t idx = 0; idx < intValues.size(); idx++ )
    {
        resultVector->at( idx ) = 1.0 * intValues[idx];
    }

    resultsData->recalculateStatistics( resultAddress );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void generateBorderResult( RimEclipseView* eclipseView )
{
    if ( eclipseView == nullptr ) return;

    if ( auto eCase = eclipseView->firstAncestorOrThisOfType<RimEclipseCase>() )
    {
        auto visibility = eclipseView->currentTotalCellVisibility();

        auto activeReservoirCellIdxs =
            eCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->activeReservoirCellIndices();
        int numActiveCells = (int)activeReservoirCellIdxs.size();

        std::vector<int> result;
        result.resize( numActiveCells, 0 ); // default 0, cells are invisible

        auto grid = eclipseView->mainGrid();

        // go through all cells, only check those visible
#pragma omp parallel for
        for ( int i = 0; i < (int)activeReservoirCellIdxs.size(); i++ )
        {
            auto cellIdx = activeReservoirCellIdxs[i];
            if ( visibility->val( cellIdx ) )
            {
                auto neighbors = grid->neighborCells( cellIdx, true /*ignore invalid k layers*/ );

                int nVisibleNeighbors = 0;
                for ( auto nIdx : neighbors )
                {
                    if ( visibility->val( nIdx ) ) nVisibleNeighbors++;
                }

                if ( nVisibleNeighbors == 6 ) // interior cell
                {
                    result[i] = 2;
                }
                else // border cell
                {
                    result[i] = 1;
                }
            }
        }

        RigEclipseResultTools::createResultVector( *eCase, "BORDER", result );

        eCase->updateConnectedEditors();
    }
}

} // namespace RigEclipseResultTools

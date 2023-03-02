/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023    Equinor ASA
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

#include "RigDepthResultAccessor.h"

#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

// #include <cmath> // Needed for HUGE_VAL on Linux

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigDepthResultAccessor::resultValues( RigEclipseCaseData*         eclipseCaseData,
                                                          RimEclipseResultDefinition* resultDefinition,
                                                          int                         gridIndex,
                                                          int                         cellIndex,
                                                          int                         currentTimeStep )
{
    std::vector<double> values;

    RigHugeValResultAccessor hugeVal;

    if ( cellIndex != cvf::UNDEFINED_SIZE_T )
    {
        size_t i     = 0;
        size_t j     = 0;
        size_t dummy = 0;

        auto kvals = kValues( eclipseCaseData, gridIndex );

        auto grid = eclipseCaseData->grid( gridIndex );

        if ( grid->ijkFromCellIndex( cellIndex, &i, &j, &dummy ) )
        {
            cvf::ref<RigResultAccessor> resultAccessor =
                RigResultAccessorFactory::createFromResultDefinition( eclipseCaseData, gridIndex, currentTimeStep, resultDefinition );

            for ( auto k : kvals )
            {
                int tmpCellIdx = grid->cellIndexFromIJK( i, j, k );

                if ( resultAccessor.notNull() )
                {
                    values.push_back( resultAccessor->cellScalar( tmpCellIdx ) );
                }
                else
                {
                    values.push_back( hugeVal.cellScalar( tmpCellIdx ) );
                }
            }
        }
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RigDepthResultAccessor::kValues( RigEclipseCaseData* eclipseCaseData, int gridIndex )
{
    std::vector<int> kvals;
    int              maxK = eclipseCaseData->grid( gridIndex )->cellCountK();

    for ( int i = 0; i < maxK; i++ )
    {
        kvals.push_back( i );
    }

    return kvals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigDepthResultAccessor::depthValues( RigEclipseCaseData* eclipseCaseData, int startCellIndex, int gridIndex )
{
    std::vector<double> depthVals;

    auto   grid  = eclipseCaseData->grid( gridIndex );
    size_t i     = 0;
    size_t j     = 0;
    size_t dummy = 0;

    if ( grid->ijkFromCellIndex( startCellIndex, &i, &j, &dummy ) )
    {
        int maxK = grid->cellCountK();

        for ( int k = 0; k < maxK; k++ )
        {
            auto cellIdx = grid->cellIndexFromIJK( i, j, k );

            auto& cell = grid->cell( cellIdx );
            if ( cell.isInvalid() )
            {
                depthVals.push_back( std::nan( "" ) );
            }
            else
            {
                auto center = cell.center();
                depthVals.push_back( -1.0 * center.z() );
            }
        }
    }
    return depthVals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigDepthResultAccessor::geometrySelectionText( RigEclipseCaseData* eclipseCaseData, size_t gridIndex, size_t cellIndex )
{
    QString text;

    if ( eclipseCaseData )
    {
        if ( cellIndex != cvf::UNDEFINED_SIZE_T )
        {
            size_t i = 0;
            size_t j = 0;
            size_t k = 0;
            if ( eclipseCaseData->grid( gridIndex )->ijkFromCellIndex( cellIndex, &i, &j, &k ) )
            {
                // Adjust to 1-based Eclipse indexing
                i++;
                j++;

                text += QString( "[%1, %2]" ).arg( i ).arg( j );
            }
        }
    }

    return text;
}

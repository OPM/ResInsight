/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RigGridQualityResultCalculator.h"

#include "RiaResultNames.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

#include <utility>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGridQualityResultCalculator::RigGridQualityResultCalculator( RigCaseCellResultsData& resultsData )
    : RigEclipseResultCalculator( resultsData )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGridQualityResultCalculator::~RigGridQualityResultCalculator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGridQualityResultCalculator::isMatching( const RigEclipseResultAddress& resVarAddr ) const
{
    return RiaResultNames::qcResultNames().contains( resVarAddr.resultName() ) &&
           resVarAddr.resultCatType() == RiaDefines::ResultCatType::QUALITY_CONTROL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGridQualityResultCalculator::calculate( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex )
{
    auto mainGrid = m_resultsData->mainGrid();
    if ( !mainGrid ) return;

    size_t negThickIdx = m_resultsData->findOrCreateScalarResultIndex( resVarAddr, false );

    const std::vector<double>& negThickResults = m_resultsData->m_cellScalarResults[negThickIdx][0];
    size_t                     cellResultCount = m_resultsData->m_activeCellInfo->reservoirActiveCellCount();

    auto activeCellInfo = m_resultsData->activeCellInfo();

    if ( m_resultsData->m_cellScalarResults[negThickIdx].empty() )
    {
        m_resultsData->m_cellScalarResults[negThickIdx].resize( 1 );
    }
    std::vector<double>& cellVolumeResults = m_resultsData->m_cellScalarResults[negThickIdx][0];

    cellVolumeResults.resize( cellResultCount, 0 );

    // loop over all cells in main grid
    for ( int nativeResvCellIndex = 0; nativeResvCellIndex < static_cast<int>( mainGrid->totalCellCount() ); nativeResvCellIndex++ )
    {
        size_t resultIndex = activeCellInfo->cellResultIndex( ReservoirCellIndex( nativeResvCellIndex ) ).value();
        if ( resultIndex != cvf::UNDEFINED_SIZE_T )
        {
            const RigCell& cell       = mainGrid->cell( nativeResvCellIndex );
            int            isNegative = 0;

            auto cornersTop = cell.faceCorners( cvf::StructGridInterface::POS_K );
            auto cornersBot = cell.faceCorners( cvf::StructGridInterface::NEG_K );

            // matching corner mapping, ref StructGridInterface::cellFaceVertexIndices
            static std::vector<std::pair<size_t, size_t>> matchingCorners = { { 0, 0 }, { 1, 3 }, { 2, 2 }, { 3, 1 } };

            for ( auto& [bIdx, tIdx] : matchingCorners )
            {
                if ( cornersTop[tIdx].z() > cornersBot[bIdx].z() )
                {
                    isNegative = 1;
                    break;
                }
            }

            m_resultsData->m_cellScalarResults[negThickIdx][0][resultIndex] = isNegative;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGridQualityResultCalculator::calculateResults( RigEclipseCaseData* data )
{
    if ( !data ) return;
    auto address = RigEclipseResultAddress( RiaDefines::ResultCatType::QUALITY_CONTROL, RiaResultNames::qcNegativeThickness() );

    RigCaseCellResultsData* resultData = data->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    RigGridQualityResultCalculator calc( *resultData );
    calc.calculate( address, 0 );
}

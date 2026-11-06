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
    if ( resVarAddr.resultName() == RiaResultNames::qcNegativeThickness() )
    {
        calculateNegativeThickness( resVarAddr );
    }
    else if ( resVarAddr.resultName() == RiaResultNames::qcConcaveProj() )
    {
        calculateConcaveProjection( resVarAddr );
    }
    else if ( resVarAddr.resultName() == RiaResultNames::qcCollapsed() )
    {
        calculateCollapsed( resVarAddr );
    }
    else if ( resVarAddr.resultName() == RiaResultNames::qcFaulted() )
    {
        calculateFaulted( resVarAddr );
    }
    else if ( RiaResultNames::qcResultNames().contains( resVarAddr.resultName() ) )
    {
        calculateAngles( resVarAddr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGridQualityResultCalculator::calculateResults( RigEclipseCaseData* data )
{
    if ( !data ) return;

    RigCaseCellResultsData* resultData = data->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    RigGridQualityResultCalculator calc( *resultData );

    auto address = RigEclipseResultAddress( RiaDefines::ResultCatType::QUALITY_CONTROL,
                                            RiaDefines::ResultDataType::INTEGER,
                                            RiaResultNames::qcNegativeThickness() );
    calc.calculate( address, 0 );

    address = RigEclipseResultAddress( RiaDefines::ResultCatType::QUALITY_CONTROL,
                                       RiaDefines::ResultDataType::INTEGER,
                                       RiaResultNames::qcConcaveProj() );
    calc.calculate( address, 0 );

    address = RigEclipseResultAddress( RiaDefines::ResultCatType::QUALITY_CONTROL,
                                       RiaDefines::ResultDataType::INTEGER,
                                       RiaResultNames::qcCollapsed() );
    calc.calculate( address, 0 );

    address =
        RigEclipseResultAddress( RiaDefines::ResultCatType::QUALITY_CONTROL, RiaDefines::ResultDataType::INTEGER, RiaResultNames::qcFaulted() );
    calc.calculate( address, 0 );

    address = RigEclipseResultAddress( RiaDefines::ResultCatType::QUALITY_CONTROL,
                                       RiaDefines::ResultDataType::DOUBLE,
                                       RiaResultNames::qcMinangleTopbase() );
    calc.calculate( address, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGridQualityResultCalculator::calculateNegativeThickness( const RigEclipseResultAddress& resVarAddr )
{
    auto mainGrid = m_resultsData->mainGrid();
    if ( !mainGrid ) return;

    size_t outIdx = m_resultsData->findOrCreateScalarResultIndex( resVarAddr, false );

    if ( m_resultsData->m_cellScalarResults[outIdx].empty() )
    {
        m_resultsData->m_cellScalarResults[outIdx].resize( 1 );
    }

    std::vector<double>& outputResults   = m_resultsData->m_cellScalarResults[outIdx][0];
    size_t               cellResultCount = m_resultsData->m_activeCellInfo->reservoirActiveCellCount();

    auto activeCellInfo = m_resultsData->activeCellInfo();

    outputResults.resize( cellResultCount, 0 );

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

            outputResults[resultIndex] = isNegative;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGridQualityResultCalculator::calculateConcaveProjection( const RigEclipseResultAddress& resVarAddr )
{
    auto mainGrid = m_resultsData->mainGrid();
    if ( !mainGrid ) return;

    size_t outIdx = m_resultsData->findOrCreateScalarResultIndex( resVarAddr, false );
    if ( m_resultsData->m_cellScalarResults[outIdx].empty() )
    {
        m_resultsData->m_cellScalarResults[outIdx].resize( 1 );
    }

    std::vector<double>& outputResults   = m_resultsData->m_cellScalarResults[outIdx][0];
    size_t               cellResultCount = m_resultsData->m_activeCellInfo->reservoirActiveCellCount();

    auto activeCellInfo = m_resultsData->activeCellInfo();

    outputResults.resize( cellResultCount, 0 );

    // loop over all cells in main grid
    for ( int nativeResvCellIndex = 0; nativeResvCellIndex < static_cast<int>( mainGrid->totalCellCount() ); nativeResvCellIndex++ )
    {
        size_t resultIndex = activeCellInfo->cellResultIndex( ReservoirCellIndex( nativeResvCellIndex ) ).value();
        if ( resultIndex != cvf::UNDEFINED_SIZE_T )
        {
            const RigCell& cell = mainGrid->cell( nativeResvCellIndex );

            int result = cell.isConcave() ? 1 : 0;

            outputResults[resultIndex] = result;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGridQualityResultCalculator::calculateCollapsed( const RigEclipseResultAddress& resVarAddr )
{
    auto mainGrid = m_resultsData->mainGrid();
    if ( !mainGrid ) return;

    size_t outIdx = m_resultsData->findOrCreateScalarResultIndex( resVarAddr, false );
    if ( m_resultsData->m_cellScalarResults[outIdx].empty() )
    {
        m_resultsData->m_cellScalarResults[outIdx].resize( 1 );
    }

    std::vector<double>& outputResults   = m_resultsData->m_cellScalarResults[outIdx][0];
    size_t               cellResultCount = m_resultsData->m_activeCellInfo->reservoirActiveCellCount();
    auto                 activeCellInfo  = m_resultsData->activeCellInfo();

    outputResults.resize( cellResultCount, 0 );

    // loop over all cells in main grid
    for ( int nativeResvCellIndex = 0; nativeResvCellIndex < static_cast<int>( mainGrid->totalCellCount() ); nativeResvCellIndex++ )
    {
        size_t resultIndex = activeCellInfo->cellResultIndex( ReservoirCellIndex( nativeResvCellIndex ) ).value();
        if ( resultIndex != cvf::UNDEFINED_SIZE_T )
        {
            const RigCell& cell        = mainGrid->cell( nativeResvCellIndex );
            int            result      = cell.isCollapsedVertically() ? 1 : 0;
            outputResults[resultIndex] = result;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGridQualityResultCalculator::calculateFaulted( const RigEclipseResultAddress& resVarAddr )
{
    static const double faultedThreshold = 1e-5; // threshold for z coord difference considering a fault

    // map from cell face corner index to neighbor cell face corner index, ref StructGridInterface::cellFaceVertexIndices
    static const std::vector<size_t> cornerMap = { 0, 3, 2, 1 };

    auto mainGrid = m_resultsData->mainGrid();
    if ( !mainGrid ) return;

    size_t outIdx = m_resultsData->findOrCreateScalarResultIndex( resVarAddr, false );
    if ( m_resultsData->m_cellScalarResults[outIdx].empty() )
    {
        m_resultsData->m_cellScalarResults[outIdx].resize( 1 );
    }

    std::vector<double>& outputResults   = m_resultsData->m_cellScalarResults[outIdx][0];
    size_t               cellResultCount = m_resultsData->m_activeCellInfo->reservoirActiveCellCount();
    auto                 activeCellInfo  = m_resultsData->activeCellInfo();

    outputResults.resize( cellResultCount, 0 );

    // loop over all cells in main grid
    for ( int nativeResvCellIndex = 0; nativeResvCellIndex < static_cast<int>( mainGrid->totalCellCount() ); nativeResvCellIndex++ )
    {
        size_t resultIndex = activeCellInfo->cellResultIndex( ReservoirCellIndex( nativeResvCellIndex ) ).value();
        if ( resultIndex != cvf::UNDEFINED_SIZE_T )
        {
            const RigCell& cell   = mainGrid->cell( nativeResvCellIndex );
            int            result = 0;

            // for every corner point in this cell, check the corresponding neighbour cell corner point and see if the difference in z
            // coordinate is above a certain threshold. If so, this cell is considered to be faulted.

            static std::vector<cvf::StructGridInterface::FaceType> faces = { cvf::StructGridInterface::POS_I,
                                                                             cvf::StructGridInterface::NEG_I,
                                                                             cvf::StructGridInterface::POS_J,
                                                                             cvf::StructGridInterface::NEG_J };

            for ( auto face : faces )
            {
                RigCell neighborCell = cell.neighborCell( face );
                if ( neighborCell.isInvalid() ) continue;

                auto cellCorners     = cell.faceCorners( face );
                auto neighborCorners = neighborCell.faceCorners( cvf::StructGridInterface::oppositeFace( face ) );

                for ( size_t i = 0; i < cellCorners.size(); i++ )
                {
                    // map from cell face corner index to neighbor cell face corner index
                    auto& toIdx = cornerMap[i];

                    double zDiff = std::abs( cellCorners[i].z() - neighborCorners[toIdx].z() );
                    if ( zDiff > faultedThreshold )
                    {
                        result += 1;
                    }
                }
            }

            outputResults[resultIndex] = result;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGridQualityResultCalculator::calculateAngles( const RigEclipseResultAddress& resVarAddr )
{
    auto mainGrid = m_resultsData->mainGrid();
    if ( !mainGrid ) return;

    size_t minangleIdx      = m_resultsData->findOrCreateScalarResultIndex( createAddress( RiaResultNames::qcMinangleTopbase() ), false );
    size_t maxangleIdx      = m_resultsData->findOrCreateScalarResultIndex( createAddress( RiaResultNames::qcMaxangleTopbase() ), false );
    size_t minangleSidesIdx = m_resultsData->findOrCreateScalarResultIndex( createAddress( RiaResultNames::qcMinangleSides() ), false );
    size_t maxangleSidesIdx = m_resultsData->findOrCreateScalarResultIndex( createAddress( RiaResultNames::qcMaxangleSides() ), false );
    size_t minangleProjIdx = m_resultsData->findOrCreateScalarResultIndex( createAddress( RiaResultNames::qcMinangleTopbaseProj() ), false );
    size_t maxangleProjIdx = m_resultsData->findOrCreateScalarResultIndex( createAddress( RiaResultNames::qcMaxangleTopbaseProj() ), false );

    m_resultsData->m_cellScalarResults[minangleIdx].resize( 1 );
    m_resultsData->m_cellScalarResults[maxangleIdx].resize( 1 );
    m_resultsData->m_cellScalarResults[minangleSidesIdx].resize( 1 );
    m_resultsData->m_cellScalarResults[maxangleSidesIdx].resize( 1 );
    m_resultsData->m_cellScalarResults[minangleProjIdx].resize( 1 );
    m_resultsData->m_cellScalarResults[maxangleProjIdx].resize( 1 );

    std::vector<double>& outputMinAngle      = m_resultsData->m_cellScalarResults[minangleIdx][0];
    std::vector<double>& outputMaxAngle      = m_resultsData->m_cellScalarResults[maxangleIdx][0];
    std::vector<double>& outputMinAngleSides = m_resultsData->m_cellScalarResults[minangleSidesIdx][0];
    std::vector<double>& outputMaxAngleSides = m_resultsData->m_cellScalarResults[maxangleSidesIdx][0];
    std::vector<double>& outputMinAngleProj  = m_resultsData->m_cellScalarResults[minangleProjIdx][0];
    std::vector<double>& outputMaxAngleProj  = m_resultsData->m_cellScalarResults[maxangleProjIdx][0];

    size_t cellResultCount = m_resultsData->m_activeCellInfo->reservoirActiveCellCount();
    auto   activeCellInfo  = m_resultsData->activeCellInfo();

    outputMinAngle.resize( cellResultCount, 0 );
    outputMaxAngle.resize( cellResultCount, 0 );
    outputMinAngleSides.resize( cellResultCount, 0 );
    outputMaxAngleSides.resize( cellResultCount, 0 );
    outputMinAngleProj.resize( cellResultCount, 0 );
    outputMaxAngleProj.resize( cellResultCount, 0 );

    // loop over all cells in main grid
    for ( int nativeResvCellIndex = 0; nativeResvCellIndex < static_cast<int>( mainGrid->totalCellCount() ); nativeResvCellIndex++ )
    {
        size_t resultIndex = activeCellInfo->cellResultIndex( ReservoirCellIndex( nativeResvCellIndex ) ).value();
        if ( resultIndex != cvf::UNDEFINED_SIZE_T )
        {
            const RigCell& cell = mainGrid->cell( nativeResvCellIndex );

            auto [minAngle, maxAngle]   = cell.minMaxAnglesTopBottom();
            outputMinAngle[resultIndex] = minAngle;
            outputMaxAngle[resultIndex] = maxAngle;

            auto [minAngleSides, maxAngleSides] = cell.minMaxAnglesSides();
            outputMinAngleSides[resultIndex]    = minAngleSides;
            outputMaxAngleSides[resultIndex]    = maxAngleSides;

            auto [minAngleProj, maxAngleProj] = cell.minMaxAnglesTopBottom( true /*ignore Z*/ );
            outputMinAngleProj[resultIndex]   = minAngleProj;
            outputMaxAngleProj[resultIndex]   = maxAngleProj;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseResultAddress RigGridQualityResultCalculator::createAddress( QString resultName )
{
    return RigEclipseResultAddress( RiaDefines::ResultCatType::QUALITY_CONTROL, RiaDefines::ResultDataType::DOUBLE, resultName );
}

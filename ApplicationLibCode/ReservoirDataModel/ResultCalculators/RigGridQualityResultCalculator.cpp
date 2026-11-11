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
void RigGridQualityResultCalculator::calculate( const RigEclipseResultAddress&, size_t timeStepIndex )
{
    auto mainGrid = m_resultsData->mainGrid();
    if ( !mainGrid ) return;

    size_t minangleIdx = m_resultsData->findOrCreateScalarResultIndex( createDoubleAddress( RiaResultNames::qcMinangleTopbase() ), false );
    size_t maxangleIdx = m_resultsData->findOrCreateScalarResultIndex( createDoubleAddress( RiaResultNames::qcMaxangleTopbase() ), false );
    size_t minangleSidesIdx = m_resultsData->findOrCreateScalarResultIndex( createDoubleAddress( RiaResultNames::qcMinangleSides() ), false );
    size_t maxangleSidesIdx = m_resultsData->findOrCreateScalarResultIndex( createDoubleAddress( RiaResultNames::qcMaxangleSides() ), false );
    size_t minangleProjIdx =
        m_resultsData->findOrCreateScalarResultIndex( createDoubleAddress( RiaResultNames::qcMinangleTopbaseProj() ), false );
    size_t maxangleProjIdx =
        m_resultsData->findOrCreateScalarResultIndex( createDoubleAddress( RiaResultNames::qcMaxangleTopbaseProj() ), false );
    size_t collapsedIdx = m_resultsData->findOrCreateScalarResultIndex( createIntegerAddress( RiaResultNames::qcCollapsed() ), false );
    size_t faultedIdx   = m_resultsData->findOrCreateScalarResultIndex( createIntegerAddress( RiaResultNames::qcFaulted() ), false );
    size_t negThicknessIdx =
        m_resultsData->findOrCreateScalarResultIndex( createIntegerAddress( RiaResultNames::qcNegativeThickness() ), false );
    size_t concaveProjIdx = m_resultsData->findOrCreateScalarResultIndex( createIntegerAddress( RiaResultNames::qcConcaveProj() ), false );

    m_resultsData->m_cellScalarResults[minangleIdx].resize( 1 );
    m_resultsData->m_cellScalarResults[maxangleIdx].resize( 1 );
    m_resultsData->m_cellScalarResults[minangleSidesIdx].resize( 1 );
    m_resultsData->m_cellScalarResults[maxangleSidesIdx].resize( 1 );
    m_resultsData->m_cellScalarResults[minangleProjIdx].resize( 1 );
    m_resultsData->m_cellScalarResults[maxangleProjIdx].resize( 1 );
    m_resultsData->m_cellScalarResults[collapsedIdx].resize( 1 );
    m_resultsData->m_cellScalarResults[faultedIdx].resize( 1 );
    m_resultsData->m_cellScalarResults[negThicknessIdx].resize( 1 );
    m_resultsData->m_cellScalarResults[concaveProjIdx].resize( 1 );

    std::vector<double>& outputMinAngle      = m_resultsData->m_cellScalarResults[minangleIdx][0];
    std::vector<double>& outputMaxAngle      = m_resultsData->m_cellScalarResults[maxangleIdx][0];
    std::vector<double>& outputMinAngleSides = m_resultsData->m_cellScalarResults[minangleSidesIdx][0];
    std::vector<double>& outputMaxAngleSides = m_resultsData->m_cellScalarResults[maxangleSidesIdx][0];
    std::vector<double>& outputMinAngleProj  = m_resultsData->m_cellScalarResults[minangleProjIdx][0];
    std::vector<double>& outputMaxAngleProj  = m_resultsData->m_cellScalarResults[maxangleProjIdx][0];
    std::vector<double>& outputCollapsed     = m_resultsData->m_cellScalarResults[collapsedIdx][0];
    std::vector<double>& outputFaulted       = m_resultsData->m_cellScalarResults[faultedIdx][0];
    std::vector<double>& outputNegThickness  = m_resultsData->m_cellScalarResults[negThicknessIdx][0];
    std::vector<double>& outputConcaveProj   = m_resultsData->m_cellScalarResults[concaveProjIdx][0];

    size_t cellResultCount = m_resultsData->m_activeCellInfo->reservoirActiveCellCount();
    auto   activeCellInfo  = m_resultsData->activeCellInfo();

    outputMinAngle.resize( cellResultCount, 0 );
    outputMaxAngle.resize( cellResultCount, 0 );
    outputMinAngleSides.resize( cellResultCount, 0 );
    outputMaxAngleSides.resize( cellResultCount, 0 );
    outputMinAngleProj.resize( cellResultCount, 0 );
    outputMaxAngleProj.resize( cellResultCount, 0 );
    outputCollapsed.resize( cellResultCount, 0 );
    outputFaulted.resize( cellResultCount, 0 );
    outputNegThickness.resize( cellResultCount, 0 );
    outputConcaveProj.resize( cellResultCount, 0 );

    // loop over all cells in main grid
#pragma omp parallel for
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

            outputCollapsed[resultIndex] = cell.isCollapsedVertically() ? 1 : 0;

            outputFaulted[resultIndex] = faulted( cell );

            outputNegThickness[resultIndex] = negativeThickness( cell );

            outputConcaveProj[resultIndex] = cell.isConcave() ? 1 : 0;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGridQualityResultCalculator::calculateResults( RigCaseCellResultsData* data )
{
    if ( !data ) return;

    RigGridQualityResultCalculator calc( *data );

    // all qc results are calculated together, so asking to calculate one of them is sufficient
    auto address = createDoubleAddress( RiaResultNames::qcNegativeThickness() );
    calc.calculate( address, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseResultAddress RigGridQualityResultCalculator::createDoubleAddress( QString resultName )
{
    return RigEclipseResultAddress( RiaDefines::ResultCatType::QUALITY_CONTROL, RiaDefines::ResultDataType::DOUBLE, resultName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseResultAddress RigGridQualityResultCalculator::createIntegerAddress( QString resultName )
{
    return RigEclipseResultAddress( RiaDefines::ResultCatType::QUALITY_CONTROL, RiaDefines::ResultDataType::INTEGER, resultName );
}

//--------------------------------------------------------------------------------------------------
/// for every corner point in this cell, check the corresponding neighbour cell corner point and see if the difference in z
/// coordinate is above the threshold. If so, this cell corner is considered to be faulted.
/// Result returns the number of faulted corners in the cell (max 24: 4 per face, 6 faces).
//--------------------------------------------------------------------------------------------------
int RigGridQualityResultCalculator::faulted( const RigCell& cell )
{
    static const double faultedThreshold = 1e-5; // threshold for z coord difference considering a fault (same as xtgeo)

    int result = 0;

    // only check side faces
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

        const auto& cornerMap = cvf::StructGridInterface::oppositeFaceVertexIndexMap();
        for ( size_t i = 0; i < cellCorners.size(); i++ )
        {
            // map from cell face corner index to neighbor cell face corner index
            double zDiff = std::abs( cellCorners[i].z() - neighborCorners[cornerMap[i]].z() );
            if ( zDiff > faultedThreshold )
            {
                result += 1;
            }
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigGridQualityResultCalculator::negativeThickness( const RigCell& cell )
{
    int isNegative = 0;

    auto cornersTop    = cell.faceCorners( cvf::StructGridInterface::POS_K );
    auto cornersBottom = cell.faceCorners( cvf::StructGridInterface::NEG_K );

    const auto& cornerMap = cvf::StructGridInterface::oppositeFaceVertexIndexMap();
    for ( size_t i = 0; i < 4; i++ )
    {
        // map from top face corner index to bottom face corner index
        if ( cornersTop[i].z() > cornersBottom[cornerMap[i]].z() )
        {
            isNegative = 1;
            break;
        }
    }

    return isNegative;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigEclipseResultAddress> RigGridQualityResultCalculator::createQcResultAddresses()
{
    std::vector<RigEclipseResultAddress> resultAddresses;

    resultAddresses.push_back( createDoubleAddress( RiaResultNames::qcMinangleTopbase() ) );
    resultAddresses.push_back( createDoubleAddress( RiaResultNames::qcMaxangleTopbase() ) );
    resultAddresses.push_back( createDoubleAddress( RiaResultNames::qcMinangleSides() ) );
    resultAddresses.push_back( createDoubleAddress( RiaResultNames::qcMaxangleSides() ) );
    resultAddresses.push_back( createDoubleAddress( RiaResultNames::qcMinangleTopbaseProj() ) );
    resultAddresses.push_back( createDoubleAddress( RiaResultNames::qcMaxangleTopbaseProj() ) );

    resultAddresses.push_back( createIntegerAddress( RiaResultNames::qcCollapsed() ) );
    resultAddresses.push_back( createIntegerAddress( RiaResultNames::qcFaulted() ) );
    resultAddresses.push_back( createIntegerAddress( RiaResultNames::qcNegativeThickness() ) );
    resultAddresses.push_back( createIntegerAddress( RiaResultNames::qcConcaveProj() ) );

    return resultAddresses;
}

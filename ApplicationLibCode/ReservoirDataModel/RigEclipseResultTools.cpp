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
#include "RiaLogging.h"
#include "RiaPorosityModel.h"

#include "RiaResultNames.h"
#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

#include "RigTypeSafeIndex.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"

namespace RigEclipseResultTools
{
namespace
{
    //--------------------------------------------------------------------------------------------------
    /// Helper function to find maximum value in a result
    //--------------------------------------------------------------------------------------------------
    int findMaxResultValue( RimEclipseCase* eclipseCase, const QString& resultName, const std::vector<RiaDefines::ResultCatType>& categories )
    {
        if ( eclipseCase == nullptr ) return 0;

        auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
        if ( !resultsData ) return 0;

        // Try to find result in the provided categories
        RigEclipseResultAddress resultAddr;
        bool                    hasResult = false;

        for ( const auto& category : categories )
        {
            RigEclipseResultAddress addr( category, RiaDefines::ResultDataType::INTEGER, resultName );
            if ( resultsData->hasResultEntry( addr ) )
            {
                resultAddr = addr;
                hasResult  = true;
                break;
            }
        }

        if ( !hasResult ) return 0;

        resultsData->ensureKnownResultLoaded( resultAddr );
        auto resultValues = resultsData->cellScalarResults( resultAddr, 0 );
        if ( resultValues.empty() ) return 0;

        // Find maximum value
        int maxValue = 0;
        for ( double value : resultValues )
        {
            maxValue = std::max( maxValue, static_cast<int>( value ) );
        }

        return maxValue;
    }
} // namespace

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
void generateBorderResult( RimEclipseCase* eclipseCase, cvf::ref<cvf::UByteArray> customVisibility, const QString& resultName )
{
    if ( eclipseCase == nullptr || customVisibility.isNull() ) return;

    auto activeReservoirCellIdxs =
        eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->activeReservoirCellIndices();
    int numActiveCells = static_cast<int>( activeReservoirCellIdxs.size() );

    size_t reservoirCellCount =
        eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->reservoirCellCount();
    std::vector<int> result( reservoirCellCount, BorderType::INVISIBLE_CELL );

    auto grid = eclipseCase->eclipseCaseData()->mainGrid();

    // go through all cells, only check those visible
#pragma omp parallel for
    for ( int i = 0; i < numActiveCells; i++ )
    {
        auto cellIdx = activeReservoirCellIdxs[i];
        if ( customVisibility->val( cellIdx.value() ) )
        {
            auto neighbors = grid->neighborCells( cellIdx.value(), true /*ignore invalid k layers*/ );

            int nVisibleNeighbors = 0;
            for ( auto nIdx : neighbors )
            {
                if ( customVisibility->val( nIdx ) ) nVisibleNeighbors++;
            }

            if ( nVisibleNeighbors == 6 )
            {
                result[cellIdx.value()] = BorderType::INTERIOR_CELL;
            }
            else
            {
                result[cellIdx.value()] = BorderType::BORDER_CELL;
            }
        }
    }

    RigEclipseResultTools::createResultVector( *eclipseCase, resultName, result );

    eclipseCase->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int generateOperNumResult( RimEclipseCase* eclipseCase, int borderCellValue )
{
    if ( eclipseCase == nullptr ) return 0;

    // Auto-determine border cell value if not specified
    if ( borderCellValue == -1 )
    {
        int maxOperNum = findMaxOperNumValue( eclipseCase );
        RiaLogging::info( QString( "Found max OPERNUM: %1" ).arg( maxOperNum ) );
        borderCellValue = maxOperNum + 1;
    }

    auto activeReservoirCellIdxs =
        eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->activeReservoirCellIndices();

    // Check if OPERNUM already exists - if so, keep existing values.
    RigEclipseResultAddress operNumAddrNative( RiaDefines::ResultCatType::STATIC_NATIVE,
                                               RiaDefines::ResultDataType::INTEGER,
                                               RiaResultNames::opernum() );
    RigEclipseResultAddress operNumAddrGenerated( RiaDefines::ResultCatType::GENERATED,
                                                  RiaDefines::ResultDataType::INTEGER,
                                                  RiaResultNames::opernum() );
    auto                    resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );

    RigEclipseResultAddress operNumAddr;
    bool                    hasExistingOperNum = false;

    if ( resultsData->hasResultEntry( operNumAddrNative ) )
    {
        operNumAddr        = operNumAddrNative;
        hasExistingOperNum = true;
    }
    else if ( resultsData->hasResultEntry( operNumAddrGenerated ) )
    {
        operNumAddr        = operNumAddrGenerated;
        hasExistingOperNum = true;
    }

    std::vector<int> result;

    if ( hasExistingOperNum )
    {
        resultsData->ensureKnownResultLoaded( operNumAddr );
        auto existingValues = resultsData->cellScalarResults( operNumAddr, 0 );
        result.resize( existingValues.size() );

        for ( size_t i = 0; i < existingValues.size(); i++ )
        {
            result[i] = static_cast<int>( existingValues[i] );
        }
    }

    // Check if BORDNUM exists to modify border cells
    RigEclipseResultAddress bordNumAddr( RiaDefines::ResultCatType::GENERATED, RiaDefines::ResultDataType::INTEGER, RiaResultNames::bordnum() );
    if ( resultsData->hasResultEntry( bordNumAddr ) )
    {
        resultsData->ensureKnownResultLoaded( bordNumAddr );
        auto bordNumValues = resultsData->cellScalarResults( bordNumAddr, 0 );
        if ( !bordNumValues.empty() )
        {
            result.resize( bordNumValues.size(), 0 );

            for ( auto activeCellIdx : activeReservoirCellIdxs )
            {
                // If BORDNUM = 1 (BORDER_CELL), assign the border cell value
                if ( static_cast<int>( bordNumValues[activeCellIdx.value()] ) == BorderType::BORDER_CELL )
                {
                    result[activeCellIdx.value()] = borderCellValue;
                }
            }
        }
    }

    RigEclipseResultTools::createResultVector( *eclipseCase, RiaResultNames::opernum(), result );

    eclipseCase->updateConnectedEditors();

    return borderCellValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int findMaxOperNumValue( RimEclipseCase* eclipseCase )
{
    // Try to find OPERNUM in both STATIC_NATIVE (from file) and GENERATED (created by us) categories
    return findMaxResultValue( eclipseCase,
                               RiaResultNames::opernum(),
                               { RiaDefines::ResultCatType::STATIC_NATIVE, RiaDefines::ResultCatType::GENERATED } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int findMaxBcconValue( RimEclipseCase* eclipseCase )
{
    // Look for BCCON in GENERATED category
    return findMaxResultValue( eclipseCase, "BCCON", { RiaDefines::ResultCatType::GENERATED } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void generateBcconResult( RimEclipseCase* eclipseCase, const cvf::Vec3st& min, const cvf::Vec3st& max )
{
    if ( eclipseCase == nullptr ) return;

    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData ) return;

    auto grid = eclipseCase->eclipseCaseData()->mainGrid();
    if ( !grid ) return;

    // Check if BORDNUM result exists
    RigEclipseResultAddress bordNumAddr( RiaDefines::ResultCatType::GENERATED, RiaDefines::ResultDataType::INTEGER, RiaResultNames::bordnum() );
    if ( !resultsData->hasResultEntry( bordNumAddr ) )
    {
        RiaLogging::warning( "BORDNUM result not found - cannot generate BCCON result" );
        return;
    }

    resultsData->ensureKnownResultLoaded( bordNumAddr );
    auto bordNumValues = resultsData->cellScalarResults( bordNumAddr, 0 );
    if ( bordNumValues.empty() ) return;

    auto activeReservoirCellIdxs =
        eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->activeReservoirCellIndices();

    size_t reservoirCellCount =
        eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->reservoirCellCount();
    std::vector<int> result( reservoirCellCount, 0 );

    // Iterate through all active cells
    for ( auto activeCellIdx : activeReservoirCellIdxs )
    {
        // Check if this cell is a border cell
        int borderValue = static_cast<int>( bordNumValues[activeCellIdx.value()] );
        if ( borderValue != BorderType::BORDER_CELL ) continue;

        // Get IJK indices for this cell
        size_t i, j, k;
        if ( !grid->ijkFromCellIndex( activeCellIdx.value(), &i, &j, &k ) ) continue;

        // Determine which face of the box this cell is on
        // Priority: I faces, then J faces, then K faces (for corner/edge cells)
        int bcconValue = 0;

        if ( i == min.x() )
        {
            bcconValue = 1; // I- face
        }
        else if ( i == max.x() )
        {
            bcconValue = 2; // I+ face
        }
        else if ( j == min.y() )
        {
            bcconValue = 3; // J- face
        }
        else if ( j == max.y() )
        {
            bcconValue = 4; // J+ face
        }
        else if ( k == min.z() )
        {
            bcconValue = 5; // K- face
        }
        else if ( k == max.z() )
        {
            bcconValue = 6; // K+ face
        }

        result[activeCellIdx.value()] = bcconValue;
    }

    RigEclipseResultTools::createResultVector( *eclipseCase, "BCCON", result );

    eclipseCase->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<BorderCellFace> generateBorderCellFaces( RimEclipseCase* eclipseCase )
{
    if ( eclipseCase == nullptr ) return {};

    auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !resultsData ) return {};

    auto grid = eclipseCase->eclipseCaseData()->mainGrid();
    if ( !grid ) return {};

    // Check if BORDNUM result exists
    RigEclipseResultAddress bordNumAddr( RiaDefines::ResultCatType::GENERATED, RiaDefines::ResultDataType::INTEGER, RiaResultNames::bordnum() );
    if ( !resultsData->hasResultEntry( bordNumAddr ) ) return {}; // BORDNUM not generated yet

    resultsData->ensureKnownResultLoaded( bordNumAddr );
    auto bordNumValues = resultsData->cellScalarResults( bordNumAddr, 0 );
    if ( bordNumValues.empty() ) return {};

    // Check if BCCON result exists to get boundary condition values
    RigEclipseResultAddress bcconAddr( RiaDefines::ResultCatType::GENERATED, RiaDefines::ResultDataType::INTEGER, "BCCON" );
    if ( !resultsData->hasResultEntry( bcconAddr ) ) return {};

    resultsData->ensureKnownResultLoaded( bcconAddr );
    std::vector<double> bcconValues = resultsData->cellScalarResults( bcconAddr, 0 );

    auto activeReservoirCellIdxs =
        eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->activeReservoirCellIndices();

    std::vector<BorderCellFace> borderCellFaces;

    // Iterate through all active cells
    for ( auto activeCellIdx : activeReservoirCellIdxs )
    {
        // Check if this cell is a border cell
        int borderValue = static_cast<int>( bordNumValues[activeCellIdx.value()] );
        if ( borderValue != BorderType::BORDER_CELL ) continue;

        // Get IJK indices for this cell
        size_t i, j, k;
        if ( !grid->ijkFromCellIndex( activeCellIdx.value(), &i, &j, &k ) ) continue;

        // Check all 6 faces
        std::vector<cvf::StructGridInterface::FaceType> faces = cvf::StructGridInterface::validFaceTypes();

        for ( auto faceType : faces )
        {
            // Get neighbor cell IJK
            size_t ni, nj, nk;
            cvf::StructGridInterface::neighborIJKAtCellFace( i, j, k, faceType, &ni, &nj, &nk );

            // Check if neighbor is within bounds
            if ( ni >= grid->cellCountI() || nj >= grid->cellCountJ() || nk >= grid->cellCountK() ) continue;

            // Get neighbor reservoir cell index
            size_t neighborReservoirIdx = grid->cellIndexFromIJK( ni, nj, nk );

            // Find active cell index for neighbor
            auto it = std::find( activeReservoirCellIdxs.begin(), activeReservoirCellIdxs.end(), ReservoirCellIndex( neighborReservoirIdx ) );
            if ( it == activeReservoirCellIdxs.end() ) continue; // Neighbor not active

            // Check if neighbor is an interior cell
            int neighborBorderValue = static_cast<int>( bordNumValues[neighborReservoirIdx] );
            if ( neighborBorderValue == BorderType::INTERIOR_CELL )
            {
                // Get boundary condition value from BCCON grid property
                int boundaryCondition = static_cast<int>( bcconValues[activeCellIdx.value()] );
                if ( boundaryCondition > 0 )
                {
                    // Add this face to the result
                    borderCellFaces.push_back( { cvf::Vec3st( i, j, k ), faceType, boundaryCondition } );
                }
            }
        }
    }

    return borderCellFaces;
}

} // namespace RigEclipseResultTools

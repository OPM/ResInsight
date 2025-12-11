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
#include "RigGridExportAdapter.h"
#include "RigMainGrid.h"

#include "RigTypeSafeIndex.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"

#include "cafVecIjk.h"

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
/// Generate border result for refined grid
/// Returns a vector sized for the refined grid with BorderType values
/// visibility: vector with 1 for visible cells, 0 for invisible cells
//--------------------------------------------------------------------------------------------------
std::vector<int> generateBorderResult( const RigGridExportAdapter& gridAdapter, const std::vector<int>& visibility )
{
    if ( visibility.empty() ) return {};

    size_t refinedNI  = gridAdapter.cellCountI();
    size_t refinedNJ  = gridAdapter.cellCountJ();
    size_t refinedNK  = gridAdapter.cellCountK();
    size_t totalCells = refinedNI * refinedNJ * refinedNK;

    std::vector<int> result( totalCells, BorderType::INVISIBLE_CELL );

    // Lambda to calculate linear index from IJK coordinates
    auto linearIndex = [refinedNI, refinedNJ]( size_t i, size_t j, size_t k ) { return k * refinedNI * refinedNJ + j * refinedNI + i; };

    // Iterate through all refined cells
#pragma omp parallel for
    for ( int idx = 0; idx < static_cast<int>( totalCells ); ++idx )
    {
        size_t i = idx % refinedNI;
        size_t j = ( idx / refinedNI ) % refinedNJ;
        size_t k = idx / ( refinedNI * refinedNJ );

        size_t linearIdx = linearIndex( i, j, k );

        // Check if this cell is visible
        if ( !visibility[linearIdx] ) continue;

        // Check all 6 neighbors
        int visibleNeighbors = 0;

        // I- neighbor
        if ( i > 0 && visibility[linearIndex( i - 1, j, k )] ) visibleNeighbors++;
        // I+ neighbor
        if ( i < refinedNI - 1 && visibility[linearIndex( i + 1, j, k )] ) visibleNeighbors++;
        // J- neighbor
        if ( j > 0 && visibility[linearIndex( i, j - 1, k )] ) visibleNeighbors++;
        // J+ neighbor
        if ( j < refinedNJ - 1 && visibility[linearIndex( i, j + 1, k )] ) visibleNeighbors++;
        // K- neighbor
        if ( k > 0 && visibility[linearIndex( i, j, k - 1 )] ) visibleNeighbors++;
        // K+ neighbor
        if ( k < refinedNK - 1 && visibility[linearIndex( i, j, k + 1 )] ) visibleNeighbors++;

        if ( visibleNeighbors == 6 )
        {
            result[linearIdx] = BorderType::INTERIOR_CELL;
        }
        else
        {
            result[linearIdx] = BorderType::BORDER_CELL;
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
/// Generate OPERNUM result for grid (supports refinement)
/// Returns a pair: first is a vector sized for the grid with OPERNUM values, second is the new opernumRegion value
/// If existing OPERNUM data is available in the eclipse case, it will be refined to match the grid dimensions
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<int>, int> generateOperNumResult( RimEclipseCase*             eclipseCase,
                                                        const RigGridExportAdapter& gridAdapter,
                                                        const std::vector<int>&     borderResult,
                                                        int                         maxOperNum,
                                                        int                         borderCellValue )
{
    CAF_ASSERT( gridAdapter.totalCells() == borderResult.size() );

    // Auto-determine border cell value if not specified
    if ( borderCellValue == -1 )
    {
        RiaLogging::info( QString( "Found max OPERNUM: %1" ).arg( maxOperNum ) );
        // If no existing OPERNUM found (maxOperNum == 0), use default value of 2
        if ( maxOperNum == 0 )
        {
            borderCellValue = 2;
        }
        else
        {
            borderCellValue = maxOperNum + 1;
        }
    }

    size_t totalCells = gridAdapter.totalCells();

    std::vector<int> result( totalCells, 1 ); // Default OPERNUM value is 1

    // Try to load existing OPERNUM data from eclipse case if available
    if ( eclipseCase != nullptr )
    {
        auto resultsData = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
        if ( resultsData )
        {
            // Try to find OPERNUM in both STATIC_NATIVE and GENERATED categories
            RigEclipseResultAddress resultAddr;
            bool                    hasResult = false;

            std::vector<RiaDefines::ResultCatType> categories = { RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                  RiaDefines::ResultCatType::GENERATED };
            for ( const auto& category : categories )
            {
                RigEclipseResultAddress addr( category, RiaDefines::ResultDataType::INTEGER, RiaResultNames::opernum() );
                if ( resultsData->hasResultEntry( addr ) )
                {
                    resultAddr = addr;
                    hasResult  = true;
                    break;
                }
            }

            if ( hasResult )
            {
                resultsData->ensureKnownResultLoaded( resultAddr );
                auto resultValues = resultsData->cellScalarResults( resultAddr, 0 );

                if ( !resultValues.empty() )
                {
                    RiaLogging::info(
                        QString( "Using existing OPERNUM data (%1 values) and refining to match grid" ).arg( resultValues.size() ) );

                    // Get the main grid to access cells
                    auto mainGrid = eclipseCase->eclipseCaseData()->mainGrid();
                    if ( mainGrid )
                    {
                        cvf::Vec3st originalMin = gridAdapter.originalMin();
                        cvf::Vec3st refinement  = gridAdapter.refinement();

                        // Refine the OPERNUM data to match the refined grid
                        // Each original cell is subdivided into refinement.x * refinement.y * refinement.z subcells
                        size_t refinedNI = gridAdapter.cellCountI();
                        size_t refinedNJ = gridAdapter.cellCountJ();
                        size_t refinedNK = gridAdapter.cellCountK();

                        for ( size_t rk = 0; rk < refinedNK; ++rk )
                        {
                            for ( size_t rj = 0; rj < refinedNJ; ++rj )
                            {
                                for ( size_t ri = 0; ri < refinedNI; ++ri )
                                {
                                    // Calculate which original cell this refined cell belongs to
                                    size_t origI = originalMin.x() + ri / refinement.x();
                                    size_t origJ = originalMin.y() + rj / refinement.y();
                                    size_t origK = originalMin.z() + rk / refinement.z();

                                    // Get the OPERNUM value from the original grid
                                    size_t origCellIdx = mainGrid->cellIndexFromIJK( origI, origJ, origK );

                                    if ( origCellIdx < resultValues.size() )
                                    {
                                        size_t refinedIdx  = rk * refinedNI * refinedNJ + rj * refinedNI + ri;
                                        result[refinedIdx] = static_cast<int>( resultValues[origCellIdx] );
                                    }
                                }
                            }
                        }

                        RiaLogging::info( QString( "Refined OPERNUM data from %1x%2x%3 to %4x%5x%6" )
                                              .arg( gridAdapter.originalMax().x() - gridAdapter.originalMin().x() + 1 )
                                              .arg( gridAdapter.originalMax().y() - gridAdapter.originalMin().y() + 1 )
                                              .arg( gridAdapter.originalMax().z() - gridAdapter.originalMin().z() + 1 )
                                              .arg( refinedNI )
                                              .arg( refinedNJ )
                                              .arg( refinedNK ) );
                    }
                }
            }
        }
    }

    // Overwrite border cells with the border cell value
    for ( size_t i = 0; i < totalCells; ++i )
    {
        if ( borderResult[i] == BorderType::BORDER_CELL )
        {
            result[i] = borderCellValue;
        }
    }

    return { result, borderCellValue };
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
std::vector<int>
    generateBcconResult( RimEclipseCase* eclipseCase, const std::vector<int>& borderResult, const caf::VecIjk0& min, const caf::VecIjk0& max )
{
    if ( eclipseCase == nullptr ) return {};

    auto grid = eclipseCase->eclipseCaseData()->mainGrid();
    if ( !grid ) return {};

    if ( borderResult.empty() )
    {
        RiaLogging::warning( "Border result is empty - cannot generate BCCON result" );
        return {};
    }

    auto activeReservoirCellIdxs =
        eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->activeReservoirCellIndices();

    size_t reservoirCellCount =
        eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->reservoirCellCount();
    std::vector<int> result( reservoirCellCount, 0 );

    // Iterate through all active cells
    for ( auto activeCellIdx : activeReservoirCellIdxs )
    {
        // Check if this cell is a border cell
        int borderValue = borderResult[activeCellIdx.value()];
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

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<BorderCellFace>
    generateBorderCellFaces( RimEclipseCase* eclipseCase, const std::vector<int>& borderResult, const std::vector<int>& bcconResult )
{
    if ( eclipseCase == nullptr ) return {};

    auto grid = eclipseCase->eclipseCaseData()->mainGrid();
    if ( !grid ) return {};

    if ( borderResult.empty() || bcconResult.empty() ) return {};

    auto activeReservoirCellIdxs =
        eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->activeReservoirCellIndices();

    std::vector<BorderCellFace> borderCellFaces;

    // Iterate through all active cells
    for ( auto activeCellIdx : activeReservoirCellIdxs )
    {
        // Check if this cell is a border cell
        int borderValue = borderResult[activeCellIdx.value()];
        if ( borderValue != BorderType::BORDER_CELL ) continue;

        // Get IJK indices for this cell
        if ( auto ijk = grid->ijkFromCellIndex( activeCellIdx.value() ) )
        {
            // Check all 6 faces
            std::vector<cvf::StructGridInterface::FaceType> faces = cvf::StructGridInterface::validFaceTypes();

            for ( auto faceType : faces )
            {
                // Get neighbor cell IJK
                size_t ni, nj, nk;
                cvf::StructGridInterface::neighborIJKAtCellFace( ijk->i(), ijk->j(), ijk->k(), faceType, &ni, &nj, &nk );

                // Check if neighbor is within bounds
                if ( ni >= grid->cellCountI() || nj >= grid->cellCountJ() || nk >= grid->cellCountK() ) continue;

                // Get neighbor reservoir cell index
                size_t neighborReservoirIdx = grid->cellIndexFromIJK( ni, nj, nk );

                // Find active cell index for neighbor
                auto it =
                    std::find( activeReservoirCellIdxs.begin(), activeReservoirCellIdxs.end(), ReservoirCellIndex( neighborReservoirIdx ) );
                if ( it == activeReservoirCellIdxs.end() ) continue; // Neighbor not active

                // Check if neighbor is an interior cell
                int neighborBorderValue = borderResult[neighborReservoirIdx];
                if ( neighborBorderValue == BorderType::INTERIOR_CELL )
                {
                    // Get boundary condition value from BCCON grid property
                    int boundaryCondition = bcconResult[activeCellIdx.value()];
                    if ( boundaryCondition > 0 )
                    {
                        // Add this face to the result
                        borderCellFaces.push_back( { *ijk, faceType, boundaryCondition } );
                    }
                }
            }
        }
    }

    return borderCellFaces;
}

//--------------------------------------------------------------------------------------------------
/// Generate BCCON result for refined grid
/// Returns a vector sized for the refined grid with BCCON values (1-6 for faces, 0 for non-border)
//--------------------------------------------------------------------------------------------------
std::vector<int> generateBcconResult( const RigGridExportAdapter& gridAdapter, const std::vector<int>& borderResult )
{
    CAF_ASSERT( gridAdapter.totalCells() == borderResult.size() );

    size_t refinedNI  = gridAdapter.cellCountI();
    size_t refinedNJ  = gridAdapter.cellCountJ();
    size_t refinedNK  = gridAdapter.cellCountK();
    size_t totalCells = refinedNI * refinedNJ * refinedNK;

    std::vector<int> result( totalCells, 0 );

    // Iterate through all refined cells
    for ( size_t k = 0; k < refinedNK; ++k )
    {
        for ( size_t j = 0; j < refinedNJ; ++j )
        {
            for ( size_t i = 0; i < refinedNI; ++i )
            {
                size_t linearIdx = k * refinedNI * refinedNJ + j * refinedNI + i;

                // Check if this cell is a border cell
                if ( borderResult[linearIdx] != BorderType::BORDER_CELL ) continue;

                // Determine which face of the box this cell is on
                // Priority: I faces, then J faces, then K faces (for corner/edge cells)
                int bcconValue = 0;

                if ( i == 0 )
                {
                    bcconValue = 1; // I- face
                }
                else if ( i == refinedNI - 1 )
                {
                    bcconValue = 2; // I+ face
                }
                else if ( j == 0 )
                {
                    bcconValue = 3; // J- face
                }
                else if ( j == refinedNJ - 1 )
                {
                    bcconValue = 4; // J+ face
                }
                else if ( k == 0 )
                {
                    bcconValue = 5; // K- face
                }
                else if ( k == refinedNK - 1 )
                {
                    bcconValue = 6; // K+ face
                }

                result[linearIdx] = bcconValue;
            }
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/// Generate border cell faces for refined grid
/// Returns border cells that have at least one face connecting to an interior cell
//--------------------------------------------------------------------------------------------------
std::vector<BorderCellFace> generateBorderCellFaces( const RigGridExportAdapter& gridAdapter,
                                                     const std::vector<int>&     borderResult,
                                                     const std::vector<int>&     bcconResult )
{
    CAF_ASSERT( borderResult.size() == gridAdapter.totalCells() );
    CAF_ASSERT( bcconResult.size() == gridAdapter.totalCells() );

    size_t refinedNI = gridAdapter.cellCountI();
    size_t refinedNJ = gridAdapter.cellCountJ();
    size_t refinedNK = gridAdapter.cellCountK();

    std::vector<BorderCellFace> borderCellFaces;

    // Lambda to calculate linear index from IJK coordinates
    auto linearIndex = [refinedNI, refinedNJ]( size_t i, size_t j, size_t k ) { return k * refinedNI * refinedNJ + j * refinedNI + i; };

    // Iterate through all refined cells
    for ( size_t k = 0; k < refinedNK; ++k )
    {
        for ( size_t j = 0; j < refinedNJ; ++j )
        {
            for ( size_t i = 0; i < refinedNI; ++i )
            {
                size_t linearIdx = linearIndex( i, j, k );

                // Check if this cell is a border cell
                if ( borderResult[linearIdx] != BorderType::BORDER_CELL ) continue;

                // Get boundary condition value
                int boundaryCondition = bcconResult[linearIdx];
                if ( boundaryCondition == 0 ) continue;

                // Check all 6 faces to find which face(s) connect to interior cells
                std::vector<std::pair<cvf::StructGridInterface::FaceType, size_t>> facesToCheck =
                    { { cvf::StructGridInterface::NEG_I, i > 0 ? linearIndex( i - 1, j, k ) : SIZE_MAX },
                      { cvf::StructGridInterface::POS_I, i < refinedNI - 1 ? linearIndex( i + 1, j, k ) : SIZE_MAX },
                      { cvf::StructGridInterface::NEG_J, j > 0 ? linearIndex( i, j - 1, k ) : SIZE_MAX },
                      { cvf::StructGridInterface::POS_J, j < refinedNJ - 1 ? linearIndex( i, j + 1, k ) : SIZE_MAX },
                      { cvf::StructGridInterface::NEG_K, k > 0 ? linearIndex( i, j, k - 1 ) : SIZE_MAX },
                      { cvf::StructGridInterface::POS_K, k < refinedNK - 1 ? linearIndex( i, j, k + 1 ) : SIZE_MAX } };

                for ( const auto& [faceType, neighborIdx] : facesToCheck )
                {
                    if ( neighborIdx == SIZE_MAX ) continue;

                    // Check if neighbor is an interior cell
                    if ( borderResult[neighborIdx] == BorderType::INTERIOR_CELL )
                    {
                        caf::VecIjk0 ijk( i, j, k );
                        borderCellFaces.push_back( { ijk, faceType, boundaryCondition } );
                    }
                }
            }
        }
    }

    return borderCellFaces;
}

} // namespace RigEclipseResultTools

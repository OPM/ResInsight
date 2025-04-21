/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RimCornerPointCase.h"

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RiaLogging.h"

#include "RifInputPropertyLoader.h"
#include "RifRoffFileTools.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RimEclipseInputProperty.h"
#include "RimReservoirCellResultsStorage.h"

#include "cafPdmObjectScriptingCapability.h"
#include "cafProgressInfo.h"

#include <QDir>
#include <QFileInfo>

#include <chrono>

#ifdef USE_OPENMP
#include <omp.h>
#endif

using namespace std::chrono;

CAF_PDM_SOURCE_INIT( RimCornerPointCase, "RimCornerPointCase" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCornerPointCase::RimCornerPointCase()
    : RimEclipseCase()
{
    CAF_PDM_InitScriptableObject( "RimCornerPointCase", ":/EclipseInput48x48.png" );
    setReservoirData( new RigEclipseCaseData( this ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCornerPointCase::~RimCornerPointCase()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCornerPointCase::openEclipseGridFile()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<RimCornerPointCase*, QString> RimCornerPointCase::createFromCoordinatesArray( int                       nx,
                                                                                        int                       ny,
                                                                                        int                       nz,
                                                                                        const std::vector<float>& coordsv,
                                                                                        const std::vector<float>& zcornsv,
                                                                                        const std::vector<float>& actnumsv )
{
    CAF_ASSERT( nx > 0 );
    CAF_ASSERT( ny > 0 );
    CAF_ASSERT( nz > 0 );

    size_t ncoord = ( nx + 1 ) * ( ny + 1 ) * 2 * 3;
    size_t nzcorn = ( nx + 1 ) * ( ny + 1 ) * ( nz + 1 ) * 4;
    size_t ntot   = nx * ny * nz;

    if ( coordsv.size() != ncoord )
        return { nullptr, QString( "Wrong size of coords array. Expected %1, but got %2" ).arg( ncoord ).arg( coordsv.size() ) };

    if ( zcornsv.size() != nzcorn )
        return { nullptr, QString( "Wrong size of z corner array. Expected %1, but got %2" ).arg( nzcorn ).arg( zcornsv.size() ) };

    if ( actnumsv.size() != ntot )
        return { nullptr, QString( "Wrong size of actnum array. Expected %1, but got %2" ).arg( ntot ).arg( actnumsv.size() ) };

    auto cornerPointCase = new RimCornerPointCase;

    buildGrid( cornerPointCase->eclipseCaseData(), nx, ny, nz, coordsv, zcornsv, actnumsv );

    return { cornerPointCase, "" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCornerPointCase::buildGrid( RigEclipseCaseData*       eclipseCase,
                                    const int                 nx,
                                    const int                 ny,
                                    const int                 nz,
                                    const std::vector<float>& cornerLines,
                                    const std::vector<float>& zCorners,
                                    const std::vector<float>& active )
{
    auto startTime = high_resolution_clock::now();

    RigActiveCellInfo* activeCellInfo = eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    CVF_ASSERT( activeCellInfo );

    RigActiveCellInfo* fractureActiveCellInfo = eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL );
    CVF_ASSERT( fractureActiveCellInfo );

    RigMainGrid* mainGrid = eclipseCase->mainGrid();
    CVF_ASSERT( mainGrid );

    cvf::Vec3st gridPointDim( nx + 1, ny + 1, nz + 1 );
    mainGrid->setGridPointDimensions( gridPointDim );
    mainGrid->setGridName( "Main grid" );

    size_t totalCellCount = nx * ny * nz;

    activeCellInfo->setGridCount( 1 );
    fractureActiveCellInfo->setGridCount( 1 );

    activeCellInfo->setReservoirCellCount( totalCellCount );
    fractureActiveCellInfo->setReservoirCellCount( totalCellCount );

    // Reserve room for the cells and nodes and fill them with data
    mainGrid->reservoirCells().reserve( totalCellCount );
    mainGrid->nodes().reserve( 8 * totalCellCount );

    int               progTicks = 100;
    caf::ProgressInfo progInfo( progTicks, "" );

    int    cellCount      = static_cast<int>( totalCellCount );
    size_t cellStartIndex = mainGrid->reservoirCells().size();
    size_t nodeStartIndex = mainGrid->nodes().size();

    RigCell defaultCell;
    defaultCell.setHostGrid( mainGrid );
    mainGrid->reservoirCells().resize( cellStartIndex + cellCount, defaultCell );

    mainGrid->nodes().resize( nodeStartIndex + static_cast<size_t>( cellCount ) * 8, cvf::Vec3d( 0, 0, 0 ) );

    // Swap i and j to get correct faces
    const size_t cellMappingECLRi[8] = { 2, 3, 1, 0, 6, 7, 5, 4 };

    cvf::Vec3d offset( 0.0, 0.0, 0.0 );
    cvf::Vec3d scale( 1.0, 1.0, 1.0 );

    // TODO: use incoming active cells info
    std::vector<int> activeCells( nx * ny * nz, 1 );
    // convertToReservoirIndexOrder<char, int>( nx, ny, nz, active, activeCells );

    // Precompute the active cell matrix index
    size_t numActiveCells = RifRoffFileTools::computeActiveCellMatrixIndex( activeCells );

    // Loop over cells and fill them with data
#pragma omp for
    for ( int gridLocalCellIndex = 0; gridLocalCellIndex < cellCount; ++gridLocalCellIndex )
    {
        RigCell& cell = mainGrid->cell( cellStartIndex + gridLocalCellIndex );

        cell.setGridLocalCellIndex( gridLocalCellIndex );

        // Active cell index
        int matrixActiveIndex = activeCells[gridLocalCellIndex];
        if ( matrixActiveIndex != -1 )
        {
            activeCellInfo->setCellResultIndex( cellStartIndex + gridLocalCellIndex, matrixActiveIndex );
        }

        cell.setParentCellIndex( cvf::UNDEFINED_SIZE_T );

        // Corner coordinates
        for ( int cIdx = 0; cIdx < 8; ++cIdx )
        {
            double* point  = mainGrid->nodes()[nodeStartIndex + (size_t)gridLocalCellIndex * 8 + cellMappingECLRi[cIdx]].ptr();
            auto    corner = RifRoffFileTools::getCorner( *mainGrid, cornerLines, zCorners, gridLocalCellIndex, cIdx, offset, scale );

            point[0] = corner.x();
            point[1] = corner.y();
            point[2] = corner.z();

            cell.cornerIndices()[cIdx] = nodeStartIndex + (size_t)gridLocalCellIndex * 8 + cIdx;
        }

        // Mark inactive long pyramid looking cells as invalid
        cell.setInvalid( cell.isLongPyramidCell() );
    }

    activeCellInfo->setGridActiveCellCounts( 0, numActiveCells );
    fractureActiveCellInfo->setGridActiveCellCounts( 0, 0 );

    mainGrid->initAllSubGridsParentGridPointer();
    activeCellInfo->computeDerivedData();
    fractureActiveCellInfo->computeDerivedData();

    auto endTime = high_resolution_clock::now();

    auto totalDuration = duration_cast<milliseconds>( endTime - startTime );
    RiaLogging::info( QString( "Total: %1 ms" ).arg( totalDuration.count() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCornerPointCase::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_caseUserDescription );
    uiOrdering.add( &m_displayNameOption );
    uiOrdering.add( &m_caseId );
    uiOrdering.add( &m_caseFileName );

    auto group = uiOrdering.addNewGroup( "Case Options" );
    group->add( &m_activeFormationNames );
    group->add( &m_flipXAxis );
    group->add( &m_flipYAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCornerPointCase::locationOnDisc() const
{
    if ( gridFileName().isEmpty() ) return QString();

    QFileInfo fi( gridFileName() );
    return fi.absolutePath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCornerPointCase::importAsciiInputProperties( const QStringList& fileNames )
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "gtest/gtest.h"

#include "RiaResultNames.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigFault.h"
#include "RigMainGrid.h"
#include "RigReservoirBuilder.h"

namespace
{
//--------------------------------------------------------------------------------------------------
/// Build a 1 x 1 x cellCount row of cells along I (no file, no view), and shift every other cell
/// down in Z so that every I interface becomes a geometric (unnamed) fault.
//--------------------------------------------------------------------------------------------------
cvf::ref<RigEclipseCaseData> buildStaggeredRowGridCase( int cellCount )
{
    RigReservoirBuilder builder;
    builder.setIJKCount( cvf::Vec3st( cellCount, 1, 1 ) );
    builder.setWorldCoordinates( cvf::Vec3d( 0.0, 0.0, 0.0 ), cvf::Vec3d( cellCount, 1.0, -1.0 ) );

    cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData( nullptr );
    builder.createGridsAndCells( eclipseCase.p() );
    eclipseCase->mainGrid()->computeCachedData();

    std::vector<cvf::Vec3d>& nodes = eclipseCase->mainGrid()->nodes();
    for ( int i = 1; i < cellCount; i += 2 )
    {
        for ( int corner = 0; corner < 8; corner++ )
        {
            nodes[static_cast<size_t>( i ) * 8 + corner].z() -= 10.0;
        }
    }

    return eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void findUnnamedFaults( RigMainGrid* mainGrid, const RigFault*& unnamedFault, const RigFault*& unnamedFaultInactive )
{
    unnamedFault         = nullptr;
    unnamedFaultInactive = nullptr;
    for ( const auto& fault : mainGrid->faults() )
    {
        if ( fault->name() == RiaResultNames::undefinedGridFaultName() ) unnamedFault = fault.p();
        if ( fault->name() == RiaResultNames::undefinedGridFaultWithInactiveName() ) unnamedFaultInactive = fault.p();
    }
}

} // namespace

//--------------------------------------------------------------------------------------------------
/// Every I interface in the staggered grid is a geometric fault. The unnamed fault faces are computed
/// in parallel, one cell range per thread, and the per-thread results are appended in thread order.
/// Verify that all interfaces are found, and that the resulting faces stay ordered by cell index.
//--------------------------------------------------------------------------------------------------
TEST( RigMainGridFaults, UnnamedFaultFacesAreFoundInCellOrder )
{
    const int cellCount = 10;

    cvf::ref<RigEclipseCaseData> eclipseCase = buildStaggeredRowGridCase( cellCount );
    RigMainGrid*                 mainGrid    = eclipseCase->mainGrid();

    RigActiveCellInfo* activeCellInfo = eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    mainGrid->calculateFaults( activeCellInfo );

    const RigFault* unnamedFault         = nullptr;
    const RigFault* unnamedFaultInactive = nullptr;
    findUnnamedFaults( mainGrid, unnamedFault, unnamedFaultInactive );

    ASSERT_NE( unnamedFault, nullptr );
    ASSERT_NE( unnamedFaultInactive, nullptr );
    EXPECT_TRUE( unnamedFaultInactive->faultFaces().empty() );

    const std::vector<RigFault::FaultFace>& faces = unnamedFault->faultFaces();
    ASSERT_EQ( static_cast<size_t>( cellCount - 1 ), faces.size() );

    for ( size_t idx = 0; idx < faces.size(); idx++ )
    {
        EXPECT_EQ( idx, faces[idx].m_nativeReservoirCellIndex ) << "Face " << idx << " is out of cell-index order";
        EXPECT_EQ( idx + 1, faces[idx].m_oppositeReservoirCellIndex );
        EXPECT_EQ( cvf::StructGridInterface::POS_I, faces[idx].m_nativeFace );
    }
}

//--------------------------------------------------------------------------------------------------
/// Fault faces with an inactive native or neighbor cell are reported in a separate fault face list.
//--------------------------------------------------------------------------------------------------
TEST( RigMainGridFaults, UnnamedFaultFacesWithInactiveCellGoToSeparateList )
{
    const int cellCount = 4;

    cvf::ref<RigEclipseCaseData> eclipseCase = buildStaggeredRowGridCase( cellCount );
    RigMainGrid*                 mainGrid    = eclipseCase->mainGrid();

    // Build an independent active cell info with cell 2 left inactive (its entry is never assigned, so it stays
    // at the default "undefined" value). The faces (1,2) and (2,3) should then be reported as inactive fault faces.
    cvf::ref<RigActiveCellInfo> activeCellInfo = new RigActiveCellInfo;
    activeCellInfo->setReservoirCellCount( static_cast<size_t>( cellCount ) );
    size_t activeIdx = 0;
    for ( int i = 0; i < cellCount; i++ )
    {
        if ( i == 2 ) continue;
        activeCellInfo->setCellResultIndex( ReservoirCellIndex( static_cast<size_t>( i ) ), ActiveCellIndex( activeIdx++ ) );
    }

    mainGrid->calculateFaults( activeCellInfo.p() );

    const RigFault* unnamedFault         = nullptr;
    const RigFault* unnamedFaultInactive = nullptr;
    findUnnamedFaults( mainGrid, unnamedFault, unnamedFaultInactive );

    ASSERT_NE( unnamedFault, nullptr );
    ASSERT_NE( unnamedFaultInactive, nullptr );

    ASSERT_EQ( 1u, unnamedFault->faultFaces().size() );
    EXPECT_EQ( 0u, unnamedFault->faultFaces()[0].m_nativeReservoirCellIndex );
    EXPECT_EQ( 1u, unnamedFault->faultFaces()[0].m_oppositeReservoirCellIndex );

    ASSERT_EQ( 2u, unnamedFaultInactive->faultFaces().size() );
    EXPECT_EQ( 1u, unnamedFaultInactive->faultFaces()[0].m_nativeReservoirCellIndex );
    EXPECT_EQ( 2u, unnamedFaultInactive->faultFaces()[0].m_oppositeReservoirCellIndex );
    EXPECT_EQ( 2u, unnamedFaultInactive->faultFaces()[1].m_nativeReservoirCellIndex );
    EXPECT_EQ( 3u, unnamedFaultInactive->faultFaces()[1].m_oppositeReservoirCellIndex );
}

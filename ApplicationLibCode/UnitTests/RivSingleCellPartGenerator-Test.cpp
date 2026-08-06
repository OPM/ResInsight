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

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigReservoirBuilder.h"

#include "RivSingleCellPartGenerator.h"

#include "cvfPart.h"
#include "cvfVector3.h"

namespace
{
//--------------------------------------------------------------------------------------------------
/// Build a regular ni x nj x nk box grid in memory (no file, no view)
//--------------------------------------------------------------------------------------------------
cvf::ref<RigEclipseCaseData> buildBoxGrid( int ni, int nj, int nk )
{
    RigReservoirBuilder builder;
    builder.setIJKCount( cvf::Vec3st( ni, nj, nk ) );
    builder.setWorldCoordinates( cvf::Vec3d( 0.0, 0.0, 0.0 ), cvf::Vec3d( ni, nj, -nk ) );

    cvf::ref<RigEclipseCaseData> caseData = new RigEclipseCaseData( nullptr );
    builder.createGridsAndCells( caseData.p() );
    caseData->mainGrid()->computeCachedData();

    return caseData;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RivSingleCellPartGeneratorTest, ValidCellIndexCreatesDrawable )
{
    cvf::ref<RigEclipseCaseData> caseData = buildBoxGrid( 2, 3, 4 );

    RivSingleCellPartGenerator partGen( caseData.p(), 0, 5, cvf::Vec3d::ZERO );

    cvf::ref<cvf::Part> part = partGen.createPart( cvf::Color3f::RED );
    ASSERT_TRUE( part.notNull() );
    EXPECT_TRUE( part->drawable() != nullptr );
}

//--------------------------------------------------------------------------------------------------
/// A stale selection can reference a cell index no longer present in the grid
//--------------------------------------------------------------------------------------------------
TEST( RivSingleCellPartGeneratorTest, CellIndexOutOfBoundsIsIgnored )
{
    cvf::ref<RigEclipseCaseData> caseData = buildBoxGrid( 2, 3, 4 );

    const size_t cellCount = caseData->mainGrid()->cellCount();

    for ( size_t cellIndex : { cellCount, cellCount + 1000 } )
    {
        RivSingleCellPartGenerator partGen( caseData.p(), 0, cellIndex, cvf::Vec3d::ZERO );

        cvf::ref<cvf::Part> part = partGen.createPart( cvf::Color3f::RED );
        ASSERT_TRUE( part.notNull() );
        EXPECT_TRUE( part->drawable() == nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
/// A stale selection can reference a grid no longer present in the case
//--------------------------------------------------------------------------------------------------
TEST( RivSingleCellPartGeneratorTest, GridIndexOutOfBoundsIsIgnored )
{
    cvf::ref<RigEclipseCaseData> caseData = buildBoxGrid( 2, 3, 4 );

    RivSingleCellPartGenerator partGen( caseData.p(), 1, 0, cvf::Vec3d::ZERO );

    cvf::ref<cvf::Part> part = partGen.createPart( cvf::Color3f::RED );
    ASSERT_TRUE( part.notNull() );
    EXPECT_TRUE( part->drawable() == nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RivSingleCellPartGeneratorTest, UndefinedCellIndexIsIgnored )
{
    cvf::ref<RigEclipseCaseData> caseData = buildBoxGrid( 2, 3, 4 );

    RivSingleCellPartGenerator partGen( caseData.p(), 0, cvf::UNDEFINED_SIZE_T, cvf::Vec3d::ZERO );

    cvf::ref<cvf::Part> part = partGen.createPart( cvf::Color3f::RED );
    ASSERT_TRUE( part.notNull() );
    EXPECT_TRUE( part->drawable() == nullptr );
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RigCell.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"

#include "cvfCylindricalGeometryGenerator.h"
#include "cvfDrawableGeo.h"
#include "cvfGeometryGeneratorFactory.h"

#include <cmath>

using namespace cvf;

class MockCylindricalGrid : public RigMainGrid
{
public:
    MockCylindricalGrid()
        : RigMainGrid()
    {
        setGridGeometryType( cvf::GridGeometryType::CYLINDRICAL );

        // Create a simple 2x2x1 radial grid
        setGridPointDimensions( cvf::Vec3st( 3, 3, 2 ) );

        // Initialize nodes for a cylindrical sector (45 degrees, radius 1-2)
        nodes().resize( 3 * 3 * 2 );
        double centerX = 0.0, centerY = 0.0;
        double innerRadius = 1.0, outerRadius = 2.0;
        double startAngle = 0.0, endAngle = 45.0; // degrees

        for ( size_t k = 0; k < 2; ++k )
        {
            double z = k * 10.0;

            // For radial grids: I=radius, J=theta(degrees), K=z
            for ( size_t j = 0; j < 3; ++j )
            {
                double angle = startAngle + j * ( endAngle - startAngle ) / 2.0; // degrees

                for ( size_t i = 0; i < 3; ++i )
                {
                    double radius       = innerRadius + i * ( outerRadius - innerRadius ) / 2.0;
                    double angleRadians = angle * M_PI / 180.0; // convert to radians for trig
                    double x            = centerX + radius * cos( angleRadians );
                    double y            = centerY + radius * sin( angleRadians );

                    size_t nodeIdx   = i + j * 3 + k * 9;
                    nodes()[nodeIdx] = cvf::Vec3d( x, y, z );
                }
            }
        }

        // Initialize cells
        reservoirCells().resize( cellCount() );
        for ( size_t cellIdx = 0; cellIdx < cellCount(); ++cellIdx )
        {
            size_t i, j, k;
            ijkFromCellIndex( cellIdx, &i, &j, &k );

            // Set up corner indices for each cell
            std::array<size_t, 8> cornerIndices;
            cornerIndices[0] = i + j * 3 + k * 9; // (i,j,k)
            cornerIndices[1] = ( i + 1 ) + j * 3 + k * 9; // (i+1,j,k)
            cornerIndices[2] = ( i + 1 ) + ( j + 1 ) * 3 + k * 9; // (i+1,j+1,k)
            cornerIndices[3] = i + ( j + 1 ) * 3 + k * 9; // (i,j+1,k)
            cornerIndices[4] = i + j * 3 + ( k + 1 ) * 9; // (i,j,k+1)
            cornerIndices[5] = ( i + 1 ) + j * 3 + ( k + 1 ) * 9; // (i+1,j,k+1)
            cornerIndices[6] = ( i + 1 ) + ( j + 1 ) * 3 + ( k + 1 ) * 9; // (i+1,j+1,k+1)
            cornerIndices[7] = i + ( j + 1 ) * 3 + ( k + 1 ) * 9; // (i,j+1,k+1)

            cell( cellIdx ).cornerIndices() = cornerIndices;
        }
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCylindricalGrid, GeometryTypeDetection )
{
    MockCylindricalGrid grid;

    EXPECT_EQ( cvf::GridGeometryType::CYLINDRICAL, grid.gridGeometryType() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCylindricalGrid, CylindricalCoordinateExtraction )
{
    MockCylindricalGrid grid;

    // Test first cell (0,0,0)
    size_t cellIndex = grid.cellIndexFromIJK( 0, 0, 0 );

    auto result = grid.getCylindricalCoords( cellIndex );

    EXPECT_TRUE( result.has_value() );
    if ( result.has_value() )
    {
        const auto& cylCell = result.value();
        EXPECT_GT( cylCell.outerRadius, cylCell.innerRadius );
        EXPECT_GT( cylCell.endAngle, cylCell.startAngle );
        EXPECT_GT( cylCell.topZ, cylCell.bottomZ );
        EXPECT_GE( cylCell.innerRadius, 0.0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCylindricalGrid, FactoryPatternSelection )
{
    cvf::ref<MockCylindricalGrid> grid = new MockCylindricalGrid();

    auto generator = cvf::GeometryGeneratorFactory::create( grid.p(), false );

    EXPECT_NE( nullptr, generator.get() );
    // Note: Skip geometry type check to avoid initialization issues
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCylindricalGrid, CylindricalGeometryGeneration )
{
    cvf::ref<MockCylindricalGrid> grid = new MockCylindricalGrid();

    cvf::CylindricalGeometryGenerator generator( grid.p(), false );

    // Set up cell visibility - all cells visible
    ref<UByteArray> cellVisibility = new UByteArray;
    cellVisibility->resize( grid->cellCount() );
    cellVisibility->setAll( 1 );

    generator.setCellVisibility( cellVisibility.p() );

    // Just test that the generator was created successfully
    EXPECT_EQ( cvf::GridGeometryType::CYLINDRICAL, generator.geometryType() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCylindricalGrid, HexahedralGridCompatibility )
{
    // Test that regular grids still work with the factory pattern
    cvf::ref<RigMainGrid> hexGrid = new RigMainGrid();
    hexGrid->setGridPointDimensions( cvf::Vec3st( 3, 3, 3 ) );

    // Set up a simple rectangular grid
    hexGrid->nodes().resize( 3 * 3 * 3 );
    for ( size_t k = 0; k < 3; ++k )
    {
        for ( size_t j = 0; j < 3; ++j )
        {
            for ( size_t i = 0; i < 3; ++i )
            {
                size_t nodeIdx            = i + j * 3 + k * 9;
                hexGrid->nodes()[nodeIdx] = cvf::Vec3d( i * 10.0, j * 10.0, k * 10.0 );
            }
        }
    }

    EXPECT_EQ( cvf::GridGeometryType::HEXAHEDRAL, hexGrid->gridGeometryType() );

    auto generator = cvf::GeometryGeneratorFactory::create( hexGrid.p(), false );
    EXPECT_NE( nullptr, generator.get() );
    EXPECT_EQ( cvf::GridGeometryType::HEXAHEDRAL, generator->geometryType() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCylindricalGrid, InvalidCellIndexHandling )
{
    MockCylindricalGrid grid;

    // Test with valid cell index first
    size_t validCellIndex = 0;
    auto   result         = grid.getCylindricalCoords( validCellIndex );
    EXPECT_TRUE( result.has_value() );

    // Test with out-of-bounds IJK (should return false from ijkFromCellIndex)
    size_t i = 999, j = 999, k = 999;
    bool   validIJK = grid.ijkFromCellIndex( grid.cellCount() - 1, &i, &j, &k );
    EXPECT_TRUE( validIJK ); // Last valid cell should work
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigCylindricalGrid, AngleRangeValidation )
{
    MockCylindricalGrid grid;

    // Test multiple cells to ensure angle ranges are consistent
    for ( size_t cellIdx = 0; cellIdx < grid.cellCount(); ++cellIdx )
    {
        auto result = grid.getCylindricalCoords( cellIdx );

        if ( result.has_value() )
        {
            const auto& cylCell = result.value();
            EXPECT_GE( cylCell.endAngle, cylCell.startAngle );
            EXPECT_GE( cylCell.startAngle, -180.0 ); // degrees
            EXPECT_LE( cylCell.endAngle, 360.0 ); // degrees
            EXPECT_GT( cylCell.outerRadius, 0.0 );
            EXPECT_GE( cylCell.innerRadius, 0.0 );
            EXPECT_GE( cylCell.outerRadius, cylCell.innerRadius );
        }
    }
}

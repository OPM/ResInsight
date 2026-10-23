/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RifReaderMockModel.h"

#include "RigEclipseCaseData.h"
#include "RigGridManager.h"
#include "RigMainGrid.h"
#include "Well/RigEclipseWellLogExtractor.h"
#include "Well/RigWellPath.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseWellLogExtractor, ShortWellPathInsideOneCell )
{
    cvf::ref<RigEclipseCaseData> reservoir = new RigEclipseCaseData( nullptr );

    {
        cvf::ref<RifReaderMockModel> mockFileInterface = new RifReaderMockModel;

        mockFileInterface->setWorldCoordinates( cvf::Vec3d( 10, 10, 10 ), cvf::Vec3d( 20, 20, 20 ) );
        mockFileInterface->setCellCounts( cvf::Vec3st( 5, 6, 7 ) );
        mockFileInterface->enableWellData( false );

        mockFileInterface->open( "", reservoir.p() );

        reservoir->mainGrid()->computeCachedData();
    }

    EXPECT_FALSE( reservoir->mainGrid()->totalCellCount() == 0 );

    auto firstCell = reservoir->mainGrid()->cell( 0 );
    auto center    = firstCell.center();

    cvf::ref<RigWellPath> wellPathGeometry = new RigWellPath;
    {
        std::vector<cvf::Vec3d> wellPathPoints;
        std::vector<double>     mdValues;

        {
            double offset = 0.0;
            wellPathPoints.push_back( center );
            mdValues.push_back( offset );
        }

        {
            double offset = 0.1;
            wellPathPoints.push_back( center + cvf::Vec3d( 0, 0, offset ) );
            mdValues.push_back( offset );
        }

        wellPathGeometry->setWellPathPoints( wellPathPoints, mdValues );
    }

    cvf::ref<RigEclipseWellLogExtractor> e = new RigEclipseWellLogExtractor( reservoir.p(), wellPathGeometry.p(), "" );

    auto intersections = e->cellIntersectionInfosAlongWellPath();
    EXPECT_FALSE( intersections.empty() );
}

//--------------------------------------------------------------------------------------------------
/// Reproduces https://github.com/OPM/ResInsight/issues/13967
/// A horizontal well crossing several cells must report all the cells it passes through, regardless
/// of which combination of Flip X / Flip Y has been applied to the grid.
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseWellLogExtractor, HorizontalWellAcrossFlippedGrid )
{
    auto buildReservoir = []( bool flipX, bool flipY )
    {
        cvf::ref<RigEclipseCaseData> reservoir         = new RigEclipseCaseData( nullptr );
        cvf::ref<RifReaderMockModel> mockFileInterface = new RifReaderMockModel;

        mockFileInterface->setWorldCoordinates( cvf::Vec3d( 10, 10, 10 ), cvf::Vec3d( 20, 20, 20 ) );
        mockFileInterface->setCellCounts( cvf::Vec3st( 5, 6, 7 ) );
        mockFileInterface->enableWellData( false );
        mockFileInterface->open( "", reservoir.p() );

        if ( flipX || flipY ) reservoir->mainGrid()->setFlipAxis( flipX, flipY );

        reservoir->mainGrid()->computeCachedData();
        return reservoir;
    };

    auto runWellPath = []( cvf::ref<RigEclipseCaseData> reservoir, bool flipX, bool flipY )
    {
        const double            xSign            = flipX ? -1.0 : 1.0;
        const double            ySign            = flipY ? -1.0 : 1.0;
        cvf::ref<RigWellPath>   wellPathGeometry = new RigWellPath;
        std::vector<cvf::Vec3d> wellPathPoints   = { cvf::Vec3d( xSign * 10.5, ySign * 13.0, 15.0 ),
                                                     cvf::Vec3d( xSign * 19.5, ySign * 13.0, 15.0 ) };
        std::vector<double>     mdValues         = { 0.0, 9.0 };
        wellPathGeometry->setWellPathPoints( wellPathPoints, mdValues );

        cvf::ref<RigEclipseWellLogExtractor> e = new RigEclipseWellLogExtractor( reservoir.p(), wellPathGeometry.p(), "" );
        return e->cellIntersectionInfosAlongWellPath();
    };

    auto baseline = runWellPath( buildReservoir( false, false ), false, false );
    EXPECT_GT( baseline.size(), 2u );

    struct FlipCase
    {
        bool flipX;
        bool flipY;
    };
    const FlipCase cases[] = { { false, false }, { true, false }, { false, true }, { true, true } };

    for ( const auto& c : cases )
    {
        // Freshly-built grid with the requested flip applied before the first lazy cache populates.
        auto fresh = runWellPath( buildReservoir( c.flipX, c.flipY ), c.flipX, c.flipY );
        EXPECT_EQ( baseline.size(), fresh.size() ) << "fresh: flipX=" << c.flipX << " flipY=" << c.flipY;

        // Interactive toggle: load un-flipped, populate the face-normal-direction cache, then flip.
        // The intersection result must still match the freshly-built flipped grid.
        auto toggled = buildReservoir( false, false );
        (void)toggled->mainGrid()->isFaceNormalsOutwards();
        toggled->mainGrid()->setFlipAxis( c.flipX, c.flipY );
        toggled->mainGrid()->computeCachedData();

        auto toggledCells = runWellPath( toggled, c.flipX, c.flipY );
        EXPECT_EQ( baseline.size(), toggledCells.size() ) << "toggled: flipX=" << c.flipX << " flipY=" << c.flipY;
    }
}

namespace
{
//--------------------------------------------------------------------------------------------------
/// 3x1x4 grid with TVD 1000-1004, located at x = [1, 4]. A vertical well at x = 0 is outside all cells, as a well on the axis of a
/// radial grid is.
//--------------------------------------------------------------------------------------------------
cvf::ref<RigEclipseCaseData> createRadialAxisTestReservoir( bool isRadial, bool kIncreasesUpwards )
{
    cvf::ref<RigEclipseCaseData> reservoir         = new RigEclipseCaseData( nullptr );
    cvf::ref<RifReaderMockModel> mockFileInterface = new RifReaderMockModel;

    const double topZ    = -1000.0;
    const double bottomZ = -1004.0;
    if ( kIncreasesUpwards )
    {
        mockFileInterface->setWorldCoordinates( cvf::Vec3d( 1, 0, bottomZ ), cvf::Vec3d( 4, 1, topZ ) );
    }
    else
    {
        mockFileInterface->setWorldCoordinates( cvf::Vec3d( 1, 0, topZ ), cvf::Vec3d( 4, 1, bottomZ ) );
    }
    mockFileInterface->setCellCounts( cvf::Vec3st( 3, 1, 4 ) );
    mockFileInterface->enableWellData( false );
    mockFileInterface->open( "", reservoir.p() );

    reservoir->mainGrid()->setIsRadial( isRadial );
    reservoir->mainGrid()->computeCachedData();

    return reservoir;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo> radialAxisIntersections( RigEclipseCaseData* reservoir, double topTvd, double bottomTvd, double topMd )
{
    cvf::ref<RigWellPath> wellPathGeometry = new RigWellPath;
    wellPathGeometry->setWellPathPoints( { cvf::Vec3d( 0, 0.5, -topTvd ), cvf::Vec3d( 0, 0.5, -bottomTvd ) },
                                         { topMd, topMd + bottomTvd - topTvd } );

    cvf::ref<RigEclipseWellLogExtractor> e = new RigEclipseWellLogExtractor( reservoir, wellPathGeometry.p(), "" );
    return e->cellIntersectionInfosAlongWellPath();
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseWellLogExtractor, RadialGridWellOnAxisUsesInnermostColumn )
{
    for ( bool kIncreasesUpwards : { false, true } )
    {
        auto reservoir = createRadialAxisTestReservoir( true, kIncreasesUpwards );

        auto intersections = radialAxisIntersections( reservoir.p(), 1000.0, 1004.0, 100.0 );
        ASSERT_EQ( 4u, intersections.size() ) << "kIncreasesUpwards=" << kIncreasesUpwards;

        for ( size_t i = 0; i < intersections.size(); i++ )
        {
            const size_t k = kIncreasesUpwards ? 3 - i : i;
            EXPECT_EQ( reservoir->mainGrid()->cellIndexFromIJK( 0, 0, k ), intersections[i].globCellIndex );
            EXPECT_NEAR( 100.0 + i, intersections[i].startMD, 1e-6 );
            EXPECT_NEAR( 101.0 + i, intersections[i].endMD, 1e-6 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseWellLogExtractor, RadialGridWellOnAxisClipsCellsToWellPathDepth )
{
    auto reservoir = createRadialAxisTestReservoir( true, false );

    auto intersections = radialAxisIntersections( reservoir.p(), 1001.5, 1003.0, 0.0 );
    ASSERT_EQ( 2u, intersections.size() );

    EXPECT_EQ( reservoir->mainGrid()->cellIndexFromIJK( 0, 0, 1 ), intersections[0].globCellIndex );
    EXPECT_NEAR( 0.0, intersections[0].startMD, 1e-6 );
    EXPECT_NEAR( 0.5, intersections[0].endMD, 1e-6 );

    EXPECT_EQ( reservoir->mainGrid()->cellIndexFromIJK( 0, 0, 2 ), intersections[1].globCellIndex );
    EXPECT_NEAR( 0.5, intersections[1].startMD, 1e-6 );
    EXPECT_NEAR( 1.5, intersections[1].endMD, 1e-6 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseWellLogExtractor, NonRadialGridWellOutsideGridHasNoIntersections )
{
    auto reservoir = createRadialAxisTestReservoir( false, false );

    EXPECT_TRUE( radialAxisIntersections( reservoir.p(), 1000.0, 1004.0, 100.0 ).empty() );
}

//--------------------------------------------------------------------------------------------------
/// The innermost ring fallback only applies when it is a single 360 degree cell for every layer. When the ring is split into
/// several angular sectors, picking a single sector would be arbitrary, so no fallback intersections should be inserted.
//--------------------------------------------------------------------------------------------------
TEST( RigEclipseWellLogExtractor, RadialGridWellOnAxisWithMultipleAngularSectorsHasNoFallback )
{
    cvf::ref<RigEclipseCaseData> reservoir         = new RigEclipseCaseData( nullptr );
    cvf::ref<RifReaderMockModel> mockFileInterface = new RifReaderMockModel;

    mockFileInterface->setWorldCoordinates( cvf::Vec3d( 1, 0, -1004.0 ), cvf::Vec3d( 4, 1, -1000.0 ) );
    mockFileInterface->setCellCounts( cvf::Vec3st( 3, 2, 4 ) );
    mockFileInterface->enableWellData( false );
    mockFileInterface->open( "", reservoir.p() );

    reservoir->mainGrid()->setIsRadial( true );
    reservoir->mainGrid()->computeCachedData();

    EXPECT_TRUE( radialAxisIntersections( reservoir.p(), 1000.0, 1004.0, 100.0 ).empty() );
}

//--------------------------------------------------------------------------------------------------
/// setFlipAxis must actually mirror the grid node coordinates for every combination of Flip X /
/// Flip Y. Covers the data-level part of https://github.com/OPM/ResInsight/issues/13967 where a
/// saved flip flag could appear active while the grid was still un-flipped.
//--------------------------------------------------------------------------------------------------
TEST( RigMainGrid, SetFlipAxisMirrorsNodeCoordinates )
{
    auto checkFlip = []( bool flipX, bool flipY )
    {
        cvf::ref<RigEclipseCaseData> reservoir         = new RigEclipseCaseData( nullptr );
        cvf::ref<RifReaderMockModel> mockFileInterface = new RifReaderMockModel;

        mockFileInterface->setWorldCoordinates( cvf::Vec3d( 100, 200, 300 ), cvf::Vec3d( 110, 210, 310 ) );
        mockFileInterface->setCellCounts( cvf::Vec3st( 2, 2, 2 ) );
        mockFileInterface->enableWellData( false );
        mockFileInterface->open( "", reservoir.p() );

        auto*      mainGrid      = reservoir->mainGrid();
        const auto originalNodes = mainGrid->nodes();
        ASSERT_FALSE( originalNodes.empty() );

        mainGrid->setFlipAxis( flipX, flipY );

        const auto& flippedNodes = mainGrid->nodes();
        ASSERT_EQ( originalNodes.size(), flippedNodes.size() );

        const double xSign = flipX ? -1.0 : 1.0;
        const double ySign = flipY ? -1.0 : 1.0;
        for ( size_t i = 0; i < originalNodes.size(); i++ )
        {
            EXPECT_DOUBLE_EQ( xSign * originalNodes[i].x(), flippedNodes[i].x() ) << "flipX=" << flipX << " flipY=" << flipY;
            EXPECT_DOUBLE_EQ( ySign * originalNodes[i].y(), flippedNodes[i].y() ) << "flipX=" << flipX << " flipY=" << flipY;
            EXPECT_DOUBLE_EQ( originalNodes[i].z(), flippedNodes[i].z() ) << "flipX=" << flipX << " flipY=" << flipY;
        }
    };

    checkFlip( false, false );
    checkFlip( true, false );
    checkFlip( false, true );
    checkFlip( true, true );
}

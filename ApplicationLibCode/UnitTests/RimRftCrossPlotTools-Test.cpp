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

#include "gtest/gtest.h"

#include "RimRftCrossPlotTools.h"

#include <limits>

//--------------------------------------------------------------------------------------------------
/// When depth-range filtering is disabled, all pressures are returned unmodified.
//--------------------------------------------------------------------------------------------------
TEST( RimRftCrossPlotToolsTest, FilterPressuresByDepthRange_DisabledReturnsAllSamples )
{
    std::vector<double> depths{ 1000.0, 2000.0, 3000.0 };
    std::vector<double> pressures{ 10.0, 20.0, 30.0 };

    auto filtered = RimRftCrossPlotTools::filterPressuresByDepthRange( depths, pressures, false, 1500.0, 2500.0 );

    EXPECT_EQ( pressures, filtered );
}

//--------------------------------------------------------------------------------------------------
/// When enabled, only pressures whose depth falls within [min, max] (inclusive) are returned.
//--------------------------------------------------------------------------------------------------
TEST( RimRftCrossPlotToolsTest, FilterPressuresByDepthRange_FiltersByInclusiveRange )
{
    std::vector<double> depths{ 1000.0, 1500.0, 2000.0, 2500.0, 3000.0 };
    std::vector<double> pressures{ 10.0, 15.0, 20.0, 25.0, 30.0 };

    auto filtered = RimRftCrossPlotTools::filterPressuresByDepthRange( depths, pressures, true, 1500.0, 2500.0 );

    std::vector<double> expected{ 15.0, 20.0, 25.0 };
    EXPECT_EQ( expected, filtered );
}

//--------------------------------------------------------------------------------------------------
/// Depths outside the requested range are excluded.
//--------------------------------------------------------------------------------------------------
TEST( RimRftCrossPlotToolsTest, FilterPressuresByDepthRange_ExcludesOutsideRange )
{
    std::vector<double> depths{ 500.0, 4000.0 };
    std::vector<double> pressures{ 5.0, 40.0 };

    auto filtered = RimRftCrossPlotTools::filterPressuresByDepthRange( depths, pressures, true, 1000.0, 3000.0 );

    EXPECT_TRUE( filtered.empty() );
}

//--------------------------------------------------------------------------------------------------
/// Mismatched depths/pressures sizes cannot be aligned, so filtering returns an empty result.
//--------------------------------------------------------------------------------------------------
TEST( RimRftCrossPlotToolsTest, FilterPressuresByDepthRange_MismatchedSizesReturnsEmpty )
{
    std::vector<double> depths{ 1000.0, 2000.0 };
    std::vector<double> pressures{ 10.0 };

    auto filtered = RimRftCrossPlotTools::filterPressuresByDepthRange( depths, pressures, true, 0.0, 5000.0 );

    EXPECT_TRUE( filtered.empty() );
}

//--------------------------------------------------------------------------------------------------
/// The mean of an empty sample set is defined as infinity (used as a "no data" sentinel).
//--------------------------------------------------------------------------------------------------
TEST( RimRftCrossPlotToolsTest, ComputeMean_EmptyReturnsInfinity )
{
    std::vector<double> samples;

    EXPECT_EQ( std::numeric_limits<double>::infinity(), RimRftCrossPlotTools::computeMean( samples ) );
}

//--------------------------------------------------------------------------------------------------
/// The mean of a normal set of samples is the arithmetic average.
//--------------------------------------------------------------------------------------------------
TEST( RimRftCrossPlotToolsTest, ComputeMean_NormalAverage )
{
    std::vector<double> samples{ 10.0, 20.0, 30.0 };

    EXPECT_DOUBLE_EQ( 20.0, RimRftCrossPlotTools::computeMean( samples ) );
}

//--------------------------------------------------------------------------------------------------
/// The depth type abbreviation used in axis/plot titles distinguishes MD from TVD.
//--------------------------------------------------------------------------------------------------
TEST( RimRftCrossPlotToolsTest, DepthTypeAbbreviation )
{
    EXPECT_EQ( QString( "MD" ), RimRftCrossPlotTools::depthTypeAbbreviation( RiaDefines::DepthType::MEASURED_DEPTH ) );
    EXPECT_EQ( QString( "TVD" ), RimRftCrossPlotTools::depthTypeAbbreviation( RiaDefines::DepthType::TRUE_VERTICAL_DEPTH ) );
}

//--------------------------------------------------------------------------------------------------
/// Disabled filtering returns no intervals regardless of mode.
//--------------------------------------------------------------------------------------------------
TEST( RimRftCrossPlotToolsTest, BuildDepthIntervals_DisabledReturnsEmpty )
{
    auto intervals = RimRftCrossPlotTools::buildDepthIntervals( false,
                                                                RimRftCrossPlotTools::DepthFilterMode::DEPTH_RANGE,
                                                                1000.0,
                                                                2000.0,
                                                                nullptr,
                                                                QString(),
                                                                {},
                                                                RiaDefines::DepthType::MEASURED_DEPTH );
    EXPECT_TRUE( intervals.empty() );
}

//--------------------------------------------------------------------------------------------------
/// Depth range mode produces a single interval matching the requested range.
//--------------------------------------------------------------------------------------------------
TEST( RimRftCrossPlotToolsTest, BuildDepthIntervals_DepthRangeProducesSingleInterval )
{
    auto intervals = RimRftCrossPlotTools::buildDepthIntervals( true,
                                                                RimRftCrossPlotTools::DepthFilterMode::DEPTH_RANGE,
                                                                1000.0,
                                                                2000.0,
                                                                nullptr,
                                                                QString(),
                                                                {},
                                                                RiaDefines::DepthType::MEASURED_DEPTH );
    ASSERT_EQ( size_t( 1 ), intervals.size() );
    EXPECT_DOUBLE_EQ( 1000.0, intervals[0].top );
    EXPECT_DOUBLE_EQ( 2000.0, intervals[0].base );
}

//--------------------------------------------------------------------------------------------------
/// Zones mode with no well formations file available produces no intervals.
//--------------------------------------------------------------------------------------------------
TEST( RimRftCrossPlotToolsTest, BuildDepthIntervals_ZonesWithoutFileReturnsEmpty )
{
    auto intervals = RimRftCrossPlotTools::buildDepthIntervals( true,
                                                                RimRftCrossPlotTools::DepthFilterMode::ZONES,
                                                                1000.0,
                                                                2000.0,
                                                                nullptr,
                                                                "WELL-A",
                                                                { "ZoneA" },
                                                                RiaDefines::DepthType::MEASURED_DEPTH );
    EXPECT_TRUE( intervals.empty() );
}

//--------------------------------------------------------------------------------------------------
/// An empty interval list means no filtering, so all pressures pass through.
//--------------------------------------------------------------------------------------------------
TEST( RimRftCrossPlotToolsTest, FilterPressuresByDepthIntervals_EmptyIntervalsReturnsAllSamples )
{
    std::vector<double> depths{ 1000.0, 2000.0, 3000.0 };
    std::vector<double> pressures{ 10.0, 20.0, 30.0 };

    auto filtered = RimRftCrossPlotTools::filterPressuresByDepthIntervals( depths, pressures, {} );

    EXPECT_EQ( pressures, filtered );
}

//--------------------------------------------------------------------------------------------------
/// Only samples within any of the given (possibly non-contiguous) intervals are returned.
//--------------------------------------------------------------------------------------------------
TEST( RimRftCrossPlotToolsTest, FilterPressuresByDepthIntervals_FiltersByMultipleIntervals )
{
    std::vector<double> depths{ 1000.0, 1500.0, 2000.0, 2500.0, 3000.0 };
    std::vector<double> pressures{ 10.0, 15.0, 20.0, 25.0, 30.0 };

    std::vector<RimRftCrossPlotTools::DepthInterval> intervals{ { 1000.0, 1500.0 }, { 2800.0, 3200.0 } };
    auto filtered = RimRftCrossPlotTools::filterPressuresByDepthIntervals( depths, pressures, intervals );

    std::vector<double> expected{ 10.0, 15.0, 30.0 };
    EXPECT_EQ( expected, filtered );
}

//--------------------------------------------------------------------------------------------------
/// A filter description is empty when filtering is disabled.
//--------------------------------------------------------------------------------------------------
TEST( RimRftCrossPlotToolsTest, DepthFilterDescription_DisabledReturnsEmpty )
{
    QString description = RimRftCrossPlotTools::depthFilterDescription( false,
                                                                        RimRftCrossPlotTools::DepthFilterMode::DEPTH_RANGE,
                                                                        RiaDefines::DepthType::MEASURED_DEPTH,
                                                                        1000.0,
                                                                        2000.0,
                                                                        {} );
    EXPECT_TRUE( description.isEmpty() );
}

//--------------------------------------------------------------------------------------------------
/// Zone mode description lists the selected zone names.
//--------------------------------------------------------------------------------------------------
TEST( RimRftCrossPlotToolsTest, DepthFilterDescription_ZonesListsSelectedZoneNames )
{
    QString description = RimRftCrossPlotTools::depthFilterDescription( true,
                                                                        RimRftCrossPlotTools::DepthFilterMode::ZONES,
                                                                        RiaDefines::DepthType::MEASURED_DEPTH,
                                                                        1000.0,
                                                                        2000.0,
                                                                        { "Valysar", "Therys" } );
    EXPECT_EQ( QString( "Zones: Valysar, Therys" ), description );
}

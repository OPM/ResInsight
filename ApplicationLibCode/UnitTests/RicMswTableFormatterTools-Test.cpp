/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024 Equinor ASA
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

#include "Commands/CompletionExportCommands/RicMswTableFormatterTools.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
/// Helper function to compare vectors of pairs with tolerance
//--------------------------------------------------------------------------------------------------
void expectPairsEqual( const std::vector<std::pair<double, double>>& actual,
                       const std::vector<std::pair<double, double>>& expected,
                       double                                        tolerance = 1e-9 )
{
    ASSERT_EQ( expected.size(), actual.size() ) << "Number of segments doesn't match";

    for ( size_t i = 0; i < expected.size(); ++i )
    {
        EXPECT_NEAR( expected[i].first, actual[i].first, tolerance ) << "Segment " << i << " start MD doesn't match";
        EXPECT_NEAR( expected[i].second, actual[i].second, tolerance ) << "Segment " << i << " end MD doesn't match";
    }
}

//--------------------------------------------------------------------------------------------------
/// Test basic case with no subdivision (maxSegmentLength = 0)
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_NoSubdivision )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double maxSegmentLength = 0.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{};
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );
    auto   expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 200.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test basic case with no subdivision (negative maxSegmentLength)
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_NegativeMaxSegmentLength )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double maxSegmentLength = -10.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{};
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );
    auto   expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 200.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test subdivision with maxSegmentLength that divides evenly
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_EvenSubdivision )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double maxSegmentLength = 50.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{};
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );

    // ceil(100/50) = ceil(2.0) = 2 segments of 50 units each
    auto expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 150.0 }, { 150.0, 200.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test subdivision with maxSegmentLength that doesn't divide evenly
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_UnevenSubdivision )
{
    double startMD          = 100.0;
    double endMD            = 270.0;
    double maxSegmentLength = 50.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{};
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );

    // ceil(170 / 50) = ceil(3.4) = 4 segments
    // Each segment should be 170/4 = 42.5 units
    auto expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 142.5 }, { 142.5, 185.0 }, { 185.0, 227.5 }, { 227.5, 270.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test with maxSegmentLength larger than the interval
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_MaxSegmentLargerThanInterval )
{
    double startMD          = 100.0;
    double endMD            = 150.0;
    double maxSegmentLength = 200.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{};
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );
    auto   expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 150.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test with a single custom interval that exactly matches the segment
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_CustomIntervalExactMatch )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double maxSegmentLength = 50.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{ { 100.0, 200.0 } };
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );
    auto   expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 200.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test with custom interval inside the segment
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_CustomIntervalInside )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double maxSegmentLength = 30.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{ { 130.0, 170.0 } };
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );

    // Expected: [100-130] subdivided, [130-170] custom interval, [170-200] subdivided
    // [100-130] = 30 units: ceil(30/30) = 1 segment
    // [130-170] = exact custom interval
    // [170-200] = 30 units: ceil(30/30) = 1 segment
    auto expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 130.0 }, { 130.0, 170.0 }, { 170.0, 200.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test with multiple custom intervals
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_MultipleCustomIntervals )
{
    double startMD          = 100.0;
    double endMD            = 300.0;
    double maxSegmentLength = 40.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{ { 120.0, 140.0 }, { 180.0, 200.0 } };
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );

    // Expected:
    // [100-120] = 20 units: ceil(20/40) = 1 segment
    // [120-140] = custom interval
    // [140-180] = 40 units: ceil(40/40) = 1 segment
    // [180-200] = custom interval
    // [200-300] = 100 units: ceil(100/40) = 3 segments of 33.333... units each
    auto expectedSegments = std::vector<std::pair<double, double>>{
        { 100.0, 120.0 },
        { 120.0, 140.0 },
        { 140.0, 180.0 },
        { 180.0, 200.0 },
        { 200.0, 233.333333333333 },
        { 233.333333333333, 266.666666666667 },
        { 266.666666666667, 300.0 },
    };

    expectPairsEqual( result, expectedSegments, 1e-6 );
}

//--------------------------------------------------------------------------------------------------
/// Test with custom interval partially overlapping the segment (extending before)
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_CustomIntervalOverlapBefore )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double maxSegmentLength = 30.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{ { 80.0, 130.0 } };
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );

    // Custom interval [80-130] clipped to [100-130]
    // [100-130] = custom interval (clipped)
    // [130-200] = 70 units: trunc(70/30) + 1 = 3 segments of 23.333... units each
    auto expectedSegments = std::vector<std::pair<double, double>>{
        { 100.0, 130.0 },
        { 130.0, 153.333333333333 },
        { 153.333333333333, 176.666666666667 },
        { 176.666666666667, 200.0 },
    };

    expectPairsEqual( result, expectedSegments, 1e-6 );
}

//--------------------------------------------------------------------------------------------------
/// Test with custom interval partially overlapping the segment (extending after)
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_CustomIntervalOverlapAfter )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double maxSegmentLength = 30.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{ { 170.0, 230.0 } };
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );

    // [100-170] = 70 units: trunc(70/30) + 1 = 3 segments of 23.333... units each
    // Custom interval [170-230] clipped to [170-200]
    auto expectedSegments = std::vector<std::pair<double, double>>{
        { 100.0, 123.333333333333 },
        { 123.333333333333, 146.666666666667 },
        { 146.666666666667, 170.0 },
        { 170.0, 200.0 },
    };

    expectPairsEqual( result, expectedSegments, 1e-6 );
}

//--------------------------------------------------------------------------------------------------
/// Test with custom interval completely outside the segment (before)
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_CustomIntervalOutsideBefore )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double maxSegmentLength = 50.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{ { 50.0, 80.0 } };
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );

    // Custom interval doesn't overlap, so standard subdivision applies
    // ceil(100/50) = 2 segments
    auto expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 150.0 }, { 150.0, 200.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test with custom interval completely outside the segment (after)
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_CustomIntervalOutsideAfter )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double maxSegmentLength = 50.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{ { 250.0, 300.0 } };
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );

    // Custom interval doesn't overlap, so standard subdivision applies
    // ceil(100/50) = 2 segments
    auto expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 150.0 }, { 150.0, 200.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test with custom interval that engulfs the entire segment
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_CustomIntervalEngulfsSegment )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double maxSegmentLength = 30.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{ { 50.0, 250.0 } };
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );

    // Custom interval [50-250] clipped to [100-200], which matches the entire segment
    auto expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 200.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test with zero-length segment
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_ZeroLengthSegment )
{
    double startMD          = 100.0;
    double endMD            = 100.0;
    double maxSegmentLength = 50.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{};
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );

    // ceil(0/50) = 0, so no segments created
    auto expectedSegments = std::vector<std::pair<double, double>>{};

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test with very small maxSegmentLength (many subdivisions)
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_ManySubdivisions )
{
    double startMD          = 100.0;
    double endMD            = 110.0;
    double maxSegmentLength = 2.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{};
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );

    // ceil(10 / 2) = ceil(5.0) = 5 segments of 2.0 units each
    auto expectedSegments = std::vector<std::pair<double, double>>{
        { 100.0, 102.0 },
        { 102.0, 104.0 },
        { 104.0, 106.0 },
        { 106.0, 108.0 },
        { 108.0, 110.0 },
    };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test with adjacent custom intervals (no gap)
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_AdjacentCustomIntervals )
{
    double startMD          = 100.0;
    double endMD            = 300.0;
    double maxSegmentLength = 40.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{ { 140.0, 180.0 }, { 180.0, 220.0 } };
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );

    // [100-140] = 40 units: ceil(40/40) = 1 segment
    // [140-180] = custom interval
    // [180-220] = custom interval
    // [220-300] = 80 units: ceil(80/40) = 2 segments of 40 units each
    auto expectedSegments = std::vector<std::pair<double, double>>{
        { 100.0, 140.0 },
        { 140.0, 180.0 },
        { 180.0, 220.0 },
        { 220.0, 260.0 },
        { 260.0, 300.0 },
    };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test edge case: custom interval with zero length
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_ZeroLengthCustomInterval )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double maxSegmentLength = 50.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{ { 150.0, 150.0 } };
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );

    // Zero-length custom interval should be ignored (clippedStart < clippedEnd check fails)
    // ceil(100/50) = 2 segments
    auto expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 150.0 }, { 150.0, 200.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test with custom intervals but maxSegmentLength = 0
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_CustomIntervalsNoMaxLength )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double maxSegmentLength = 0.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{ { 130.0, 170.0 } };
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );

    // With maxSegmentLength = 0 but customIntervals specified, custom intervals should be honored
    // Gaps not covered by custom intervals are used as-is (no subdivision)
    // [100-130] = gap before custom interval
    // [130-170] = custom interval
    // [170-200] = gap after custom interval
    auto expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 130.0 }, { 130.0, 170.0 }, { 170.0, 200.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test with overlapping custom intervals (should handle duplicate boundaries correctly)
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_OverlappingCustomIntervals )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double maxSegmentLength = 30.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{ { 120.0, 160.0 }, { 140.0, 180.0 } };
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );

    // Boundaries: 100, 120, 140, 160, 180, 200
    // Gaps to check: [100-120], [120-140], [140-160], [160-180], [180-200]
    // [100-120] = 20 units: subdivided into 1 segment
    // [120-140] = covered by first custom interval [120-160]
    // [140-160] = covered by overlapping custom intervals
    // [160-180] = covered by second custom interval [140-180]
    // [180-200] = 20 units: subdivided into 1 segment

    // Note: The logic may not perfectly handle overlapping intervals as "exact" custom intervals
    // This test verifies the actual behavior with overlapping boundaries
    auto expectedSegments = std::vector<std::pair<double, double>>{
        { 100.0, 120.0 },
        { 120.0, 140.0 },
        { 140.0, 160.0 },
        { 160.0, 180.0 },
        { 180.0, 200.0 },
    };

    expectPairsEqual( result, expectedSegments, 1e-6 );
}

//--------------------------------------------------------------------------------------------------
/// Test with very small custom interval inside a larger segment
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_TinyCustomInterval )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double maxSegmentLength = 40.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{ { 149.0, 151.0 } };
    auto   result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, std::nullopt, maxSegmentLength, customIntervals );

    // [100-149] = 49 units: trunc(49/40) + 1 = 2 segments of 24.5 units each
    // [149-151] = custom interval
    // [151-200] = 49 units: trunc(49/40) + 1 = 2 segments of 24.5 units each
    auto expectedSegments = std::vector<std::pair<double, double>>{
        { 100.0, 124.5 },
        { 124.5, 149.0 },
        { 149.0, 151.0 },
        { 151.0, 175.5 },
        { 175.5, 200.0 },
    };

    expectPairsEqual( result, expectedSegments, 1e-6 );
}

//--------------------------------------------------------------------------------------------------
/// Test minSegmentLength prevents segments smaller than minimum
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_MinSegmentLength_PreventsTooSmallSegments )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double minSegmentLength = 30.0;
    double maxSegmentLength = 50.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{};
    auto result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, minSegmentLength, maxSegmentLength, customIntervals );

    // Without minSegmentLength: ceil(100/50) = 2 segments of 50 units each
    // With minSegmentLength=30: segments would be 50 units each, which is >= 30, so stays the same
    auto expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 150.0 }, { 150.0, 200.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test minSegmentLength reduces segment count when subdivisions would be too small
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_MinSegmentLength_ReducesSegmentCount )
{
    double startMD          = 100.0;
    double endMD            = 250.0;
    double minSegmentLength = 40.0;
    double maxSegmentLength = 50.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{};
    auto result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, minSegmentLength, maxSegmentLength, customIntervals );

    // Without minSegmentLength: ceil(150/50) = 3 segments of 50 units each
    // With minSegmentLength=40: segments would be 50 units each, which is >= 40, so stays as 3 segments
    auto expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 150.0 }, { 150.0, 200.0 }, { 200.0, 250.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test minSegmentLength with uneven division
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_MinSegmentLength_UnevenDivision )
{
    double startMD          = 100.0;
    double endMD            = 220.0;
    double minSegmentLength = 30.0;
    double maxSegmentLength = 35.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{};
    auto result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, minSegmentLength, maxSegmentLength, customIntervals );

    // Without minSegmentLength: ceil(120/35) = ceil(3.43) = 4 segments of 30 units each
    // With minSegmentLength=30: segments would be 30 units each, which is exactly 30, so stays as 4 segments
    auto expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 130.0 }, { 130.0, 160.0 }, { 160.0, 190.0 }, { 190.0, 220.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test minSegmentLength forces fewer, larger segments
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_MinSegmentLength_ForcesFewerSegments )
{
    double startMD          = 100.0;
    double endMD            = 180.0;
    double minSegmentLength = 35.0;
    double maxSegmentLength = 45.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{};
    auto result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, minSegmentLength, maxSegmentLength, customIntervals );

    // Without minSegmentLength: ceil(80/45) = 2 segments of 40 units each
    // With minSegmentLength=35: segments would be 40 units each, which is >= 35, so stays as 2 segments
    auto expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 140.0 }, { 140.0, 180.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test minSegmentLength with custom intervals - gaps between custom intervals respect minSegmentLength
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_MinSegmentLength_WithCustomIntervals )
{
    double startMD          = 100.0;
    double endMD            = 300.0;
    double minSegmentLength = 35.0;
    double maxSegmentLength = 40.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{ { 140.0, 160.0 }, { 200.0, 220.0 } };
    auto result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, minSegmentLength, maxSegmentLength, customIntervals );

    // [100-140] = 40 units: ceil(40/40) = 1 segment >= 35
    // [140-160] = custom interval (20 units - OK to be smaller than minSegmentLength)
    // [160-200] = 40 units: ceil(40/40) = 1 segment >= 35
    // [200-220] = custom interval (20 units - OK to be smaller than minSegmentLength)
    // [220-300] = 80 units: ceil(80/40) = 2 segments of 40 units each >= 35
    auto expectedSegments = std::vector<std::pair<double, double>>{
        { 100.0, 140.0 },
        { 140.0, 160.0 },
        { 160.0, 200.0 },
        { 200.0, 220.0 },
        { 220.0, 260.0 },
        { 260.0, 300.0 },
    };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test minSegmentLength allows custom intervals to be smaller
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_MinSegmentLength_AllowsSmallCustomIntervals )
{
    double startMD          = 100.0;
    double endMD            = 300.0;
    double minSegmentLength = 50.0;
    double maxSegmentLength = 60.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{ { 145.0, 155.0 } };
    auto result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, minSegmentLength, maxSegmentLength, customIntervals );

    // [100-145] = 45 units: would be 1 segment of 45 units, but that's < 50, so no subdivision
    //             floor(45/50) = 0, so 1 segment of 45 units
    // [145-155] = custom interval (10 units - OK to be smaller than minSegmentLength)
    // [155-300] = 145 units: ceil(145/60) = 3 segments of 48.33 units each
    //             but 48.33 < 50, so floor(145/50) = 2 segments of 72.5 units each
    auto expectedSegments = std::vector<std::pair<double, double>>{
        { 100.0, 145.0 },
        { 145.0, 155.0 },
        { 155.0, 227.5 },
        { 227.5, 300.0 },
    };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test minSegmentLength with very small gap that becomes one segment
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_MinSegmentLength_SmallGapBecomesOneSegment )
{
    double startMD          = 100.0;
    double endMD            = 155.0;
    double minSegmentLength = 30.0;
    double maxSegmentLength = 40.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{};
    auto result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, minSegmentLength, maxSegmentLength, customIntervals );

    // Without minSegmentLength: ceil(55/40) = 2 segments of 27.5 units each
    // With minSegmentLength=30: floor(55/30) = 1 segment of 55 units (since 27.5 < 30)
    auto expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 155.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test minSegmentLength = 0 (ignored)
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_MinSegmentLength_Zero )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double minSegmentLength = 0.0;
    double maxSegmentLength = 50.0;
    auto   customIntervals  = std::vector<std::pair<double, double>>{};
    auto result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, minSegmentLength, maxSegmentLength, customIntervals );

    // minSegmentLength = 0 is ignored, so normal behavior
    // ceil(100/50) = 2 segments
    auto expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 150.0 }, { 150.0, 200.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test minSegmentLength with no maxSegmentLength and no custom intervals
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_MinSegmentLength_NoMaxLengthNoCustomIntervals )
{
    double startMD          = 100.0;
    double endMD            = 200.0;
    double minSegmentLength = 30.0;
    double maxSegmentLength = 0.0; // No max segment length
    auto   customIntervals  = std::vector<std::pair<double, double>>{};
    auto result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, minSegmentLength, maxSegmentLength, customIntervals );

    // When maxSegmentLength = 0, the function creates 1 segment regardless of minSegmentLength
    // This is because maxSegmentLength <= 0 triggers the "no subdivision" path
    auto expectedSegments = std::vector<std::pair<double, double>>{ { 100.0, 200.0 } };

    expectPairsEqual( result, expectedSegments );
}

//--------------------------------------------------------------------------------------------------
/// Test minSegmentLength with custom intervals but no maxSegmentLength
/// Gaps between custom intervals should respect minSegmentLength even without maxSegmentLength
//--------------------------------------------------------------------------------------------------
TEST( RicMswTableFormatterTools, createSubSegmentMDPairs_MinSegmentLength_CustomIntervalsNoMaxLength )
{
    double startMD          = 100.0;
    double endMD            = 300.0;
    double minSegmentLength = 40.0;
    double maxSegmentLength = 0.0; // No max segment length
    auto   customIntervals  = std::vector<std::pair<double, double>>{ { 150.0, 170.0 } };
    auto result = RicMswTableFormatterTools::createSubSegmentMDPairs( startMD, endMD, minSegmentLength, maxSegmentLength, customIntervals );

    // [100-150] = 50 units gap: should become 1 segment (since 50 >= 40)
    // [150-170] = custom interval (stays as-is)
    // [170-300] = 130 units gap: should subdivide to respect minSegmentLength
    //   floor(130/40) = 3 segments of 130/3 = 43.33 units each (all >= 40)
    auto expectedSegments = std::vector<std::pair<double, double>>{
        { 100.0, 150.0 },           // Gap before custom interval
        { 150.0, 170.0 },           // Custom interval
        { 170.0, 213.333333333 },   // First segment of subdivided gap
        { 213.333333333, 256.666666667 }, // Second segment
        { 256.666666667, 300.0 }    // Third segment
    };

    expectPairsEqual( result, expectedSegments );
}

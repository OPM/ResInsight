#include "gtest/gtest.h"

#include "RimSummaryTableTools.h"

#include <algorithm>
#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryTableToolsTest, SortVectorDataOnDateIsIndependentOfInputOrder )
{
    using namespace RimSummaryTableTools;

    auto vector = []( const QString& category, double value, time_t first, time_t last )
    {
        return VectorData{ .category = category, .name = "WOPR", .values = { value }, .firstTimeStep = first, .lastTimeStep = last, .timeSteps = {} };
    };

    const std::vector<VectorData> input = { vector( "E-2H", 1.0, 20, 30 ),
                                            vector( "Z-1H", 0.0, 0, 0 ),
                                            vector( "B-1H", 1.0, 20, 30 ),
                                            vector( "C-1H", 1.0, 10, 40 ),
                                            vector( "A-1H", 0.0, 0, 0 ),
                                            vector( "D-1H", 1.0, 20, 25 ) };

    const std::vector<QString> expected = { "C-1H", "D-1H", "B-1H", "E-2H", "A-1H", "Z-1H" };

    for ( bool reverse : { false, true } )
    {
        TableData tableData;
        tableData.vectorDataCollection = input;
        if ( reverse ) std::reverse( tableData.vectorDataCollection.begin(), tableData.vectorDataCollection.end() );

        sortVectorDataOnDate( tableData );

        EXPECT_EQ( expected, categoryNames( tableData.vectorDataCollection ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryTableToolsTest, OutsideFilterExcludesRowsWithAllValuesInRange )
{
    using namespace RimSummaryTableTools;

    const ValueFilter outside{ .type = ValueFilterType::OUTSIDE, .lowerThreshold = -50.0, .upperThreshold = 50.0 };

    EXPECT_FALSE( hasValueMatchingFilter( { 0.0, 0.0, 0.0 }, outside ) );
    EXPECT_FALSE( hasValueMatchingFilter( { -10.0, 0.0, 20.0 }, outside ) );
    EXPECT_TRUE( hasValueMatchingFilter( { 0.0, 80.0 }, outside ) );
    EXPECT_TRUE( hasValueMatchingFilter( { 0.0, -80.0 }, outside ) );

    // Values equal to a threshold are inside the range
    EXPECT_FALSE( hasValueMatchingFilter( { -50.0, 50.0 }, outside ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryTableToolsTest, ValueFilterTypes )
{
    using namespace RimSummaryTableTools;

    const ValueFilter none;
    const ValueFilter above{ .type = ValueFilterType::ABOVE, .lowerThreshold = 0.0 };
    const ValueFilter below{ .type = ValueFilterType::BELOW, .upperThreshold = 0.0 };
    const ValueFilter between{ .type = ValueFilterType::BETWEEN, .lowerThreshold = -50.0, .upperThreshold = 50.0 };

    // Without a filter, values above zero are significant
    EXPECT_FALSE( isValueMatchingFilter( 0.0, none ) );
    EXPECT_TRUE( isValueMatchingFilter( 0.1, none ) );

    // Above and below use strict comparison, so zero values do not match a zero threshold
    EXPECT_FALSE( isValueMatchingFilter( 0.0, above ) );
    EXPECT_TRUE( isValueMatchingFilter( 0.1, above ) );
    EXPECT_FALSE( isValueMatchingFilter( 0.0, below ) );
    EXPECT_TRUE( isValueMatchingFilter( -0.1, below ) );

    // Between includes the thresholds
    EXPECT_TRUE( isValueMatchingFilter( -50.0, between ) );
    EXPECT_TRUE( isValueMatchingFilter( 50.0, between ) );
    EXPECT_FALSE( isValueMatchingFilter( 50.1, between ) );
    EXPECT_FALSE( isValueMatchingFilter( -50.1, between ) );
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryTableToolsTest, LegacyThresholdKeepsValuesEqualToThreshold )
{
    using namespace RimSummaryTableTools;

    const auto filter = valueFilterFromLegacyThreshold( 50.0 );

    EXPECT_EQ( ValueFilterType::ABOVE, filter.type );
    EXPECT_TRUE( hasValueMatchingFilter( { 0.0, 50.0 }, filter ) );
    EXPECT_FALSE( hasValueMatchingFilter( { 0.0, 49.99 }, filter ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryTableToolsTest, ValuesForTimeStepsAreAlignedToTimeSteps )
{
    using namespace RimSummaryTableTools;

    const VectorData vectorData{ .category      = "A-1H",
                                 .name          = "WOPR",
                                 .values        = { 1.0, 2.0 },
                                 .firstTimeStep = 0,
                                 .lastTimeStep  = 0,
                                 .timeSteps     = { 20, 30 } };

    const auto values = valuesForTimeSteps( vectorData, { 10, 20, 25, 30, 40 } );

    ASSERT_EQ( 5u, values.size() );
    EXPECT_TRUE( std::isnan( values[0] ) );
    EXPECT_DOUBLE_EQ( 1.0, values[1] );
    EXPECT_TRUE( std::isnan( values[2] ) );
    EXPECT_DOUBLE_EQ( 2.0, values[3] );
    EXPECT_TRUE( std::isnan( values[4] ) );
}

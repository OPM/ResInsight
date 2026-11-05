#include "gtest/gtest.h"

#include "RimSummaryTableTools.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryTableToolsTest, SortVectorDataOnDateIsIndependentOfInputOrder )
{
    using namespace RimSummaryTableTools;

    auto vector = []( const QString& category, double value, time_t first, time_t last )
    { return VectorData{ .category = category, .name = "WOPR", .values = { value }, .firstTimeStep = first, .lastTimeStep = last }; };

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
TEST( RimSummaryTableToolsTest, LowerThresholdKeepsLargePositiveAndNegativeValues )
{
    using namespace RimSummaryTableTools;

    const std::vector<double> onlyZeros     = { 0.0, 0.0, 0.0 };
    const std::vector<double> smallValues   = { -10.0, 0.0, 20.0 };
    const std::vector<double> largePositive = { 0.0, 80.0 };
    const std::vector<double> largeNegative = { 0.0, -80.0 };

    EXPECT_FALSE( hasValueAboveThreshold( onlyZeros, 50.0, -50.0 ) );
    EXPECT_FALSE( hasValueAboveThreshold( smallValues, 50.0, -50.0 ) );
    EXPECT_TRUE( hasValueAboveThreshold( largePositive, 50.0, -50.0 ) );
    EXPECT_TRUE( hasValueAboveThreshold( largeNegative, 50.0, -50.0 ) );

    // Without lower threshold, negative values are not considered
    EXPECT_FALSE( hasValueAboveThreshold( largeNegative, 50.0 ) );

    // Without upper threshold, positive values are not considered
    EXPECT_FALSE( hasValueAboveThreshold( largePositive, std::nullopt, -50.0 ) );
    EXPECT_TRUE( hasValueAboveThreshold( largeNegative, std::nullopt, -50.0 ) );

    // Without any threshold, values above zero are significant
    EXPECT_FALSE( hasValueAboveThreshold( onlyZeros, std::nullopt, std::nullopt ) );
    EXPECT_TRUE( hasValueAboveThreshold( smallValues, std::nullopt, std::nullopt ) );
    EXPECT_FALSE( hasValueAboveThreshold( largeNegative, std::nullopt, std::nullopt ) );

    // Values equal to a threshold are included, consistent with the row filter in the Summary Table
    EXPECT_TRUE( hasValueAboveThreshold( { 50.0 }, 50.0, std::nullopt ) );
    EXPECT_TRUE( hasValueAboveThreshold( { -50.0 }, std::nullopt, -50.0 ) );
    EXPECT_FALSE( hasValueAboveThreshold( { 49.9, -49.9 }, 50.0, -50.0 ) );
}

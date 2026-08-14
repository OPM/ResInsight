#include "gtest/gtest.h"

#include "Histogram/RimGridStatisticsHistogramDataSource.h"
#include "Histogram/RimHistogramDataSource.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimHistogramDataSourceTest, CumulativeLineGraphRelativePercent )
{
    std::vector<size_t> bins = { 1, 2, 3 };

    std::vector<double> frequencies =
        RimHistogramDataSource::computeHistogramFrequencies( bins,
                                                             RimHistogramPlot::GraphType::LINE_GRAPH,
                                                             RimHistogramPlot::FrequencyType::RELATIVE_FREQUENCY_PERCENT,
                                                             true );

    ASSERT_EQ( 3u, frequencies.size() );
    EXPECT_NEAR( 100.0 / 6.0, frequencies[0], 1e-9 );
    EXPECT_NEAR( 50.0, frequencies[1], 1e-9 );
    EXPECT_NEAR( 100.0, frequencies[2], 1e-9 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimHistogramDataSourceTest, CumulativeLineGraphAbsolute )
{
    std::vector<size_t> bins = { 1, 2, 3 };

    std::vector<double> frequencies = RimHistogramDataSource::computeHistogramFrequencies( bins,
                                                                                           RimHistogramPlot::GraphType::LINE_GRAPH,
                                                                                           RimHistogramPlot::FrequencyType::ABSOLUTE_FREQUENCY,
                                                                                           true );

    ASSERT_EQ( 3u, frequencies.size() );
    EXPECT_NEAR( 1.0, frequencies[0], 1e-9 );
    EXPECT_NEAR( 3.0, frequencies[1], 1e-9 );
    EXPECT_NEAR( 6.0, frequencies[2], 1e-9 );
}

//--------------------------------------------------------------------------------------------------
/// The cumulative sum must be applied per bin before the bar graph expansion: the expanded vector
/// starts with a 0.0 closer and each cumulative bin value is duplicated. Unlike a regular
/// histogram, a cumulative curve has no closing 0.0 at the end: it ends at its maximum.
//--------------------------------------------------------------------------------------------------
TEST( RimHistogramDataSourceTest, CumulativeBarGraphAbsolute )
{
    std::vector<size_t> bins = { 1, 2, 3 };

    std::vector<double> frequencies = RimHistogramDataSource::computeHistogramFrequencies( bins,
                                                                                           RimHistogramPlot::GraphType::BAR_GRAPH,
                                                                                           RimHistogramPlot::FrequencyType::ABSOLUTE_FREQUENCY,
                                                                                           true );

    std::vector<double> expected = { 0.0, 1.0, 1.0, 3.0, 3.0, 6.0, 6.0 };
    ASSERT_EQ( expected.size(), frequencies.size() );
    for ( size_t i = 0; i < expected.size(); i++ )
    {
        EXPECT_NEAR( expected[i], frequencies[i], 1e-9 );
    }

    std::vector<double> xValues =
        RimHistogramDataSource::computeHistogramBins( 0.0, 3.0, static_cast<int>( bins.size() ), RimHistogramPlot::GraphType::BAR_GRAPH, true );
    EXPECT_EQ( frequencies.size(), xValues.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimHistogramDataSourceTest, NonCumulativeBarGraphBinsAndFrequenciesMatch )
{
    std::vector<size_t> bins = { 1, 2, 3 };

    std::vector<double> frequencies =
        RimHistogramDataSource::computeHistogramFrequencies( bins,
                                                             RimHistogramPlot::GraphType::BAR_GRAPH,
                                                             RimHistogramPlot::FrequencyType::ABSOLUTE_FREQUENCY );

    std::vector<double> xValues =
        RimHistogramDataSource::computeHistogramBins( 0.0, 3.0, static_cast<int>( bins.size() ), RimHistogramPlot::GraphType::BAR_GRAPH );
    EXPECT_EQ( frequencies.size(), xValues.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimHistogramDataSourceTest, NonCumulativeIsUnchanged )
{
    std::vector<size_t> bins = { 1, 2, 3 };

    std::vector<double> frequencies =
        RimHistogramDataSource::computeHistogramFrequencies( bins,
                                                             RimHistogramPlot::GraphType::LINE_GRAPH,
                                                             RimHistogramPlot::FrequencyType::ABSOLUTE_FREQUENCY );

    ASSERT_EQ( 3u, frequencies.size() );
    EXPECT_NEAR( 1.0, frequencies[0], 1e-9 );
    EXPECT_NEAR( 2.0, frequencies[1], 1e-9 );
    EXPECT_NEAR( 3.0, frequencies[2], 1e-9 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimHistogramDataSourceTest, LinearBinsUnchangedWithDefaultBinningMode )
{
    std::vector<double> xValues = RimHistogramDataSource::computeHistogramBins( 0.0, 10.0, 2, RimHistogramPlot::GraphType::BAR_GRAPH, false );

    std::vector<double> expected = { 0.0, 0.0, 5.0, 5.0, 10.0, 10.0 };
    ASSERT_EQ( expected.size(), xValues.size() );
    for ( size_t i = 0; i < expected.size(); i++ )
    {
        EXPECT_NEAR( expected[i], xValues[i], 1e-9 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimHistogramDataSourceTest, LogarithmicBinsBarGraph )
{
    std::vector<double> xValues = RimHistogramDataSource::computeHistogramBins( 1.0,
                                                                                1000.0,
                                                                                3,
                                                                                RimHistogramPlot::GraphType::BAR_GRAPH,
                                                                                false,
                                                                                RigHistogramCalculator::BinningMode::LOGARITHMIC );

    std::vector<double> expected = { 1.0, 1.0, 10.0, 10.0, 100.0, 100.0, 1000.0, 1000.0 };
    ASSERT_EQ( expected.size(), xValues.size() );
    for ( size_t i = 0; i < expected.size(); i++ )
    {
        EXPECT_NEAR( expected[i], xValues[i], 1e-9 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimHistogramDataSourceTest, LogarithmicBinsCumulativeBarGraph )
{
    std::vector<double> xValues = RimHistogramDataSource::computeHistogramBins( 1.0,
                                                                                1000.0,
                                                                                3,
                                                                                RimHistogramPlot::GraphType::BAR_GRAPH,
                                                                                true,
                                                                                RigHistogramCalculator::BinningMode::LOGARITHMIC );

    // A cumulative curve is not closed on the right side
    std::vector<double> expected = { 1.0, 1.0, 10.0, 10.0, 100.0, 100.0, 1000.0 };
    ASSERT_EQ( expected.size(), xValues.size() );
    for ( size_t i = 0; i < expected.size(); i++ )
    {
        EXPECT_NEAR( expected[i], xValues[i], 1e-9 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimHistogramDataSourceTest, LogarithmicBinsLineGraph )
{
    std::vector<double> xValues = RimHistogramDataSource::computeHistogramBins( 1.0,
                                                                                1000.0,
                                                                                3,
                                                                                RimHistogramPlot::GraphType::LINE_GRAPH,
                                                                                false,
                                                                                RigHistogramCalculator::BinningMode::LOGARITHMIC );

    // Bin centers are the geometric means of the bin edges
    std::vector<double> expected = { std::pow( 10.0, 0.5 ), std::pow( 10.0, 1.5 ), std::pow( 10.0, 2.5 ) };
    ASSERT_EQ( expected.size(), xValues.size() );
    for ( size_t i = 0; i < expected.size(); i++ )
    {
        EXPECT_NEAR( expected[i], xValues[i], 1e-9 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimHistogramDataSourceTest, ComputeBinRange )
{
    using BinRangeMode = RimHistogramDataSource::BinRangeMode;
    using BinningMode  = RigHistogramCalculator::BinningMode;

    const double smallestPositive = 0.01;

    {
        auto [min, max] =
            RimHistogramDataSource::computeBinRange( BinRangeMode::AUTOMATIC, 5.0, 6.0, -2.0, 100.0, BinningMode::LINEAR, smallestPositive );
        EXPECT_DOUBLE_EQ( -2.0, min );
        EXPECT_DOUBLE_EQ( 100.0, max );
    }

    {
        auto [min, max] =
            RimHistogramDataSource::computeBinRange( BinRangeMode::USER_DEFINED, 5.0, 6.0, -2.0, 100.0, BinningMode::LINEAR, smallestPositive );
        EXPECT_DOUBLE_EQ( 5.0, min );
        EXPECT_DOUBLE_EQ( 6.0, max );
    }

    {
        // A non-positive minimum is replaced by the smallest positive value for logarithmic binning
        auto [min, max] =
            RimHistogramDataSource::computeBinRange( BinRangeMode::AUTOMATIC, 5.0, 6.0, -2.0, 100.0, BinningMode::LOGARITHMIC, smallestPositive );
        EXPECT_DOUBLE_EQ( smallestPositive, min );
        EXPECT_DOUBLE_EQ( 100.0, max );
    }

    {
        auto [min, max] =
            RimHistogramDataSource::computeBinRange( BinRangeMode::USER_DEFINED, 0.0, 6.0, -2.0, 100.0, BinningMode::LOGARITHMIC, smallestPositive );
        EXPECT_DOUBLE_EQ( smallestPositive, min );
        EXPECT_DOUBLE_EQ( 6.0, max );
    }

    {
        // A positive minimum is used unchanged for logarithmic binning
        auto [min, max] =
            RimHistogramDataSource::computeBinRange( BinRangeMode::USER_DEFINED, 5.0, 6.0, -2.0, 100.0, BinningMode::LOGARITHMIC, smallestPositive );
        EXPECT_DOUBLE_EQ( 5.0, min );
        EXPECT_DOUBLE_EQ( 6.0, max );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimHistogramDataSourceTest, UserDefinedRangeFilterText )
{
    EXPECT_EQ( "Filter: User defined x-range [0.1..100]", RimHistogramDataSource::userDefinedRangeFilterText( 0.1, 100.0 ).toStdString() );
    EXPECT_EQ( "Filter: User defined x-range [-2.5..0]", RimHistogramDataSource::userDefinedRangeFilterText( -2.5, 0.0 ).toStdString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimHistogramDataSourceTest, FilterDescriptionsDefaultIsEmpty )
{
    RimGridStatisticsHistogramDataSource dataSource;
    EXPECT_TRUE( dataSource.filterDescriptions().empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimHistogramDataSourceTest, ShouldEnableLogarithmicBinning )
{
    // Selecting a logarithmic property enables logarithmic binning
    EXPECT_TRUE( RimGridStatisticsHistogramDataSource::shouldEnableLogarithmicBinning( "", "PERMX" ) );
    EXPECT_TRUE( RimGridStatisticsHistogramDataSource::shouldEnableLogarithmicBinning( "PORO", "PERMZ" ) );
    EXPECT_TRUE( RimGridStatisticsHistogramDataSource::shouldEnableLogarithmicBinning( "PORO", "TRANX" ) );
    EXPECT_TRUE( RimGridStatisticsHistogramDataSource::shouldEnableLogarithmicBinning( "PERMX", "PERMZ" ) );

    // Linear properties never enable logarithmic binning
    EXPECT_FALSE( RimGridStatisticsHistogramDataSource::shouldEnableLogarithmicBinning( "", "PORO" ) );
    EXPECT_FALSE( RimGridStatisticsHistogramDataSource::shouldEnableLogarithmicBinning( "PERMX", "PORO" ) );

    // An unchanged property must not re-enable logarithmic binning: the user stays in control
    EXPECT_FALSE( RimGridStatisticsHistogramDataSource::shouldEnableLogarithmicBinning( "PERMX", "PERMX" ) );
}

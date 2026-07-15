#include "gtest/gtest.h"

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

#include "gtest/gtest.h"

#include "RigEnsembleFractureStatisticsCalculator.h"
#include "RigStimPlanFractureDefinition.h"

#include <QStringList>

//--------------------------------------------------------------------------------------------------
/// A fracture definition without any conductivity result must not be indexed out of range
//--------------------------------------------------------------------------------------------------
TEST( RigEnsembleFractureStatisticsCalculatorTest, NoConductivityResultNames )
{
    cvf::ref<RigStimPlanFractureDefinition> definition = new RigStimPlanFractureDefinition;
    EXPECT_TRUE( definition->conductivityResultNames().isEmpty() );

    std::vector<cvf::ref<RigStimPlanFractureDefinition>> definitions = { definition };

    for ( auto propertyType : RigEnsembleFractureStatisticsCalculator::propertyTypes() )
    {
        if ( propertyType == RigEnsembleFractureStatisticsCalculator::PropertyType::FORMATION_DIP ) continue;

        auto values = RigEnsembleFractureStatisticsCalculator::calculateProperty( definitions, propertyType );
        EXPECT_TRUE( values.empty() );
    }
}

//--------------------------------------------------------------------------------------------------
/// The default binning arguments must produce an invalid histogram for empty input, for both the
/// default and the custom binning code paths
//--------------------------------------------------------------------------------------------------
TEST( RigEnsembleFractureStatisticsCalculatorTest, EmptyDefinitionsProduceInvalidHistogram )
{
    std::vector<cvf::ref<RigStimPlanFractureDefinition>> definitions;

    {
        RigHistogramData histogramData =
            RigEnsembleFractureStatisticsCalculator::createStatisticsData( definitions,
                                                                           RigEnsembleFractureStatisticsCalculator::PropertyType::HEIGHT,
                                                                           50 );
        EXPECT_FALSE( histogramData.isHistogramVectorValid() );
    }

    {
        RigHistogramData histogramData =
            RigEnsembleFractureStatisticsCalculator::createStatisticsData( definitions,
                                                                           RigEnsembleFractureStatisticsCalculator::PropertyType::HEIGHT,
                                                                           50,
                                                                           RigHistogramCalculator::BinningMode::LOGARITHMIC,
                                                                           std::make_pair( 1.0, 100.0 ),
                                                                           RigHistogramCalculator::OutOfRangeHandling::INCLUDE_IN_BOUNDARY_BINS );
        EXPECT_FALSE( histogramData.isHistogramVectorValid() );
    }
}

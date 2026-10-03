#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifStimPlanXmlReader.h"
#include "RigEnsembleFractureStatisticsCalculator.h"
#include "RigStimPlanFractureDefinition.h"

#include <QStringList>

static const QString CALCULATOR_TEST_DATA_DIRECTORY = QString( "%1/RifStimPlanXmlReader/" ).arg( TEST_DATA_DIR );

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
/// When none of the fracture definitions have a "width" result, the per-definition width grid batch
/// ends up empty while the conductivity grid batch does not. Computing WIDTH/PERMEABILITY statistics
/// must not index the empty width grid batch out of range.
//--------------------------------------------------------------------------------------------------
TEST( RigEnsembleFractureStatisticsCalculatorTest, CalculatePropertyWidthWithoutWidthDataDoesNotCrash )
{
    QString fileName = CALCULATOR_TEST_DATA_DIRECTORY + "small_fracture.xml";

    double                           conductivityScaleFactor = 1.0;
    RiaDefines::EclipseUnitSystem    unit                    = RiaDefines::EclipseUnitSystem::UNITS_METRIC;
    QString                          errorMessage;
    RifStimPlanXmlReader::MirrorMode mode = RifStimPlanXmlReader::MirrorMode::MIRROR_AUTO;

    cvf::ref<RigStimPlanFractureDefinition> fractureDataOne =
        RifStimPlanXmlReader::readStimPlanXMLFile( fileName, conductivityScaleFactor, mode, unit, &errorMessage );
    cvf::ref<RigStimPlanFractureDefinition> fractureDataTwo =
        RifStimPlanXmlReader::readStimPlanXMLFile( fileName, conductivityScaleFactor, mode, unit, &errorMessage );

    ASSERT_TRUE( fractureDataOne.notNull() );
    ASSERT_TRUE( fractureDataTwo.notNull() );
    ASSERT_FALSE( fractureDataOne->conductivityResultNames().isEmpty() );

    std::vector<cvf::ref<RigStimPlanFractureDefinition>> definitions = { fractureDataOne, fractureDataTwo };

    EXPECT_NO_FATAL_FAILURE(
        RigEnsembleFractureStatisticsCalculator::calculateProperty( definitions, RigEnsembleFractureStatisticsCalculator::PropertyType::WIDTH ) );
    EXPECT_NO_FATAL_FAILURE(
        RigEnsembleFractureStatisticsCalculator::calculateProperty( definitions,
                                                                    RigEnsembleFractureStatisticsCalculator::PropertyType::PERMEABILITY ) );

    auto widthValues =
        RigEnsembleFractureStatisticsCalculator::calculateProperty( definitions, RigEnsembleFractureStatisticsCalculator::PropertyType::WIDTH );
    EXPECT_TRUE( widthValues.empty() );
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

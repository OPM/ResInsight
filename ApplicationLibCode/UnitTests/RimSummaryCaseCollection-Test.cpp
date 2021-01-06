#include "gtest/gtest.h"

#include "RimSummaryCaseCollection.h"

#include <random>

#include <QDebug>

class RimSummaryCaseCollection_TESTER
{
public:
    static void test1();
};

TEST( RimSummaryCaseCollection, EnsembleParameter )
{
    RimSummaryCaseCollection_TESTER::test1();
}

void RimSummaryCaseCollection_TESTER::test1()
{
    std::random_device                     rd;
    std::mt19937                           gen( rd() );
    std::uniform_real_distribution<double> meanDistribution( -10000.0, 10000.0 );
    std::uniform_real_distribution<double> variationDistribution( 0.0, 5000.0 );
    std::uniform_int_distribution<size_t>  countDistribution( 1u, 1000u );
    size_t                                 N = 1000;

    std::vector<EnsembleParameter> parameters;
    for ( size_t i = 0; i < N; ++i )
    {
        EnsembleParameter param;
        param.type = EnsembleParameter::TYPE_NUMERIC;

        size_t valueCount = countDistribution( gen );

        double maxValue = -std::numeric_limits<double>::max();
        double minValue = std::numeric_limits<double>::max();

        // Add a few with zero variation
        if ( i % 100 )
        {
            double value = (double)i;
            maxValue     = value;
            minValue     = value;
            for ( size_t j = 0; j < valueCount; ++j )
            {
                param.values.push_back( value );
            }
        }
        else
        {
            double                                 meanValue = meanDistribution( gen );
            double                                 range     = variationDistribution( gen );
            std::uniform_real_distribution<double> valueDistribution( meanValue - range, meanValue + range );
            for ( size_t j = 0; j < valueCount; ++j )
            {
                double value = valueDistribution( gen );
                maxValue     = std::max( maxValue, value );
                minValue     = std::min( minValue, value );
                param.values.push_back( QVariant( value ) );
            }
        }

        param.minValue = minValue;
        param.maxValue = maxValue;

        double normStdDev = param.normalizedStdDeviation();
        EXPECT_GE( normStdDev, 0.0 );
        EXPECT_LE( normStdDev, std::sqrt( 2.0 ) );
        param.name = QString( "%1" ).arg( i );
        parameters.push_back( param );
    }

    size_t previousSize = parameters.size();
    RimSummaryCaseCollection::sortByBinnedVariation( parameters );
    size_t currentSize = parameters.size();
    EXPECT_EQ( previousSize, currentSize );

    int currentVariation = (int)EnsembleParameter::HIGH_VARIATION;
    for ( const EnsembleParameter& nameParamPair : parameters )
    {
        if ( nameParamPair.normalizedStdDeviation() == 0.0 )
        {
            EXPECT_EQ( nameParamPair.variationBin, (int)EnsembleParameter::NO_VARIATION );
        }
        else
        {
            EXPECT_GE( nameParamPair.variationBin, (int)EnsembleParameter::LOW_VARIATION );
        }
        EXPECT_LE( nameParamPair.variationBin, (int)EnsembleParameter::HIGH_VARIATION );
        EXPECT_LE( nameParamPair.variationBin, currentVariation );
        currentVariation = nameParamPair.variationBin;
    }
}

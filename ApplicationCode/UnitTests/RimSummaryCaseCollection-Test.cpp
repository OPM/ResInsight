#include "gtest/gtest.h"

#include "RimSummaryCaseCollection.h"

#include <random>

#include <QDebug>

TEST(RimSummaryCaseCollection, EnsembleParameter)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> meanDistribution(-10000.0, 10000.0);
    std::uniform_real_distribution<double> variationDistribution(0.0, 5000.0);
    std::uniform_int_distribution<size_t> countDistribution(1u, 1000u);
    size_t N = 1000;

    std::vector<EnsembleParameter::NameParameterPair> parameters;
    for (size_t i = 0; i < N; ++i)
    {
        EnsembleParameter param;
        param.type = EnsembleParameter::TYPE_NUMERIC;

        size_t valueCount = countDistribution(gen);
        double meanValue  = meanDistribution(gen);
        double range      = variationDistribution(gen);
        std::uniform_real_distribution<double> valueDistribution(meanValue - range, meanValue + range);
        double maxValue = -std::numeric_limits<double>::max();
        double minValue = std::numeric_limits<double>::max();
        for (size_t j = 0; j < valueCount; ++j)
        {
            double value = valueDistribution(gen);
            maxValue = std::max(maxValue, value);
            minValue = std::min(minValue, value);
            param.values.push_back(QVariant(value));
        }
        
        param.minValue = minValue;
        param.maxValue = maxValue;

        double normStdDev = param.normalizedStdDeviation();
        EXPECT_GE(normStdDev, 0.0);
        EXPECT_LE(normStdDev, std::sqrt(2.0));
        parameters.push_back(std::make_pair(QString("%1").arg(i), param));
    }
    size_t previousSize = parameters.size();
    EnsembleParameter::sortByBinnedVariation(parameters);
    size_t currentSize = parameters.size();
    EXPECT_EQ(previousSize, currentSize);

    int currentVariation = (int)EnsembleParameter::HIGH_VARIATION;
    for (const EnsembleParameter::NameParameterPair& nameParamPair : parameters)
    {
        EXPECT_GE(nameParamPair.second.variationBin, (int) EnsembleParameter::LOW_VARIATION);
        EXPECT_LE(nameParamPair.second.variationBin, (int) EnsembleParameter::HIGH_VARIATION);
        EXPECT_LE(nameParamPair.second.variationBin, currentVariation);
        currentVariation = nameParamPair.second.variationBin;
    }
}

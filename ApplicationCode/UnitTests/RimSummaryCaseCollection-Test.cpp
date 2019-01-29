#include "gtest/gtest.h"

#include "RimSummaryCaseCollection.h"

#include <random>

#include <QDebug>

TEST(RimSummaryCaseCollection, logarithmicVariationIndex)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> meanDistribution(-10000.0, 10000.0);
    std::uniform_real_distribution<double> variationDistribution(0.0, 5000.0);
    std::uniform_int_distribution<size_t> countDistribution(1u, 1000u);
    size_t N = 1000;
    std::map<int, size_t> indexCounts;
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
        
        param.calculateStdDeviation();
        param.minValue = minValue;
        param.maxValue = maxValue;
        int variationIndex = param.logarithmicVariationIndex();
        EXPECT_GE(variationIndex, -1);
        EXPECT_LE(variationIndex, 2);        
        indexCounts[variationIndex]++;
    }
    
    for (auto countPair : indexCounts)
    {
        qDebug() << "Variation index " << countPair.first << " count = " << countPair.second;
    }
}

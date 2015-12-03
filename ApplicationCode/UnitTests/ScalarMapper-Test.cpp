#include "gtest/gtest.h"

#include "cvfScalarMapperDiscreteLinear.h"

#include <QDebug>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperTest, TestHumanReadableTickmarks)
{
    cvf::ref<cvf::ScalarMapperDiscreteLinear> m_linDiscreteScalarMapper = new cvf::ScalarMapperDiscreteLinear;


    double adjustedMin = 0.0;
    double adjustedMax = 0.0;

    adjustedMin = 2141234;
    adjustedMax = 2165239;

    size_t m_numLevels = 10;

    m_linDiscreteScalarMapper->setRange(adjustedMin, adjustedMax);
    m_linDiscreteScalarMapper->setLevelCount(m_numLevels, true);

    std::vector<double> tickValues;
    m_linDiscreteScalarMapper->majorTickValues(&tickValues);

    for (size_t i = 0; i < tickValues.size(); i++)
    {
        qDebug() << i << "  " << tickValues[i];
    }
}


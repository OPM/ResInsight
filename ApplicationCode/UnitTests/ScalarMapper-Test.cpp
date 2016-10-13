#include "gtest/gtest.h"

#include "cvfScalarMapperDiscreteLinear.h"

#include <QDebug>

#include "RigFemPartResultsCollection.h"

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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OffshoreSphericalCoords, OffshoreSphericalCoords)
{
    {
        cvf::Vec3f vec(0, 0, 0);
        OffshoreSphericalCoords spCoord(vec);
        EXPECT_NEAR(spCoord.inc(), 0.0, 1e-10);
        EXPECT_NEAR(spCoord.azi(), 0.0, 1e-10);
        EXPECT_NEAR(spCoord.r(), 0.0, 1e-10);
    }

    {
        cvf::Vec3f vec(1, 0, 0);
        OffshoreSphericalCoords spCoord(vec);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.inc()), 90.0, 1e-10);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.azi()), 90.0, 1e-10);
        EXPECT_NEAR(spCoord.r(), 1.0, 1e-10);
    }

    {
        cvf::Vec3f vec(-1, 0, 0);
        OffshoreSphericalCoords spCoord(vec);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.inc()), 90.0, 1e-10);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.azi()), -90.0, 1e-10);
        EXPECT_NEAR(spCoord.r(), 1.0, 1e-10);
    }

    {
        cvf::Vec3f vec(0, 1, 0);
        OffshoreSphericalCoords spCoord(vec);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.inc()), 90.0, 1e-10);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.azi()),  0.0, 1e-10);
        EXPECT_NEAR(spCoord.r(), 1.0, 1e-10);
    }

    {
        cvf::Vec3f vec(0.000001f, -3, 0);
        OffshoreSphericalCoords spCoord(vec);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.inc()), 90.0, 1e-10);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.azi()), 179.9999, 1e-4);
        EXPECT_NEAR(spCoord.r(), 3.0, 1e-5);
    }
    {
        cvf::Vec3f vec(-0.000001f, -3, 0);
        OffshoreSphericalCoords spCoord(vec);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.inc()), 90.0, 1e-10);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.azi()), -179.9999, 1e-4);
        EXPECT_NEAR(spCoord.r(), 3.0, 1e-5);
    }

    {
        cvf::Vec3f vec(0, 0, 1);
        OffshoreSphericalCoords spCoord(vec);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.inc()), 180.0, 1e-10);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.azi()), 0.0, 1e-4);
        EXPECT_NEAR(spCoord.r(), 1.0, 1e-5);
    }

    {
        cvf::Vec3f vec(0, 0, -1);
        OffshoreSphericalCoords spCoord(vec);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.inc()), 0.0, 1e-10);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.azi()), 0.0, 1e-4);
        EXPECT_NEAR(spCoord.r(), 1.0, 1e-5);
    }

    {
        cvf::Vec3f vec(1, 0, -1);
        OffshoreSphericalCoords spCoord(vec);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.inc()), 45.0, 1e-5);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.azi()), 90.0, 1e-4);
        EXPECT_NEAR(spCoord.r(), sqrt(2), 1e-5);
    }

    {
        cvf::Vec3f vec(1.5f, 1.5f, 1.5f);
        OffshoreSphericalCoords spCoord(vec);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.inc()), 125.264396, 1e-5);
        EXPECT_NEAR(cvf::Math::toDegrees(spCoord.azi()), 45.0, 1e-4);
        EXPECT_NEAR(spCoord.r(), vec.length(), 1e-6);
    }

}


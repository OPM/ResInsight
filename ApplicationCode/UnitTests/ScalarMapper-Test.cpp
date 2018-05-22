#include "gtest/gtest.h"

#include "cvfScalarMapperDiscreteLinear.h"

#include <QDebug>

#include "RigFemPartResultsCollection.h"
#include "cafTickMarkGenerator.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperTest, TickMarkGenerator)
{
    EXPECT_EQ(10.0e6, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(5.5e6) );
    EXPECT_EQ(5.0e6, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(2.1e6) );
    EXPECT_EQ(2.0e6, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(1.1e6) );
    EXPECT_EQ(1.0e6, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(1.0e6) );
    EXPECT_EQ(1.0e6, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(0.9e6) );

    EXPECT_EQ(50.0, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(50.0) );
    EXPECT_EQ(50.0, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(21.00023) );
    EXPECT_EQ(20.0, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(11.2324556) );
    EXPECT_EQ(10.0, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(5.5) );
    EXPECT_EQ(5.0, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(2.1) );
    EXPECT_EQ(2.0, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(1.1) );
    EXPECT_EQ(1.0, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(1.0) );
    EXPECT_EQ(1.0, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(0.9) );
    EXPECT_EQ(1.0, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(0.7) );
    EXPECT_EQ(0.5, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(0.5) );
    EXPECT_EQ(0.5, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(0.435) );
    EXPECT_EQ(0.5, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(0.200001) );
    EXPECT_EQ(0.2, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(0.20000) );
    EXPECT_EQ(0.2, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(0.12) );
    EXPECT_EQ(0.1, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(0.056) );
  
    EXPECT_EQ(0.5e-6, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(0.200001e-6) );
    EXPECT_EQ(0.2e-6, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(0.20000e-6) );
    EXPECT_EQ(0.2e-6, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(0.12e-6) );
    EXPECT_EQ(0.1e-6, caf::TickMarkGenerator::roundUpToLog_1_2_5_10(0.056e-6) );

    {
        caf::TickMarkGenerator tickGen(1234.34521, 2346.67293, 104.2768);

        EXPECT_EQ(size_t(5), tickGen.tickMarkValues().size());

        EXPECT_EQ(1400, tickGen.tickMarkValues()[0]);
        EXPECT_EQ(1600, tickGen.tickMarkValues()[1]);
        EXPECT_EQ(1800, tickGen.tickMarkValues()[2]);
        EXPECT_EQ(2000, tickGen.tickMarkValues()[3]);
        EXPECT_EQ(2200, tickGen.tickMarkValues()[4]);
    }
    {
        caf::TickMarkGenerator tickGen(0.02134, 0.17829, 0.03267);

        EXPECT_EQ(size_t(3), tickGen.tickMarkValues().size());

        EXPECT_NEAR(0.05, tickGen.tickMarkValues()[0], 1e-15);
        EXPECT_NEAR(0.1, tickGen.tickMarkValues()[1], 1e-15);
        EXPECT_NEAR(0.15, tickGen.tickMarkValues()[2], 1e-15);
    }

    {
        caf::TickMarkGenerator tickGen(0.02134, 0.0335, 0.001267);

        EXPECT_EQ(size_t(6), tickGen.tickMarkValues().size());

        EXPECT_EQ(0.022, tickGen.tickMarkValues()[0]);
        EXPECT_EQ(0.024, tickGen.tickMarkValues()[1]);
        EXPECT_NEAR(0.026, tickGen.tickMarkValues()[2], 1e-15);
        EXPECT_EQ(0.028, tickGen.tickMarkValues()[3]);
        EXPECT_EQ(0.03, tickGen.tickMarkValues()[4]);
        EXPECT_EQ(0.032, tickGen.tickMarkValues()[5]);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ScalarMapperTest, TestHumanReadableTickmarks)
{
    cvf::ref<cvf::ScalarMapperDiscreteLinear> m_linDiscreteScalarMapper = new cvf::ScalarMapperDiscreteLinear;


    double adjustedMin = 2141234;
    double adjustedMax = 2165239;

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

#include "../cafTensor/cafTensor3.h"
#include "cvfMath.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Tensor, TensorRotation)
{

    {
        caf::Ten3f orgT(1.0f, 0.5f, 0.2f, 0, 0, 0);

        cvf::Mat3f rotMx = cvf::Mat3f::fromRotation(cvf::Vec3f(1, 0, 0), cvf::PI_F);

        caf::Ten3f rotT = orgT.rotated(rotMx);

        EXPECT_NEAR(1.0f, rotT[caf::Ten3f::SXX], 1e-4);
        EXPECT_NEAR(0.5f, rotT[caf::Ten3f::SYY], 1e-4);
        EXPECT_NEAR(0.2f, rotT[caf::Ten3f::SZZ], 1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SXY], 1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SZX], 1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SYZ], 1e-4);
    }

    {
        caf::Ten3f orgT(1.0f, 0.5f, 0.2f, 0, 0, 0);

        cvf::Mat3f rotMx = cvf::Mat3f::fromRotation(cvf::Vec3f(1, 0, 0), 0.5*cvf::PI_F);

        caf::Ten3f rotT = orgT.rotated(rotMx);

        EXPECT_NEAR(1.0f, rotT[caf::Ten3f::SXX], 1e-4);
        EXPECT_NEAR(0.2f, rotT[caf::Ten3f::SYY], 1e-4);
        EXPECT_NEAR(0.5f, rotT[caf::Ten3f::SZZ], 1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SXY], 1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SZX], 1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SYZ], 1e-4);
    }

    {
        caf::Ten3f orgT(1.0f, 0.5f, 0.2f, 0, 0, 0);

        cvf::Mat3f rotMx = cvf::Mat3f::fromRotation(cvf::Vec3f(0, 0, 1), 0.5*cvf::PI_F);

        caf::Ten3f rotT = orgT.rotated(rotMx);

        EXPECT_NEAR(0.5f, rotT[caf::Ten3f::SXX], 1e-4);
        EXPECT_NEAR(1.0f, rotT[caf::Ten3f::SYY], 1e-4);
        EXPECT_NEAR(0.2f, rotT[caf::Ten3f::SZZ], 1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SXY], 1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SZX], 1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SYZ], 1e-4);
    }

    {
        caf::Ten3f orgT(1.0f, 0.5f, 0.2f, 0, 0, 0);

        cvf::Mat3f rotMx = cvf::Mat3f::fromRotation(cvf::Vec3f(0, 0, 1), 0.25*cvf::PI_F);

        caf::Ten3f rotT = orgT.rotated(rotMx);

        EXPECT_NEAR(0.75f,rotT[caf::Ten3f::SXX],  1e-4);
        EXPECT_NEAR(0.75f,rotT[caf::Ten3f::SYY],  1e-4);
        EXPECT_NEAR(0.2f, rotT[caf::Ten3f::SZZ], 1e-4);
        EXPECT_NEAR(0.25f,rotT[caf::Ten3f::SXY],  1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SZX], 1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SYZ], 1e-4);
    }

    {
        caf::Ten3f orgT(0.75f, 0.75f, 0.2f, 0.25, 0, 0);

        cvf::Mat3f rotMx = cvf::Mat3f::fromRotation(cvf::Vec3f(0, 0, 1), -0.25*cvf::PI_F);

        caf::Ten3f rotT = orgT.rotated(rotMx);

        EXPECT_NEAR(1.0f, rotT[caf::Ten3f::SXX], 1e-4);
        EXPECT_NEAR(0.5f, rotT[caf::Ten3f::SYY], 1e-4);
        EXPECT_NEAR(0.2f, rotT[caf::Ten3f::SZZ], 1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SXY], 1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SZX], 1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SYZ], 1e-4);
    }

    {
        caf::Ten3f orgT(1.0f, 0.5f, 0.2f, 0, 0, 0);

        cvf::Mat3f rotMx = cvf::Mat3f::fromRotation(cvf::Vec3f(1, 1, 1), 0.2*cvf::PI_F);

        caf::Ten3f rotT = orgT.rotated(rotMx);

        EXPECT_NEAR(0.8320561f, rotT[caf::Ten3f::SXX], 1e-4);
        EXPECT_NEAR(0.5584094f, rotT[caf::Ten3f::SYY], 1e-4);
        EXPECT_NEAR(0.3095343f, rotT[caf::Ten3f::SZZ], 1e-4);
        EXPECT_NEAR(0.2091861f, rotT[caf::Ten3f::SXY], 1e-4);
        EXPECT_NEAR(-0.2258091f, rotT[caf::Ten3f::SZX], 1e-4);
        EXPECT_NEAR(0.0166228f,  rotT[caf::Ten3f::SYZ], 1e-4);
    }
    
    {
        caf::Ten3f orgT(0.8320561f, 0.5584094f, 0.3095343f, 0.2091861f, 0.0166228f, -0.2258091f);

        cvf::Mat3f rotMx = cvf::Mat3f::fromRotation(cvf::Vec3f(1, 1, 1), -0.2*cvf::PI_F);

        caf::Ten3f rotT = orgT.rotated(rotMx);

        EXPECT_NEAR(1.0f, rotT[caf::Ten3f::SXX], 1e-4);
        EXPECT_NEAR(0.5f, rotT[caf::Ten3f::SYY], 1e-4);
        EXPECT_NEAR(0.2f, rotT[caf::Ten3f::SZZ], 1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SXY], 1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SZX], 1e-4);
        EXPECT_NEAR(0.0f, rotT[caf::Ten3f::SYZ], 1e-4);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(Tensor, TensorAddScale)
{
    caf::Ten3f orgT1(1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f);
    caf::Ten3f orgT2(1.6f, 1.5f, 1.4f, 1.3f, 1.2f, 1.1f);

    caf::Ten3f newT = orgT1 + orgT2;

    EXPECT_NEAR(2.7f, newT[caf::Ten3f::SXX], 1e-4);
    EXPECT_NEAR(2.7f, newT[caf::Ten3f::SYY], 1e-4);
    EXPECT_NEAR(2.7f, newT[caf::Ten3f::SZZ], 1e-4);
    EXPECT_NEAR(2.7f, newT[caf::Ten3f::SXY], 1e-4);
    EXPECT_NEAR(2.7f, newT[caf::Ten3f::SZX], 1e-4);
    EXPECT_NEAR(2.7f, newT[caf::Ten3f::SYZ], 1e-4);

    newT = newT*0.5;

    EXPECT_NEAR(1.35f, newT[caf::Ten3f::SXX], 1e-4);
    EXPECT_NEAR(1.35f, newT[caf::Ten3f::SYY], 1e-4);
    EXPECT_NEAR(1.35f, newT[caf::Ten3f::SZZ], 1e-4);
    EXPECT_NEAR(1.35f, newT[caf::Ten3f::SXY], 1e-4);
    EXPECT_NEAR(1.35f, newT[caf::Ten3f::SZX], 1e-4);
    EXPECT_NEAR(1.35f, newT[caf::Ten3f::SYZ], 1e-4);

}
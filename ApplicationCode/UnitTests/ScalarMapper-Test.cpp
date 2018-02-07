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
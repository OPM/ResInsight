#include "gtest/gtest.h"

#include "RiaNumericalTools.h"

TEST( RiaNumericalTools, LogTenFunctions )
{
    {
        // Negative values will return zero
        double value = -0.0015;

        auto exponentCeil = RiaNumericalTools::computeTenExponentCeil( value );
        EXPECT_EQ( 0.0f, exponentCeil );

        auto exponentFloor = RiaNumericalTools::computeTenExponentFloor( value );
        EXPECT_EQ( 0.0f, exponentFloor );
    }

    {
        double value = 0.15;

        auto exponentCeil = RiaNumericalTools::computeTenExponentCeil( value );
        EXPECT_EQ( 0.0f, exponentCeil );

        auto exponentFloor = RiaNumericalTools::computeTenExponentFloor( value );
        EXPECT_EQ( -1.0f, exponentFloor );
    }

    {
        double value = 1.5;

        auto exponentCeil = RiaNumericalTools::computeTenExponentCeil( value );
        EXPECT_EQ( 1.0f, exponentCeil );

        auto exponentFloor = RiaNumericalTools::computeTenExponentFloor( value );
        EXPECT_EQ( 0.0f, exponentFloor );
    }

    {
        double value = 15;

        auto exponentCeil = RiaNumericalTools::computeTenExponentCeil( value );
        EXPECT_EQ( 2.0f, exponentCeil );

        auto exponentFloor = RiaNumericalTools::computeTenExponentFloor( value );
        EXPECT_EQ( 1.0f, exponentFloor );
    }
}

TEST( RiaNumericalTools, RoundToSignificant )
{
    struct TestValues
    {
        double value;
        double expectedCeil;
        double expectedFloor;
    };

    TestValues testValues[] = {
        { -0.00152, -0.0015, -0.0016 },
        { -15, -15, -15 },
        { -159, -150, -160 },
        { 0.02, 0.02, 0.02 },
        { 152, 160.0, 150.0 },
        { 1520, 1600.0, 1500.0 },
        { 15913, 16000.0, 15000.0 },
    };

    for ( const auto& testValue : testValues )
    {
        auto valueCeil = RiaNumericalTools::roundToNumSignificantDigitsCeil( testValue.value, 2 );
        EXPECT_EQ( testValue.expectedCeil, valueCeil );

        auto valueFloor = RiaNumericalTools::roundToNumSignificantDigitsFloor( testValue.value, 2 );
        EXPECT_EQ( testValue.expectedFloor, valueFloor );
    }
}

TEST( RiaNumericalTools, ValueInRange )
{
    struct TestValues
    {
        double value;
        double minimum;
        double maximum;
        bool   expectedResult;
    };

    TestValues testValues[] = {
        { 5, 0, 10, true },
        { -5, 0, 10, false },
        { 1.0, 1.0, 2.0, true },
        { 2.0, 1.0, 2.0, true },
        { 2.1, 1.0, 2.0, false },
    };

    for ( const auto& testValue : testValues )
    {
        auto isInRange = RiaNumericalTools::isValueInRange( testValue.value, std::pair( testValue.minimum, testValue.maximum ) );
        EXPECT_EQ( testValue.expectedResult, isInRange );
    }
}

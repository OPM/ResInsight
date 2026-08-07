#include "gtest/gtest.h"

#include "RiaNumericalTools.h"

#include <cmath>

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
        // Zero will return zero, as log10() is not defined for zero
        double value = 0.0;

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

TEST( RiaNumericalTools, ComputeTenExponentFloor )
{
    struct TestValues
    {
        double value;
        double expectedExponent;
    };

    TestValues testValues[] = {
        // Zero and negative values are not defined for log10(), and return zero
        { -1150.0, 0.0 },
        { -0.5, 0.0 },
        { 0.0, 0.0 },
        // Values below one give a negative exponent
        { 0.0005, -4.0 },
        { 0.05, -2.0 },
        { 0.1, -1.0 },
        { 0.5, -1.0 },
        // Values above one give a positive exponent
        { 1.0, 0.0 },
        { 9.99, 0.0 },
        { 10.0, 1.0 },
        { 1150.0, 3.0 },
    };

    for ( const auto& testValue : testValues )
    {
        auto exponentFloor = RiaNumericalTools::computeTenExponentFloor( testValue.value );
        EXPECT_EQ( testValue.expectedExponent, exponentFloor ) << "Failed for value " << testValue.value;

        // The lower limit of a logarithmic range is the closest power of ten below the value
        if ( testValue.value > 0.0 )
        {
            EXPECT_EQ( pow( 10.0, testValue.expectedExponent ), RiaNumericalTools::roundToClosestPowerOfTenFloor( testValue.value ) )
                << "Failed for value " << testValue.value;
        }
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

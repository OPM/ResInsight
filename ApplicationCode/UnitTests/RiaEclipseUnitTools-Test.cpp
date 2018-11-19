#include "gtest/gtest.h"

#include "RiaEclipseUnitTools.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaEclipseUnitTools, TestConversionToMeter)
{
    double deltaRange = 1e-7;

    {
        double sourceValue = RiaEclipseUnitTools::feetPerMeter();
        QString unitText = "ft";
        double destValue = RiaEclipseUnitTools::convertToMeter(sourceValue, unitText);
        EXPECT_NEAR(1.0, destValue, deltaRange);
    }

    {
        double sourceValue = RiaEclipseUnitTools::feetPerMeter() * 12.0;
        QString unitText = "in";
        double destValue = RiaEclipseUnitTools::convertToMeter(sourceValue, unitText);
        EXPECT_NEAR(1.0, destValue, deltaRange);
    }

    {
        double sourceValue = 1.0;
        QString unitText = "m";
        double destValue = RiaEclipseUnitTools::convertToMeter(sourceValue, unitText);
        EXPECT_NEAR(1.0, destValue, deltaRange);
    }

    {
        double sourceValue = 100.0;
        QString unitText = "cm";
        double destValue = RiaEclipseUnitTools::convertToMeter(sourceValue, unitText);
        EXPECT_NEAR(1.0, destValue, deltaRange);
    }

    {
        double sourceValue = 123.0;
        QString unitText = "mm";
        double destValue = RiaEclipseUnitTools::convertToMeter(sourceValue, unitText);
        EXPECT_NEAR(0.123, destValue, deltaRange);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaEclipseUnitTools, TestConversionToFeet)
{
    double deltaRange = 1e-7;

    {
        double sourceValue = 1.0;
        QString unitText = "ft";
        double destValue = RiaEclipseUnitTools::convertToFeet(sourceValue, unitText);
        EXPECT_NEAR(1.0, destValue, deltaRange);
    }

    {
        double sourceValue = 1.0 * 12.0;
        QString unitText = "in";
        double destValue = RiaEclipseUnitTools::convertToFeet(sourceValue, unitText);
        EXPECT_NEAR(1.0, destValue, deltaRange);
    }

    {
        double sourceValue = RiaEclipseUnitTools::meterPerFeet();
        QString unitText = "m";
        double destValue = RiaEclipseUnitTools::convertToFeet(sourceValue, unitText);
        EXPECT_NEAR(1.0, destValue, deltaRange);
    }

    {
        double sourceValue = RiaEclipseUnitTools::meterPerFeet() * 100.0;
        QString unitText = "cm";
        double destValue = RiaEclipseUnitTools::convertToFeet(sourceValue, unitText);
        EXPECT_NEAR(1.0, destValue, deltaRange);
    }
}

#include "gtest/gtest.h"

#include "RifEclipseSummaryAddress.h"
#include "RimCurveStyleSettings.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimCurveStyleSettingsTest, DefaultsControlColorByCase )
{
    RimCurveStyleSettings settings;

    EXPECT_EQ( RimCurveStyleSettings::Dimension::CASE, settings.colorDimension() );
    EXPECT_EQ( RimCurveStyleSettings::Dimension::NONE, settings.symbolDimension() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimCurveStyleSettingsTest, DimensionValues )
{
    using D = RimCurveStyleSettings::Dimension;

    const auto well   = RifEclipseSummaryAddress::wellAddress( "WOPR", "A-1" );
    const auto region = RifEclipseSummaryAddress::regionAddress( "ROPR", 3 );

    EXPECT_EQ( "A-1", RimCurveStyleSettings::dimensionValue( D::WELL, well, "case" ) );
    EXPECT_EQ( "case", RimCurveStyleSettings::dimensionValue( D::CASE, well, "case" ) );
    EXPECT_EQ( "WOPR", RimCurveStyleSettings::dimensionValue( D::VECTOR, well, "case" ) );
    EXPECT_TRUE( RimCurveStyleSettings::dimensionValue( D::REGION, well, "case" ).empty() );
    EXPECT_EQ( "3", RimCurveStyleSettings::dimensionValue( D::REGION, region, "case" ) );
}

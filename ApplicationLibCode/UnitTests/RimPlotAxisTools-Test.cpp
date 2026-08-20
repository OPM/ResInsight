#include "gtest/gtest.h"

#include "RimPlotAxisProperties.h"
#include "Tools/RimPlotAxisTools.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
/// The tick values for the range 8e9 - 1.1e10 displayed using a scale factor of 1e9
//--------------------------------------------------------------------------------------------------
static std::vector<double> tickValuesForScaledRange()
{
    return { 8.0e9, 8.5e9, 9.0e9, 9.5e9, 10.0e9, 10.5e9, 11.0e9 };
}

//--------------------------------------------------------------------------------------------------
/// The 'g' format used by the automatic number format interprets the precision as the number of
/// significant digits. Using the number of decimals directly caused 10.5 to be displayed as 11, and
/// the same label was displayed twice.
//--------------------------------------------------------------------------------------------------
TEST( RimPlotAxisTools, AutoFormatKeepsRequestedNumberOfDecimals )
{
    const std::vector<QString> expectedTexts = { "8", "8.5", "9", "9.5", "10", "10.5", "11" };

    const auto tickValues = tickValuesForScaledRange();
    ASSERT_EQ( expectedTexts.size(), tickValues.size() );

    for ( size_t i = 0; i < tickValues.size(); i++ )
    {
        auto text = RimPlotAxisTools::axisValueText( tickValues[i], 1.0e9, 2, RimPlotAxisProperties::NUMBER_FORMAT_AUTO );

        EXPECT_EQ( expectedTexts[i].toStdString(), text.toStdString() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Trailing zeros are not displayed by the automatic number format, and increasing the number of
/// decimals does not change the text for values that require fewer decimals
//--------------------------------------------------------------------------------------------------
TEST( RimPlotAxisTools, AutoFormatIsConcise )
{
    const auto autoFormat = RimPlotAxisProperties::NUMBER_FORMAT_AUTO;

    EXPECT_EQ( "10", RimPlotAxisTools::axisValueText( 10.0e9, 1.0e9, 2, autoFormat ).toStdString() );
    EXPECT_EQ( "10.5", RimPlotAxisTools::axisValueText( 10.5e9, 1.0e9, 4, autoFormat ).toStdString() );
    EXPECT_EQ( "1234.5", RimPlotAxisTools::axisValueText( 1234.5, 1.0, 2, autoFormat ).toStdString() );
    EXPECT_EQ( "0.25", RimPlotAxisTools::axisValueText( 0.25, 1.0, 2, autoFormat ).toStdString() );
    EXPECT_EQ( "-10.5", RimPlotAxisTools::axisValueText( -10.5, 1.0, 2, autoFormat ).toStdString() );
}

//--------------------------------------------------------------------------------------------------
/// The decimal and scientific formats interpret the precision as the number of decimals
//--------------------------------------------------------------------------------------------------
TEST( RimPlotAxisTools, DecimalAndScientificFormat )
{
    EXPECT_EQ( "10.50", RimPlotAxisTools::axisValueText( 10.5e9, 1.0e9, 2, RimPlotAxisProperties::NUMBER_FORMAT_DECIMAL ).toStdString() );
    EXPECT_EQ( "10.5", RimPlotAxisTools::axisValueText( 10.5e9, 1.0e9, 1, RimPlotAxisProperties::NUMBER_FORMAT_DECIMAL ).toStdString() );

    EXPECT_EQ( "1.05e+01", RimPlotAxisTools::axisValueText( 10.5e9, 1.0e9, 2, RimPlotAxisProperties::NUMBER_FORMAT_SCIENTIFIC ).toStdString() );
}

//--------------------------------------------------------------------------------------------------
/// Values close to zero are displayed as zero, and a scale factor of one leaves the value unchanged
//--------------------------------------------------------------------------------------------------
TEST( RimPlotAxisTools, ZeroAndUnitScaleFactor )
{
    const auto autoFormat = RimPlotAxisProperties::NUMBER_FORMAT_AUTO;

    EXPECT_EQ( "0", RimPlotAxisTools::axisValueText( 0.0, 1.0e9, 2, autoFormat ).toStdString() );
    EXPECT_EQ( "0", RimPlotAxisTools::axisValueText( 1.0e-20, 1.0, 2, autoFormat ).toStdString() );

    EXPECT_EQ( "8.5", RimPlotAxisTools::axisValueText( 8.5, 1.0, 2, autoFormat ).toStdString() );
}

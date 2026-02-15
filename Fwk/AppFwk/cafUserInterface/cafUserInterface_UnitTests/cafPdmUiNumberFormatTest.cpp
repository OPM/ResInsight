
#include "gtest/gtest.h"

#include "cafPdmUiNumberFormat.h"

using namespace caf;

TEST( PdmUiNumberFormatTest, ValueToTextFixed )
{
    QString result = PdmUiNumberFormat::valueToText( 3.14159, NumberFormatType::FIXED, 2 );
    EXPECT_EQ( result.toStdString(), "3.14" );

    result = PdmUiNumberFormat::valueToText( 0.0, NumberFormatType::FIXED, 3 );
    EXPECT_EQ( result.toStdString(), "0.000" );

    result = PdmUiNumberFormat::valueToText( -1.5, NumberFormatType::FIXED, 1 );
    EXPECT_EQ( result.toStdString(), "-1.5" );
}

TEST( PdmUiNumberFormatTest, ValueToTextScientific )
{
    QString result = PdmUiNumberFormat::valueToText( 1000.0, NumberFormatType::SCIENTIFIC, 2 );
    EXPECT_EQ( result.toStdString(), "1.00e+03" );

    result = PdmUiNumberFormat::valueToText( 0.001, NumberFormatType::SCIENTIFIC, 1 );
    EXPECT_EQ( result.toStdString(), "1.0e-03" );
}

TEST( PdmUiNumberFormatTest, ValueToTextAuto )
{
    QString result = PdmUiNumberFormat::valueToText( 42.0, NumberFormatType::AUTO, 6 );
    EXPECT_EQ( result.toStdString(), "42" );
}

TEST( PdmUiNumberFormatTest, SprintfFormatFixed )
{
    QString result = PdmUiNumberFormat::sprintfFormat( NumberFormatType::FIXED, 3 );
    EXPECT_EQ( result.toStdString(), "%.3f" );
}

TEST( PdmUiNumberFormatTest, SprintfFormatScientific )
{
    QString result = PdmUiNumberFormat::sprintfFormat( NumberFormatType::SCIENTIFIC, 2 );
    EXPECT_EQ( result.toStdString(), "%.2e" );
}

TEST( PdmUiNumberFormatTest, SprintfFormatAuto )
{
    QString result = PdmUiNumberFormat::sprintfFormat( NumberFormatType::AUTO, 4 );
    EXPECT_EQ( result.toStdString(), "%.4g" );
}

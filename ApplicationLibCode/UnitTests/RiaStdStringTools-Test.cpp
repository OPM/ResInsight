#include "gtest/gtest.h"

#include "RiaStdStringTools.h"

#include <QLocale>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStdStringToolsTest, ParseNumbers )
{
    auto decimalPoint = QLocale::c().decimalPoint().toLatin1();

    {
        std::string text = "8.73705e+06";

        EXPECT_TRUE( RiaStdStringTools::isNumber( text, decimalPoint ) );
    }

    {
        std::string text = "-8.73705e-06";

        EXPECT_TRUE( RiaStdStringTools::isNumber( text, decimalPoint ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStdStringToolsTest, EditDistance )
{
    // Equal string needs zero edits
    EXPECT_EQ( 0, RiaStdStringTools::computeEditDistance( "same", "same" ) );

    // Empty strings are also zero edits
    EXPECT_EQ( 0, RiaStdStringTools::computeEditDistance( "", "" ) );

    // Examples from wikipedia
    EXPECT_EQ( 3, RiaStdStringTools::computeEditDistance( "kitten", "sitting" ) );
    EXPECT_EQ( 3, RiaStdStringTools::computeEditDistance( "sitting", "kitten" ) );
    EXPECT_EQ( 3, RiaStdStringTools::computeEditDistance( "Saturday", "Sunday" ) );
    EXPECT_EQ( 3, RiaStdStringTools::computeEditDistance( "Sunday", "Saturday" ) );
}

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

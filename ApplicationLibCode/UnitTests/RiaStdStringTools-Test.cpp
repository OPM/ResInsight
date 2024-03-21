#include "gtest/gtest.h"

#include "RiaStdStringTools.h"

#include <QLocale>
#include <charconv>

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStdStringToolsTest, TrimStrings )
{
    // Test replace of tabs with space
    {
        std::string text = "test\t\tnext word";

        replace( text.begin(), text.end(), '\t', ' ' );

        std::string expectedText = "test  next word";
        EXPECT_EQ( text, expectedText );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStdStringToolsTest, TrimString )
{
    std::vector<std::pair<std::string, std::string>> testData = {
        std::make_pair( "   bla  ", "bla" ),
        std::make_pair( "bla ", "bla" ),
        std::make_pair( " bla", "bla" ),
        std::make_pair( "\tbla", "bla" ),
        std::make_pair( "\tbla \n\t", "bla" ),
        std::make_pair( "\tbla\v", "bla" ),
        std::make_pair( "bla", "bla" ),
        std::make_pair( "", "" ),
    };

    for ( auto [input, expectedText] : testData )
    {
        EXPECT_EQ( RiaStdStringTools::trimString( input ), expectedText );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStdStringToolsTest, LeftTrimString )
{
    std::vector<std::pair<std::string, std::string>> testData = {
        std::make_pair( "   bla  ", "bla  " ),
        std::make_pair( "bla ", "bla " ),
        std::make_pair( " bla", "bla" ),
        std::make_pair( "\tbla", "bla" ),
        std::make_pair( "\tbla \n\t", "bla \n\t" ),
        std::make_pair( "\tbla\v", "bla\v" ),
        std::make_pair( "bla", "bla" ),
        std::make_pair( "", "" ),
    };

    for ( auto [input, expectedText] : testData )
    {
        EXPECT_EQ( RiaStdStringTools::leftTrimString( input ), expectedText );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStdStringToolsTest, RightTrimString )
{
    std::vector<std::pair<std::string, std::string>> testData = {
        std::make_pair( "   bla  ", "   bla" ),
        std::make_pair( "bla ", "bla" ),
        std::make_pair( " bla", " bla" ),
        std::make_pair( "\tbla", "\tbla" ),
        std::make_pair( "\tbla \n\t", "\tbla" ),
        std::make_pair( "\tbla\v", "\tbla" ),
        std::make_pair( "bla", "bla" ),
        std::make_pair( "", "" ),
    };

    for ( auto [input, expectedText] : testData )
    {
        EXPECT_EQ( RiaStdStringTools::rightTrimString( input ), expectedText );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStdStringToolsTest, RemoveWhitespace )
{
    std::vector<std::pair<std::string, std::string>> testData = {
        std::make_pair( "   bla  ", "bla" ),
        std::make_pair( "bla ", "bla" ),
        std::make_pair( " bla", "bla" ),
        std::make_pair( "\tbla", "bla" ),
        std::make_pair( "\tbla \n\t ,bla", "bla,bla" ),
        std::make_pair( "\tbla\v", "bla" ),
        std::make_pair( "bla", "bla" ),
        std::make_pair( "", "" ),
    };

    for ( auto [input, expectedText] : testData )
    {
        EXPECT_EQ( RiaStdStringTools::removeWhitespace( input ), expectedText );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStdStringToolsTest, DISABLED_PerformanceConversion )
{
    size_t      itemCount     = 1000000;
    std::string valueAsString = "1234567";

    int intValue = 0;

    {
        auto start = std::chrono::high_resolution_clock::now();

        for ( size_t i = 0; i < itemCount; i++ )
        {
            intValue = std::stoi( valueAsString );
        }

        auto                          end  = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "std::stoi (no try/catch) " << std::setw( 9 ) << diff.count() << " s\n";
    }

    {
        auto start = std::chrono::high_resolution_clock::now();

        for ( size_t i = 0; i < itemCount; i++ )
        {
            try
            {
                intValue = std::stoi( valueAsString );
            }
            catch ( ... )
            {
            }
        }

        auto                          end  = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "std::stoi incl try/catch" << std::setw( 9 ) << diff.count() << " s\n";
    }

    {
        auto start = std::chrono::high_resolution_clock::now();

        for ( size_t i = 0; i < itemCount; i++ )
        {
            std::from_chars( valueAsString.data(), valueAsString.data() + valueAsString.size(), intValue );
        }

        auto                          end  = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "std::from_chars " << std::setw( 9 ) << diff.count() << " s\n";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStdStringToolsTest, ValuesFromRangeSelection )
{
    std::string   testString     = "1,2,5-10,15";
    std::set<int> expectedValues = { 1, 2, 5, 6, 7, 8, 9, 10, 15 };

    auto actualValues = RiaStdStringTools::valuesFromRangeSelection( testString );

    ASSERT_EQ( expectedValues, actualValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStdStringToolsTest, ValuesFromRangeSelectionMinMax )
{
    std::string   testString     = "-3, 5, 6-8, 10, 15-";
    int           minimumValue   = 1;
    int           maximumValue   = 20;
    std::set<int> expectedValues = { 1, 2, 3, 5, 6, 7, 8, 10, 15, 16, 17, 18, 19, 20 };

    auto actualValues = RiaStdStringTools::valuesFromRangeSelection( testString, minimumValue, maximumValue );

    ASSERT_EQ( expectedValues, actualValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStdStringToolsTest, TestInvalidRangeStrings )
{
    int minimumValue = 1;
    int maximumValue = 20;

    {
        // Handle blank space and inverted from/to
        std::string   testString     = "5,   -3,   9-8";
        std::set<int> expectedValues = { 1, 2, 3, 5, 8, 9 };
        auto          actualValues   = RiaStdStringTools::valuesFromRangeSelection( testString, minimumValue, maximumValue );

        ASSERT_EQ( expectedValues, actualValues );
    }

    {
        // If the range is invalid, the result should be an empty set
        std::string   testString     = "5, a";
        std::set<int> expectedValues = {};
        auto          actualValues   = RiaStdStringTools::valuesFromRangeSelection( testString, minimumValue, maximumValue );

        ASSERT_EQ( expectedValues, actualValues );
    }
}

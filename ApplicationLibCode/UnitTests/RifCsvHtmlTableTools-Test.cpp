#include "gtest/gtest.h"

#include "RifCsvHtmlTableTools.h"

TEST( RifCsvHtmlTableTools, GeneratesTableFromCommaSeparatedText )
{
    const QString content = "Well,Zone,Top,Base\n"
                            "A-1,Upper,100.0,110.0\n"
                            "A-1,Lower,110.0,120.0\n";

    auto result = RifCsvHtmlTableTools::generateHtmlTableFromText( content );
    ASSERT_TRUE( result.has_value() );

    EXPECT_TRUE( result->contains( "<th>Well</th>" ) );
    EXPECT_TRUE( result->contains( "<th>Zone</th>" ) );
    EXPECT_TRUE( result->contains( "<td>A-1</td>" ) );
    EXPECT_TRUE( result->contains( "<td>Upper</td>" ) );
    EXPECT_TRUE( result->contains( "<td align=right>100.0</td>" ) );
}

TEST( RifCsvHtmlTableTools, GeneratesTableFromSemicolonSeparatedText )
{
    const QString content = "Well;Zone;Top;Base\n"
                            "A-1;Upper;100.0;110.0\n";

    auto result = RifCsvHtmlTableTools::generateHtmlTableFromText( content );
    ASSERT_TRUE( result.has_value() );
    EXPECT_TRUE( result->contains( "<th>Well</th>" ) );
    EXPECT_TRUE( result->contains( "<td>A-1</td>" ) );
}

TEST( RifCsvHtmlTableTools, DetectsDelimiter )
{
    EXPECT_EQ( QChar( ',' ), RifCsvHtmlTableTools::detectDelimiter( "a,b,c" ) );
    EXPECT_EQ( QChar( ';' ), RifCsvHtmlTableTools::detectDelimiter( "a;b;c" ) );
    EXPECT_EQ( QChar( '\t' ), RifCsvHtmlTableTools::detectDelimiter( "a\tb\tc" ) );
}

TEST( RifCsvHtmlTableTools, EmptyContentGivesError )
{
    auto result = RifCsvHtmlTableTools::generateHtmlTableFromText( "" );
    EXPECT_FALSE( result.has_value() );
}

TEST( RifCsvHtmlTableTools, RespectsMaxRowCount )
{
    const QString content = "A,B\n1,2\n3,4\n5,6\n";

    auto result = RifCsvHtmlTableTools::generateHtmlTableFromText( content, 1 );
    ASSERT_TRUE( result.has_value() );

    EXPECT_TRUE( result->contains( "<td align=right>1</td>" ) );
    EXPECT_FALSE( result->contains( "<td align=right>3</td>" ) );
}

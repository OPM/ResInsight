#include "gtest/gtest.h"

#include "RifTextDataTableFormatter.h"

#include <QString>
#include <QStringList>

TEST( RifTextDataTableFormatter, BasicUsage )
{
    QString                   tableText;
    QTextStream               stream( &tableText );
    RifTextDataTableFormatter formatter( stream );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "Well" ),
        RifTextDataTableColumn( "Integer Number" ),
        RifTextDataTableColumn( "IntNumer 2" ),
        RifTextDataTableColumn( "IntNumer 3" ),
    };

    formatter.header( header );

    formatter.add( "well a" );
    formatter.add( 1 );
    formatter.add( 2 );
    formatter.add( 3 );
    formatter.rowCompleted();

    formatter.add( "well B" );
    formatter.add( 12 );
    formatter.add( 23 );
    formatter.add( 233 );
    formatter.rowCompleted();

    formatter.tableCompleted();

    std::cout << tableText.toStdString();
}

TEST( RifTextDataTableFormatter, NoPrefix )
{
    QString                   tableText;
    QTextStream               stream( &tableText );
    RifTextDataTableFormatter formatter( stream );

    formatter.setTableRowPrependText( "   " );
    formatter.setTableRowLineAppendText( "" );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "Well" ),
        RifTextDataTableColumn( "Integer Number", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "IntNumer 2", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "IntNumer 3", RifTextDataTableDoubleFormatting(), RIGHT ),
    };

    formatter.header( header );

    formatter.add( "well a" );
    formatter.add( 1 );
    formatter.add( 2 );
    formatter.add( 3 );
    formatter.rowCompleted();

    formatter.add( "well B" );
    formatter.add( 12 );
    formatter.add( 231 );
    formatter.add( 23123 );
    formatter.rowCompleted();

    formatter.tableCompleted();

    std::cout << tableText.toStdString();
}

TEST( RifTextDataTableFormatter, LongLine )
{
    QString                   tableText;
    QTextStream               stream( &tableText );
    RifTextDataTableFormatter formatter( stream );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "50 Character Well Name" ),
        RifTextDataTableColumn( "10 Int #1", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #2", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #3", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #4", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #5", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #6", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #7", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #8", RifTextDataTableDoubleFormatting(), RIGHT ),
    };

    formatter.header( header );
    QString fiftyCharacterWellName = "01234567890123456789012345678901234567890123456789";
    formatter.add( fiftyCharacterWellName );
    for ( int i = 0; i < 8; ++i )
    {
        formatter.add( std::numeric_limits<int>::max() ); // 10 characters
    }
    int fullLineLength = formatter.tableRowPrependText().length() + 9 * formatter.columnSpacing() + 50 + 8 * 10 +
                         formatter.tableRowAppendText().length();
    int tableWidth = formatter.tableWidth();
    EXPECT_EQ( tableWidth, fullLineLength );
    EXPECT_GT( tableWidth, formatter.maxDataRowWidth() );

    formatter.rowCompleted();
    formatter.tableCompleted();

    QStringList tableLines = tableText.split( QRegExp( "[\r\n]" ), QString::SkipEmptyParts );
    for ( QString line : tableLines )
    {
        std::cout << QString( "Line: \"%1\"" ).arg( line ).toStdString() << std::endl;
        if ( !line.startsWith( formatter.commentPrefix() ) )
        {
            EXPECT_LE( line.length(), formatter.maxDataRowWidth() );
        }
    }
}

TEST( RifTextDataTableFormatter, LongLine132 )
{
    QString                   tableText;
    QTextStream               stream( &tableText );
    RifTextDataTableFormatter formatter( stream );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "10 Char" ),
        RifTextDataTableColumn( "10 Int #1", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #2", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #3", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #4", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #5", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #6", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #7", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "I", RifTextDataTableDoubleFormatting(), RIGHT ),
    };

    formatter.header( header );
    QString tenCharacterWellName = "0123456789";
    formatter.add( tenCharacterWellName );
    for ( int i = 0; i < 7; ++i )
    {
        formatter.add( std::numeric_limits<int>::max() ); // 10 characters
    }
    formatter.add( 11 );

    int fullLineLength = formatter.tableRowPrependText().length() + 9 * formatter.columnSpacing() + 10 + 7 * 10 + 2 +
                         formatter.tableRowAppendText().length();
    int tableWidth = formatter.tableWidth();
    EXPECT_GE( tableWidth, fullLineLength );
    EXPECT_EQ( formatter.maxDataRowWidth(), fullLineLength );

    formatter.rowCompleted();
    formatter.tableCompleted();

    QStringList tableLines = tableText.split( QRegExp( "[\r\n]" ), QString::SkipEmptyParts );
    for ( QString line : tableLines )
    {
        std::cout << QString( "Line: \"%1\"" ).arg( line ).toStdString() << std::endl;
        if ( line.startsWith( "0" ) )
        {
            EXPECT_EQ( line.length(), formatter.maxDataRowWidth() );
        }
    }
}

TEST( RifTextDataTableFormatter, LongLine133 )
{
    QString                   tableText;
    QTextStream               stream( &tableText );
    RifTextDataTableFormatter formatter( stream );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "10 Char" ),
        RifTextDataTableColumn( "10 Int #1", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #2", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #3", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #4", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #5", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #6", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "10 Int #7", RifTextDataTableDoubleFormatting(), RIGHT ),
        RifTextDataTableColumn( "I", RifTextDataTableDoubleFormatting(), RIGHT ),
    };

    formatter.header( header );
    QString fiftyCharacterWellName = "0123456789";
    formatter.add( fiftyCharacterWellName );
    for ( int i = 0; i < 7; ++i )
    {
        formatter.add( std::numeric_limits<int>::max() ); // 10 characters
    }
    formatter.add( 111 );

    int fullLineLength = formatter.tableRowPrependText().length() + 9 * formatter.columnSpacing() + 10 + 7 * 10 + 3 +
                         formatter.tableRowAppendText().length();
    int tableWidth = formatter.tableWidth();
    EXPECT_GE( tableWidth, fullLineLength );
    EXPECT_LT( formatter.maxDataRowWidth(), fullLineLength );

    formatter.rowCompleted();
    formatter.tableCompleted();

    QStringList tableLines = tableText.split( QRegExp( "[\r\n]" ), QString::SkipEmptyParts );
    for ( QString line : tableLines )
    {
        std::cout << QString( "Line: \"%1\"" ).arg( line ).toStdString() << std::endl;
        if ( line.startsWith( "0" ) )
        {
            EXPECT_LE( line.length(), formatter.maxDataRowWidth() );
        }
    }
}

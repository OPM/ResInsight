#include "gtest/gtest.h"

#include "RifReportTableFormatter.h"

#include <limits>

namespace
{
RiaReportTable createTable()
{
    RiaReportTable table;
    table.title   = "Rates";
    table.columns = { RiaReportColumn{ .title = "Well", .alignment = LEFT },
                      RiaReportColumn{ .title = "Rate", .unit = "Sm3/d", .doubleFormat = RifTextDataTableDoubleFormatting( RIF_FLOAT, 2 ) } };
    table.rows    = { { QString( "A-1" ), 1234.5678 }, { QString( "B-2" ), std::numeric_limits<double>::quiet_NaN() } };
    return table;
}
} // namespace

TEST( RifReportTableFormatter, FormatsCellsWithColumnPrecisionAndOverrides )
{
    RiaReportColumn column{ .title = "Value", .doubleFormat = RifTextDataTableDoubleFormatting( RIF_FLOAT, 2 ) };

    RifReportFormatOptions options;
    EXPECT_EQ( "3.14", RifReportTableFormatter::formatCell( 3.14159, column, options ) );
    EXPECT_EQ( "42", RifReportTableFormatter::formatCell( 42, column, options ) );
    EXPECT_EQ( "text", RifReportTableFormatter::formatCell( QString( "text" ), column, options ) );

    options.doublePrecision = 4;
    EXPECT_EQ( "3.1416", RifReportTableFormatter::formatCell( 3.14159, column, options ) );

    options.doubleFormat    = RifReportFormatOptions::DoubleFormat::SCIENTIFIC;
    options.doublePrecision = 2;
    EXPECT_EQ( "1.23E+03", RifReportTableFormatter::formatCell( 1234.5, column, options ) );

    options.doubleFormat    = RifReportFormatOptions::DoubleFormat::CONCISE;
    options.doublePrecision = 3;
    EXPECT_EQ( "1.23e+03", RifReportTableFormatter::formatCell( 1234.5, column, options ) );
}

TEST( RifReportTableFormatter, FormatsUndefinedValues )
{
    RiaReportColumn        column{ .title = "Value" };
    RifReportFormatOptions options;
    options.undefinedValueText = "N/A";

    EXPECT_EQ( "N/A", RifReportTableFormatter::formatCell( RiaReportCellValue(), column, options ) );
    EXPECT_EQ( "N/A", RifReportTableFormatter::formatCell( std::numeric_limits<double>::quiet_NaN(), column, options ) );
    EXPECT_EQ( "N/A", RifReportTableFormatter::formatCell( std::numeric_limits<double>::infinity(), column, options ) );
    EXPECT_EQ( "N/A", RifReportTableFormatter::formatCell( QDateTime(), column, options ) );
}

TEST( RifReportTableFormatter, FormatsDates )
{
    RiaReportColumn        column{ .title = "Date" };
    RifReportFormatOptions options;

    const QDateTime midnight( QDate( 2026, 1, 31 ), QTime( 0, 0 ) );
    const QDateTime withTime( QDate( 2026, 1, 31 ), QTime( 12, 30, 15 ) );
    EXPECT_EQ( "2026-01-31", RifReportTableFormatter::formatCell( midnight, column, options ) );
    EXPECT_EQ( "2026-01-31 12:30:15", RifReportTableFormatter::formatCell( withTime, column, options ) );

    options.dateFormat = "dd.MM.yyyy";
    EXPECT_EQ( "31.01.2026", RifReportTableFormatter::formatCell( withTime, column, options ) );
}

TEST( RifReportTableFormatter, CsvUsesDelimiterAndEscapesFields )
{
    RiaReportTable table;
    table.columns = { RiaReportColumn{ .title = "Name" }, RiaReportColumn{ .title = "Value", .unit = "m" } };
    table.rows    = { { QString( "a;b" ), 1 }, { QString( "say \"hi\"" ), 2 } };

    RifReportFormatOptions options;
    options.delimiter = ";";
    EXPECT_EQ( "Name;Value [m]\n\"a;b\";1\n\"say \"\"hi\"\"\";2\n", RifReportTableFormatter::toCsv( table, options ) );

    options.delimiter = "\t";
    EXPECT_EQ( "Name\tValue [m]\na;b\t1\n\"say \"\"hi\"\"\"\t2\n", RifReportTableFormatter::toCsv( table, options ) );
}

TEST( RifReportTableFormatter, TextIsColumnAligned )
{
    const QString text = RifReportTableFormatter::toText( createTable(), RifReportFormatOptions() );

    EXPECT_EQ( "Rates\n"
               "\n"
               "Well         Rate\n"
               "          [Sm3/d]\n"
               "A-1       1234.57\n"
               "B-2             -\n"
               "\n",
               text.toStdString() );
}

TEST( RifReportTableFormatter, HtmlEscapesTextAndShowsUnits )
{
    RiaReportTable table;
    table.title   = "<Title>";
    table.columns = { RiaReportColumn{ .title = "A&B", .unit = "m<3>", .alignment = LEFT } };
    table.rows    = { { QString( "<script>" ) } };

    const QString html = RifReportTableFormatter::toHtml( table, RifReportFormatOptions() );

    EXPECT_TRUE( html.contains( "&lt;Title&gt;" ) );
    EXPECT_TRUE( html.contains( "A&amp;B<br><span class=\"unit\">m&lt;3&gt;</span>" ) );
    EXPECT_TRUE( html.contains( "&lt;script&gt;" ) );
    EXPECT_FALSE( html.contains( "<script>" ) );
}

TEST( RifReportTableFormatter, HtmlShowsHeadAndTailOfLargeTables )
{
    RiaReportTable table;
    table.columns = { RiaReportColumn{ .title = "Index" } };
    for ( int i = 0; i < 20; i++ )
    {
        table.rows.push_back( { i } );
    }

    RifReportFormatOptions options;
    options.htmlHeadRowCount = 5;
    options.htmlTailRowCount = 3;

    const auto range = RifReportTableFormatter::htmlRowRange( table.rows.size(), options );
    EXPECT_EQ( 5u, range.headRowCount );
    EXPECT_EQ( 12u, range.omittedRowCount );
    EXPECT_EQ( 3u, range.tailRowCount );
    EXPECT_EQ( 17u, range.tailStartRowIndex );

    const QString html = RifReportTableFormatter::toHtml( table, options );
    EXPECT_TRUE( html.contains( ">4</td>" ) );
    EXPECT_FALSE( html.contains( ">5</td>" ) );
    EXPECT_FALSE( html.contains( ">16</td>" ) );
    EXPECT_TRUE( html.contains( ">17</td>" ) );
    EXPECT_TRUE( html.contains( ">19</td>" ) );
    EXPECT_TRUE( html.contains( "... 12 rows omitted ..." ) );

    // Text and CSV always contain all rows
    const QString csv = RifReportTableFormatter::toCsv( table, options );
    EXPECT_EQ( 21, csv.count( '\n' ) );
}

TEST( RifReportTableFormatter, HtmlRowRangeKeepsAllRowsForSmallTables )
{
    RifReportFormatOptions options;
    options.htmlHeadRowCount = 5;
    options.htmlTailRowCount = 3;

    const auto range = RifReportTableFormatter::htmlRowRange( 8, options );
    EXPECT_EQ( 8u, range.headRowCount );
    EXPECT_EQ( 0u, range.omittedRowCount );
    EXPECT_EQ( 0u, range.tailRowCount );
}

TEST( RifReportTableFormatter, CsvOfSingleTableSectionHasNoTitleLines )
{
    RiaReportSection section{ .title = "Section", .tables = { createTable() } };

    RifReportFormatOptions options;
    const QString          csv = RifReportTableFormatter::format( "Report", { section }, RifReportTableFormatter::Format::CSV, options );
    EXPECT_EQ( "Well,Rate [Sm3/d]\nA-1,1234.57\nB-2,-\n", csv );

    section.tables.push_back( createTable() );
    const QString csvTwoTables = RifReportTableFormatter::format( "Report", { section }, RifReportTableFormatter::Format::CSV, options );
    EXPECT_TRUE( csvTwoTables.startsWith( "Section - Rates\nWell,Rate [Sm3/d]\n" ) );
}

TEST( RifReportTableFormatter, LoadsPendingSectionTablesWhenFormatting )
{
    int factoryCallCount = 0;

    RiaReportSection section;
    section.title        = "Lazy";
    section.tableFactory = [&factoryCallCount]()
    {
        factoryCallCount++;
        return std::vector<RiaReportTable>{ createTable() };
    };

    RifReportFormatOptions options;
    const QString          csv = RifReportTableFormatter::format( QString(), { section }, RifReportTableFormatter::Format::CSV, options );

    EXPECT_EQ( 1, factoryCallCount );
    EXPECT_TRUE( csv.contains( "A-1,1234.57" ) );
    EXPECT_TRUE( section.hasPendingTables() );

    section.loadTables();
    EXPECT_EQ( 2, factoryCallCount );
    EXPECT_FALSE( section.hasPendingTables() );
    EXPECT_EQ( 1u, section.tables.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifReportTableFormatter, FiltersAndSortsRows )
{
    RiaReportTable table;
    table.columns = { RiaReportColumn{ .title = "Well", .alignment = LEFT }, RiaReportColumn{ .title = "Rate" } };
    table.rows    = { { QString( "P-10" ), 3.0 },
                      { QString( "P-2" ), std::numeric_limits<double>::quiet_NaN() },
                      { QString( "I-1" ), 1 },
                      { QString( "P-1" ), 2.5 } };

    const RifReportFormatOptions options;

    EXPECT_EQ( ( std::vector<size_t>{ 0, 1, 2, 3 } ), RifReportTableFormatter::filteredAndSortedRows( table, "", -1, true, options ) );

    // Text is sorted in natural order
    EXPECT_EQ( ( std::vector<size_t>{ 2, 3, 1, 0 } ), RifReportTableFormatter::filteredAndSortedRows( table, "", 0, true, options ) );

    // Undefined values are last in both directions
    EXPECT_EQ( ( std::vector<size_t>{ 2, 3, 0, 1 } ), RifReportTableFormatter::filteredAndSortedRows( table, "", 1, true, options ) );
    EXPECT_EQ( ( std::vector<size_t>{ 0, 3, 2, 1 } ), RifReportTableFormatter::filteredAndSortedRows( table, "", 1, false, options ) );

    // The filter is case insensitive and is applied before sorting
    EXPECT_EQ( ( std::vector<size_t>{ 0, 3, 1 } ), RifReportTableFormatter::filteredAndSortedRows( table, " p-", 1, false, options ) );
    EXPECT_TRUE( RifReportTableFormatter::filteredAndSortedRows( table, "no match", 0, true, options ).empty() );
}

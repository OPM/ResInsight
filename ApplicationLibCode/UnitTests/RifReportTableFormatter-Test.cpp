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

TEST( RifReportTableFormatter, TextIsTabSeparated )
{
    const QString text = RifReportTableFormatter::toText( createTable(), RifReportFormatOptions() );

    EXPECT_EQ( "Rates\n"
               "\n"
               "Well\tRate [Sm3/d]\n"
               "A-1\t1234.57\n"
               "B-2\t-\n",
               text.toStdString() );
}

TEST( RifReportTableFormatter, DisplayRowRangeShowsHeadAndTailOfLargeTables )
{
    RifReportFormatOptions options;
    options.displayHeadRowCount = 5;
    options.displayTailRowCount = 3;

    const auto range = RifReportTableFormatter::displayRowRange( 20, options );
    EXPECT_EQ( 5u, range.headRowCount );
    EXPECT_EQ( 12u, range.omittedRowCount );
    EXPECT_EQ( 3u, range.tailRowCount );
    EXPECT_EQ( 17u, range.tailStartRowIndex );

    const auto smallRange = RifReportTableFormatter::displayRowRange( 8, options );
    EXPECT_EQ( 8u, smallRange.headRowCount );
    EXPECT_EQ( 0u, smallRange.omittedRowCount );
    EXPECT_EQ( 0u, smallRange.tailRowCount );
}

TEST( RifReportTableFormatter, ColumnHeaderTextKeepsPercentPlaceholders )
{
    RiaReportColumn column{ .title = "Ratio %2", .unit = "%" };
    EXPECT_EQ( "Ratio %2 [%]", RifReportTableFormatter::columnHeaderText( column ) );
}

TEST( RifReportTableFormatter, CsvOfSingleTableSectionHasNoTitleLines )
{
    RiaReportSection section{ .title = "Section", .tables = { createTable() } };

    RifReportFormatOptions options;
    const QString          csv = RifReportTableFormatter::format( "Report", { section }, RifReportTableFormatter::Format::CSV, options );
    EXPECT_EQ( "Well,Rate [Sm3/d]\nA-1,1234.57\nB-2,\n", csv );

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
TEST( RifReportTableFormatter, SortsRows )
{
    RiaReportTable table;
    table.columns = { RiaReportColumn{ .title = "Well", .alignment = LEFT }, RiaReportColumn{ .title = "Rate" } };
    table.rows    = { { QString( "P-10" ), 3.0 },
                      { QString( "P-2" ), std::numeric_limits<double>::quiet_NaN() },
                      { QString( "I-1" ), 1 },
                      { QString( "P-1" ), 2.5 } };

    EXPECT_EQ( ( std::vector<size_t>{ 0, 1, 2, 3 } ), RifReportTableFormatter::sortedRows( table, -1, true ) );

    // Text is sorted in natural order
    EXPECT_EQ( ( std::vector<size_t>{ 2, 3, 1, 0 } ), RifReportTableFormatter::sortedRows( table, 0, true ) );

    // Undefined values are last in both directions
    EXPECT_EQ( ( std::vector<size_t>{ 2, 3, 0, 1 } ), RifReportTableFormatter::sortedRows( table, 1, true ) );
    EXPECT_EQ( ( std::vector<size_t>{ 0, 3, 2, 1 } ), RifReportTableFormatter::sortedRows( table, 1, false ) );
}

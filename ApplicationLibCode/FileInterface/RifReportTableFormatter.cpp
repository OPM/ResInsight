/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RifReportTableFormatter.h"

#include "RifCsvDataTableFormatter.h"
#include "RifTextDataTableFormatter.h"

#include <QCollator>
#include <QTextStream>

#include <algorithm>
#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifReportTableFormatter::formatCell( const RiaReportCellValue& value, const RiaReportColumn& column, const RifReportFormatOptions& options )
{
    if ( isUndefined( value ) ) return options.undefinedValueText;

    if ( const auto* doubleValue = std::get_if<double>( &value ) )
    {
        const auto formatting = effectiveDoubleFormatting( column, options );
        switch ( formatting.format )
        {
            case RIF_SCIENTIFIC:
                return QString::number( *doubleValue, 'E', formatting.precision );
            case RIF_CONSISE:
                return QString::number( *doubleValue, 'g', std::max( 1, formatting.precision ) );
            case RIF_FLOAT:
            default:
                return QString::number( *doubleValue, 'f', formatting.precision );
        }
    }

    if ( const auto* intValue = std::get_if<int>( &value ) ) return QString::number( *intValue );

    if ( const auto* textValue = std::get_if<QString>( &value ) ) return *textValue;

    if ( const auto* dateTimeValue = std::get_if<QDateTime>( &value ) )
    {
        if ( !options.dateFormat.isEmpty() ) return dateTimeValue->toString( options.dateFormat );

        const bool isMidnight = dateTimeValue->time() == QTime( 0, 0 );
        return dateTimeValue->toString( isMidnight ? "yyyy-MM-dd" : "yyyy-MM-dd hh:mm:ss" );
    }

    return options.undefinedValueText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReportTableFormatter::isUndefined( const RiaReportCellValue& value )
{
    if ( std::holds_alternative<std::monostate>( value ) ) return true;
    if ( const auto* doubleValue = std::get_if<double>( &value ) ) return !std::isfinite( *doubleValue );
    if ( const auto* dateTimeValue = std::get_if<QDateTime>( &value ) ) return !dateTimeValue->isValid();

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReportTableFormatter::isNumeric( const RiaReportCellValue& value )
{
    return std::holds_alternative<double>( value ) || std::holds_alternative<int>( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifReportTableFormatter::columnHeaderText( const RiaReportColumn& column )
{
    if ( column.unit.isEmpty() ) return column.title;

    return QString( "%1 [%2]" ).arg( column.title ).arg( column.unit );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReportRowRange RifReportTableFormatter::htmlRowRange( size_t rowCount, const RifReportFormatOptions& options )
{
    const size_t headCount = static_cast<size_t>( std::max( 0, options.htmlHeadRowCount ) );
    const size_t tailCount = static_cast<size_t>( std::max( 0, options.htmlTailRowCount ) );

    RifReportRowRange range;
    if ( rowCount <= headCount + tailCount )
    {
        range.headRowCount = rowCount;
        return range;
    }

    range.headRowCount      = headCount;
    range.tailRowCount      = tailCount;
    range.omittedRowCount   = rowCount - headCount - tailCount;
    range.tailStartRowIndex = rowCount - tailCount;

    return range;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifReportTableFormatter::omittedRowsText( size_t omittedRowCount )
{
    return QString( "... %1 rows omitted ..." ).arg( omittedRowCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RifReportTableFormatter::filteredAndSortedRows( const RiaReportTable&         table,
                                                                    const QString&                filterText,
                                                                    int                           sortColumn,
                                                                    bool                          ascending,
                                                                    const RifReportFormatOptions& options )
{
    const QString filter = filterText.trimmed();

    auto cellValue = []( const std::vector<RiaReportCellValue>& row, size_t column ) -> RiaReportCellValue
    { return column < row.size() ? row[column] : RiaReportCellValue(); };

    std::vector<size_t> rowIndices;
    rowIndices.reserve( table.rows.size() );
    for ( size_t rowIndex = 0; rowIndex < table.rows.size(); rowIndex++ )
    {
        bool isMatch = filter.isEmpty();
        for ( size_t col = 0; col < table.columns.size() && !isMatch; col++ )
        {
            isMatch = formatCell( cellValue( table.rows[rowIndex], col ), table.columns[col], options ).contains( filter, Qt::CaseInsensitive );
        }
        if ( isMatch ) rowIndices.push_back( rowIndex );
    }

    if ( sortColumn < 0 || sortColumn >= static_cast<int>( table.columns.size() ) ) return rowIndices;

    QCollator collator;
    collator.setNumericMode( true );
    collator.setCaseSensitivity( Qt::CaseInsensitive );

    // Numbers, then dates, then text
    auto typeRank = []( const RiaReportCellValue& value )
    {
        if ( isNumeric( value ) ) return 0;
        if ( std::holds_alternative<QDateTime>( value ) ) return 1;
        return 2;
    };

    auto toDouble = []( const RiaReportCellValue& value )
    {
        if ( const auto* intValue = std::get_if<int>( &value ) ) return static_cast<double>( *intValue );
        return std::get<double>( value );
    };

    // Returns negative, zero or positive like strcmp
    auto compare = [&]( const RiaReportCellValue& a, const RiaReportCellValue& b ) -> int
    {
        const int rankA = typeRank( a );
        const int rankB = typeRank( b );
        if ( rankA != rankB ) return rankA - rankB;

        if ( rankA == 0 )
        {
            const double valueA = toDouble( a );
            const double valueB = toDouble( b );
            return valueA < valueB ? -1 : ( valueB < valueA ? 1 : 0 );
        }
        if ( rankA == 1 )
        {
            const auto& firstDate  = std::get<QDateTime>( a );
            const auto& secondDate = std::get<QDateTime>( b );
            return firstDate < secondDate ? -1 : ( secondDate < firstDate ? 1 : 0 );
        }
        return collator.compare( std::get<QString>( a ), std::get<QString>( b ) );
    };

    const size_t column = static_cast<size_t>( sortColumn );
    std::stable_sort( rowIndices.begin(),
                      rowIndices.end(),
                      [&]( size_t rowA, size_t rowB )
                      {
                          const auto valueA = cellValue( table.rows[rowA], column );
                          const auto valueB = cellValue( table.rows[rowB], column );

                          const bool isUndefinedA = isUndefined( valueA );
                          const bool isUndefinedB = isUndefined( valueB );
                          if ( isUndefinedA || isUndefinedB ) return !isUndefinedA && isUndefinedB;

                          const int result = compare( valueA, valueB );
                          return ascending ? result < 0 : result > 0;
                      } );

    return rowIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifReportTableFormatter::toText( const RiaReportTable& table, const RifReportFormatOptions& options )
{
    QString     text;
    QTextStream stream( &text );

    if ( !table.title.isEmpty() ) stream << table.title << "\n";
    if ( !table.description.isEmpty() ) stream << table.description << "\n";
    if ( !table.title.isEmpty() || !table.description.isEmpty() ) stream << "\n";

    if ( table.columns.empty() ) return text;

    RifTextDataTableFormatter formatter( stream );
    formatter.setHeaderPrefix( "" );
    formatter.setCommentPrefix( "" );
    formatter.setTableRowPrependText( "" );
    formatter.setTableRowLineAppendText( "" );
    formatter.setColumnSpacing( 3 );
    formatter.setUnlimitedDataRowWidth();

    const bool anyUnit =
        std::any_of( table.columns.begin(), table.columns.end(), []( const RiaReportColumn& c ) { return !c.unit.isEmpty(); } );

    std::vector<RifTextDataTableColumn> header;
    for ( const auto& column : table.columns )
    {
        if ( anyUnit )
        {
            const QString unitText = column.unit.isEmpty() ? QString() : QString( "[%1]" ).arg( column.unit );
            header.emplace_back( column.title, unitText, column.doubleFormat, column.alignment );
        }
        else
        {
            header.emplace_back( column.title, column.doubleFormat, column.alignment );
        }
    }
    formatter.header( header );

    for ( const auto& row : table.rows )
    {
        for ( size_t col = 0; col < table.columns.size(); col++ )
        {
            const RiaReportCellValue value = col < row.size() ? row[col] : RiaReportCellValue();
            formatter.add( formatCell( value, table.columns[col], options ) );
        }
        formatter.rowCompleted();
    }
    formatter.tableCompleted();

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifReportTableFormatter::toCsv( const RiaReportTable& table, const RifReportFormatOptions& options )
{
    if ( table.columns.empty() ) return {};

    QString     text;
    QTextStream stream( &text );

    RifCsvDataTableFormatter formatter( stream, options.delimiter );
    formatter.setUseQuotes( false );

    std::vector<RifTextDataTableColumn> header;
    for ( const auto& column : table.columns )
    {
        header.emplace_back( escapeCsvField( columnHeaderText( column ), options.delimiter ), column.doubleFormat, column.alignment );
    }
    formatter.header( header );

    for ( const auto& row : table.rows )
    {
        for ( size_t col = 0; col < table.columns.size(); col++ )
        {
            const RiaReportCellValue value = col < row.size() ? row[col] : RiaReportCellValue();
            formatter.add( escapeCsvField( formatCell( value, table.columns[col], options ), options.delimiter ) );
        }
        formatter.rowCompleted();
    }
    formatter.tableCompleted();

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifReportTableFormatter::toHtml( const RiaReportTable& table, const RifReportFormatOptions& options, bool includeStyleSheet )
{
    QString html;
    if ( includeStyleSheet ) html += QString( "<style>%1</style>\n" ).arg( htmlStyleSheet() );

    html += htmlTableBody( table, options );
    return html;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifReportTableFormatter::format( const RiaReportTable& table, Format format, const RifReportFormatOptions& options )
{
    switch ( format )
    {
        case Format::CSV:
            return toCsv( table, options );
        case Format::HTML:
            return toHtml( table, options );
        case Format::TEXT:
        default:
            return toText( table, options );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifReportTableFormatter::format( const RiaReportSection& section, Format format, const RifReportFormatOptions& options )
{
    return RifReportTableFormatter::format( QString(), { section }, format, options );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifReportTableFormatter::format( const QString&                       title,
                                         const std::vector<RiaReportSection>& sections,
                                         Format                               format,
                                         const RifReportFormatOptions&        options )
{
    if ( std::any_of( sections.begin(), sections.end(), []( const RiaReportSection& section ) { return section.hasPendingTables(); } ) )
    {
        auto loadedSections = sections;
        for ( auto& section : loadedSections )
        {
            section.loadTables();
        }
        return RifReportTableFormatter::format( title, loadedSections, format, options );
    }

    size_t tableCount = 0;
    for ( const auto& section : sections )
    {
        tableCount += section.tables.size();
    }

    QString text;

    if ( format == Format::HTML )
    {
        text += QString( "<html><head><meta charset=\"utf-8\"><title>%1</title><style>%2</style></head><body>\n" )
                    .arg( title.toHtmlEscaped() )
                    .arg( htmlStyleSheet() );
        if ( !title.isEmpty() ) text += QString( "<h1>%1</h1>\n" ).arg( title.toHtmlEscaped() );

        for ( const auto& section : sections )
        {
            if ( sections.size() > 1 && !section.title.isEmpty() ) text += QString( "<h2>%1</h2>\n" ).arg( section.title.toHtmlEscaped() );
            for ( const auto& table : section.tables )
            {
                text += htmlTableBody( table, options );
            }
        }
        text += "</body></html>\n";
        return text;
    }

    if ( format == Format::CSV )
    {
        // A single table is written as plain CSV, without any title lines
        for ( const auto& section : sections )
        {
            for ( const auto& table : section.tables )
            {
                if ( tableCount > 1 )
                {
                    QStringList titles;
                    if ( !section.title.isEmpty() ) titles.push_back( section.title );
                    if ( !table.title.isEmpty() ) titles.push_back( table.title );
                    if ( !text.isEmpty() ) text += "\n";
                    if ( !titles.isEmpty() ) text += escapeCsvField( titles.join( " - " ), options.delimiter ) + "\n";
                }
                text += toCsv( table, options );
            }
        }
        return text;
    }

    if ( !title.isEmpty() ) text += title + "\n\n";

    for ( const auto& section : sections )
    {
        if ( sections.size() > 1 && !section.title.isEmpty() )
        {
            text += section.title + "\n" + QString( section.title.length(), '=' ) + "\n\n";
        }
        for ( const auto& table : section.tables )
        {
            text += toText( table, options ) + "\n";
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// Shared style for HTML output. Alternating rows use classes, as the Qt rich text engine does not
/// support structural pseudo-classes.
//--------------------------------------------------------------------------------------------------
QString RifReportTableFormatter::htmlStyleSheet()
{
    return "body { font-family: 'Segoe UI', Arial, sans-serif; font-size: 9pt; }"
           "h1 { font-size: 13pt; }"
           "h2 { font-size: 11pt; margin-top: 16px; }"
           "h3.ri-title { font-size: 10pt; margin-bottom: 2px; }"
           "p.ri-description { color: #6a737d; margin-top: 0px; }"
           "table.ri-report { border-collapse: collapse; margin-bottom: 12px; }"
           "table.ri-report th { background-color: #e8ebef; font-weight: bold; padding: 4px 10px; border-bottom: 1px solid #c4cad1; "
           "vertical-align: bottom; }"
           "table.ri-report td { padding: 3px 10px; border-bottom: 1px solid #edf0f2; }"
           "table.ri-report tr.odd td { background-color: #f6f8fa; }"
           "table.ri-report .num { text-align: right; }"
           "table.ri-report .text { text-align: left; }"
           "table.ri-report span.unit { font-weight: normal; font-size: 8pt; color: #6a737d; }"
           "table.ri-report td.undefined { color: #8a939d; }"
           "table.ri-report tr.omitted td { color: #8a939d; font-style: italic; text-align: center; }";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifTextDataTableDoubleFormatting RifReportTableFormatter::effectiveDoubleFormatting( const RiaReportColumn&        column,
                                                                                     const RifReportFormatOptions& options )
{
    RifTextDataTableDoubleFormatting formatting = column.doubleFormat;

    switch ( options.doubleFormat )
    {
        case RifReportFormatOptions::DoubleFormat::FLOAT:
            formatting.format = RIF_FLOAT;
            break;
        case RifReportFormatOptions::DoubleFormat::SCIENTIFIC:
            formatting.format = RIF_SCIENTIFIC;
            break;
        case RifReportFormatOptions::DoubleFormat::CONCISE:
            formatting.format = RIF_CONSISE;
            break;
        case RifReportFormatOptions::DoubleFormat::COLUMN_DEFAULT:
        default:
            break;
    }

    if ( options.doublePrecision >= 0 ) formatting.precision = options.doublePrecision;
    formatting.precision = std::clamp( formatting.precision, 0, 16 );

    return formatting;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifReportTableFormatter::escapeCsvField( const QString& text, const QString& delimiter )
{
    const bool needsQuotes = ( !delimiter.isEmpty() && text.contains( delimiter ) ) || text.contains( '"' ) || text.contains( '\n' ) ||
                             text.contains( '\r' );
    if ( !needsQuotes ) return text;

    QString escaped = text;
    escaped.replace( "\"", "\"\"" );
    return "\"" + escaped + "\"";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifReportTableFormatter::htmlTableBody( const RiaReportTable& table, const RifReportFormatOptions& options )
{
    auto alignClass = []( const RiaReportColumn& column ) { return column.alignment == RIGHT ? QString( "num" ) : QString( "text" ); };
    auto alignAttr  = []( const RiaReportColumn& column ) { return column.alignment == RIGHT ? QString( "right" ) : QString( "left" ); };

    QString html;
    if ( !table.title.isEmpty() ) html += QString( "<h3 class=\"ri-title\">%1</h3>\n" ).arg( table.title.toHtmlEscaped() );
    if ( !table.description.isEmpty() )
    {
        html += QString( "<p class=\"ri-description\">%1</p>\n" ).arg( table.description.toHtmlEscaped().replace( "\n", "<br>" ) );
    }

    if ( table.columns.empty() ) return html;

    html += "<table class=\"ri-report\" cellspacing=\"0\" cellpadding=\"4\">\n<thead><tr>";
    for ( const auto& column : table.columns )
    {
        QString headerText = column.title.toHtmlEscaped();
        if ( !column.unit.isEmpty() ) headerText += QString( "<br><span class=\"unit\">%1</span>" ).arg( column.unit.toHtmlEscaped() );

        html += QString( "<th class=\"%1\" align=\"%2\">%3</th>" ).arg( alignClass( column ) ).arg( alignAttr( column ) ).arg( headerText );
    }
    html += "</tr></thead>\n<tbody>\n";

    auto appendRow = [&]( size_t rowIndex )
    {
        const auto& row = table.rows[rowIndex];
        html += QString( "<tr class=\"%1\">" ).arg( rowIndex % 2 == 0 ? "even" : "odd" );
        for ( size_t col = 0; col < table.columns.size(); col++ )
        {
            const RiaReportCellValue value  = col < row.size() ? row[col] : RiaReportCellValue();
            const auto&              column = table.columns[col];

            QString cellClass = alignClass( column );
            if ( isUndefined( value ) ) cellClass += " undefined";

            html += QString( "<td class=\"%1\" align=\"%2\">%3</td>" )
                        .arg( cellClass )
                        .arg( alignAttr( column ) )
                        .arg( formatCell( value, column, options ).toHtmlEscaped() );
        }
        html += "</tr>\n";
    };

    const auto range = htmlRowRange( table.rows.size(), options );
    for ( size_t i = 0; i < range.headRowCount; i++ )
    {
        appendRow( i );
    }

    if ( range.omittedRowCount > 0 )
    {
        html += QString( "<tr class=\"omitted\"><td colspan=\"%1\" align=\"center\"><i>%2</i></td></tr>\n" )
                    .arg( table.columns.size() )
                    .arg( omittedRowsText( range.omittedRowCount ).toHtmlEscaped() );
    }

    for ( size_t i = range.tailStartRowIndex; i < range.tailStartRowIndex + range.tailRowCount; i++ )
    {
        appendRow( i );
    }

    html += "</tbody></table>\n";
    return html;
}

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
#include <numeric>

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

    return QString( "%1 [%2]" ).arg( column.title, column.unit );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReportRowRange RifReportTableFormatter::displayRowRange( size_t rowCount, const RifReportFormatOptions& options )
{
    const size_t headCount = static_cast<size_t>( std::max( 0, options.displayHeadRowCount ) );
    const size_t tailCount = static_cast<size_t>( std::max( 0, options.displayTailRowCount ) );

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
std::vector<size_t> RifReportTableFormatter::sortedRows( const RiaReportTable& table, int sortColumn, bool ascending )
{
    auto cellValue = []( const std::vector<RiaReportCellValue>& row, size_t column ) -> RiaReportCellValue
    { return column < row.size() ? row[column] : RiaReportCellValue(); };

    std::vector<size_t> rowIndices( table.rows.size() );
    std::iota( rowIndices.begin(), rowIndices.end(), size_t( 0 ) );

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
/// Tab separated text, which spreadsheets split into columns when pasted
//--------------------------------------------------------------------------------------------------
QString RifReportTableFormatter::toText( const RiaReportTable& table, const RifReportFormatOptions& options )
{
    QString text;

    if ( !table.title.isEmpty() ) text += table.title + "\n";
    if ( !table.description.isEmpty() ) text += table.description + "\n";
    if ( !table.title.isEmpty() || !table.description.isEmpty() ) text += "\n";

    if ( table.columns.empty() ) return text;

    // Tabs and line breaks inside a cell would split the cell
    auto cellText = []( QString value )
    {
        value.replace( '\t', ' ' );
        value.replace( '\r', ' ' );
        value.replace( '\n', ' ' );
        return value;
    };

    QStringList values;
    for ( const auto& column : table.columns )
    {
        values.push_back( cellText( columnHeaderText( column ) ) );
    }
    text += values.join( '\t' ) + "\n";

    for ( const auto& row : table.rows )
    {
        values.clear();
        for ( size_t col = 0; col < table.columns.size(); col++ )
        {
            const RiaReportCellValue value = col < row.size() ? row[col] : RiaReportCellValue();
            values.push_back( cellText( formatCell( value, table.columns[col], options ) ) );
        }
        text += values.join( '\t' ) + "\n";
    }

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

    // Spreadsheets read an empty field as a missing number, and any other text as a text cell
    RifReportFormatOptions csvOptions = options;
    csvOptions.undefinedValueText     = QString();

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
            formatter.add( escapeCsvField( formatCell( value, table.columns[col], csvOptions ), options.delimiter ) );
        }
        formatter.rowCompleted();
    }
    formatter.tableCompleted();

    return text;
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
    if ( section.hasPendingTables() )
    {
        auto loadedSection = section;
        loadedSection.loadTables();
        return formatLoadedSections( QString(), { &loadedSection }, format, options );
    }

    return formatLoadedSections( QString(), { &section }, format, options );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifReportTableFormatter::format( const QString&                       title,
                                         const std::vector<RiaReportSection>& sections,
                                         Format                               format,
                                         const RifReportFormatOptions&        options )
{
    // Only sections with pending tables are copied, as the tables can be large
    std::vector<RiaReportSection>        loadedSections;
    std::vector<const RiaReportSection*> sectionPointers;
    loadedSections.reserve( sections.size() );
    for ( const auto& section : sections )
    {
        if ( section.hasPendingTables() )
        {
            loadedSections.push_back( section );
            loadedSections.back().loadTables();
            sectionPointers.push_back( &loadedSections.back() );
        }
        else
        {
            sectionPointers.push_back( &section );
        }
    }

    return formatLoadedSections( title, sectionPointers, format, options );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifReportTableFormatter::formatLoadedSections( const QString&                              title,
                                                       const std::vector<const RiaReportSection*>& sectionPointers,
                                                       Format                                      format,
                                                       const RifReportFormatOptions&               options )
{
    size_t tableCount = 0;
    for ( const auto* section : sectionPointers )
    {
        tableCount += section->tables.size();
    }

    QString text;

    if ( format == Format::CSV )
    {
        // A single table is written as plain CSV, without any title lines
        for ( const auto* section : sectionPointers )
        {
            for ( const auto& table : section->tables )
            {
                if ( tableCount > 1 )
                {
                    QStringList titles;
                    if ( !section->title.isEmpty() ) titles.push_back( section->title );
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

    for ( const auto* section : sectionPointers )
    {
        if ( sectionPointers.size() > 1 && !section->title.isEmpty() )
        {
            text += section->title + "\n" + QString( section->title.length(), '=' ) + "\n\n";
        }
        for ( const auto& table : section->tables )
        {
            text += toText( table, options ) + "\n";
        }
    }

    return text;
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

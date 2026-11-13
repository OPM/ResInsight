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

#pragma once

#include "RiaReportTable.h"

#include <QString>

#include <vector>

//==================================================================================================
///
//==================================================================================================
struct RifReportFormatOptions
{
    enum class DoubleFormat
    {
        COLUMN_DEFAULT,
        FLOAT,
        SCIENTIFIC,
        CONCISE
    };

    int          doublePrecision = -1; // Negative: use the precision hint of the column
    DoubleFormat doubleFormat    = DoubleFormat::COLUMN_DEFAULT;
    QString      delimiter       = ",";
    QString      dateFormat; // Empty: ISO date, with time of day if not midnight
    QString      undefinedValueText = "-";
    int          htmlHeadRowCount   = 1000;
    int          htmlTailRowCount   = 50;
};

//==================================================================================================
/// Row selection used when only the head and tail of a large table are shown
//==================================================================================================
struct RifReportRowRange
{
    size_t headRowCount      = 0;
    size_t omittedRowCount   = 0;
    size_t tailRowCount      = 0;
    size_t tailStartRowIndex = 0;
};

//==================================================================================================
/// Formats RiaReportTable data as fixed-width text, CSV or HTML. All text formatting of report
/// data is done here, so number precision, separators and undefined values are consistent.
//==================================================================================================
class RifReportTableFormatter
{
public:
    enum class Format
    {
        TEXT,
        CSV,
        HTML
    };

    static QString formatCell( const RiaReportCellValue& value, const RiaReportColumn& column, const RifReportFormatOptions& options );
    static bool    isUndefined( const RiaReportCellValue& value );
    static bool    isNumeric( const RiaReportCellValue& value );
    static QString columnHeaderText( const RiaReportColumn& column );

    static RifReportRowRange htmlRowRange( size_t rowCount, const RifReportFormatOptions& options );
    static QString           omittedRowsText( size_t omittedRowCount );

    // Indices of the rows where any formatted cell contains the filter text (case insensitive), sorted on
    // the given column. A negative sort column keeps the original order. Undefined values are sorted last.
    static std::vector<size_t> filteredAndSortedRows( const RiaReportTable&         table,
                                                      const QString&                filterText,
                                                      int                           sortColumn,
                                                      bool                          ascending,
                                                      const RifReportFormatOptions& options );

    static QString toText( const RiaReportTable& table, const RifReportFormatOptions& options );
    static QString toCsv( const RiaReportTable& table, const RifReportFormatOptions& options );
    static QString toHtml( const RiaReportTable& table, const RifReportFormatOptions& options, bool includeStyleSheet = true );

    static QString format( const RiaReportTable& table, Format format, const RifReportFormatOptions& options );
    static QString format( const RiaReportSection& section, Format format, const RifReportFormatOptions& options );
    static QString
        format( const QString& title, const std::vector<RiaReportSection>& sections, Format format, const RifReportFormatOptions& options );

    static QString htmlStyleSheet();

private:
    static RifTextDataTableDoubleFormatting effectiveDoubleFormatting( const RiaReportColumn& column, const RifReportFormatOptions& options );
    static QString escapeCsvField( const QString& text, const QString& delimiter );
    static QString htmlTableBody( const RiaReportTable& table, const RifReportFormatOptions& options );
};

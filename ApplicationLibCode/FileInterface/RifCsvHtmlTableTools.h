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

#include <QString>
#include <QStringList>

#include <expected>
#include <vector>

//==================================================================================================
/// Generates a simple HTML preview table from CSV data with a header row, for display in a
/// read-only field using the caf::PdmUiTextEditor HTML text mode (e.g. a quick content preview in
/// the property editor). Not a full CSV import framework: quoted fields and embedded delimiters are
/// not supported, only plain comma/semicolon/tab separated values.
//==================================================================================================
class RifCsvHtmlTableTools
{
public:
    // Reads a CSV file, auto-detects the delimiter from the header line, and returns an HTML table
    // of its content, or an error message if the file could not be read. If maxRowCount is >= 0,
    // only the first maxRowCount data rows (after the header) are included.
    static std::expected<QString, QString> generateHtmlTableFromFile( const QString& filePath, int maxRowCount = -1 );

    // As generateHtmlTableFromFile(), but operates on already-read text content (e.g. for testing,
    // or content read through some other mechanism than a local file).
    static std::expected<QString, QString> generateHtmlTableFromText( const QString& content, int maxRowCount = -1 );

    // Builds an HTML table string from an already parsed header and set of data rows. Rows with a
    // different number of columns than the header are rendered as-is.
    static QString generateHtmlTable( const QStringList& header, const std::vector<QStringList>& rows );

    static QChar detectDelimiter( const QString& line );
};

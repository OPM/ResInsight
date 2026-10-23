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

#include "RifCsvHtmlTableTools.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
/// Picks the most likely delimiter by checking for the presence of common CSV separators, in order
/// of specificity (semicolon and tab are less likely to appear incidentally than comma).
//--------------------------------------------------------------------------------------------------
QChar RifCsvHtmlTableTools::detectDelimiter( const QString& line )
{
    if ( line.contains( ';' ) ) return ';';
    if ( line.contains( '\t' ) ) return '\t';
    return ',';
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<QString, QString> RifCsvHtmlTableTools::generateHtmlTableFromFile( const QString& filePath, int maxRowCount )
{
    QFile file( filePath );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        return std::unexpected( QString( "Could not open file: %1" ).arg( filePath ) );
    }

    QTextStream stream( &file );
    return generateHtmlTableFromText( stream.readAll(), maxRowCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<QString, QString> RifCsvHtmlTableTools::generateHtmlTableFromText( const QString& content, int maxRowCount )
{
    QStringList lines = content.split( '\n' );
    while ( !lines.isEmpty() && lines.last().trimmed().isEmpty() )
    {
        lines.removeLast();
    }

    if ( lines.isEmpty() )
    {
        return std::unexpected( QString( "Content is empty" ) );
    }

    const QString     rawHeader  = lines.takeFirst();
    const QString     headerLine = rawHeader.endsWith( '\r' ) ? rawHeader.chopped( 1 ) : rawHeader;
    const QChar       delimiter  = detectDelimiter( headerLine );
    const QStringList header     = headerLine.split( delimiter );

    std::vector<QStringList> rows;
    int                      rowCount = 0;
    for ( const QString& rawLine : lines )
    {
        if ( maxRowCount >= 0 && rowCount >= maxRowCount ) break;

        const QString line = rawLine.endsWith( '\r' ) ? rawLine.chopped( 1 ) : rawLine;
        if ( line.trimmed().isEmpty() ) continue;

        rows.push_back( line.split( delimiter ) );
        rowCount++;
    }

    return generateHtmlTable( header, rows );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifCsvHtmlTableTools::generateHtmlTable( const QStringList& header, const std::vector<QStringList>& rows )
{
    QString html( "<table border=1 cellspacing=0 cellpadding=3>"
                  "  <thead>"
                  "    <tr bgcolor=lightblue>" );

    for ( const QString& column : header )
    {
        html += QString( "<th>%1</th>" ).arg( column.trimmed().toHtmlEscaped() );
    }
    html += "</tr></thead><tbody>";

    for ( const QStringList& row : rows )
    {
        html += "<tr>";
        for ( const QString& cell : row )
        {
            const QString trimmedCell = cell.trimmed();

            bool isNumber = false;
            trimmedCell.toDouble( &isNumber );
            const QString align = isNumber ? " align=right" : "";

            html += QString( "<td%1>%2</td>" ).arg( align ).arg( trimmedCell.toHtmlEscaped() );
        }
        html += "</tr>";
    }

    html += "</tbody></table>";

    return html;
}

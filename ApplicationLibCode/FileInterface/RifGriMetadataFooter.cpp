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

#include "RifGriMetadataFooter.h"

#include <QFile>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifGriMetadataFooter::markerLine()
{
    return "----RESINSIGHT_METADATA_V1----";
}

//--------------------------------------------------------------------------------------------------
/// Append the metadata footer to an existing file. Keys and values must not contain newlines,
/// and keys must not contain '='.
//--------------------------------------------------------------------------------------------------
bool RifGriMetadataFooter::appendFooter( const QString& fileName, const std::vector<std::pair<QString, QString>>& keyValues )
{
    QFile file( fileName );
    if ( !file.open( QIODevice::Append ) ) return false;

    QString footer = "\n" + markerLine() + "\n";
    for ( const auto& [key, value] : keyValues )
    {
        footer += key + "=" + value + "\n";
    }

    return file.write( footer.toUtf8() ) != -1;
}

//--------------------------------------------------------------------------------------------------
/// Read the metadata footer from the end of a file. Returns std::nullopt when no footer is found.
//--------------------------------------------------------------------------------------------------
std::optional<std::map<QString, QString>> RifGriMetadataFooter::readFooter( const QString& fileName )
{
    QFile file( fileName );
    if ( !file.open( QIODevice::ReadOnly ) ) return std::nullopt;

    // The footer is small, reading the last part of the file is sufficient
    const qint64 maxFooterSize = 64 * 1024;
    const qint64 readPosition  = std::max( qint64( 0 ), file.size() - maxFooterSize );
    if ( !file.seek( readPosition ) ) return std::nullopt;

    const QByteArray tail        = file.readAll();
    const QByteArray marker      = ( markerLine() + "\n" ).toUtf8();
    const int        markerIndex = tail.lastIndexOf( marker );
    if ( markerIndex < 0 ) return std::nullopt;

    std::map<QString, QString> keyValues;

    const QString footerText = QString::fromUtf8( tail.mid( markerIndex + marker.size() ) );
    for ( const QString& line : footerText.split( '\n', Qt::SkipEmptyParts ) )
    {
        const int separatorIndex = line.indexOf( '=' );
        if ( separatorIndex < 1 ) return std::nullopt;

        keyValues[line.left( separatorIndex )] = line.mid( separatorIndex + 1 );
    }

    if ( keyValues.empty() ) return std::nullopt;

    return keyValues;
}

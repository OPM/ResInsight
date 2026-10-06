/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RifWellPathFormationReader.h"

#include <QFile>
#include <QStringList>
#include <QTextStream>

#include <vector>

namespace
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList parseHeader( const QString& line )
{
    QString header = line.toLower();
    header.removeIf( []( QChar c ) { return c.isSpace(); } );

    return header.split( ';' );
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RifWellPathFormationReader::WellFormations, QString> RifWellPathFormationReader::readWellFormations( const QString& filePath )
{
    QFile file( filePath );
    if ( !file.open( QFile::ReadOnly | QFile::Text ) )
    {
        return std::unexpected( QString( "Failed to open %1" ).arg( filePath ) );
    }

    return parseWellFormations( QString::fromUtf8( file.readAll() ), filePath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RifWellPathFormationReader::WellFormations, QString> RifWellPathFormationReader::parseWellFormations( const QString& content,
                                                                                                                    const QString& filePath )
{
    const auto parseFailure = QString( "Failed to parse %1 as a well pick file" ).arg( filePath );

    QString     text = content;
    QTextStream stream( &text );

    QStringList header;
    while ( header.size() < 3 )
    {
        if ( stream.atEnd() ) return std::unexpected( parseFailure );

        header = parseHeader( stream.readLine() );
    }

    const int wellNameIndex = header.indexOf( "wellname" );
    const int unitNameIndex = header.indexOf( "unitname" );
    const int mdTopIndex    = header.indexOf( "topmd" );
    const int mdBaseIndex   = header.indexOf( "basemd" );
    const int tvdTopIndex   = header.indexOf( "toptvdss" );
    const int tvdBaseIndex  = header.indexOf( "basetvdss" );

    const bool hasMd  = mdTopIndex != -1 && mdBaseIndex != -1;
    const bool hasTvd = tvdTopIndex != -1 && tvdBaseIndex != -1;

    if ( wellNameIndex == -1 || unitNameIndex == -1 )
    {
        return std::unexpected( parseFailure );
    }

    if ( !hasMd && !hasTvd )
    {
        return std::unexpected( parseFailure + ". Neither MD or TVD is present." );
    }

    std::map<QString, std::vector<RigWellPathFormation>> formationsPerWell;

    while ( !stream.atEnd() )
    {
        const QStringList columns = stream.readLine().split( ';' );
        if ( columns.size() != header.size() ) continue;

        const QString wellName = columns[wellNameIndex];
        const QString unitName = columns[unitNameIndex].trimmed();
        if ( wellName.trimmed().isEmpty() && unitName.isEmpty() ) continue;

        RigWellPathFormation formation;
        formation.formationName = unitName;

        if ( hasMd )
        {
            formation.mdTop  = columns[mdTopIndex].toDouble();
            formation.mdBase = columns[mdBaseIndex].toDouble();
        }

        if ( hasTvd )
        {
            formation.tvdTop  = -columns[tvdTopIndex].toDouble();
            formation.tvdBase = -columns[tvdBaseIndex].toDouble();
        }

        formationsPerWell[wellName].push_back( formation );
    }

    if ( formationsPerWell.empty() )
    {
        return std::unexpected( parseFailure );
    }

    WellFormations result;
    for ( const auto& [wellName, formations] : formationsPerWell )
    {
        result.emplace( wellName, RigWellPathFormations( formations, filePath, wellName ) );
    }

    return result;
}

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

#include <optional>
#include <vector>

namespace
{
//--------------------------------------------------------------------------------------------------
/// The FMU formations.csv format is comma-separated, the "well pick" formats are semicolon-separated
//--------------------------------------------------------------------------------------------------
QChar detectDelimiter( const QString& line )
{
    return line.contains( ',' ) ? ',' : ';';
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList parseHeader( const QString& line, QChar delimiter )
{
    QString header = line.toLower();
    header.removeIf( []( QChar c ) { return c.isSpace() || c == '_'; } );

    return header.split( delimiter );
}

//--------------------------------------------------------------------------------------------------
/// Returns the index of the first of the candidate column names present in the header, or -1
//--------------------------------------------------------------------------------------------------
int findColumn( const QStringList& header, std::initializer_list<const char*> candidates )
{
    for ( const char* candidate : candidates )
    {
        const int index = header.indexOf( QString( candidate ) );
        if ( index != -1 ) return index;
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<double> toOptionalDouble( const QStringList& columns, int index )
{
    if ( index == -1 || index >= columns.size() ) return std::nullopt;

    bool         ok    = false;
    const double value = columns[index].toDouble( &ok );
    return ok ? std::optional( value ) : std::nullopt;
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
    QChar       delimiter = ';';
    while ( header.size() < 3 )
    {
        if ( stream.atEnd() ) return std::unexpected( parseFailure );

        const QString rawHeaderLine = stream.readLine();
        delimiter                   = detectDelimiter( rawHeaderLine );
        header                      = parseHeader( rawHeaderLine, delimiter );
    }

    // The "well pick" formats use wellname/unitname, the FMU formations.csv format uses well/zone (and
    // ignores zone_code). Both support md and/or tvd, and the FMU format can also include an x/y
    // position for the zone top.
    //
    // "tvdss" columns hold a positive magnitude in the opposite (RMS, Z-up) sign convention and are
    // negated to match ResInsight's positive-down TVD. Plain "tvd" columns (as used in the FMU
    // formations.csv) are already positive-down and are used as-is.
    const int wellNameIndex  = findColumn( header, { "wellname", "well" } );
    const int unitNameIndex  = findColumn( header, { "unitname", "zone" } );
    const int mdTopIndex     = header.indexOf( "topmd" );
    const int mdBaseIndex    = header.indexOf( "basemd" );
    const int tvdSSTopIndex  = header.indexOf( "toptvdss" );
    const int tvdSSBaseIndex = header.indexOf( "basetvdss" );
    const int tvdTopIndex    = tvdSSTopIndex != -1 ? tvdSSTopIndex : header.indexOf( "toptvd" );
    const int tvdBaseIndex   = tvdSSBaseIndex != -1 ? tvdSSBaseIndex : header.indexOf( "basetvd" );
    const int xIndex         = header.indexOf( "xutme" );
    const int yIndex         = header.indexOf( "yutmn" );

    const bool hasMd   = mdTopIndex != -1 && mdBaseIndex != -1;
    const bool hasTvd  = tvdTopIndex != -1 && tvdBaseIndex != -1;
    const bool isTvdSS = tvdSSTopIndex != -1 && tvdSSBaseIndex != -1;
    const bool hasXY   = xIndex != -1 && yIndex != -1;

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
        const QStringList columns = stream.readLine().split( delimiter );
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
            const double sign = isTvdSS ? -1.0 : 1.0;
            formation.tvdTop  = sign * columns[tvdTopIndex].toDouble();
            formation.tvdBase = sign * columns[tvdBaseIndex].toDouble();
        }

        if ( hasXY )
        {
            auto x = toOptionalDouble( columns, xIndex );
            auto y = toOptionalDouble( columns, yIndex );
            if ( x && y ) formation.topXY = cvf::Vec2d( *x, *y );
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

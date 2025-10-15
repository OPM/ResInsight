/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024  Equinor ASA
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

#include "RifPolygonReader.h"

#include "RiaTextStringTools.h"

#include "RifAsciiDataParseOptions.h"

#include "RifCsvUserDataParser.h"

#include <QFileInfo>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<int, std::vector<cvf::Vec3d>>> RifPolygonReader::parsePolygonFile( const QString& fileName, QString* errorMessage )
{
    QFileInfo fi( fileName );

    QFile dataFile( fileName );

    if ( !dataFile.open( QFile::ReadOnly ) )
    {
        if ( errorMessage ) ( *errorMessage ) += "Could not open file: " + fileName + "\n";
        return {};
    }

    QTextStream stream( &dataFile );
    auto        fileContent = stream.readAll();

    if ( fi.suffix().trimmed().toLower() == "csv" )
    {
        return parseTextCsv( fileContent, errorMessage );
    }
    else
    {
        auto polygons = parseText( fileContent, errorMessage );

        std::vector<std::pair<int, std::vector<cvf::Vec3d>>> polygonsWithIds;
        for ( auto& polygon : polygons )
        {
            polygonsWithIds.push_back( std::make_pair( -1, polygon ) );
        }

        return polygonsWithIds;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::Vec3d>> RifPolygonReader::parseText( const QString& content, QString* errorMessage )
{
    std::vector<std::vector<cvf::Vec3d>> polylines( 1 );

    QString     myString = content;
    QTextStream stream( &myString );
    int         lineNumber = 1;
    while ( !stream.atEnd() )
    {
        QString     line            = stream.readLine();
        QStringList commentLineSegs = line.split( "#" );
        if ( commentLineSegs.empty() ) continue; // Empty line

        QStringList lineSegs = RiaTextStringTools::splitSkipEmptyParts( commentLineSegs[0], QRegularExpression( "\\s+" ) );

        if ( lineSegs.empty() ) continue; // No data

        if ( lineSegs.size() != 3 )
        {
            if ( errorMessage ) ( *errorMessage ) += "Unexpected number of words on line: " + QString::number( lineNumber ) + "\n";
            continue;
        }

        {
            bool   isNumberParsingOk = true;
            bool   isOk              = true;
            double x                 = lineSegs[0].toDouble( &isOk );
            isNumberParsingOk &= isOk;
            double y = lineSegs[1].toDouble( &isOk );
            isNumberParsingOk &= isOk;
            double z = lineSegs[2].toDouble( &isOk );
            isNumberParsingOk &= isOk;

            if ( !isNumberParsingOk )
            {
                if ( errorMessage ) ( *errorMessage ) += "Could not read the point at line: " + QString::number( lineNumber ) + "\n";
                continue;
            }

            if ( x == 999.0 && y == 999.0 && z == 999.0 ) // New PolyLine
            {
                polylines.push_back( std::vector<cvf::Vec3d>() );
                continue;
            }

            cvf::Vec3d point( x, y, -z );
            polylines.back().push_back( point );
        }

        ++lineNumber;
    }

    if ( polylines.back().empty() )
    {
        polylines.pop_back();
    }

    return polylines;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<int, std::vector<cvf::Vec3d>>> RifPolygonReader::parseTextCsv( const QString& content, QString* errorMessage )
{
    RifCsvUserDataPastedTextParser parser( content );

    RifAsciiDataParseOptions parseOptions;
    parseOptions.cellSeparator    = ",";
    parseOptions.decimalSeparator = ".";

    std::vector<std::pair<QString, std::vector<double>>> readValues;

    auto parseResult = parser.parse( parseOptions );
    if ( parseResult )
    {
        for ( auto s : parser.tableData().columnInfos() )
        {
            if ( s.dataType != Column::NUMERIC ) continue;

            QString             columnName = QString::fromStdString( s.columnName() );
            bool                isNumber   = false;
            auto                value      = columnName.toDouble( &isNumber );
            std::vector<double> values     = s.values;
            if ( isNumber )
            {
                values.insert( values.begin(), value );
            }
            readValues.push_back( { columnName, values } );
        }
    }

    if ( readValues.size() == 4 )
    {
        // Three first columns represent XYZ, last column polygon ID

        const auto firstSize = readValues[0].second.size();
        if ( ( firstSize == readValues[1].second.size() ) && ( firstSize == readValues[2].second.size() ) &&
             ( firstSize == readValues[3].second.size() ) )
        {
            std::vector<std::pair<int, std::vector<cvf::Vec3d>>> polylines;

            std::vector<cvf::Vec3d> polygon;

            int polygonId = -1;
            for ( size_t i = 0; i < firstSize; i++ )
            {
                int currentPolygonId = static_cast<int>( readValues[3].second[i] );
                if ( polygonId != currentPolygonId )
                {
                    if ( !polygon.empty() ) polylines.push_back( std::make_pair( polygonId, polygon ) );
                    polygon.clear();
                    polygonId = currentPolygonId;
                }

                cvf::Vec3d point( readValues[0].second[i], readValues[1].second[i], -readValues[2].second[i] );

                polygon.push_back( point );
            }

            if ( !polygon.empty() ) polylines.push_back( std::make_pair( polygonId, polygon ) );

            return polylines;
        }
    }

    if ( readValues.size() == 3 )
    {
        std::vector<cvf::Vec3d> points;

        for ( size_t i = 0; i < readValues[0].second.size(); i++ )
        {
            cvf::Vec3d point( readValues[0].second[i], readValues[1].second[i], -readValues[2].second[i] );
            points.push_back( point );
        }

        int polygonId = -1;
        return { std::make_pair( polygonId, points ) };
    }

    return {};
}

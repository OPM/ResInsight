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

#include <QTextStream>

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

        QStringList lineSegs = RiaTextStringTools::splitSkipEmptyParts( commentLineSegs[0], QRegExp( "\\s+" ) );

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

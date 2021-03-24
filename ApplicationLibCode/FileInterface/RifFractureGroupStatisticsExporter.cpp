/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-    Equinor ASA
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

#include "RifFractureGroupStatisticsExporter.h"

// #include "RiaEclipseUnitTools.h"

// #include "RimStimPlanModel.h"
#include "RigSlice2D.h"
#include "cafAssert.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifFractureGroupStatisticsExporter::writeAsStimPlanXml( const RigSlice2D&          statistics,
                                                             const QString&             filePath,
                                                             const std::vector<double>& gridXs,
                                                             const std::vector<double>& gridYs,
                                                             double                     time )
{
    QFile data( filePath );
    if ( !data.open( QFile::WriteOnly | QFile::Truncate ) )
    {
        return false;
    }

    QTextStream stream( &data );
    appendHeaderToStream( stream );
    appendGridDimensionsToStream( stream, gridXs, gridYs );
    appendPropertiesToStream( stream, statistics, gridYs, time );
    appendFooterToStream( stream );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifFractureGroupStatisticsExporter::appendHeaderToStream( QTextStream& stream )
{
    stream << "<?xml version=\"1.0\" ?>" << endl << "<contours>" << endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifFractureGroupStatisticsExporter::appendPropertiesToStream( QTextStream&               stream,
                                                                   const RigSlice2D&          statistics,
                                                                   const std::vector<double>& gridYs,
                                                                   double                     time )
{
    CAF_ASSERT( statistics.ny() == gridYs.size() );

    // TODO:
    QString propertyName = "conductivity";
    QString propertyUnit = "cm";

    stream << "<properties>" << endl;
    stream << QString( "<property name=\"%1\" uom=\"%2\">" ).arg( propertyName ).arg( propertyUnit ) << endl;
    stream << QString( "<time value=\"%1\">" ).arg( time ) << endl;

    for ( size_t i = 0; i < gridYs.size(); i++ )
    {
        stream << "<depth>" << gridYs[i] << "</depth>" << endl;
        stream << "<data>[";
        for ( size_t x = 0; x < statistics.nx(); x++ )
        {
            stream << statistics.getValue( x, i ) << " ";
        }
        stream << "]</data>" << endl;
    }

    stream << "</time>" << endl;
    stream << "</property>" << endl;
    stream << "</properties>" << endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifFractureGroupStatisticsExporter::appendGridDimensionsToStream( QTextStream&               stream,
                                                                       const std::vector<double>& gridXs,
                                                                       const std::vector<double>& gridYs )
{
    stream << QString( "<grid xCount=\"%1\" yCount=\"%2\">" ).arg( gridXs.size() ).arg( gridYs.size() ) << endl;

    stream << "<xs>[";
    for ( auto x : gridXs )
        stream << x << " ";
    stream << "]</xs>" << endl;

    stream << "<ys>[";
    for ( auto y : gridYs )
        stream << y << " ";
    stream << "]</ys>" << endl;

    stream << "</grid>" << endl;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifFractureGroupStatisticsExporter::appendFooterToStream( QTextStream& stream )
{
    stream << "</contours>" << endl;
}

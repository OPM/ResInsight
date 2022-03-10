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

#include "RifEnsembleFractureStatisticsExporter.h"

#include "RiaDefines.h"
#include "RigSlice2D.h"

#include "caf.h"
#include "cafAssert.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEnsembleFractureStatisticsExporter::writeAsStimPlanXml( const std::vector<std::shared_ptr<RigSlice2D>>& statistics,
                                                                const std::vector<std::pair<QString, QString>>& properties,
                                                                const QString&                             filePath,
                                                                const std::vector<double>&                 gridXs,
                                                                const std::vector<double>&                 gridYs,
                                                                double                                     time,
                                                                RiaDefines::EclipseUnitSystem              unitSystem,
                                                                RigStimPlanFractureDefinition::Orientation orientation )
{
    QFile data( filePath );
    if ( !data.open( QFile::WriteOnly | QFile::Truncate ) )
    {
        return false;
    }

    QTextStream stream( &data );
    appendHeaderToStream( stream );
    appendOrientationToStream( stream, orientation );
    appendGridDimensionsToStream( stream, gridXs, gridYs, unitSystem );
    appendPropertiesToStream( stream, statistics, properties, gridYs, time );
    appendFooterToStream( stream );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEnsembleFractureStatisticsExporter::appendHeaderToStream( QTextStream& stream )
{
    stream << "<?xml version=\"1.0\" ?>" << caf::endl << "<contours>" << caf::endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEnsembleFractureStatisticsExporter::appendPropertiesToStream(
    QTextStream&                                    stream,
    const std::vector<std::shared_ptr<RigSlice2D>>& statistics,
    const std::vector<std::pair<QString, QString>>& properties,
    const std::vector<double>&                      gridYs,
    double                                          time )
{
    CAF_ASSERT( statistics.size() == properties.size() );

    stream << "<properties>" << caf::endl;

    for ( size_t s = 0; s < statistics.size(); s++ )
    {
        QString propertyName = properties[s].first;
        QString propertyUnit = properties[s].second;

        stream << QString( "<property name=\"%1\" uom=\"%2\">" ).arg( propertyName ).arg( propertyUnit ) << caf::endl;
        stream << QString( "<time value=\"%1\">" ).arg( time ) << caf::endl;

        CAF_ASSERT( statistics[s]->ny() == gridYs.size() );

        // Need to write these in reverse because the depth tag is ignored in
        // in the reader (depths from <grid><ys> are used).
        for ( int i = static_cast<int>( gridYs.size() ) - 1; i >= 0; i-- )
        {
            stream << "<depth>" << gridYs[i] << "</depth>" << caf::endl;
            stream << "<data>[";
            for ( size_t x = 0; x < statistics[s]->nx(); x++ )
            {
                stream << statistics[s]->getValue( x, i ) << " ";
            }
            stream << "]</data>" << caf::endl;
        }

        stream << "</time>" << caf::endl;
        stream << "</property>" << caf::endl;
    }
    stream << "</properties>" << caf::endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEnsembleFractureStatisticsExporter::appendOrientationToStream( QTextStream& stream,
                                                                       RigStimPlanFractureDefinition::Orientation orientation )
{
    if ( orientation != RigStimPlanFractureDefinition::Orientation::UNDEFINED )
    {
        QString orientationString = getStringForOrientation( orientation );
        stream << QString( "<orientation>%1</orientation>" ).arg( orientationString ) << caf::endl;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEnsembleFractureStatisticsExporter::appendGridDimensionsToStream( QTextStream&                  stream,
                                                                          const std::vector<double>&    gridXs,
                                                                          const std::vector<double>&    gridYs,
                                                                          RiaDefines::EclipseUnitSystem unitSystem )
{
    QString unitString = getStringForUnitSystem( unitSystem );
    stream << QString( "<grid xCount=\"%1\" yCount=\"%2\" uom=\"%3\">" ).arg( gridXs.size() ).arg( gridYs.size() ).arg( unitString )
           << caf::endl;

    stream << "<xs>[";
    for ( auto x : gridXs )
        stream << x << " ";
    stream << "]</xs>" << caf::endl;

    stream << "<ys>[";
    for ( auto y : gridYs )
        stream << y << " ";
    stream << "]</ys>" << caf::endl;

    stream << "</grid>" << caf::endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEnsembleFractureStatisticsExporter::appendFooterToStream( QTextStream& stream )
{
    stream << "</contours>" << caf::endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEnsembleFractureStatisticsExporter::getStringForUnitSystem( RiaDefines::EclipseUnitSystem unitSystem )
{
    if ( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
        return "m";
    else if ( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        return "ft";
    else
        return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEnsembleFractureStatisticsExporter::getStringForOrientation( RigStimPlanFractureDefinition::Orientation orientation )
{
    if ( orientation == RigStimPlanFractureDefinition::Orientation::TRANSVERSE )
        return "transverse";
    else if ( orientation == RigStimPlanFractureDefinition::Orientation::LONGITUDINAL )
        return "longitudinal";
    else
        return "";
}

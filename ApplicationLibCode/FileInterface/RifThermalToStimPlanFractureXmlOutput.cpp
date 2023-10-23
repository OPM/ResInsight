/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-    Equinor ASA
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

#include "RifThermalToStimPlanFractureXmlOutput.h"

#include "RigFractureGrid.h"

#include "RimThermalFractureTemplate.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifThermalToStimPlanFractureXmlOutput::writeToFile( RimThermalFractureTemplate* fractureTemplate, const QString& filepath )
{
    CAF_ASSERT( fractureTemplate );

    QFile data( filepath );
    if ( !data.open( QFile::WriteOnly | QFile::Truncate ) ) return false;

    // Can use a fake depths here: depths must be adjusted when importing
    double                     fakeDepth    = 1000.0;
    cvf::cref<RigFractureGrid> fractureGrid = fractureTemplate->createFractureGrid( fakeDepth );
    if ( fractureGrid.isNull() ) return false;

    QTextStream stream( &data );
    appendHeaderToStream( stream );

    appendGridDefinitionToStream( stream, *fractureGrid );

    appendPropertiesToStream( stream, *fractureTemplate, *fractureGrid );

    appendFooterToStream( stream );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifThermalToStimPlanFractureXmlOutput::appendHeaderToStream( QTextStream& stream )
{
    stream << "<?xml version=\"1.0\" ?>" << '\n' << "<contours>" << '\n';
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifThermalToStimPlanFractureXmlOutput::appendGridDefinitionToStream( QTextStream& stream, const RigFractureGrid& fractureGrid )
{
    size_t xCount = fractureGrid.iCellCount();
    size_t yCount = fractureGrid.jCellCount();

    stream << QString( "<grid xCount=\"%1\" yCount=\"%2\" uom=\"m\">" ).arg( xCount ).arg( yCount );

    std::vector<double> xs;
    for ( size_t i = 0; i < xCount; i++ )
    {
        size_t idx          = fractureGrid.getGlobalIndexFromIJ( i, 0 );
        auto   fractureCell = fractureGrid.cellFromIndex( idx );
        xs.push_back( fractureCell.centerPosition().x() );
    }

    std::vector<double> ys;
    for ( size_t j = 0; j < yCount; j++ )
    {
        size_t idx          = fractureGrid.getGlobalIndexFromIJ( 0, j );
        auto   fractureCell = fractureGrid.cellFromIndex( idx );
        ys.push_back( -fractureCell.centerPosition().y() );
    }

    appendDataVector( stream, "xs", xs );
    appendDataVector( stream, "ys", ys );

    stream << "</grid>\n";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifThermalToStimPlanFractureXmlOutput::appendPropertiesToStream( QTextStream&                      stream,
                                                                      const RimThermalFractureTemplate& fractureTemplate,
                                                                      const RigFractureGrid&            fractureGrid )
{
    stream << "<properties>\n";

    auto resultNamesWithUnit = fractureTemplate.uiResultNamesWithUnit();

    for ( auto [name, unit] : resultNamesWithUnit )
    {
        stream << QString( "<property name=\"%1\" uom=\"%2\">\n" ).arg( name ).arg( unit );
        stream << "<time value=\"1\">\n";

        std::vector<std::vector<double>> results = fractureTemplate.resultValues( name, unit, fractureTemplate.activeTimeStepIndex() );

        for ( int j = static_cast<int>( fractureGrid.jCellCount() ) - 1; j >= 0; j-- )
        {
            size_t idx          = fractureGrid.getGlobalIndexFromIJ( 0, j );
            auto   fractureCell = fractureGrid.cellFromIndex( idx );
            double depth        = -fractureCell.centerPosition().y();

            stream << QString( "<depth>%1</depth>\n" ).arg( depth );

            std::vector<double> data;
            for ( size_t i = 1; i < results[j].size() - 1; i++ )
                data.push_back( results[j][i] );

            appendDataVector( stream, "data", data );
        }

        stream << "</time>\n";
        stream << "</property>\n";
    }
    stream << "</properties>\n";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifThermalToStimPlanFractureXmlOutput::appendDataVector( QTextStream& stream, const QString& name, const std::vector<double>& vals )
{
    stream << QString( "<%1>[" ).arg( name );

    for ( auto v : vals )
    {
        stream << v << " ";
    }

    stream << QString( "]</%1>\n" ).arg( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifThermalToStimPlanFractureXmlOutput::appendFooterToStream( QTextStream& stream )
{
    stream << "</contours>" << '\n';
}

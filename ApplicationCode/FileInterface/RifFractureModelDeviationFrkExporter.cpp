/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-    Equinor ASA
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

#include "RifFractureModelDeviationFrkExporter.h"

#include "RimFractureModel.h"
#include "RimWellPath.h"

#include "RigWellPathGeometryExporter.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifFractureModelDeviationFrkExporter::writeToFile( RimFractureModel* fractureModel, const QString& filepath )
{
    RimWellPath* wellPath = fractureModel->wellPath();
    if ( !wellPath )
    {
        return false;
    }

    QFile data( filepath );
    if ( !data.open( QFile::WriteOnly | QFile::Truncate ) )
    {
        return false;
    }

    QTextStream stream( &data );
    appendHeaderToStream( stream );

    bool                useMdRkb   = false;
    double              mdStepSize = 5.0;
    std::vector<double> xValues;
    std::vector<double> yValues;
    std::vector<double> tvdValues;
    std::vector<double> mdValues;
    RigWellPathGeometryExporter::exportWellPathGeometry( wellPath, mdStepSize, xValues, yValues, tvdValues, mdValues, useMdRkb );
    convertFromMeterToFeet( mdValues );
    convertFromMeterToFeet( tvdValues );

    std::vector<double> exportTvdValues;
    std::vector<double> exportMdValues;
    fixupDepthValuesForExport( tvdValues, mdValues, exportTvdValues, exportMdValues );

    appendToStream( stream, "mdArray", exportMdValues );
    appendToStream( stream, "tvdArray", exportTvdValues );

    appendFooterToStream( stream );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifFractureModelDeviationFrkExporter::appendHeaderToStream( QTextStream& stream )
{
    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl << "<deviation>" << endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifFractureModelDeviationFrkExporter::appendToStream( QTextStream&               stream,
                                                           const QString&             label,
                                                           const std::vector<double>& values )
{
    stream.setRealNumberPrecision( 20 );
    stream << "<cNamedSet>" << endl
           << "<name>" << endl
           << label << endl
           << "</name>" << endl
           << "<dimCount>" << endl
           << 1 << endl
           << "</dimCount>" << endl
           << "<sizes>" << endl
           << values.size() << endl
           << "</sizes>" << endl
           << "<data>" << endl;
    for ( auto val : values )
    {
        stream << val << endl;
    }

    stream << "</data>" << endl << "</cNamedSet>" << endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifFractureModelDeviationFrkExporter::appendFooterToStream( QTextStream& stream )
{
    stream << "</deviation>" << endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifFractureModelDeviationFrkExporter::convertFromMeterToFeet( std::vector<double>& data )
{
    for ( size_t i = 0; i < data.size(); i++ )
    {
        data[i] = RiaEclipseUnitTools::meterToFeet( data[i] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifFractureModelDeviationFrkExporter::fixupDepthValuesForExport( const std::vector<double>& tvdValues,
                                                                      const std::vector<double>& mdValues,
                                                                      std::vector<double>&       exportTvdValues,
                                                                      std::vector<double>&       exportMdValues )
{
    if ( tvdValues.empty() || mdValues.empty() ) return;

    exportMdValues.push_back( mdValues[0] );
    exportTvdValues.push_back( tvdValues[0] );

    for ( size_t i = 1; i < tvdValues.size(); i++ )
    {
        double changeMd  = mdValues[i] - exportMdValues[i - 1];
        double changeTvd = tvdValues[i] - exportTvdValues[i - 1];

        // Stimplan checks that the change in MD is larger than or equal to change in TVD.
        // This condition is not always satisfied due to the interpolation of TVDs.
        // Move the MD value to produce a file which can be imported.
        if ( changeMd < changeTvd )
        {
            exportMdValues.push_back( exportMdValues[i - 1] + changeTvd );
        }
        else
        {
            exportMdValues.push_back( mdValues[i] );
        }

        exportTvdValues.push_back( tvdValues[i] );
    }
}

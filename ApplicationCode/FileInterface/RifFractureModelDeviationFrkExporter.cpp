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
#include "RimFractureModelPlot.h"
#include "RimWellPath.h"

#include "RigWellPathGeometryExporter.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifFractureModelDeviationFrkExporter::writeToFile( RimFractureModelPlot* plot, const QString& filepath )
{
    RimFractureModel* fractureModel = plot->fractureModel();
    if ( !fractureModel )
    {
        return false;
    }

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

    appendToStream( stream, "mdArray", mdValues );
    appendToStream( stream, "tvdArray", tvdValues );

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

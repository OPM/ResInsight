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

#include "RifOsduWellPathReader.h"

#include "RiaTextStringTools.h"

#include "SummaryPlotCommands/RicPasteAsciiDataToSummaryPlotFeatureUi.h"

#include "RifCsvUserDataParser.h"

#include "RigWellPath.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include <QFileInfo>
#include <QTextStream>

std::pair<cvf::ref<RigWellPath>, QString> RifOsduWellPathReader::parseCsv( const QString& content )
{
    QString                        errorMessage;
    RifCsvUserDataPastedTextParser parser( content, &errorMessage );

    AsciiDataParseOptions parseOptions;
    parseOptions.cellSeparator    = ",";
    parseOptions.decimalSeparator = ".";

    std::vector<std::pair<QString, std::vector<double>>> readValues;

    if ( parser.parse( parseOptions ) )
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

    const int MD_INDEX  = 0;
    const int TVD_INDEX = 1;
    const int X_INDEX   = 4;
    const int Y_INDEX   = 5;

    if ( readValues.size() == 10 )
    {
        const size_t firstSize = readValues[MD_INDEX].second.size();
        if ( ( firstSize == readValues[TVD_INDEX].second.size() ) && ( firstSize == readValues[X_INDEX].second.size() ) &&
             ( firstSize == readValues[Y_INDEX].second.size() ) )
        {
            std::vector<cvf::Vec3d> wellPathPoints;
            std::vector<double>     measuredDepths;

            for ( size_t i = 0; i < firstSize; i++ )
            {
                cvf::Vec3d point( readValues[X_INDEX].second[i], readValues[Y_INDEX].second[i], -readValues[TVD_INDEX].second[i] );
                double     md = readValues[MD_INDEX].second[i];

                wellPathPoints.push_back( point );
                measuredDepths.push_back( md );
            }

            return { new RigWellPath( wellPathPoints, measuredDepths ), "" };
        }
    }

    return { nullptr, "Oh no!" };
}

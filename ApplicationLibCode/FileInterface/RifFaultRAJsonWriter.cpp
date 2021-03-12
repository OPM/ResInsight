/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021  Equinor ASA
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

#include "RifFaultRAJSonWriter.h"

#include "RimFaultRAPostprocSettings.h"
#include "RimFaultRAPreprocSettings.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifFaultRAJSonWriter::writeToFile( RimFaultRAPreprocSettings& settings, QString& outErrorText )
{
    QString filename = settings.preprocParameterFilename();

    outErrorText = "Unable to write to file \"" + filename + "\" - ";

    QFile file( filename );
    if ( file.open( QIODevice::ReadWrite ) )
    {
        QTextStream stream( &file );

        stream << "{" << endl;
        stream << "\"odb_path\": \"" + settings.geomechCaseFilename() + "\"," << endl;
        stream << "\"time_start\": \"" + settings.startTimeStep() + "\"," << endl;
        stream << "\"time_end\": \"" + settings.endTimeStep() + "\"" << endl;
        stream << "}" << endl;

        file.close();
    }
    else
    {
        outErrorText += "Could not open file.";
        return false;
    }

    outErrorText = "";
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifFaultRAJSonWriter::writeToFile( RimFaultRAPostprocSettings& settings, QString& outErrorText )
{
    QString filename = settings.postprocParameterFilename();

    outErrorText = "Unable to write to file \"" + filename + "\" - ";

    QFile file( filename );
    if ( file.open( QIODevice::ReadWrite ) )
    {
        QTextStream stream( &file );

        stream << "{" << endl;

        if ( QFile::exists( settings.macrisCalcCalibPath() ) )
            stream << "\"MacrisCalcCalibration_path\": \"" + settings.macrisCalcCalibPath() + "\"," << endl;

        if ( QFile::exists( settings.macrisCalcPath() ) )
            stream << "\"MacrisCalc_path\": \"" + settings.macrisCalcPath() + "\"," << endl;

        stream << "\"base_directory_path\": \"" + settings.databaseDirectory() + "\"," << endl;

        QStringList timesteps;
        for ( int step : settings.stepsToLoad() )
            timesteps.push_back( QString::number( step ) );

        stream << "\"tsurf_loadsteps\": [ " + timesteps.join( ',' ) + " ]" << endl;
        stream << "}" << endl;

        file.close();
    }
    else
    {
        outErrorText += "Could not open file.";
        return false;
    }

    outErrorText = "";
    return true;
}

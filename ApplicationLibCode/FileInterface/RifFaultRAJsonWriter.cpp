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

#include "RifFaultRAJsonWriter.h"

#include "RimFaultRAPostprocSettings.h"
#include "RimFaultRAPreprocSettings.h"
#include "RimGenericParameter.h"
#include "RimParameterGroup.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifFaultRAJSonWriter::writeToPreprocFile( RimFaultRAPreprocSettings& settings, QString& outErrorText )
{
    QString filename = settings.preprocParameterFilename();

    outErrorText = "Unable to write to file \"" + filename + "\" - ";

    QFile file( filename );
    if ( file.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) )
    {
        QTextStream stream( &file );

        stream << "{" << endl;
        stream << "\"odb_path\": \"" + settings.geomechCaseFilename() + "\"," << endl;
        stream << "\"time_start\": \"" + settings.startTimeStepGeoMech() + "\"," << endl;
        stream << "\"time_end\": \"" + settings.endTimeStepGeoMech() + "\"," << endl;
        stream << "\"out_path\": \"" + settings.outputAbaqusDirectory() + "\"" << endl;
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
bool RifFaultRAJSonWriter::writeToPostprocFile( int faultID, RimFaultRAPostprocSettings* settings, QString& outErrorText )
{
    QString filename = settings->postprocParameterFilename( faultID );

    outErrorText = "Unable to write to file \"" + filename + "\" - ";

    QFile file( filename );
    if ( file.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) )
    {
        QTextStream stream( &file );

        stream << "{" << endl;

        if ( settings->geomechEnabled() )
        {
            if ( QFile::exists( settings->advancedMacrisDatabase() ) )
                stream << "\"MacrisCalcCalibration_path\": \"" + settings->advancedMacrisDatabase() + "\"," << endl;
        }

        if ( QFile::exists( settings->basicMacrisDatabase() ) )
            stream << "\"MacrisCalc_path\": \"" + settings->basicMacrisDatabase() + "\"," << endl;

        stream << "\"base_directory_path\": \"" + settings->outputBaseDirectory() + "\"," << endl;

        for ( auto p : settings->parameters()->parameters() )
        {
            stream << "\"" + p->name() + "\" : " + p->stringValue() + "," << endl;
        }

        stream << "\"tsurf_loadsteps\": [ " + settings->stepsToLoad().join( ',' ) + " ]" << endl;
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

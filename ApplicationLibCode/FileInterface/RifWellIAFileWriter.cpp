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

#include "RifWellIAFileWriter.h"

#include "RimGenericParameter.h"
#include "RimParameterGroup.h"
#include "RimParameterGroups.h"
#include "RimWellIAModelData.h"
#include "RimWellIASettings.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifWellIAFileWriter::writeToJsonFile( RimWellIASettings& settings, QString& outErrorText )
{
    QString filename = settings.jsonInputFilename();

    outErrorText = "Unable to write to file \"" + filename + "\" - ";

    QFile file( filename );
    if ( file.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) )
    {
        QTextStream stream( &file );

        stream << "{" << endl;
        stream << "\"comments\": \"All units are SI unless mentioned otherwise; temperature is in Celcius; use forward "
                  "slash (/) in 'directory' definition\","
               << endl;
        stream << "\"directory\": \"" + settings.outputBaseDirectory() + "\"," << endl;
        stream << "\"output_name\": \"" + settings.name() + "\"";

        RimParameterGroups mergedGroups;

        bool mergeInCommentParameter = true;

        for ( auto& group : settings.inputParameterGroups() )
        {
            mergedGroups.mergeGroup( group, mergeInCommentParameter );
        }
        for ( auto& group : settings.resinsightParameterGroups() )
        {
            mergedGroups.mergeGroup( group, mergeInCommentParameter );
        }

        for ( auto& group : mergedGroups.groups() )
        {
            stream << "," << endl;

            stream << "\"" + group->name() + "\": {" << endl;

            const auto& parameters = group->parameters();

            for ( size_t i = 0; i < parameters.size(); )
            {
                stream << "   \"" + parameters[i]->name() + "\": " + parameters[i]->jsonValue();

                i++;
                if ( i < parameters.size() )
                {
                    stream << ",";
                }
                stream << endl;
            }

            stream << "   }";
        }

        stream << endl << "}" << endl;
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
bool RifWellIAFileWriter::writeToCSVFile( RimWellIASettings& settings, QString& outErrorText )
{
    QString filename = settings.csvInputFilename();

    outErrorText = "Unable to write to file \"" + filename + "\" - ";

    QFile file( filename );
    if ( file.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) )
    {
        QTextStream stream( &file );

        stream << "Time_days, Pcasing_Pa, P_form_Pa, Temp_C,"
                  "U1-1,U2,U3,U1-2,U2,U3,U1-3,U2,U3,U1-4,U2,U3,U1-5,U2,U3,U1-6,U2,U3,U1-7,U2,U3,U1-8,U2,U3,"
                  "X,Y,Z,X2,Y,Z,X3,Y,Z,X4,Y,Z,X5,Y,Z,X6,Y,Z,X7,Y,Z,X8,Y,Z";
        stream << endl;

        for ( auto& modeldata : settings.modelData() )
        {
            stream << modeldata->dayOffset();
            stream << ",";
            stream << modeldata->casingPressure();
            stream << ",";
            stream << modeldata->formationPressure();
            stream << ",";
            stream << modeldata->temperature();

            for ( auto& u : modeldata->displacements() )
            {
                stream << ",";
                stream << u.x();
                stream << ",";
                stream << u.y();
                stream << ",";
                stream << u.z();
            }

            for ( auto& pos : settings.modelBoxVertices() )
            {
                stream << ",";
                stream << pos.x();
                stream << ",";
                stream << pos.y();
                stream << ",";
                stream << pos.z();
            }
            stream << endl;
        }
    }
    else
    {
        outErrorText += "Could not open file.";
        return false;
    }

    outErrorText = "";
    return true;
}

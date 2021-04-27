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

#include "RifFaultRAXmlWriter.h"

#include "RimFaultRASettings.h"
#include "RimGenericParameter.h"

#include <QFile>
#include <QXmlStreamReader>

RifFaultRAXmlWriter::RifFaultRAXmlWriter( RimFaultRASettings* settings )
    : m_settings( settings )
{
}

RifFaultRAXmlWriter::~RifFaultRAXmlWriter()
{
}

bool RifFaultRAXmlWriter::writeCalculateFile( QString filename, int faultID, QString& outErrorText )
{
    std::list<RimGenericParameter*> paramlist = m_settings->basicParameters( faultID );
    return writeParametersToXML( filename, paramlist, outErrorText );
}

bool RifFaultRAXmlWriter::writeCalibrateFile( QString filename, int faultID, QString& outErrorText )
{
    std::list<RimGenericParameter*> paramlist = m_settings->advancedParameters( faultID );
    return writeParametersToXML( filename, paramlist, outErrorText );
}

bool RifFaultRAXmlWriter::writeParametersToXML( QString filename, std::list<RimGenericParameter*>& params, QString& outErrorText )
{
    bool bResult = false;

    outErrorText = "Unable to write to file \"" + filename + "\" - ";

    QFile file( filename );
    if ( file.open( QIODevice::ReadWrite ) )
    {
        QTextStream stream( &file );

        stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << endl;
        stream << "<project type_id=\"0\">" << endl;

        for ( auto& p : params )
        {
            QString tmpStr = QString( "<%1>%2</%1>" ).arg( p->name(), p->stringValue() );
            stream << tmpStr << endl;
        }
        stream << "</project>" << endl;

        bResult = true;
    }
    else
    {
        outErrorText += "Could not open file.";
    }

    if ( bResult ) outErrorText = "";

    return bResult;
}

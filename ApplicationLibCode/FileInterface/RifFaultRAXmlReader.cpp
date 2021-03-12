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

#include "RifFaultRAXmlReader.h"

#include <QFile>
#include <QXmlStreamReader>

RifFaultRAXmlReader::RifFaultRAXmlReader( QString filename )
    : m_filename( filename )
{
}

RifFaultRAXmlReader::~RifFaultRAXmlReader()
{
}

bool RifFaultRAXmlReader::parseFile( QString& outErrorText )
{
    m_parameterMap.clear();

    outErrorText = "XML read error: ";

    QFile dataFile( m_filename );
    if ( !dataFile.open( QFile::ReadOnly ) )
    {
        outErrorText += "Could not open file: " + m_filename + "\n";
        return false;
    }

    QXmlStreamReader xml;
    xml.setDevice( &dataFile );
    QString parameter;
    QString unit;

    while ( !xml.atEnd() )
    {
        if ( xml.readNextStartElement() )
        {
            if ( xml.name() == "project" ) continue;

            QString paramName = xml.name().toString();
            QString value     = xml.readElementText();

            m_parameterMap[paramName] = value;
        }
    }

    dataFile.close();

    outErrorText = "";
    return true;
}

std::map<QString, QString>& RifFaultRAXmlReader::parameters()
{
    return m_parameterMap;
}

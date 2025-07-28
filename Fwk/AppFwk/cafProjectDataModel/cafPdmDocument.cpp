//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafPdmDocument.h"

#include <QFile>
#include <QXmlStreamReader>

namespace caf
{
CAF_PDM_SOURCE_INIT( PdmDocument, "PdmDocument" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmDocument::PdmDocument()
{
    CAF_PDM_InitFieldNoDefault( &m_fileName, "DocumentFileName", "File Name" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmDocument::fileName() const
{
    return m_fileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmDocument::setFileName( const QString& fileName )
{
    m_fileName = fileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> PdmDocument::readFile( const std::vector<PdmDeprecation>& deprecations )
{
    QFile xmlFile( m_fileName );
    if ( !xmlFile.open( QIODevice::ReadOnly | QIODevice::Text ) ) return {};

    return readFile( &xmlFile, deprecations );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> PdmDocument::readFile( QIODevice* xmlFile, const std::vector<PdmDeprecation>& deprecations )
{
    QXmlStreamReader xmlStream( xmlFile );

    std::vector<QString> deprecationMessages;
    while ( !xmlStream.atEnd() )
    {
        xmlStream.readNext();
        if ( xmlStream.isStartElement() )
        {
            if ( !matchesClassKeyword( xmlStream.name().toString() ) )
            {
                return deprecationMessages;
            }
            auto deprecationMessagesForField =
                readFields( xmlStream, PdmDefaultObjectFactory::instance(), false, deprecations );
            deprecationMessages.insert( deprecationMessages.end(),
                                        deprecationMessagesForField.begin(),
                                        deprecationMessagesForField.end() );
        }
    }

    return deprecationMessages;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmDocument::writeFile()
{
    QFile xmlFile( m_fileName );
    if ( !xmlFile.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;

    writeFile( &xmlFile );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmDocument::writeFile( QIODevice* xmlFile )
{
    // Ask all objects to make them ready to write themselves to file
    setupBeforeSaveRecursively();

    QXmlStreamWriter xmlStream( xmlFile );
    writeDocumentToXmlStream( xmlStream );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmDocument::documentAsString()
{
    // Ask all objects to make them ready to write themselves to file
    setupBeforeSaveRecursively();

    QString          content;
    QXmlStreamWriter xmlStream( &content );
    writeDocumentToXmlStream( xmlStream );

    return content;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const caf::PdmFieldHandle* PdmDocument::fileNameHandle() const
{
    return dynamic_cast<const caf::PdmFieldHandle*>( &m_fileName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmDocument::updateUiIconStateRecursively( PdmObjectHandle* object )
{
    if ( object == nullptr ) return;
    std::vector<PdmFieldHandle*> fields = object->fields();

    std::vector<PdmObjectHandle*> children;
    for ( size_t fIdx = 0; fIdx < fields.size(); ++fIdx )
    {
        if ( fields[fIdx] )
        {
            auto fieldChildren = fields[fIdx]->children();
            children.insert( children.end(), fieldChildren.begin(), fieldChildren.end() );
        }
    }

    for ( size_t cIdx = 0; cIdx < children.size(); ++cIdx )
    {
        PdmDocument::updateUiIconStateRecursively( children[cIdx] );
    }

    PdmUiObjectHandle* uiObjectHandle = uiObj( object );
    if ( uiObjectHandle )
    {
        uiObjectHandle->updateUiIconFromToggleField();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmDocument::writeDocumentToXmlStream( QXmlStreamWriter& xmlStream )
{
    xmlStream.setAutoFormatting( true );

    xmlStream.writeStartDocument();
    QString className = classKeyword();

    xmlStream.writeStartElement( "", className );
    writeFields( xmlStream );
    xmlStream.writeEndElement();

    xmlStream.writeEndDocument();
}

} // End of namespace caf

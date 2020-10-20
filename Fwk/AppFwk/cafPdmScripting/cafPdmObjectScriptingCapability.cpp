//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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
#include "cafPdmObjectScriptingCapability.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmScriptIOMessages.h"
#include "cafPdmXmlFieldHandle.h"

#include <QTextStream>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmObjectScriptingCapability::PdmObjectScriptingCapability( PdmObjectHandle* owner, bool giveOwnership )
    : m_owner( owner )
{
    m_owner->addCapability( this, giveOwnership );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmObjectScriptingCapability::~PdmObjectScriptingCapability()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectScriptingCapability::readFields( QTextStream&         inputStream,
                                               PdmObjectFactory*    objectFactory,
                                               PdmScriptIOMessages* errorMessageContainer )
{
    std::set<QString> readFields;
    bool              isLastArgumentRead = false;
    while ( !inputStream.atEnd() && !isLastArgumentRead )
    {
        // Read field keyword
        bool    fieldDataFound       = false;
        bool    isEndOfArgumentFound = false;
        QString keyword;
        {
            errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
            {
                QChar currentChar;
                while ( !inputStream.atEnd() )
                {
                    currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );

                    if ( currentChar == QChar( '=' ) || currentChar == QChar( ')' ) || currentChar == QChar( ',' ) ||
                         currentChar.isSpace() )
                    {
                        break;
                    }
                    else
                    {
                        keyword += currentChar;
                    }
                }

                if ( currentChar.isSpace() )
                {
                    errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
                    currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                }

                if ( currentChar == QChar( '=' ) )
                {
                    fieldDataFound = true;
                }
                else if ( currentChar == QChar( ')' ) )
                {
                    if ( !keyword.isNull() )
                    {
                        errorMessageContainer->addError(
                            QString( "Can't find the '=' after the argument named '%1' in the command '%2'" )
                                .arg( keyword )
                                .arg( errorMessageContainer->currentCommand ) );
                    }
                    isLastArgumentRead = true;
                }
                else if ( currentChar == QChar( ',' ) )
                {
                    errorMessageContainer->addError(
                        QString( "Can't find the '=' after the argument named '%1' in the command '%2'" )
                            .arg( keyword )
                            .arg( errorMessageContainer->currentCommand ) );
                    isEndOfArgumentFound = true;
                }
                else
                {
                    errorMessageContainer->addError(
                        QString( "Can't find the '=' after the argument named '%1' in the command '%2'" )
                            .arg( keyword )
                            .arg( errorMessageContainer->currentCommand ) );
                }
            }

            if ( readFields.count( keyword ) )
            {
                // Warning message: Referenced the same argument several times
                errorMessageContainer->addWarning( "The argument: \"" + keyword +
                                                   "\" is referenced several times in the command: \"" +
                                                   errorMessageContainer->currentCommand + "\"" );
            }
        }

        if ( fieldDataFound )
        {
            // Make field read its data

            PdmFieldHandle* fieldHandle = m_owner->findField( keyword );
            if ( fieldHandle && fieldHandle->xmlCapability() &&
                 fieldHandle->capability<PdmAbstractFieldScriptingCapability>() )
            {
                PdmXmlFieldHandle*                   xmlFieldHandle = fieldHandle->xmlCapability();
                PdmAbstractFieldScriptingCapability* scriptability =
                    fieldHandle->capability<PdmAbstractFieldScriptingCapability>();

                if ( xmlFieldHandle->isIOReadable() )
                {
                    errorMessageContainer->currentArgument = keyword;
                    scriptability->writeToField( inputStream, objectFactory, errorMessageContainer );
                    errorMessageContainer->currentArgument = keyword;
                }
            }
            else
            {
                // Error message: Unknown argument name
                errorMessageContainer->addWarning( "The argument: \"" + keyword + "\" does not exist in the command: \"" +
                                                   errorMessageContainer->currentCommand + "\"" );
            }
        }

        // Skip to end of argument ',' or end of call ')'
        if ( !( isLastArgumentRead || isEndOfArgumentFound ) )
        {
            QChar currentChar;
            bool  isOutsideQuotes = true;
            while ( !inputStream.atEnd() )
            {
                currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                if ( isOutsideQuotes )
                {
                    if ( currentChar == QChar( ',' ) )
                    {
                        break;
                    }

                    if ( currentChar == QChar( ')' ) )
                    {
                        isLastArgumentRead = true;
                        break;
                    }
                    if ( currentChar == QChar( '\"' ) )
                    {
                        isOutsideQuotes = false;
                    }
                }
                else
                {
                    if ( currentChar == QChar( '\"' ) )
                    {
                        isOutsideQuotes = true;
                    }

                    if ( currentChar == QChar( '\\' ) )
                    {
                        currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectScriptingCapability::writeFields( QTextStream& outputStream ) const
{
    std::vector<PdmFieldHandle*> fields;
    m_owner->fields( fields );
    int writtenFieldCount = 0;
    for ( size_t it = 0; it < fields.size(); ++it )
    {
        const PdmXmlFieldHandle*                   xmlField = fields[it]->xmlCapability();
        const PdmAbstractFieldScriptingCapability* scriptability =
            fields[it]->capability<PdmAbstractFieldScriptingCapability>();
        if ( scriptability && xmlField && xmlField->isIOWritable() )
        {
            QString keyword = xmlField->fieldHandle()->keyword();
            CAF_ASSERT( PdmXmlObjectHandle::isValidXmlElementName( keyword ) );

            if ( writtenFieldCount >= 1 )
            {
                outputStream << ", ";
            }

            outputStream << keyword << " = ";
            scriptability->readFromField( outputStream );

            writtenFieldCount++;
        }
    }
}

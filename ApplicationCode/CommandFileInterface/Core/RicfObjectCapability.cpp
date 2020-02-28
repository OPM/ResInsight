/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicfObjectCapability.h"
#include "RicfFieldHandle.h"
#include "RicfMessages.h"

#include "cafPdmObject.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmXmlFieldHandle.h"

#include <QTextStream>

std::map<QString, QString> RicfObjectCapability::s_classKeywordToScriptClassName;
std::map<QString, QString> RicfObjectCapability::s_scriptClassNameToClassKeyword;
std::map<QString, QString> RicfObjectCapability::s_scriptClassComments;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfObjectCapability::RicfObjectCapability( caf::PdmObjectHandle* owner, bool giveOwnership )
    : m_owner( owner )
{
    m_owner->addCapability( this, giveOwnership );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfObjectCapability::~RicfObjectCapability()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfObjectCapability::readFields( QTextStream&           inputStream,
                                       caf::PdmObjectFactory* objectFactory,
                                       RicfMessages*          errorMessageContainer )
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

            caf::PdmFieldHandle* fieldHandle = m_owner->findField( keyword );
            if ( fieldHandle && fieldHandle->xmlCapability() && fieldHandle->capability<RicfFieldHandle>() )
            {
                caf::PdmXmlFieldHandle* xmlFieldHandle = fieldHandle->xmlCapability();
                RicfFieldHandle*        rcfField       = fieldHandle->capability<RicfFieldHandle>();

                if ( xmlFieldHandle->isIOReadable() )
                {
                    errorMessageContainer->currentArgument = keyword;
                    rcfField->readFieldData( inputStream, objectFactory, errorMessageContainer );
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
void RicfObjectCapability::writeFields( QTextStream& outputStream ) const
{
    std::vector<caf::PdmFieldHandle*> fields;
    m_owner->fields( fields );
    int writtenFieldCount = 0;
    for ( size_t it = 0; it < fields.size(); ++it )
    {
        const caf::PdmXmlFieldHandle* xmlField = fields[it]->xmlCapability();
        const RicfFieldHandle*        rcfField = fields[it]->capability<RicfFieldHandle>();
        if ( rcfField && xmlField && xmlField->isIOWritable() )
        {
            QString keyword = xmlField->fieldHandle()->keyword();
            CAF_ASSERT( caf::PdmXmlObjectHandle::isValidXmlElementName( keyword ) );

            if ( writtenFieldCount >= 1 )
            {
                outputStream << ", ";
            }

            outputStream << keyword << " = ";
            rcfField->writeFieldData( outputStream );

            writtenFieldCount++;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfObjectCapability::registerScriptClassNameAndComment( const QString& classKeyword,
                                                              const QString& scriptClassName,
                                                              const QString& scriptClassComment )
{
    s_classKeywordToScriptClassName[classKeyword]    = scriptClassName;
    s_scriptClassNameToClassKeyword[scriptClassName] = classKeyword;
    s_scriptClassComments[classKeyword]              = scriptClassComment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicfObjectCapability::scriptClassNameFromClassKeyword( const QString& classKeyword )
{
    auto it = s_classKeywordToScriptClassName.find( classKeyword );
    if ( it != s_classKeywordToScriptClassName.end() )
    {
        return it->second;
    }
    return classKeyword;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicfObjectCapability::classKeywordFromScriptClassName( const QString& scriptClassName )
{
    auto it = s_scriptClassNameToClassKeyword.find( scriptClassName );
    if ( it != s_scriptClassNameToClassKeyword.end() )
    {
        return it->second;
    }
    return scriptClassName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicfObjectCapability::scriptClassComment( const QString& classKeyword )
{
    auto it = s_scriptClassComments.find( classKeyword );
    if ( it != s_scriptClassComments.end() )
    {
        return it->second;
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicfObjectCapability::isScriptable( const caf::PdmObject* object )
{
    return s_classKeywordToScriptClassName.find( object->classKeyword() ) != s_classKeywordToScriptClassName.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfObjectCapability::addCapabilityToObject( caf::PdmObject* object,
                                                  const QString&  scriptClassName,
                                                  const QString&  scriptClassComment )
{
    if ( !object->capability<RicfObjectCapability>() )
    {
        new RicfObjectCapability( object, true );
    }
    RicfObjectCapability::registerScriptClassNameAndComment( object->classKeyword(), scriptClassName, scriptClassComment );
}

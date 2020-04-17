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
#include "cafPdmFieldIOScriptability.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldScriptabilityIOHandler<QString>::writeToField( QString&                  fieldValue,
                                                            QTextStream&              inputStream,
                                                            caf::PdmScriptIOMessages* errorMessageContainer,
                                                            bool                      stringsAreQuoted )
{
    fieldValue = "";

    errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
    QString accumulatedFieldValue;

    QChar currentChar;
    bool  validStringStart = !stringsAreQuoted;
    bool  validStringEnd   = !stringsAreQuoted;
    if ( stringsAreQuoted )
    {
        currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
        if ( currentChar == QChar( '"' ) )
        {
            validStringStart = true;
        }
    }

    if ( validStringStart )
    {
        while ( !inputStream.atEnd() )
        {
            currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
            if ( currentChar != QChar( '\\' ) )
            {
                if ( currentChar == QChar( '"' ) ) // End Quote
                {
                    // Reached end of string
                    validStringEnd = true;
                    break;
                }
                else
                {
                    accumulatedFieldValue += currentChar;
                }
            }
            else
            {
                currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                accumulatedFieldValue += currentChar;
            }
        }
    }
    if ( !validStringStart )
    {
        // Unexpected start of string, Missing '"'
        // Error message
        errorMessageContainer->addError(
            "String argument does not seem to be quoted. Missing the start '\"' in the \"" +
            errorMessageContainer->currentArgument + "\" argument of the command: \"" +
            errorMessageContainer->currentCommand + "\"" );
        // Could interpret as unquoted text
    }
    else if ( !validStringEnd )
    {
        // Unexpected end of string, Missing '"'
        // Error message
        errorMessageContainer->addError( "String argument does not seem to be quoted. Missing the end '\"' in the \"" +
                                         errorMessageContainer->currentArgument + "\" argument of the command: \"" +
                                         errorMessageContainer->currentCommand + "\"" );
        // Could interpret as unquoted text
    }

    if ( accumulatedFieldValue != "None" )
    {
        fieldValue = accumulatedFieldValue;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldScriptabilityIOHandler<QString>::readFromField( const QString& fieldValue,
                                                             QTextStream&   outputStream,
                                                             bool           quoteStrings,
                                                             bool           quoteNonBuiltin )
{
    outputStream << "\"";
    for ( int i = 0; i < fieldValue.size(); ++i )
    {
        if ( fieldValue[i] == QChar( '"' ) || fieldValue[i] == QChar( '\\' ) )
        {
            outputStream << "\\";
        }
        outputStream << fieldValue[i];
    }
    outputStream << "\"";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldScriptabilityIOHandler<bool>::writeToField( bool&                     fieldValue,
                                                         QTextStream&              inputStream,
                                                         caf::PdmScriptIOMessages* errorMessageContainer,
                                                         bool                      stringsAreQuoted )
{
    errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
    QString accumulatedFieldValue;
    QChar   nextChar;
    QChar   currentChar;
    while ( !inputStream.atEnd() )
    {
        nextChar = errorMessageContainer->peekNextChar( inputStream );
        if ( nextChar.isLetter() )
        {
            currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
            accumulatedFieldValue += currentChar;
        }
        else
        {
            break;
        }
    }
    // Accept TRUE or False in any case combination.
    bool evaluatesToTrue  = QString::compare( accumulatedFieldValue, QString( "true" ), Qt::CaseInsensitive ) == 0;
    bool evaluatesToFalse = QString::compare( accumulatedFieldValue, QString( "false" ), Qt::CaseInsensitive ) == 0;
    if ( evaluatesToTrue == evaluatesToFalse )
    {
        QString formatString(
            "Boolean argument \"%1\" for the command \"%2\" does not evaluate to either true or false" );
        QString errorMessage =
            formatString.arg( errorMessageContainer->currentArgument ).arg( errorMessageContainer->currentCommand );
        errorMessageContainer->addError( errorMessage );
    }
    fieldValue = evaluatesToTrue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldScriptabilityIOHandler<bool>::readFromField( const bool&  fieldValue,
                                                          QTextStream& outputStream,
                                                          bool         quoteStrings,
                                                          bool         quoteNonBuiltin )
{
    // Lower-case true/false is used in the documentation.
    outputStream << ( fieldValue ? "true" : "false" );
}

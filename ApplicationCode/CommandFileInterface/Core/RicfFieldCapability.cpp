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

#include "RicfFieldCapability.h"

#include "RiaColorTools.h"

#include <QColor>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfFieldIOHandler<QString>::writeToField( QString&                  fieldValue,
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

    fieldValue = accumulatedFieldValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfFieldIOHandler<QString>::readFromField( const QString& fieldValue,
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
void RicfFieldIOHandler<bool>::writeToField( bool&                     fieldValue,
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
void RicfFieldIOHandler<bool>::readFromField( const bool&  fieldValue,
                                              QTextStream& outputStream,
                                              bool         quoteStrings,
                                              bool         quoteNonBuiltin )
{
    // Lower-case true/false is used in the documentation.
    outputStream << ( fieldValue ? "true" : "false" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfFieldIOHandler<cvf::Color3f>::writeToField( cvf::Color3f&             fieldValue,
                                                     QTextStream&              inputStream,
                                                     caf::PdmScriptIOMessages* errorMessageContainer,
                                                     bool                      stringsAreQuoted )
{
    QString fieldStringValue;
    RicfFieldIOHandler<QString>::writeToField( fieldStringValue, inputStream, errorMessageContainer, stringsAreQuoted );

    QColor qColor( fieldStringValue );
    if ( qColor.isValid() )
    {
        fieldValue = RiaColorTools::fromQColorTo3f( qColor );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfFieldIOHandler<cvf::Color3f>::readFromField( const cvf::Color3f& fieldValue,
                                                      QTextStream&        outputStream,
                                                      bool                quoteStrings,
                                                      bool                quoteNonBuiltin )
{
    QColor  qColor           = RiaColorTools::toQColor( fieldValue );
    QString fieldStringValue = qColor.name();
    RicfFieldIOHandler<QString>::readFromField( fieldStringValue, outputStream, quoteStrings );
}

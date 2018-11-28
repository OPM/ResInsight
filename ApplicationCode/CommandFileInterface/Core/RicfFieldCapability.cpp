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
#include "RicfMessages.h"




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfFieldReader<QString>::readFieldData(QString& fieldValue, QTextStream& inputStream, RicfMessages* errorMessageContainer)
{
    fieldValue = "";

    errorMessageContainer->skipWhiteSpaceWithLineNumberCount(inputStream);
    QString accumulatedFieldValue;
    QChar currentChar;
    currentChar = errorMessageContainer->readCharWithLineNumberCount(inputStream);
    if ( currentChar == QChar('"') )
    {
        while ( !inputStream.atEnd() )
        {
            currentChar = errorMessageContainer->readCharWithLineNumberCount(inputStream);
            if ( currentChar != QChar('\\') )
            {
                if ( currentChar == QChar('"') ) // End Quote
                {
                    // Reached end of string
                    break;
                }
                else
                {
                    accumulatedFieldValue += currentChar;
                }
            }
            else
            {
                currentChar = errorMessageContainer->readCharWithLineNumberCount(inputStream);
                accumulatedFieldValue += currentChar;
            }
        }
    }
    else
    {
        // Unexpected start of string, Missing '"'
        // Error message
        errorMessageContainer->addError("String argument does not seem to be quoted. Missing the start '\"' in the \"" 
                                        + errorMessageContainer->currentArgument + "\" argument of the command: \"" 
                                        + errorMessageContainer->currentCommand + "\"" );
        // Could interpret as unquoted text
    }

    fieldValue = accumulatedFieldValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfFieldWriter<QString>::writeFieldData(const QString& fieldValue, QTextStream& outputStream)
{
    outputStream << "\"";
    for ( int i = 0; i < fieldValue.size(); ++i )
    {
        if ( fieldValue[i] == QChar('"') || fieldValue[i] ==  QChar('\\') )
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
void RicfFieldReader<bool>::readFieldData(bool& fieldValue, QTextStream& inputStream, RicfMessages* errorMessageContainer)
{
    errorMessageContainer->skipWhiteSpaceWithLineNumberCount(inputStream);
    QString accumulatedFieldValue;
    QChar nextChar;
    QChar currentChar;
    while (!inputStream.atEnd())
    {
        nextChar = errorMessageContainer->peekNextChar(inputStream);
        if (nextChar.isLetter())
        {
            currentChar = errorMessageContainer->readCharWithLineNumberCount(inputStream);
            accumulatedFieldValue += currentChar;
        }
        else
        {
            break;
        }
    }
    // Accept TRUE or False in any case combination.
    bool evaluatesToTrue  = QString::compare(accumulatedFieldValue, QString("true"), Qt::CaseInsensitive) == 0;
    bool evaluatesToFalse = QString::compare(accumulatedFieldValue, QString("false"), Qt::CaseInsensitive) == 0;
    if (evaluatesToTrue == evaluatesToFalse)
    {
        QString formatString("Boolean argument \"%1\" for the command \"%2\" does not evaluate to either true or false");
        QString errorMessage = formatString.arg(errorMessageContainer->currentArgument).arg(errorMessageContainer->currentCommand);
        errorMessageContainer->addError(errorMessage);
    }
    fieldValue = evaluatesToTrue;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicfFieldWriter<bool>::writeFieldData(const bool& fieldValue, QTextStream& outputStream)
{
    // Lower-case true/false is used in the documentation.
    outputStream << (fieldValue ? "true" : "false");
}

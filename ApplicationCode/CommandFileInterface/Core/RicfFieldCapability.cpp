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

    inputStream.skipWhiteSpace();
    QString accumulatedFieldValue;
    QChar currentChar;
    inputStream >> currentChar;
    if ( currentChar == QChar('"') )
    {
        while ( !inputStream.atEnd() )
        {
            inputStream >> currentChar;
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
                inputStream >> currentChar;
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

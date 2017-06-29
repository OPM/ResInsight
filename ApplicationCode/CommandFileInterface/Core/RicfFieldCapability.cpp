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



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfFieldReader<QString>::readFieldData(QString& fieldValue, QTextStream& inputStream)
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
                    // Read and eat , or ) ?

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

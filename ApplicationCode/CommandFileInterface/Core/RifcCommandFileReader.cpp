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

#include "RifcCommandFileReader.h"

#include "RicfCommandObject.h"
#include "RicfObjectCapability.h"
#include "RicfMessages.h"

#include "cafPdmObjectFactory.h"

#include <QTextStream>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RicfCommandObject*> RicfCommandFileReader::readCommands(QTextStream& inputStream, 
                                                                    caf::PdmObjectFactory* objectFactory,
                                                                    RicfMessages* errorMessageContainer)
{
    std::vector<RicfCommandObject*> readCommands;

    while ( !inputStream.atEnd() )
    {
        errorMessageContainer->skipWhiteSpaceWithLineNumberCount(inputStream);
        // Read command name
        QString commandName;
        {
            errorMessageContainer->skipWhiteSpaceWithLineNumberCount(inputStream);
            while ( !inputStream.atEnd() )
            {
                QChar currentChar = errorMessageContainer->readCharWithLineNumberCount(inputStream);

                if (currentChar == QChar('#'))
                {
                    errorMessageContainer->skipLineWithLineNumberCount(inputStream);
                    errorMessageContainer->skipWhiteSpaceWithLineNumberCount(inputStream);
                    currentChar = QChar();
                }
                else if ( currentChar.isSpace() )
                {
                    errorMessageContainer->skipWhiteSpaceWithLineNumberCount(inputStream);
                    QChar isBracket('a');
                    isBracket = errorMessageContainer->readCharWithLineNumberCount(inputStream);
                    if ( isBracket != QChar('(') )
                    {
                        // Error, could not find start bracket for command
                        errorMessageContainer->addError("Could not find start bracket for command " + commandName);

                        return readCommands;
                    }
                    break;
                }
                else if ( currentChar == QChar('(') )
                {
                    break;
                }

                if (!currentChar.isNull())
                {
                    commandName += currentChar;
                }
            }
        }

        if (commandName.isEmpty() && inputStream.atEnd())
        {
            // Read past the last command
            break;
        }

        CAF_ASSERT(objectFactory);
        caf::PdmObjectHandle* obj = objectFactory->create(commandName);
        RicfCommandObject* cObj = dynamic_cast<RicfCommandObject*>(obj);

        if ( cObj == nullptr )
        {
            errorMessageContainer->addError("The command: \"" + commandName + "\" does not exist.");

            // Error: Unknown command
            // Skip to end of command
            QChar currentChar;
            bool isOutsideQuotes = true;
            while ( !inputStream.atEnd() )
            {
                currentChar = errorMessageContainer->readCharWithLineNumberCount(inputStream);
                if ( isOutsideQuotes )
                {
                    if ( currentChar == QChar(')') )
                    {
                        break;
                    }
                    if ( currentChar == QChar('\"') )
                    {
                        isOutsideQuotes = false;
                    }
                }
                else
                {
                    if ( currentChar == QChar('\"') )
                    {
                        isOutsideQuotes = true;
                    }

                    if ( currentChar == QChar('\\') )
                    {
                        currentChar = errorMessageContainer->readCharWithLineNumberCount(inputStream);
                    }
                }
            }
        }
        else
        {
            readCommands.push_back(cObj);
            auto rcfCap = cObj->capability<RicfObjectCapability>();
            errorMessageContainer->currentCommand = commandName;
            rcfCap->readFields(inputStream, objectFactory, errorMessageContainer);
            errorMessageContainer->currentCommand = "";
        }
    }

    return readCommands;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfCommandFileReader::writeCommands(QTextStream& outputStream, const std::vector<RicfCommandObject*>& commandsToWrite)
{
    for (const auto& cmdObj : commandsToWrite)
    {
        auto rcfCap = cmdObj->capability<RicfObjectCapability>();
        if (!rcfCap) continue;

        outputStream << cmdObj->classKeyword();
        outputStream << "(";

        rcfCap->writeFields(outputStream);

        outputStream << ")";
    }
}

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
#include "cafPdmObjectHandle.h"
#include <QTextStream>
#include "RicfFieldHandle.h"
#include "cafPdmXmlFieldHandle.h"
#include "RicfMessages.h"




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfObjectCapability::RicfObjectCapability(caf::PdmObjectHandle* owner, bool giveOwnership)
{
    m_owner = owner;
    m_owner->addCapability(this, giveOwnership);
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
void RicfObjectCapability::readFields(QTextStream& inputStream, 
                                      caf::PdmObjectFactory* objectFactory, 
                                      RicfMessages* errorMessageContainer)
{
    std::set<QString> readFields;
    bool isLastArgumentRead = false;
    while ( !inputStream.atEnd() && !isLastArgumentRead )
    {
        // Read field keyword
        bool fieldDataFound = true;
        bool isEndOfArgumentFound = false;
        QString keyword;
        {
            inputStream.skipWhiteSpace();
            while ( !inputStream.atEnd() )
            {
                QChar currentChar;
                inputStream >> currentChar;
                if ( currentChar.isSpace() )
                {
                    // Must skip to, and read "="

                    inputStream.skipWhiteSpace();
                    inputStream >> currentChar;

                    if ( currentChar != QChar('=') )
                    {
                        // Error message: Missing "=" after argument name
                        errorMessageContainer->addError("Can't find the '=' after the argument  named: \"" + keyword + "\" in the command: \"" + errorMessageContainer->currentCommand + "\"" ); 
                        fieldDataFound = false;
                        if (currentChar == QChar(')') )
                        {
                            isLastArgumentRead = true;
                        }
                        else if (currentChar == QChar(',') )
                        {
                            isEndOfArgumentFound = true;
                        }
                    }
                    break;
                }

                if ( currentChar == QChar('=') )
                {
                    break;
                }

                keyword += currentChar;
            }

            if ( readFields.count(keyword) )
            {
                // Warning message: Referenced the same argument several times
                errorMessageContainer->addWarning("The argument: \"" + keyword + "\" is referenced several times in the command: \"" + errorMessageContainer->currentCommand + "\"" ); 
            }
        }

        if (fieldDataFound)
        {
            // Make field read its data

            caf::PdmFieldHandle* fieldHandle = m_owner->findField(keyword);
            if ( fieldHandle && fieldHandle->xmlCapability() && fieldHandle->capability<RicfFieldHandle>() )
            {
                caf::PdmXmlFieldHandle* xmlFieldHandle = fieldHandle->xmlCapability();
                RicfFieldHandle* rcfField = fieldHandle->capability<RicfFieldHandle>();

                if ( xmlFieldHandle->isIOReadable() )
                {
                    errorMessageContainer->currentArgument = keyword;
                    rcfField->readFieldData(inputStream, objectFactory, errorMessageContainer);
                    errorMessageContainer->currentArgument = keyword;
                }

            }
            else
            {
                // Error message: Unknown argument name
                errorMessageContainer->addWarning("The argument: \"" + keyword + "\" does not exist in the command: \"" + errorMessageContainer->currentCommand + "\"");
            }
        }

        // Skip to end of argument ',' or end of call ')' 
        if (!(isLastArgumentRead || isEndOfArgumentFound) )
        {
            QChar currentChar;
            bool isOutsideQuotes = true;
            while ( !inputStream.atEnd() )
            {
                inputStream >> currentChar;
                if ( isOutsideQuotes )
                {
                    if ( currentChar == QChar(',') )
                    {
                        break;
                    }

                    if ( currentChar == QChar(')') )
                    {
                        isLastArgumentRead = true;
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
                        inputStream >> currentChar;
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfObjectCapability::writeFields(QTextStream& outputStream) const
{
    std::vector<caf::PdmFieldHandle*> fields;
    m_owner->fields(fields);
    int writtenFieldCount = 0;
    for ( size_t it = 0; it < fields.size(); ++it )
    {
        const caf::PdmXmlFieldHandle* xmlField = fields[it]->xmlCapability();
        const RicfFieldHandle* rcfField = fields[it]->capability<RicfFieldHandle>();
        if ( rcfField && xmlField && xmlField->isIOWritable() )
        {
            QString keyword = xmlField->fieldHandle()->keyword();
            CAF_ASSERT(caf::PdmXmlObjectHandle::isValidXmlElementName(keyword));

            if ( writtenFieldCount >= 1 )
            {
                outputStream << ", ";
                ++writtenFieldCount;
            }

            outputStream << keyword << " = ";
            rcfField->writeFieldData(outputStream);
        }
    }
}

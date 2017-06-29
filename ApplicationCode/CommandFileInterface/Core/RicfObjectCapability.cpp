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
void RicfObjectCapability::readFields(QTextStream& inputStream, caf::PdmObjectFactory* objectFactory)
{
    std::set<QString> readFields;
    bool readLastArgument = false;
    while ( !inputStream.atEnd() && !readLastArgument )
    {
        // Read field keyword

        QString keyword;
        {
            inputStream.skipWhiteSpace();
            while ( !inputStream.atEnd() )
            {
                QChar currentChar;
                inputStream >> currentChar;
                if ( currentChar.isSpace() || currentChar == QChar('=') )
                {
                    break;
                }

                keyword += currentChar;
            }

            if ( readFields.count(keyword) )
            {
                // Warning message: Referenced the same argument several times
            }
        }

        // Make field read its data

        caf::PdmFieldHandle* fieldHandle = m_owner->findField(keyword);
        if ( fieldHandle && fieldHandle->xmlCapability() && fieldHandle->capability<RicfFieldHandle>() )
        {
            caf::PdmXmlFieldHandle* xmlFieldHandle = fieldHandle->xmlCapability();
            RicfFieldHandle* rcfField = fieldHandle->capability<RicfFieldHandle>();

            if ( xmlFieldHandle->isIOReadable() )
            {
                rcfField->readFieldData(inputStream, objectFactory);
            }

        }
        else
        {
            // Error message: Unknown argument name
        }

        // Skip to end of argument ',' or end of call ')' 
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
                        readLastArgument = true;
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

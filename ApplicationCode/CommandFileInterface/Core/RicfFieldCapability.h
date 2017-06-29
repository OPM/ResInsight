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

#pragma once

#include "RicfFieldHandle.h"
#include "RicfMessages.h"

#include <QTextStream>
#include <QString>


template <typename DataType>
struct RicfFieldReader
{
    static void    readFieldData(DataType & fieldValue, QTextStream& inputStream, RicfMessages* errorMessageContainer)
    {
        inputStream >> fieldValue;
        if (inputStream.status() == QTextStream::ReadCorruptData)
        {
            errorMessageContainer->addError("Argument value is unreadable in the argument: \"" 
                                            + errorMessageContainer->currentArgument + "\" in the command: \"" 
                                            + errorMessageContainer->currentCommand + "\"" );

            inputStream.setStatus( QTextStream::Ok);
        }
    }
};

template <typename DataType>
struct RicfFieldWriter
{
    static void    writeFieldData(const DataType & fieldValue, QTextStream&  outputStream)
    {
        outputStream << fieldValue; 
    }
};

template <>
struct RicfFieldReader<QString>
{
    static void    readFieldData(QString & fieldValue, QTextStream& inputStream, RicfMessages* errorMessageContainer);
};

template <>
struct RicfFieldWriter<QString>
{
    static void    writeFieldData(const QString & fieldValue, QTextStream&  outputStream);
};



//==================================================================================================
//
// 
//
//==================================================================================================
template < typename FieldType>
class RicfFieldCapability : public RicfFieldHandle
{
public:
    RicfFieldCapability(FieldType* field, bool giveOwnership) : RicfFieldHandle(field, giveOwnership) { m_field = field; }

    // Xml Serializing
public:
    virtual void        readFieldData (QTextStream& inputStream, caf::PdmObjectFactory* objectFactory, RicfMessages* errorMessageContainer) override
    {
        //m_field->xmlCapability()->assertValid(); 
        typename FieldType::FieldDataType value; 
        RicfFieldReader<typename FieldType::FieldDataType>::readFieldData(value, inputStream, errorMessageContainer);  
        m_field->setValue(value); 
    }

    virtual void        writeFieldData(QTextStream& outputStream) const override
    { 
        //m_field->xmlCapability()->assertValid(); 
        RicfFieldWriter<typename FieldType::FieldDataType>::writeFieldData(m_field->value(), outputStream); 
    }

private:
    FieldType* m_field;
};


template<typename FieldType>
void AddRicfCapabilityToField(FieldType* field)
{
    if(field->template capability< RicfFieldCapability<FieldType> >() == NULL)
    {
        new RicfFieldCapability<FieldType>(field, true);
    }
}



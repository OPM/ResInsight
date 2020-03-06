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

#include "cafAppEnum.h"
#include "cafPdmScriptIOMessages.h"

#include "cvfColor3.h"

#include <QString>
#include <QTextStream>

template <typename DataType>
struct RicfFieldIOHandler
{
    static void writeToField( DataType&                 fieldValue,
                              QTextStream&              inputStream,
                              caf::PdmScriptIOMessages* errorMessageContainer,
                              bool                      stringsAreQuoted = true )
    {
        inputStream >> fieldValue;
        if ( inputStream.status() == QTextStream::ReadCorruptData )
        {
            errorMessageContainer->addError( "Argument value is unreadable in the argument: \"" +
                                             errorMessageContainer->currentArgument + "\" in the command: \"" +
                                             errorMessageContainer->currentCommand + "\"" );

            inputStream.setStatus( QTextStream::Ok );
        }
    }

    static void readFromField( const DataType& fieldValue,
                               QTextStream&    outputStream,
                               bool            quoteStrings     = true,
                               bool            quoteNonBuiltins = false )
    {
        outputStream << fieldValue;
    }
};

template <>
struct RicfFieldIOHandler<QString>
{
    static void writeToField( QString&                  fieldValue,
                              QTextStream&              inputStream,
                              caf::PdmScriptIOMessages* errorMessageContainer,
                              bool                      stringsAreQuoted = true );
    static void readFromField( const QString& fieldValue,
                               QTextStream&   outputStream,
                               bool           quoteStrings     = true,
                               bool           quoteNonBuiltins = false );
};

template <>
struct RicfFieldIOHandler<bool>
{
    static void writeToField( bool&                     fieldValue,
                              QTextStream&              inputStream,
                              caf::PdmScriptIOMessages* errorMessageContainer,
                              bool                      stringsAreQuoted = true );
    static void readFromField( const bool&  fieldValue,
                               QTextStream& outputStream,
                               bool         quoteStrings     = true,
                               bool         quoteNonBuiltins = false );
};

template <>
struct RicfFieldIOHandler<cvf::Color3f>
{
    static void writeToField( cvf::Color3f&             fieldValue,
                              QTextStream&              inputStream,
                              caf::PdmScriptIOMessages* errorMessageContainer,
                              bool                      stringsAreQuoted = true );
    static void readFromField( const cvf::Color3f& fieldValue,
                               QTextStream&        outputStream,
                               bool                quoteStrings     = true,
                               bool                quoteNonBuiltins = false );
};

template <typename T>
struct RicfFieldIOHandler<caf::AppEnum<T>>
{
    static void writeToField( caf::AppEnum<T>&          fieldValue,
                              QTextStream&              inputStream,
                              caf::PdmScriptIOMessages* errorMessageContainer,
                              bool                      stringsAreQuoted = true )
    {
        errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
        QString accumulatedFieldValue;
        QChar   nextChar;
        QChar   currentChar;
        while ( !inputStream.atEnd() )
        {
            nextChar = errorMessageContainer->peekNextChar( inputStream );
            if ( nextChar.isLetterOrNumber() || nextChar == QChar( '_' ) )
            {
                currentChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                accumulatedFieldValue += currentChar;
            }
            else
            {
                break;
            }
        }
        if ( !fieldValue.setFromText( accumulatedFieldValue ) )
        {
            // Unexpected enum value
            // Error message
            errorMessageContainer->addError( "Argument must be valid enum value. " +
                                             errorMessageContainer->currentArgument + "\" argument of the command: \"" +
                                             errorMessageContainer->currentCommand + "\"" );
        }
    }

    static void readFromField( const caf::AppEnum<T>& fieldValue,
                               QTextStream&           outputStream,
                               bool                   quoteStrings     = true,
                               bool                   quoteNonBuiltins = false )
    {
        if ( quoteNonBuiltins )
        {
            outputStream << "\"" << fieldValue << "\"";
        }
        else
        {
            outputStream << fieldValue;
        }
    }
};

template <typename T>
struct RicfFieldIOHandler<std::vector<T>>
{
    static void writeToField( std::vector<T>&           fieldValue,
                              QTextStream&              inputStream,
                              caf::PdmScriptIOMessages* errorMessageContainer,
                              bool                      stringsAreQuoted = true )
    {
        errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
        QChar chr = errorMessageContainer->readCharWithLineNumberCount( inputStream );
        if ( chr == QChar( '[' ) )
        {
            while ( !inputStream.atEnd() )
            {
                errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
                QChar nextChar = errorMessageContainer->peekNextChar( inputStream );
                if ( nextChar == QChar( ']' ) )
                {
                    nextChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                    break;
                }
                else if ( nextChar == QChar( ',' ) )
                {
                    nextChar = errorMessageContainer->readCharWithLineNumberCount( inputStream );
                    errorMessageContainer->skipWhiteSpaceWithLineNumberCount( inputStream );
                }

                T value;
                RicfFieldIOHandler<T>::writeToField( value, inputStream, errorMessageContainer, true );
                fieldValue.push_back( value );
            }
        }
        else
        {
            errorMessageContainer->addError( "Array argument is missing start '['. " +
                                             errorMessageContainer->currentArgument + "\" argument of the command: \"" +
                                             errorMessageContainer->currentCommand + "\"" );
        }
    }

    static void readFromField( const std::vector<T>& fieldValue,
                               QTextStream&          outputStream,
                               bool                  quoteStrings     = true,
                               bool                  quoteNonBuiltins = false )
    {
        outputStream << "[";
        for ( size_t i = 0; i < fieldValue.size(); ++i )
        {
            RicfFieldIOHandler<T>::readFromField( fieldValue[i], outputStream, quoteNonBuiltins );
            if ( i < fieldValue.size() - 1 )
            {
                outputStream << ", ";
            }
        }
        outputStream << "]";
    }
};

//==================================================================================================
//
//
//
//==================================================================================================
template <typename FieldType>
class RicfFieldCapability : public RicfFieldHandle
{
public:
    RicfFieldCapability( FieldType* field, const QString& fieldName, bool giveOwnership )
        : RicfFieldHandle( field, fieldName, giveOwnership )
    {
        m_field = field;
    }

    // Xml Serializing
public:
    void writeToField( QTextStream&              inputStream,
                       caf::PdmObjectFactory*    objectFactory,
                       caf::PdmScriptIOMessages* errorMessageContainer,
                       bool                      stringsAreQuoted = true ) override
    {
        typename FieldType::FieldDataType value;
        RicfFieldIOHandler<typename FieldType::FieldDataType>::writeToField( value,
                                                                             inputStream,
                                                                             errorMessageContainer,
                                                                             stringsAreQuoted );

        if ( this->isIOWriteable() )
        {
            m_field->setValue( value );
        }
    }

    void readFromField( QTextStream& outputStream, bool quoteStrings = true, bool quoteNonBuiltins = false ) const override
    {
        RicfFieldIOHandler<typename FieldType::FieldDataType>::readFromField( m_field->value(),
                                                                              outputStream,
                                                                              quoteStrings,
                                                                              quoteNonBuiltins );
    }

private:
    FieldType* m_field;
};

template <typename FieldType>
void AddRicfCapabilityToField( FieldType* field, const QString& fieldName )
{
    if ( field->template capability<RicfFieldCapability<FieldType>>() == nullptr )
    {
        new RicfFieldCapability<FieldType>( field, fieldName, true );
    }
}

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
#pragma once

#include "cafAppEnum.h"
#include "cafPdmFieldScriptability.h"
#include "cafPdmScriptIOMessages.h"

#include "cvfColor3.h"

#include <QString>
#include <QTextStream>

#define CAF_PDM_InitScriptableValueField( field, keyword, default, uiName, iconResourceName, toolTip, whatsThis ) \
    CAF_PDM_InitField( field,                                                                                     \
                       keyword,                                                                                   \
                       default,                                                                                   \
                       uiName,                                                                                    \
                       iconResourceName,                                                                          \
                       caf::PdmFieldScriptability::helpString( toolTip, keyword ),                                \
                       whatsThis );                                                                               \
    caf::AddValueFieldScriptabilityToField( field, keyword )

#define CAF_PDM_InitScriptableValueFieldNoDefault( field, keyword, uiName, iconResourceName, toolTip, whatsThis ) \
    CAF_PDM_InitFieldNoDefault( field,                                                                            \
                                keyword,                                                                          \
                                uiName,                                                                           \
                                iconResourceName,                                                                 \
                                caf::PdmFieldScriptability::helpString( toolTip, keyword ),                       \
                                whatsThis );                                                                      \
    caf::AddValueFieldScriptabilityToField( field, keyword )

#define CAF_PDM_InitScriptableValueFieldTranslated( field, keyword, scriptKeyword, default, uiName, iconResourceName, toolTip, whatsThis ) \
    CAF_PDM_InitField( field,                                                                                                              \
                       keyword,                                                                                                            \
                       default,                                                                                                            \
                       uiName,                                                                                                             \
                       iconResourceName,                                                                                                   \
                       caf::PdmFieldScriptability::helpString( toolTip, scriptKeyword ),                                                   \
                       whatsThis );                                                                                                        \
    caf::AddValueFieldScriptabilityToField( field, scriptKeyword )

#define CAF_PDM_InitScriptableValueFieldNoDefaultTranslated( field,                               \
                                                             keyword,                             \
                                                             scriptKeyword,                       \
                                                             uiName,                              \
                                                             iconResourceName,                    \
                                                             toolTip,                             \
                                                             whatsThis )                          \
    CAF_PDM_InitFieldNoDefault( field,                                                            \
                                keyword,                                                          \
                                uiName,                                                           \
                                iconResourceName,                                                 \
                                caf::PdmFieldScriptability::helpString( toolTip, scriptKeyword ), \
                                whatsThis );                                                      \
    caf::AddValueFieldScriptabilityToField( field, scriptKeyword )

namespace caf
{
template <typename DataType>
struct PdmFieldScriptabilityIOHandler
{
    static void writeToField( DataType&            fieldValue,
                              QTextStream&         inputStream,
                              PdmScriptIOMessages* errorMessageContainer,
                              bool                 stringsAreQuoted = true )
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
struct PdmFieldScriptabilityIOHandler<QString>
{
    static void writeToField( QString&             fieldValue,
                              QTextStream&         inputStream,
                              PdmScriptIOMessages* errorMessageContainer,
                              bool                 stringsAreQuoted = true );
    static void readFromField( const QString& fieldValue,
                               QTextStream&   outputStream,
                               bool           quoteStrings     = true,
                               bool           quoteNonBuiltins = false );
};

template <>
struct PdmFieldScriptabilityIOHandler<bool>
{
    static void writeToField( bool&                fieldValue,
                              QTextStream&         inputStream,
                              PdmScriptIOMessages* errorMessageContainer,
                              bool                 stringsAreQuoted = true );
    static void readFromField( const bool&  fieldValue,
                               QTextStream& outputStream,
                               bool         quoteStrings     = true,
                               bool         quoteNonBuiltins = false );
};

template <>
struct PdmFieldScriptabilityIOHandler<cvf::Color3f>
{
    static void writeToField( cvf::Color3f&        fieldValue,
                              QTextStream&         inputStream,
                              PdmScriptIOMessages* errorMessageContainer,
                              bool                 stringsAreQuoted = true );
    static void readFromField( const cvf::Color3f& fieldValue,
                               QTextStream&        outputStream,
                               bool                quoteStrings     = true,
                               bool                quoteNonBuiltins = false );
};

template <typename T>
struct PdmFieldScriptabilityIOHandler<AppEnum<T>>
{
    static void writeToField( AppEnum<T>&          fieldValue,
                              QTextStream&         inputStream,
                              PdmScriptIOMessages* errorMessageContainer,
                              bool                 stringsAreQuoted = true )
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

    static void readFromField( const AppEnum<T>& fieldValue,
                               QTextStream&      outputStream,
                               bool              quoteStrings     = true,
                               bool              quoteNonBuiltins = false )
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
struct PdmFieldScriptabilityIOHandler<std::vector<T>>
{
    static void writeToField( std::vector<T>&      fieldValue,
                              QTextStream&         inputStream,
                              PdmScriptIOMessages* errorMessageContainer,
                              bool                 stringsAreQuoted = true )
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
                PdmFieldScriptabilityIOHandler<T>::writeToField( value, inputStream, errorMessageContainer, true );
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
            PdmFieldScriptabilityIOHandler<T>::readFromField( fieldValue[i], outputStream, quoteNonBuiltins );
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
class PdmValueFieldScriptability : public PdmFieldScriptability
{
public:
    PdmValueFieldScriptability( FieldType* field, const QString& fieldName, bool giveOwnership )
        : PdmFieldScriptability( field, fieldName, giveOwnership )
    {
        m_field = field;
    }

    // Xml Serializing
public:
    void writeToField( QTextStream&         inputStream,
                       PdmObjectFactory*    objectFactory,
                       PdmScriptIOMessages* errorMessageContainer,
                       bool                 stringsAreQuoted = true ) override
    {
        typename FieldType::FieldDataType value;
        PdmFieldScriptabilityIOHandler<typename FieldType::FieldDataType>::writeToField( value,
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
        PdmFieldScriptabilityIOHandler<typename FieldType::FieldDataType>::readFromField( m_field->value(),
                                                                                          outputStream,
                                                                                          quoteStrings,
                                                                                          quoteNonBuiltins );
    }

private:
    FieldType* m_field;
};

template <typename FieldType>
void AddValueFieldScriptabilityToField( FieldType* field, const QString& fieldName )
{
    if ( field->template capability<PdmValueFieldScriptability<FieldType>>() == nullptr )
    {
        new PdmValueFieldScriptability<FieldType>( field, fieldName, true );
    }
}

} // namespace caf

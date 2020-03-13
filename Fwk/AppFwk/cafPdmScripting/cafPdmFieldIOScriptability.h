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
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"
#include "cafPdmScriptIOMessages.h"

#include <QString>
#include <QStringList>
#include <QTextStream>

#define CAF_PDM_InitScriptableFieldWithIO( field, keyword, default, uiName, iconResourceName, toolTip, whatsThis ) \
    CAF_PDM_InitField( field,                                                                                      \
                       keyword,                                                                                    \
                       default,                                                                                    \
                       uiName,                                                                                     \
                       iconResourceName,                                                                           \
                       caf::PdmFieldScriptability::helpString( toolTip, keyword ),                                 \
                       whatsThis );                                                                                \
    caf::AddFieldIOScriptabilityToField( field, keyword )

#define CAF_PDM_InitScriptableFieldWithIONoDefault( field, keyword, uiName, iconResourceName, toolTip, whatsThis ) \
    CAF_PDM_InitFieldNoDefault( field,                                                                             \
                                keyword,                                                                           \
                                uiName,                                                                            \
                                iconResourceName,                                                                  \
                                caf::PdmFieldScriptability::helpString( toolTip, keyword ),                        \
                                whatsThis );                                                                       \
    caf::AddFieldIOScriptabilityToField( field, keyword )

#define CAF_PDM_InitScriptableFieldWithIOAndScriptName( field,                           \
                                                        keyword,                         \
                                                        scriptKeyword,                   \
                                                        default,                         \
                                                        uiName,                          \
                                                        iconResourceName,                \
                                                        toolTip,                         \
                                                        whatsThis )                      \
    CAF_PDM_InitField( field,                                                            \
                       keyword,                                                          \
                       default,                                                          \
                       uiName,                                                           \
                       iconResourceName,                                                 \
                       caf::PdmFieldScriptability::helpString( toolTip, scriptKeyword ), \
                       whatsThis );                                                      \
    caf::AddFieldIOScriptabilityToField( field, scriptKeyword )

#define CAF_PDM_InitScriptableFieldWithIOAndScriptNameNoDefault( field,                           \
                                                                 keyword,                         \
                                                                 scriptKeyword,                   \
                                                                 uiName,                          \
                                                                 iconResourceName,                \
                                                                 toolTip,                         \
                                                                 whatsThis )                      \
    CAF_PDM_InitFieldNoDefault( field,                                                            \
                                keyword,                                                          \
                                uiName,                                                           \
                                iconResourceName,                                                 \
                                caf::PdmFieldScriptability::helpString( toolTip, scriptKeyword ), \
                                whatsThis );                                                      \
    caf::AddFieldIOScriptabilityToField( field, scriptKeyword )

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

template <typename T>
struct PdmFieldScriptabilityIOHandler<std::vector<T*>>
{
    static void writeToField( std::vector<T*>&       fieldValue,
                              const std::vector<T*>& allObjectsOfType,
                              QTextStream&           inputStream,
                              PdmScriptIOMessages*   errorMessageContainer )
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

                T* value;
                PdmFieldScriptabilityIOHandler<T*>::writeToField( value, allObjectsOfType, inputStream, errorMessageContainer );
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

    static void readFromField( const std::vector<T*>& fieldValue,
                               QTextStream&           outputStream,
                               bool                   quoteStrings     = true,
                               bool                   quoteNonBuiltins = false )
    {
        outputStream << "[";
        for ( size_t i = 0; i < fieldValue.size(); ++i )
        {
            PdmFieldScriptabilityIOHandler<T*>::readFromField( fieldValue[i], outputStream, quoteNonBuiltins );
            if ( i < fieldValue.size() - 1 )
            {
                outputStream << ", ";
            }
        }
        outputStream << "]";
    }
};

template <typename DataType>
struct PdmFieldScriptabilityIOHandler<DataType*>
{
    static void writeToField( DataType*&                    fieldValue,
                              const std::vector<DataType*>& allObjectsOfType,
                              QTextStream&                  inputStream,
                              PdmScriptIOMessages*          errorMessageContainer )
    {
        fieldValue = nullptr; // Default initialized to nullptr

        QString fieldString;
        PdmFieldScriptabilityIOHandler<QString>::writeToField( fieldString, inputStream, errorMessageContainer, true );

        if ( inputStream.status() == QTextStream::ReadCorruptData )
        {
            errorMessageContainer->addError( "Argument value is unreadable in the argument: \"" +
                                             errorMessageContainer->currentArgument + "\" in the command: \"" +
                                             errorMessageContainer->currentCommand + "\"" );

            inputStream.setStatus( QTextStream::Ok );
            return;
        }

        if ( fieldString.isEmpty() ) return;

        QStringList classAndAddress = fieldString.split( ":" );
        CAF_ASSERT( classAndAddress.size() == 2 );

        qulonglong address = classAndAddress[1].toULongLong();
        for ( DataType* object : allObjectsOfType )
        {
            if ( reinterpret_cast<qulonglong>( object ) == address )
            {
                fieldValue = object;
                break;
            }
        }
    }

    static void readFromField( const DataType* fieldValue,
                               QTextStream&    outputStream,
                               bool            quoteStrings     = true,
                               bool            quoteNonBuiltins = false )
    {
        outputStream
            << QString( "%1:%2" ).arg( DataType::classKeywordStatic() ).arg( reinterpret_cast<uint64_t>( fieldValue ) );
    }
};

//==================================================================================================
//
//
//
//==================================================================================================
template <typename FieldType>
class PdmFieldIOScriptability : public PdmFieldScriptability
{
public:
    PdmFieldIOScriptability( FieldType* field, const QString& fieldName, bool giveOwnership )
        : PdmFieldScriptability( field, fieldName, giveOwnership )
    {
        m_field = field;
    }

    // Xml Serializing
public:
    void writeToField( QTextStream&          inputStream,
                       PdmObjectFactory*     objectFactory,
                       PdmScriptIOMessages*  errorMessageContainer,
                       bool                  stringsAreQuoted    = true,
                       caf::PdmObjectHandle* existingObjectsRoot = nullptr ) override
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

template <typename DataType>
class PdmFieldIOScriptability<PdmPtrField<DataType*>> : public PdmFieldScriptability
{
public:
    PdmFieldIOScriptability( PdmPtrField<DataType*>* field, const QString& fieldName, bool giveOwnership )
        : PdmFieldScriptability( field, fieldName, giveOwnership )
    {
        m_field = field;
    }

    // Xml Serializing
public:
    void writeToField( QTextStream&          inputStream,
                       PdmObjectFactory*     objectFactory,
                       PdmScriptIOMessages*  errorMessageContainer,
                       bool                  stringsAreQuoted    = true,
                       caf::PdmObjectHandle* existingObjectsRoot = nullptr ) override
    {
        std::vector<DataType*> allObjectsOfType;
        existingObjectsRoot->descendantsIncludingThisOfType( allObjectsOfType );

        DataType* object = nullptr;
        PdmFieldScriptabilityIOHandler<DataType*>::writeToField( object, allObjectsOfType, inputStream, errorMessageContainer );

        if ( object && this->isIOWriteable() )
        {
            m_field->setValue( object );
        }
    }

    void readFromField( QTextStream& outputStream, bool quoteStrings = true, bool quoteNonBuiltins = false ) const override
    {
        PdmFieldScriptabilityIOHandler<DataType*>::readFromField( m_field->value(),
                                                                  outputStream,
                                                                  quoteStrings,
                                                                  quoteNonBuiltins );
    }

private:
    PdmPtrField<DataType*>* m_field;
};

template <typename DataType>
class PdmFieldIOScriptability<PdmPtrArrayField<DataType*>> : public PdmFieldScriptability
{
public:
    PdmFieldIOScriptability( PdmPtrArrayField<DataType*>* field, const QString& fieldName, bool giveOwnership )
        : PdmFieldScriptability( field, fieldName, giveOwnership )
    {
        m_field = field;
    }

    // Xml Serializing
public:
    void writeToField( QTextStream&          inputStream,
                       PdmObjectFactory*     objectFactory,
                       PdmScriptIOMessages*  errorMessageContainer,
                       bool                  stringsAreQuoted    = true,
                       caf::PdmObjectHandle* existingObjectsRoot = nullptr ) override
    {
        std::vector<DataType*> allObjectsOfType;
        existingObjectsRoot->descendantsIncludingThisOfType( allObjectsOfType );

        std::vector<DataType*> objects;
        PdmFieldScriptabilityIOHandler<std::vector<DataType*>>::writeToField( objects,
                                                                              allObjectsOfType,
                                                                              inputStream,
                                                                              errorMessageContainer );

        if ( this->isIOWriteable() )
        {
            m_field->setValue( objects );
        }
    }

    void readFromField( QTextStream& outputStream, bool quoteStrings = true, bool quoteNonBuiltins = false ) const override
    {
        PdmFieldScriptabilityIOHandler<std::vector<DataType*>>::readFromField( m_field->ptrReferencedObjects(),
                                                                               outputStream,
                                                                               quoteStrings,
                                                                               quoteNonBuiltins );
    }

private:
    PdmPtrArrayField<DataType*>* m_field;
};

template <typename FieldType>
void AddFieldIOScriptabilityToField( FieldType* field, const QString& fieldName )
{
    if ( field->template capability<PdmFieldIOScriptability<FieldType>>() == nullptr )
    {
        new PdmFieldIOScriptability<FieldType>( field, fieldName, true );
    }
}

} // namespace caf

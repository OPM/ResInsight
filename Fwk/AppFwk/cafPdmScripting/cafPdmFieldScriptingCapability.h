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
#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"
#include "cafPdmScriptIOMessages.h"

#include <QIODevice>
#include <QTextStream>

#define CAF_PDM_InitScriptableField( field, keyword, default, uiName, ... )                          \
    {                                                                                                \
        std::vector<QString> arguments = { __VA_ARGS__ };                                            \
        QString              iconResourceName;                                                       \
        QString              toolTip;                                                                \
        QString              whatsThis;                                                              \
        if ( !arguments.empty() ) iconResourceName = arguments[0];                                   \
        if ( arguments.size() > 1 ) toolTip = arguments[1];                                          \
        if ( arguments.size() > 2 ) whatsThis = arguments[2];                                        \
                                                                                                     \
        CAF_PDM_InitField( field,                                                                    \
                           keyword,                                                                  \
                           default,                                                                  \
                           uiName,                                                                   \
                           iconResourceName,                                                         \
                           caf::PdmAbstractFieldScriptingCapability::helpString( toolTip, keyword ), \
                           whatsThis );                                                              \
        caf::AddScriptingCapabilityToField( field, keyword );                                        \
    }

#define CAF_PDM_InitScriptableFieldNoDefault( field, keyword, uiName, ... )                                   \
    {                                                                                                         \
        std::vector<QString> arguments = { __VA_ARGS__ };                                                     \
        QString              iconResourceName;                                                                \
        QString              toolTip;                                                                         \
        QString              whatsThis;                                                                       \
        if ( !arguments.empty() ) iconResourceName = arguments[0];                                            \
        if ( arguments.size() > 1 ) toolTip = arguments[1];                                                   \
        if ( arguments.size() > 2 ) whatsThis = arguments[2];                                                 \
                                                                                                              \
        CAF_PDM_InitFieldNoDefault( field,                                                                    \
                                    keyword,                                                                  \
                                    uiName,                                                                   \
                                    iconResourceName,                                                         \
                                    caf::PdmAbstractFieldScriptingCapability::helpString( toolTip, keyword ), \
                                    whatsThis );                                                              \
        caf::AddScriptingCapabilityToField( field, keyword );                                                 \
    }

#define CAF_PDM_InitScriptableFieldWithScriptKeyword( field, keyword, scriptKeyword, default, uiName, ... ) \
    {                                                                                                       \
        std::vector<QString> arguments = { __VA_ARGS__ };                                                   \
        QString              iconResourceName;                                                              \
        QString              toolTip;                                                                       \
        QString              whatsThis;                                                                     \
        if ( !arguments.empty() ) iconResourceName = arguments[0];                                          \
        if ( arguments.size() > 1 ) toolTip = arguments[1];                                                 \
        if ( arguments.size() > 2 ) whatsThis = arguments[2];                                               \
                                                                                                            \
        CAF_PDM_InitField( field,                                                                           \
                           keyword,                                                                         \
                           default,                                                                         \
                           uiName,                                                                          \
                           iconResourceName,                                                                \
                           caf::PdmAbstractFieldScriptingCapability::helpString( toolTip, scriptKeyword ),  \
                           whatsThis );                                                                     \
        caf::AddScriptingCapabilityToField( field, scriptKeyword );                                         \
    }

#define CAF_PDM_InitScriptableFieldWithScriptKeywordNoDefault( field, keyword, scriptKeyword, uiName, ... )         \
    {                                                                                                               \
        std::vector<QString> arguments = { __VA_ARGS__ };                                                           \
        QString              iconResourceName;                                                                      \
        QString              toolTip;                                                                               \
        QString              whatsThis;                                                                             \
        if ( !arguments.empty() ) iconResourceName = arguments[0];                                                  \
        if ( arguments.size() > 1 ) toolTip = arguments[1];                                                         \
        if ( arguments.size() > 2 ) whatsThis = arguments[2];                                                       \
                                                                                                                    \
        CAF_PDM_InitFieldNoDefault( field,                                                                          \
                                    keyword,                                                                        \
                                    uiName,                                                                         \
                                    iconResourceName,                                                               \
                                    caf::PdmAbstractFieldScriptingCapability::helpString( toolTip, scriptKeyword ), \
                                    whatsThis );                                                                    \
        caf::AddScriptingCapabilityToField( field, scriptKeyword );                                                 \
    }

namespace caf
{
template <typename DataType>
struct PdmFieldScriptingCapabilityIOHandler
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
struct PdmFieldScriptingCapabilityIOHandler<QString>
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
struct PdmFieldScriptingCapabilityIOHandler<bool>
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
struct PdmFieldScriptingCapabilityIOHandler<AppEnum<T>>
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
            errorMessageContainer->addError( "Failed to set enum text value \"" + accumulatedFieldValue +
                                             "\" for the command: \"" + errorMessageContainer->currentCommand + "\"" );
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
struct PdmFieldScriptingCapabilityIOHandler<std::vector<T>>
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
            std::vector<QString> allValues;
            QString              currentValue;
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
                    if ( !currentValue.isEmpty() ) allValues.push_back( currentValue );
                    currentValue = "";
                }
                else
                {
                    currentValue += errorMessageContainer->readCharWithLineNumberCount( inputStream );
                }
            }
            if ( !currentValue.isEmpty() ) allValues.push_back( currentValue );

            for ( QString textValue : allValues )
            {
                QTextStream singleValueStream( &textValue, QIODevice::ReadOnly );
                T           singleValue;
                PdmFieldScriptingCapabilityIOHandler<T>::writeToField( singleValue,
                                                                       singleValueStream,
                                                                       errorMessageContainer,
                                                                       stringsAreQuoted );
                fieldValue.push_back( singleValue );
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
            PdmFieldScriptingCapabilityIOHandler<T>::readFromField( fieldValue[i], outputStream, quoteNonBuiltins );
            if ( i < fieldValue.size() - 1 )
            {
                outputStream << ", ";
            }
        }
        outputStream << "]";
    }
};

template <typename T>
struct PdmFieldScriptingCapabilityIOHandler<std::vector<T*>>
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
            std::vector<QString> allValues;
            QString              currentValue;
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
                    if ( !currentValue.isEmpty() ) allValues.push_back( currentValue );
                    currentValue = "";
                }
                else
                {
                    currentValue += errorMessageContainer->readCharWithLineNumberCount( inputStream );
                }
            }
            if ( !currentValue.isEmpty() ) allValues.push_back( currentValue );

            for ( QString textValue : allValues )
            {
                QTextStream singleValueStream( &textValue, QIODevice::ReadOnly );
                T*          singleValue;
                PdmFieldScriptingCapabilityIOHandler<T*>::writeToField( singleValue,
                                                                        allObjectsOfType,
                                                                        singleValueStream,
                                                                        errorMessageContainer );
                fieldValue.push_back( singleValue );
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
            PdmFieldScriptingCapabilityIOHandler<T*>::readFromField( fieldValue[i], outputStream, quoteNonBuiltins );
            if ( i < fieldValue.size() - 1 )
            {
                outputStream << ", ";
            }
        }
        outputStream << "]";
    }
};

template <typename DataType>
struct PdmFieldScriptingCapabilityIOHandler<DataType*>
{
    static void writeToField( DataType*&                    fieldValue,
                              const std::vector<DataType*>& allObjectsOfType,
                              QTextStream&                  inputStream,
                              PdmScriptIOMessages*          errorMessageContainer )
    {
        fieldValue = nullptr; // Default initialized to nullptr

        QString fieldString;
        bool    stringsAreQuoted = false;
        PdmFieldScriptingCapabilityIOHandler<QString>::writeToField( fieldString,
                                                                     inputStream,
                                                                     errorMessageContainer,
                                                                     stringsAreQuoted );

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
        if ( classAndAddress.size() == 2 )
        {
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
    }

    static void readFromField( const DataType* fieldValue,
                               QTextStream&    outputStream,
                               bool            quoteStrings     = true,
                               bool            quoteNonBuiltins = false )
    {
        if ( fieldValue )
        {
            QString textOutput =
                QString( "%1:%2" ).arg( DataType::classKeywordStatic() ).arg( reinterpret_cast<uint64_t>( fieldValue ) );
            if ( quoteNonBuiltins )
            {
                outputStream << QString( "\"%1\"" ).arg( textOutput );
            }
            else
            {
                outputStream << textOutput;
            }
        }
    }
};

//==================================================================================================
//
//
//
//==================================================================================================
template <typename FieldType>
class PdmFieldScriptingCapability : public PdmAbstractFieldScriptingCapability
{
public:
    PdmFieldScriptingCapability( FieldType* field, const QString& fieldName, bool giveOwnership )
        : PdmAbstractFieldScriptingCapability( field, fieldName, giveOwnership )
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
        PdmFieldScriptingCapabilityIOHandler<typename FieldType::FieldDataType>::writeToField( value,
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
        PdmFieldScriptingCapabilityIOHandler<typename FieldType::FieldDataType>::readFromField( m_field->value(),
                                                                                                outputStream,
                                                                                                quoteStrings,
                                                                                                quoteNonBuiltins );
    }

private:
    FieldType* m_field;
};

template <typename DataType>
class PdmFieldScriptingCapability<PdmPtrField<DataType*>> : public PdmAbstractFieldScriptingCapability
{
public:
    PdmFieldScriptingCapability( PdmPtrField<DataType*>* field, const QString& fieldName, bool giveOwnership )
        : PdmAbstractFieldScriptingCapability( field, fieldName, giveOwnership )
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
        std::vector<DataType*> allObjectsOfType = existingObjectsRoot->descendantsIncludingThisOfType<DataType>();

        DataType* object = nullptr;
        PdmFieldScriptingCapabilityIOHandler<DataType*>::writeToField( object,
                                                                       allObjectsOfType,
                                                                       inputStream,
                                                                       errorMessageContainer );

        if ( object && this->isIOWriteable() )
        {
            m_field->setValue( object );
        }
    }

    void readFromField( QTextStream& outputStream, bool quoteStrings = true, bool quoteNonBuiltins = false ) const override
    {
        PdmFieldScriptingCapabilityIOHandler<DataType*>::readFromField( m_field->value(),
                                                                        outputStream,
                                                                        quoteStrings,
                                                                        quoteNonBuiltins );
    }

private:
    PdmPtrField<DataType*>* m_field;
};

template <typename DataType>
class PdmFieldScriptingCapability<PdmChildField<DataType*>> : public PdmAbstractFieldScriptingCapability
{
public:
    PdmFieldScriptingCapability( PdmChildField<DataType*>* field, const QString& fieldName, bool giveOwnership )
        : PdmAbstractFieldScriptingCapability( field, fieldName, giveOwnership )
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
        std::vector<DataType*> allObjectsOfType = existingObjectsRoot->descendantsIncludingThisOfType<DataType>();

        DataType* object = nullptr;
        PdmFieldScriptingCapabilityIOHandler<DataType*>::writeToField( object,
                                                                       allObjectsOfType,
                                                                       inputStream,
                                                                       errorMessageContainer );

        if ( object && this->isIOWriteable() )
        {
            m_field->setValue( object );
        }
    }

    void readFromField( QTextStream& outputStream, bool quoteStrings = true, bool quoteNonBuiltins = false ) const override
    {
        PdmFieldScriptingCapabilityIOHandler<DataType*>::readFromField( m_field->value(),
                                                                        outputStream,
                                                                        quoteStrings,
                                                                        quoteNonBuiltins );
    }

private:
    PdmChildField<DataType*>* m_field;
};

template <typename DataType>
class PdmFieldScriptingCapability<PdmPtrArrayField<DataType*>> : public PdmAbstractFieldScriptingCapability
{
public:
    PdmFieldScriptingCapability( PdmPtrArrayField<DataType*>* field, const QString& fieldName, bool giveOwnership )
        : PdmAbstractFieldScriptingCapability( field, fieldName, giveOwnership )
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
        std::vector<DataType*> allObjectsOfType = existingObjectsRoot->descendantsIncludingThisOfType<DataType>();

        std::vector<DataType*> objects;
        PdmFieldScriptingCapabilityIOHandler<std::vector<DataType*>>::writeToField( objects,
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
        PdmFieldScriptingCapabilityIOHandler<std::vector<DataType*>>::readFromField( m_field->ptrReferencedObjectsByType(),
                                                                                     outputStream,
                                                                                     quoteStrings,
                                                                                     quoteNonBuiltins );
    }

private:
    PdmPtrArrayField<DataType*>* m_field;
};

template <typename DataType>
class PdmFieldScriptingCapability<PdmChildArrayField<DataType*>> : public PdmAbstractFieldScriptingCapability
{
public:
    PdmFieldScriptingCapability( PdmChildArrayField<DataType*>* field, const QString& fieldName, bool giveOwnership )
        : PdmAbstractFieldScriptingCapability( field, fieldName, giveOwnership )
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
        std::vector<DataType*> allObjectsOfType = existingObjectsRoot->descendantsIncludingThisOfType<DataType>();

        std::vector<DataType*> objects;
        PdmFieldScriptingCapabilityIOHandler<std::vector<DataType*>>::writeToField( objects,
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
        PdmFieldScriptingCapabilityIOHandler<std::vector<DataType*>>::readFromField( m_field->childrenByType(),
                                                                                     outputStream,
                                                                                     quoteStrings,
                                                                                     quoteNonBuiltins );
    }

private:
    PdmChildArrayField<DataType*>* m_field;
};

template <typename DataType>
class PdmFieldScriptingCapability<PdmField<caf::AppEnum<DataType>>> : public PdmAbstractFieldScriptingCapability
{
public:
    PdmFieldScriptingCapability( PdmField<caf::AppEnum<DataType>>* field, const QString& fieldName, bool giveOwnership )
        : PdmAbstractFieldScriptingCapability( field, fieldName, giveOwnership )
    {
        m_field = field;
    }

    void writeToField( QTextStream&          inputStream,
                       PdmObjectFactory*     objectFactory,
                       PdmScriptIOMessages*  errorMessageContainer,
                       bool                  stringsAreQuoted    = true,
                       caf::PdmObjectHandle* existingObjectsRoot = nullptr ) override
    {
        if ( this->isIOWriteable() )
        {
            AppEnum<DataType> value;

            PdmFieldScriptingCapabilityIOHandler<caf::AppEnum<DataType>>::writeToField( value,
                                                                                        inputStream,
                                                                                        errorMessageContainer,
                                                                                        stringsAreQuoted );
            m_field->setValue( value );
        }
    }

    void readFromField( QTextStream& outputStream, bool quoteStrings = true, bool quoteNonBuiltins = false ) const override
    {
        PdmFieldScriptingCapabilityIOHandler<caf::AppEnum<DataType>>::readFromField( m_field->value(),
                                                                                     outputStream,
                                                                                     quoteStrings,
                                                                                     quoteNonBuiltins );
    }

    QStringList enumScriptTexts() const override
    {
        QStringList enumTexts;

        for ( size_t i = 0; i < caf::AppEnum<DataType>::size(); i++ )
        {
            auto enumText = caf::AppEnum<DataType>::text( caf::AppEnum<DataType>::fromIndex( i ) );
            enumTexts.push_back( enumText );
        }

        return enumTexts;
    }

private:
    PdmField<caf::AppEnum<DataType>>* m_field;
};

template <typename FieldType>
void AddScriptingCapabilityToField( FieldType* field, const QString& fieldName )
{
    if ( field->template capability<PdmFieldScriptingCapability<FieldType>>() == nullptr )
    {
        new PdmFieldScriptingCapability<FieldType>( field, fieldName, true );
    }
}

} // namespace caf

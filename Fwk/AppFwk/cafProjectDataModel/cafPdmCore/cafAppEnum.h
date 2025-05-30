//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#include "cafAssert.h"
#include "cafPdmFieldHandle.h"

#include <QString>
#include <QStringList>

#include <map>
#include <vector>

namespace caf
{
//==================================================================================================
/// An enum class to make it easier to handle IO and UI based on the enum.
/// Usage:
/// In Header file of SomeClass:
///    enum SomeEnumType
///    {
///       A = 2,
///       B = 7
///    };
///   caf::AppEnum<SomeEnumType> m_enumValue;
///
/// In C++ file :
///    namespace caf {
///    template<>
///    void caf::AppEnum<SomeClass::SomeEnumType>::setUp()
///    {
///        addItem(SomeClass::A,           "A",         "An A letter");
///        addItem(SomeClass::B,           "B",         "A B letter");
///        setDefault(SomeClass::B);
///    }
///    }
/// General use:
///
///    m_enumValue = A;
///    if (m_enumValue == A || m_enumValue != B ){}
///
///    switch (m_enumValue)
///    {
///    case A:
///        break;
///    case B:
///        break;
///    }
///
///    cout << m_enumValue.text();
///    m_enumValue.setFromText("A");
///
///    for (size_t i = 0; i < caf::AppEnum<SomeClass::SomeEnumType>::size(); ++i)
///        cout << caf::AppEnum<SomeClass::SomeEnumType>::text(caf::AppEnum<SomeClass::SomeEnumType>::fromIndex(i)) <<
///        endl;
///
///
///
///   Create a list of OptionItemInfos from AppEnum
///     QList<caf::PdmOptionItemInfo> options;
///     for (size_t i = 0; i < caf::AppEnum<TestEnumType>::size(); ++i)
///     {
///         options.push_back(caf::PdmOptionItemInfo(caf::AppEnum<TestEnumType>::uiTextFromIndex(i),
///         caf::AppEnum<TestEnumType>::fromIndex(i)));
///     }
///
/// Subset of enums:
///
///   Define a subset of the enum to be used in a field. Assign a value from the subset to make sure the field
///   contains a valid value when the constructor is done.
///
///     CAF_PDM_InitField( &m_enum2Field, "Enum2Field", MyEnumType::T6, "Subset using setEnumSubset()" );
///     caf::AppEnum<MyEnumType>::setEnumSubset( &m_enum2Field, { MyEnumType::T2, MyEnumType::T6 } );
///
///
//==================================================================================================

class AppEnumInterface
{
    // This non-templated base class is used in cafInternalPdmStreamOperators.h to create a QTextStream operator. Having
    // the QTextStream operator working on each templated type was seen as a performance issue when profiling use of
    // include files
public:
    virtual QString textForSerialization() const                   = 0;
    virtual void    setTextForSerialization( const QString& text ) = 0;
};

template <class T>
class AppEnum : public AppEnumInterface
{
public:
    AppEnum() { m_value = EnumMapper::instance()->defaultValue(); }
    AppEnum( T value )
        : m_value( value )
    {
    }

    static void setEnumSubset( caf::PdmFieldHandle* fieldKeyword, std::vector<T> subset )
    {
        if ( !fieldKeyword ) return;
        m_enumSubset[fieldKeyword->keyword()] = subset;
    }
    static std::vector<T> enumSubset( QString fieldKeyword )
    {
        auto it = m_enumSubset.find( fieldKeyword );
        if ( it != m_enumSubset.end() ) return it->second;

        return {};
    }

    operator T() const { return m_value; }

    T       value() const { return m_value; }
    size_t  index() const { return EnumMapper::instance()->index( m_value ); }
    QString text() const { return EnumMapper::instance()->text( m_value ); }
    QString uiText() const { return EnumMapper::instance()->uiText( m_value ); }

    AppEnum& operator=( T value )
    {
        m_value = value;
        return *this;
    }
    bool setFromText( const QString& text ) { return EnumMapper::instance()->enumVal( m_value, text ); }
    bool setFromIndex( size_t index ) { return EnumMapper::instance()->enumVal( m_value, index ); }

    QString textForSerialization() const override { return text(); }
    void    setTextForSerialization( const QString& text ) override { setFromText( text ); }

    // Static interface to access the properties of the enum definition

    static bool   isValid( const QString& text ) { return EnumMapper::instance()->isValid( text ); }
    static bool   isValid( size_t index ) { return index < EnumMapper::instance()->size(); }
    static size_t size() { return EnumMapper::instance()->size(); }

    static QStringList uiTexts() { return EnumMapper::instance()->uiTexts(); }
    static T           fromIndex( size_t idx )
    {
        T val;
        EnumMapper::instance()->enumVal( val, idx );
        return val;
    }
    static T fromText( const QString& text )
    {
        T val;
        EnumMapper::instance()->enumVal( val, text );
        return val;
    }
    static size_t  index( T enumValue ) { return EnumMapper::instance()->index( enumValue ); }
    static QString text( T enumValue ) { return EnumMapper::instance()->text( enumValue ); }
    static QString textFromIndex( size_t idx ) { return text( fromIndex( idx ) ); }
    static QString uiText( T enumValue ) { return EnumMapper::instance()->uiText( enumValue ); }
    static QString uiTextFromIndex( size_t idx ) { return uiText( fromIndex( idx ) ); }

private:
    //==================================================================================================
    /// The setup method is supposed to be specialized for each and every type instantiation of this class,
    /// and is supposed to set up the mapping between enum values, text and ui-text using the \m addItem
    /// method. It may also set a default value using \m setDefault
    //==================================================================================================
    static void setUp();
    static void addItem( T enumVal, const QString& text, const QString& uiText, const QStringList& aliases = {} )
    {
        EnumMapper::instance()->addItem( enumVal, text, uiText, aliases );
    }

    static void setDefault( T defaultEnumValue ) { EnumMapper::instance()->setDefault( defaultEnumValue ); }

    T m_value;

    static std::map<QString, std::vector<T>> m_enumSubset;

    //==================================================================================================
    /// A private class to handle the instance of the mapping vector.
    /// all access methods could have been placed directly in the \class AppEnum class,
    /// but AppEnum implementation gets nicer this way.
    /// The real core of this class is the vector map member and the static instance method
    //==================================================================================================

    class EnumMapper
    {
    private:
        class EnumData
        {
        public:
            EnumData( T enumVal, const QString& text, const QString& uiText, const QStringList& aliases )
                : m_enumVal( enumVal )
                , m_text( text )
                , m_uiText( uiText )
                , m_aliases( aliases )
            {
            }

            bool isMatching( const QString& text ) const { return ( text == m_text || m_aliases.contains( text ) ); }

            T           m_enumVal;
            QString     m_text;
            QString     m_uiText;
            QStringList m_aliases;
        };

    public:
        void addItem( T enumVal, const QString& text, QString uiText, const QStringList& aliases )
        {
            // Make sure the alias text is unique for enum
            for ( const auto& alias : aliases )
            {
                for ( const auto& enumData : instance()->m_mapping )
                {
                    CAF_ASSERT( !enumData.isMatching( alias ) );
                }
            }

            // Make sure the text is trimmed, as this text is streamed to XML and will be trimmed when read back
            // from XML text https://github.com/OPM/ResInsight/issues/7829
            instance()->m_mapping.push_back( EnumData( enumVal, text.trimmed(), uiText, aliases ) );
        }

        static EnumMapper* instance()
        {
            static EnumMapper storedInstance;
            static bool       isInitialized = false;
            if ( !isInitialized )
            {
                isInitialized = true;
                AppEnum<T>::setUp();
            }
            return &storedInstance;
        }

        void setDefault( T defaultEnumValue )
        {
            m_defaultValue      = defaultEnumValue;
            m_defaultValueIsSet = true;
        }

        T defaultValue() const
        {
            if ( m_defaultValueIsSet )
            {
                return m_defaultValue;
            }
            else
            {
                // CAF_ASSERT(m_mapping.size());
                return m_mapping[0].m_enumVal;
            }
        }

        bool isValid( const QString& text ) const
        {
            size_t idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                if ( text == m_mapping[idx].m_text ) return true;
            }

            return false;
        }

        size_t size() const { return m_mapping.size(); }

        bool enumVal( T& value, const QString& text ) const
        {
            value = defaultValue();

            QString trimmedText = text.trimmed();

            for ( size_t idx = 0; idx < m_mapping.size(); ++idx )
            {
                // Make sure the text parsed from a text stream is trimmed
                // https://github.com/OPM/ResInsight/issues/7829
                if ( m_mapping[idx].isMatching( trimmedText ) )
                {
                    value = m_mapping[idx].m_enumVal;
                    return true;
                }
            }
            return false;
        }

        bool enumVal( T& value, size_t index ) const
        {
            value = defaultValue();
            if ( index < m_mapping.size() )
            {
                value = m_mapping[index].m_enumVal;
                return true;
            }
            else
                return false;
        }

        size_t index( T enumValue ) const
        {
            size_t idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                if ( enumValue == m_mapping[idx].m_enumVal ) return idx;
            }

            return idx;
        }

        QString uiText( T value ) const
        {
            size_t idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                if ( value == m_mapping[idx].m_enumVal ) return m_mapping[idx].m_uiText;
            }
            return "";
        }

        QStringList uiTexts() const
        {
            QStringList uiTextList;
            size_t      idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                uiTextList.append( m_mapping[idx].m_uiText );
            }
            return uiTextList;
        }

        QString text( T value ) const
        {
            size_t idx;
            for ( idx = 0; idx < m_mapping.size(); ++idx )
            {
                if ( value == m_mapping[idx].m_enumVal ) return m_mapping[idx].m_text;
            }
            return "";
        }

    private:
        EnumMapper()
            : m_defaultValue( T() )
            , m_defaultValueIsSet( false )
        {
        }

        friend class AppEnum<T>;

        std::vector<EnumData> m_mapping;
        T                     m_defaultValue;
        bool                  m_defaultValueIsSet;
    };
};

template <class T>
std::map<QString, std::vector<T>> AppEnum<T>::m_enumSubset;

} // namespace caf

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

#include "cafPdmFieldHandle.h"
#include "cafPdmFieldTraits.h"

#include <QString>
#include <QStringList>

#include <map>
#include <optional>
#include <type_traits>
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

//==================================================================================================
/// Non-template backing store for AppEnum<T>. Holds the (text, uiText, aliases) mapping keyed by
/// the enum value cast to int. By keeping all loops, lookups, and QString machinery in this
/// non-template class, every member is compiled exactly once instead of once per enum type.
/// AppEnum<T> reduces to a thin static_cast shim around the relevant methods.
//==================================================================================================
class AppEnumMapperBase
{
public:
    void        addItem( int enumVal, const QString& text, QString uiText, const QStringList& aliases );
    void        setDefault( int defaultEnumValue );
    int         defaultValue() const;
    bool        isValid( const QString& text ) const;
    size_t      size() const;
    bool        enumVal( int& value, const QString& text ) const;
    bool        enumVal( int& value, size_t index ) const;
    size_t      index( int enumValue ) const;
    QString     uiText( int value ) const;
    QStringList uiTexts() const;
    QString     text( int value ) const;

private:
    struct EnumData
    {
        int         m_enumVal;
        QString     m_text;
        QString     m_uiText;
        QStringList m_aliases;
        bool        isMatching( const QString& text ) const;
    };

    std::vector<EnumData> m_mapping;
    std::optional<int>    m_defaultValue;
};

template <class T>
class AppEnum : public AppEnumInterface
{
    static_assert( std::is_enum_v<T>, "AppEnum<T> requires T to be an enum type." );
    static_assert( sizeof( std::underlying_type_t<T> ) <= sizeof( int ),
                   "AppEnum stores enum values as int; T's underlying type is wider than int." );

public:
    AppEnum() { m_value = static_cast<T>( mapper().defaultValue() ); }
    AppEnum( T value )
        : m_value( value )
    {
    }

    static void setEnumSubset( caf::PdmFieldHandle* fieldHandle, std::vector<T> subset )
    {
        if ( !fieldHandle ) return;
        QString key       = createEnumSubsetKey( fieldHandle );
        m_enumSubset[key] = subset;
    }
    static std::vector<T> enumSubset( caf::PdmFieldHandle* fieldHandle )
    {
        if ( !fieldHandle ) return {};
        QString key = createEnumSubsetKey( fieldHandle );
        auto    it  = m_enumSubset.find( key );
        if ( it != m_enumSubset.end() ) return it->second;

        return {};
    }

private:
    static QString createEnumSubsetKey( caf::PdmFieldHandle* fieldHandle )
    {
        if ( !fieldHandle ) return QString();

        // Create a unique key by combining the owner class name with the field keyword
        // This prevents collisions when different object types use the same field keyword
        QString ownerClass = fieldHandle->ownerClass();
        return ownerClass + "::" + fieldHandle->keyword();
    }

public:
    operator T() const { return m_value; }

    T       value() const { return m_value; }
    size_t  index() const { return mapper().index( static_cast<int>( m_value ) ); }
    QString text() const { return mapper().text( static_cast<int>( m_value ) ); }
    QString uiText() const { return mapper().uiText( static_cast<int>( m_value ) ); }

    AppEnum& operator=( T value )
    {
        m_value = value;
        return *this;
    }
    bool setFromText( const QString& text )
    {
        int  v  = 0;
        bool ok = mapper().enumVal( v, text );
        m_value = static_cast<T>( v );
        return ok;
    }
    bool setFromIndex( size_t index )
    {
        int  v  = 0;
        bool ok = mapper().enumVal( v, index );
        m_value = static_cast<T>( v );
        return ok;
    }

    QString textForSerialization() const override { return text(); }
    void    setTextForSerialization( const QString& text ) override { setFromText( text ); }

    // Static interface to access the properties of the enum definition

    static bool   isValid( const QString& text ) { return mapper().isValid( text ); }
    static bool   isValid( size_t index ) { return index < mapper().size(); }
    static size_t size() { return mapper().size(); }

    static QStringList uiTexts() { return mapper().uiTexts(); }
    static T           fromIndex( size_t idx )
    {
        int v = 0;
        mapper().enumVal( v, idx );
        return static_cast<T>( v );
    }
    static T fromText( const QString& text )
    {
        int v = 0;
        mapper().enumVal( v, text );
        return static_cast<T>( v );
    }
    static size_t  index( T enumValue ) { return mapper().index( static_cast<int>( enumValue ) ); }
    static QString text( T enumValue ) { return mapper().text( static_cast<int>( enumValue ) ); }
    static QString textFromIndex( size_t idx ) { return text( fromIndex( idx ) ); }
    static QString uiText( T enumValue ) { return mapper().uiText( static_cast<int>( enumValue ) ); }
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
        mapper().addItem( static_cast<int>( enumVal ), text, uiText, aliases );
    }

    static void setDefault( T defaultEnumValue ) { mapper().setDefault( static_cast<int>( defaultEnumValue ) ); }

    // Per-T storage for the mapper. The base class methods are non-template so they're compiled
    // once total; only this accessor and the trivial static_cast shims above are instantiated per T.
    //
    // The explicit isInitialized flag (rather than e.g. std::call_once or a magic-static lambda) is
    // deliberate: setUp() invokes addItem(), which calls mapper() recursively. Setting the flag
    // before invoking setUp() lets the re-entrant calls return the storedInstance instead of
    // deadlocking. First-access must therefore happen single-threaded; see
    // CreateObjectInMultipleThreads in cafPdmBasicTest.cpp.
    static AppEnumMapperBase& mapper()
    {
        static AppEnumMapperBase storedInstance;
        static bool              isInitialized = false;
        if ( !isInitialized )
        {
            isInitialized = true;
            AppEnum<T>::setUp();
        }
        return storedInstance;
    }

    T m_value;

    static std::map<QString, std::vector<T>> m_enumSubset; // Key format: "ownerClass::fieldKeyword"
};

template <class T>
std::map<QString, std::vector<T>> AppEnum<T>::m_enumSubset;

template <typename T>
QVariant pdmToVariant( const AppEnum<T>& value )
{
    return QVariant( static_cast<int>( static_cast<T>( value ) ) );
}

template <typename T>
void pdmFromVariant( const QVariant& v, AppEnum<T>& out )
{
    out = static_cast<T>( v.toInt() );
}

template <typename T>
struct PdmVariantEqualImpl<AppEnum<T>>
{
    static bool equal( const QVariant& a, const QVariant& b ) { return a.toInt() == b.toInt(); }
};

} // namespace caf

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

#include "cafAppEnumMapper.h"
#include "cafAssert.h"

#include "cafTypeNameHelper.h"
#include <QString>
#include <QStringList>

#include <vector>

namespace caf
{
class AppEnumInterface
{
public:
    virtual QString textForSerialization() const                   = 0;
    virtual void    setTextForSerialization( const QString& text ) = 0;
};

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
//==================================================================================================

template <class T>
class AppEnum : public AppEnumInterface
{
public:
    AppEnum()
    {
        auto enumInteger = EnumMapper::instance()->defaultEnumValue( caf::cafTypeName<T>() );
        m_value          = caf::convertToEnum<T>( enumInteger );
    }
    AppEnum( T value )
        : m_value( value )
    {
    }

    operator T() const { return m_value; }

    T      value() const { return m_value; }
    size_t index() const
    {
        return EnumMapper::instance()->index( caf::cafTypeName<T>(), caf::convertToInteger( m_value ) );
    }
    QString text() const
    {
        return EnumMapper::instance()->text( caf::cafTypeName<T>(), caf::convertToInteger( m_value ) );
    }
    QString uiText() const
    {
        return EnumMapper::instance()->uiText( caf::cafTypeName<T>(), caf::convertToInteger( m_value ) );
    }

    AppEnum& operator=( T value )
    {
        m_value = value;
        return *this;
    }
    bool setFromText( const QString& text )
    {
        m_value = fromText( text );
        return true;
    }
    bool setFromIndex( size_t index )
    {
        m_value = fromIndex( index );
        return true;
    }

    QString textForSerialization() const override { return text(); }

    void setTextForSerialization( const QString& text ) override { setFromText( text ); }

    // Static interface to access the properties of the enum definition

    static bool   isValid( const QString& text ) { return EnumMapper::instance()->isValid( text ); }
    static bool   isValid( size_t index ) { return index < EnumMapper::instance()->size(); }
    static size_t size() { return EnumMapper::instance()->size( caf::cafTypeName<T>() ); }

    static QStringList uiTexts() { return EnumMapper::instance()->uiTexts( caf::cafTypeName<T>() ); }
    static T           fromIndex( size_t idx )
    {
        auto enumInteger = EnumMapper::instance()->fromIndex( caf::cafTypeName<T>(), idx );
        return caf::convertToEnum<T>( enumInteger );
    }
    static T fromText( const QString& text )
    {
        auto enumInteger = EnumMapper::instance()->fromText( caf::cafTypeName<T>(), text );
        return caf::convertToEnum<T>( enumInteger );
    }
    static size_t index( T enumValue )
    {
        return EnumMapper::instance()->index( caf::cafTypeName<T>(), caf::convertToInteger( enumValue ) );
    }
    static QString text( T enumValue )
    {
        return EnumMapper::instance()->text( caf::cafTypeName<T>(), caf::convertToInteger( enumValue ) );
    }
    static QString textFromIndex( size_t idx ) { return text( fromIndex( idx ) ); }
    static QString uiText( T enumValue )
    {
        return EnumMapper::instance()->uiText( caf::cafTypeName<T>(), caf::convertToInteger( enumValue ) );
    }
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
        auto key         = caf::cafTypeName<T>();
        auto enumInteger = caf::convertToInteger<T>( enumVal );
        EnumMapper::instance()->addItem( key, enumInteger, text, uiText, aliases );
    }

    static void setDefault( T defaultEnumValue )
    {
        EnumMapper::instance()->setDefault( caf::cafTypeName<T>(), caf::convertToInteger<T>( defaultEnumValue ) );
    }

    T m_value;

    //==================================================================================================
    /// A private class to handle the instance of the mapping vector.
    /// all access methods could have been placed directly in the \class AppEnum class,
    /// but AppEnum implementation gets nicer this way.
    /// The real core of this class is the vector map member and the static instance method
    //==================================================================================================

    class EnumMapper
    {
    public:
        void addItem( T enumVal, const QString& text, QString uiText, const QStringList& aliases )
        {
            caf::AppEnumMapper::instance()->addItem( caf::cafTypeName<T>(),
                                                     caf::convertToInteger<T>( enumVal ),
                                                     text,
                                                     uiText,
                                                     aliases );
        }

        static caf::AppEnumMapper* instance()
        {
            static bool isInitialized = false;
            if ( !isInitialized )
            {
                isInitialized = true;
                AppEnum<T>::setUp();
            }
            return caf::AppEnumMapper::instance();
        }
    };
};

} // namespace caf

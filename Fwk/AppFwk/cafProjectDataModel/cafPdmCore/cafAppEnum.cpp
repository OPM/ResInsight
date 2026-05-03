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

// AppEnumMapperBase holds the type-erased mapping (int <-> text/uiText/aliases) shared by every
// AppEnum<T> instantiation. Defining its members in this translation unit instead of inline in the
// class template means each lookup and QString helper is compiled once for the whole program rather
// than once per enum type, which keeps build times and binary size in check.

#include "cafAppEnum.h"

#include "cafAssert.h"

namespace caf
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool AppEnumMapperBase::EnumData::isMatching( const QString& text ) const
{
    return text == m_text || m_aliases.contains( text );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void AppEnumMapperBase::addItem( int enumVal, const QString& text, QString uiText, const QStringList& aliases )
{
    // Make sure the alias text is unique for enum
    for ( const auto& alias : aliases )
    {
        for ( const auto& enumData : m_mapping )
        {
            CAF_ASSERT( !enumData.isMatching( alias ) );
        }
    }

    // Make sure the text is trimmed, as this text is streamed to XML and will be trimmed when read back
    // from XML text https://github.com/OPM/ResInsight/issues/7829
    m_mapping.push_back( { enumVal, text.trimmed(), uiText, aliases } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void AppEnumMapperBase::setDefault( int defaultEnumValue )
{
    m_defaultValue = defaultEnumValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int AppEnumMapperBase::defaultValue() const
{
    if ( m_defaultValue.has_value() ) return *m_defaultValue;
    CAF_ASSERT( !m_mapping.empty() );
    return m_mapping[0].m_enumVal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool AppEnumMapperBase::isValid( const QString& text ) const
{
    for ( size_t idx = 0; idx < m_mapping.size(); ++idx )
    {
        if ( text == m_mapping[idx].m_text ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t AppEnumMapperBase::size() const
{
    return m_mapping.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool AppEnumMapperBase::enumVal( int& value, const QString& text ) const
{
    value = defaultValue();

    QString trimmedText = text.trimmed();

    for ( size_t idx = 0; idx < m_mapping.size(); ++idx )
    {
        if ( m_mapping[idx].isMatching( trimmedText ) )
        {
            value = m_mapping[idx].m_enumVal;
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool AppEnumMapperBase::enumVal( int& value, size_t index ) const
{
    value = defaultValue();
    if ( index < m_mapping.size() )
    {
        value = m_mapping[index].m_enumVal;
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t AppEnumMapperBase::index( int enumValue ) const
{
    size_t idx;
    for ( idx = 0; idx < m_mapping.size(); ++idx )
    {
        if ( enumValue == m_mapping[idx].m_enumVal ) return idx;
    }
    return idx; // returns size() if not found, matching prior behavior
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString AppEnumMapperBase::uiText( int value ) const
{
    for ( size_t idx = 0; idx < m_mapping.size(); ++idx )
    {
        if ( value == m_mapping[idx].m_enumVal ) return m_mapping[idx].m_uiText;
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList AppEnumMapperBase::uiTexts() const
{
    QStringList uiTextList;
    for ( size_t idx = 0; idx < m_mapping.size(); ++idx )
    {
        uiTextList.append( m_mapping[idx].m_uiText );
    }
    return uiTextList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString AppEnumMapperBase::text( int value ) const
{
    for ( size_t idx = 0; idx < m_mapping.size(); ++idx )
    {
        if ( value == m_mapping[idx].m_enumVal ) return m_mapping[idx].m_text;
    }
    return "";
}

} // namespace caf

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

#include "cafPdmOptionItemInfo.h"

#include "cafPdmObjectHandle.h"
#include "cafPdmPointer.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmOptionItemInfo::PdmOptionItemInfo( const QString&      anOptionUiText,
                                      const QVariant&     aValue,
                                      bool                isReadOnly,
                                      const IconProvider& anIcon )
    : m_optionUiText( anOptionUiText )
    , m_value( aValue )
    , m_isReadOnly( isReadOnly )
    , m_iconProvider( anIcon )
    , m_level( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmOptionItemInfo::PdmOptionItemInfo( const QString&        anOptionUiText,
                                      caf::PdmObjectHandle* obj,
                                      bool                  isReadOnly,
                                      const IconProvider&   anIcon )
    : m_optionUiText( anOptionUiText )
    , m_isReadOnly( isReadOnly )
    , m_iconProvider( anIcon )
    , m_level( 0 )
{
    m_value = QVariant::fromValue( caf::PdmPointer<caf::PdmObjectHandle>( obj ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmOptionItemInfo PdmOptionItemInfo::createHeader( const QString& anOptionUiText, bool isReadOnly, const IconProvider& anIcon )
{
    PdmOptionItemInfo header( anOptionUiText, QVariant(), isReadOnly, anIcon );

    return header;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmOptionItemInfo::setLevel( int level )
{
    m_level = level;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmOptionItemInfo::optionUiText() const
{
    return m_optionUiText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant PdmOptionItemInfo::value() const
{
    return m_value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmOptionItemInfo::isReadOnly() const
{
    return m_isReadOnly;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmOptionItemInfo::isHeading() const
{
    return !m_value.isValid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<QIcon> PdmOptionItemInfo::icon() const
{
    return m_iconProvider.icon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int PdmOptionItemInfo::level() const
{
    return m_level;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList PdmOptionItemInfo::extractUiTexts( const QList<PdmOptionItemInfo>& optionList )
{
    QStringList texts;

    for ( const auto& option : optionList )
    {
        texts.push_back( option.optionUiText() );
    }

    return texts;
}

} // End of namespace caf

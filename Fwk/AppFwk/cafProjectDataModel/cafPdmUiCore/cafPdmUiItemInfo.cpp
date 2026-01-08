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

#include "cafPdmUiItemInfo.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiItemInfo::PdmUiItemInfo( const QString& uiName,
                              IconProvider   iconProvider,
                              QString        toolTip,
                              QString        whatsThis,
                              QString        extraDebugText )
    : m_uiName( uiName )
    , m_iconProvider( iconProvider )
    , m_toolTip( toolTip )
    , m_whatsThis( whatsThis )
    , m_extraDebugText( extraDebugText )
    , m_labelPosition( LabelPosition::LEFT )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiItemInfo::PdmUiItemInfo( const QString& uiName,
                              QString        iconResourceLocation,
                              QString        toolTip,
                              QString        whatsThis,
                              QString        extraDebugText )
    : m_uiName( uiName )
    , m_iconProvider( iconResourceLocation )
    , m_toolTip( toolTip )
    , m_whatsThis( whatsThis )
    , m_extraDebugText( extraDebugText )
    , m_labelPosition( LabelPosition::LEFT )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<QIcon> PdmUiItemInfo::icon() const
{
    return m_iconProvider.icon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const IconProvider& PdmUiItemInfo::iconProvider() const
{
    return m_iconProvider;
}

} // End of namespace caf

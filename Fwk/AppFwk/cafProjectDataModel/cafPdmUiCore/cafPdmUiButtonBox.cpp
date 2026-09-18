//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2026 Equinor ASA
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

#include "cafPdmUiButtonBox.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiButtonBox::PdmUiButtonBox()
{
    // No separate label -- the box itself fills the whole row/cell it is given, like a
    // QDialogButtonBox does inside a real QDialog.
    setUiLabelPosition( PdmUiItemInfo::LabelPosition::HIDDEN );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiButtonBox::ButtonSpec& PdmUiButtonBox::addButton( const QString& text, const ClickCallback& callback )
{
    m_buttons.push_back( { text, callback, true, QString() } );
    return m_buttons.back();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::list<PdmUiButtonBox::ButtonSpec>& PdmUiButtonBox::buttons() const
{
    return m_buttons;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiButtonBox::isUiGroup() const
{
    return false;
}

} // End of namespace caf

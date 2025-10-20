//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2017 Ceetron Solutions AS
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

#include "cafPdmUiItem.h"

#include <functional>

namespace caf
{
//==================================================================================================
/// Class representing a button to be displayed in the GUI without connection to a PDM field
//==================================================================================================

class PdmUiButton : public PdmUiItem
{
public:
    using ClickCallback = std::function<void()>;

    PdmUiButton();
    PdmUiButton( const QString& buttonText, const ClickCallback& callback );

    void setIcon( const IconProvider& iconProvider );
    void setIconFromResourceString( const QString& iconResourceName );

    void          setAlignment( Qt::Alignment alignment );
    Qt::Alignment alignment() const;

    void          setClickCallback( const ClickCallback& callback );
    ClickCallback clickCallback() const;

    bool isUiGroup() const override;

private:
    Qt::Alignment m_alignment;
    ClickCallback m_clickCallback;
};

} // End of namespace caf

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

#pragma once

#include "cafPdmUiItem.h"

#include <QString>

#include <functional>
#include <list>

namespace caf
{
//==================================================================================================
/// Class representing a row of action buttons (e.g. "Apply"/"Cancel") laid out using a real
/// QDialogButtonBox, without connection to a PDM field. Mirrors the native platform button-box
/// look (buttons packed tightly, pinned to one side, e.g. the right side on Windows) used by real
/// QDialogs (see e.g. the Preferences dialog), reused here for buttons embedded directly in an
/// inline property-panel form.
//==================================================================================================
class PdmUiButtonBox : public PdmUiItem
{
public:
    using ClickCallback = std::function<void()>;

    struct ButtonSpec
    {
        QString        text;
        ClickCallback  callback;
        bool           enabled{ true };
        QString        toolTip;
    };

    PdmUiButtonBox();

    // Returns a stable reference into the button box that can be used to update the button's
    // enabled state/tooltip on later calls to defineUiOrdering() (a new PdmUiButtonBox is created
    // fresh each time, so nothing needs to be reset -- the returned reference is only valid for
    // the lifetime of this particular PdmUiButtonBox instance).
    ButtonSpec& addButton( const QString& text, const ClickCallback& callback );

    const std::list<ButtonSpec>& buttons() const;

    bool isUiGroup() const override;

private:
    std::list<ButtonSpec> m_buttons;
};

} // End of namespace caf
